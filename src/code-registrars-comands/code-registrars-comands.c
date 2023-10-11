// Incluisão das bibliotecas
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdlib.h>
//#include "DHT.h"

// Configuração os pinos
#define RS PH3
#define E PH5
#define BUZZER PH6
#define DADOS_LCD PORTA
#define nibble_dados 1
#define button_select PC0
#define button_left PC1
#define button_rigth PC2
#define led_red PC3
#define led_yellow PC4
#define led_green PC5
#define MAX_NUM_STR_LEN 12
#define DHT22_PIN PE4
#define DHT_PIN PE4
#define POTENTIOMETER_PIN PF0
#define BUFFER_SIZE 100

// Definindo as variáveis globais
bool enable_lft = 1;
bool enable_slt = 1;
bool enable_rgt = 1;
bool enable_c = 1;
bool control_button_lft = 0;
bool control_button_slt = 1;
bool control_button_rgt = 0;
bool set_umi = 1;
bool set_temp = 0;

int mode = 0;
int mode_screen = 0;
int default_umi_min = 40;
int default_umi_max = 40;
int default_temp_min = 20;
int default_temp_max = 20;
uint8_t humidity11, temperature11;
uint16_t humidity, temperature, potValue;
uint16_t umidadeBuffer[BUFFER_SIZE]; // Arrays para armazenar os últimos valores de umidade e temperatura
uint16_t temperaturaBuffer[BUFFER_SIZE];
uint8_t bufferIndex = 0; // Índice para controlar a posição atual no buffer circular

float h;
float t;

const uint8_t flameCharacter[8] = {
    0b00100,
    0b01110,
    0b01110,
    0b11111,
    0b01110,
    0b01110,
    0b00100,
    0b00000
};

void createFlameCharacter() {
    // Define o endereço da memória de caracteres personalizados (de 0 a 7)
    cmd_LCD(0x40 | (0 << 3), 0); // Endereço 0, altere para outros endereços se necessário

    // Envie os padrões de pixels para o controlador do LCD
    for (int i = 0; i < 8; i++) {
        cmd_LCD(flameCharacter[i], 1);
    }
}

void armazenarDados(float umidade, float temperatura) {
    umidadeBuffer[bufferIndex] = umidade;
    temperaturaBuffer[bufferIndex] = temperatura;

    // Incrementa o índice e verifica se ultrapassou o tamanho do buffer
    bufferIndex++;
    if (bufferIndex >= BUFFER_SIZE) {
        bufferIndex = 0;  // Volta para o início do buffer se atingir o final
    }
}

// Responsável por controlar as operações de escrita/leitura no display
void pulse_E() {
    _delay_ms(1);
    PORTH |= (1 << E);
    _delay_ms(1);
    PORTH &= ~(1 << E);
    _delay_ms(1);
}

void startSignal() {
    DDRE |= (1 << DHT22_PIN);      // Configura o pino como saída
    PORTE &= ~(1 << DHT22_PIN);    // Envia sinal baixo
    _delay_ms(1);
    PORTE |= (1 << DHT22_PIN);     // Envia sinal alto
    _delay_us(40);
}

void responseSignal() {
    DDRE &= ~(1 << DHT22_PIN);     // Configura o pino como entrada
    while (PINE & (1 << DHT22_PIN));   // Espera pelo pulso baixo
    while (!(PINE & (1 << DHT22_PIN)));  // Espera pelo pulso alto
    while (PINE & (1 << DHT22_PIN));    // Espera pelo pulso baixo
}

uint8_t readDHT22Byte() {
    uint8_t dataByte = 0;
    for (uint8_t i = 0; i < 8; i++) {
        while (!(PINE & (1 << DHT22_PIN)));   // Espera pelo bit de dados (pulso alto)
        _delay_us(50);
        if (PINE & (1 << DHT22_PIN))
            dataByte = (dataByte << 1) | 0x01;
        else
            dataByte = (dataByte << 1);
        while (PINE & (1 << DHT22_PIN));    // Espera pelo pulso baixo
    }
    return dataByte;
}

void getData(uint16_t *humidity, uint16_t *temperature) {
    startSignal();
    responseSignal();

    uint8_t RH_high, RH_low, temp_high, temp_low, checksum;
    RH_high = readDHT22Byte();
    RH_low = readDHT22Byte();
    temp_high = readDHT22Byte();
    temp_low = readDHT22Byte();
    checksum = readDHT22Byte();

    *humidity = (RH_high << 8) | RH_low;
    *temperature = (temp_high << 8) | temp_low;
}

float convertTemperature(uint16_t temperature) {
    return (float)temperature/10.0;
}

float convertHumidity(uint16_t humidity) {
    return (float)humidity/10.0;
}

void startSignalDHT11() {
    DDRE |= (1 << DHT_PIN);       // Configura o pino como saída
    PORTE &= ~(1 << DHT_PIN);     // Envia sinal baixo
    _delay_ms(18);                // Duração mínima do sinal baixo para iniciar a comunicação
    PORTE |= (1 << DHT_PIN);      // Envia sinal alto
    _delay_us(20);                // Duração do sinal alto
}

void responseSignalDHT11() {
    DDRE &= ~(1 << DHT_PIN);      // Configura o pino como entrada
    while (PINE & (1 << DHT_PIN));   // Espera pelo pulso baixo
    while (!(PINE & (1 << DHT_PIN)));  // Espera pelo pulso alto
    while (PINE & (1 << DHT_PIN));    // Espera pelo pulso baixo
}

uint8_t readDHT11Byte() {
    uint8_t dataByte = 0;
    for (uint8_t i = 0; i < 8; i++) {
        while (!(PINE & (1 << DHT_PIN)));   // Espera pelo bit de dados (pulso alto)
        _delay_us(30);  // Duração mínima do bit de dados de acordo com a especificação do DHT11
        if (PINE & (1 << DHT_PIN))
            dataByte = (dataByte << 1) | 0x01;
        else
            dataByte = (dataByte << 1);
        while (PINE & (1 << DHT_PIN));    // Espera pelo pulso baixo
    }
    return dataByte;
}

void getDataDHT11(uint16_t *humidity, uint16_t *temperature) {
    startSignalDHT11();
    Serial.println("Passou");
    responseSignalDHT11();

    uint8_t RH_low, RH_high, temp_low, temp_high, checksum;
    RH_high = readDHT11Byte();
    RH_low = readDHT11Byte();
    temp_high = readDHT11Byte();
    temp_low = readDHT11Byte();
    checksum = readDHT11Byte();

    *humidity = RH_high;
    *temperature = temp_high;
}

float convertTemperatureDHT11(uint16_t temperature) {
    return (float)temperature;
}

float convertHumidityDHT11(uint16_t humidity) {
    return (float)humidity;
}


// Envia comandos para o LCD utilizando a interface de 4 bits
void cmd_LCD(unsigned char c, char cd) {
    if (cd == 0) {
        PORTH &= ~(1 << RS);
    } else {
        PORTH |= (1 << RS);
    }

    if (nibble_dados) {
        DADOS_LCD = (DADOS_LCD & 0x0F) | (c & 0xF0);
    } else {
        DADOS_LCD = (DADOS_LCD & 0xF0) | (c >> 4);
    }

    pulse_E();

    if (nibble_dados) {
        DADOS_LCD = (DADOS_LCD & 0x0F) | ((c << 4) & 0xF0);
    } else {
        DADOS_LCD = (DADOS_LCD & 0xF0) | (c & 0x0F);
    }

    pulse_E();

    if ((cd == 0) && (c < 4)) {
        _delay_ms(2);
    }
}

// Inicializa as configurações dos pinos, inicializa o LCD e define seu modo de operação
void start() {
  DDRA = 0b11110000;
  DDRH = 0b01101000;
  DDRC = 0b00111000;

  sei();
  DDRD &= ~(1 << PD0);
  PORTD |= (1 << PD0);
  DDRD &= ~(1 << PD1);
  PORTD |= (1 << PD1);
  DDRD &= ~(1 << PD2);
  PORTD |= (1 << PD2);

  EICRB |= (1 << ISC01) | (1 << ISC00);   // Configurar para detecção de borda de descida
  EIMSK |= (1 << INT0);                   // Habilitar a interrupção externa 0
  EICRB |= (1 << ISC11) | (1 << ISC10);   // Configurar para detecção de borda de descida
  EIMSK |= (1 << INT1);                   // Habilitar a interrupção externa 1
  EICRB |= (1 << ISC21) | (1 << ISC20);   // Configurar para detecção de borda de descida
  EIMSK |= (1 << INT2);                   // Habilitar a interrupção externa 2

  // Habilitar interrupções globais

  PORTC |= (1 << button_select);
  PORTC |= (1 << button_left);
  PORTC |= (1 << button_rigth);

  PORTH &= ~(1 << RS);
  PORTH &= ~(1 << E);

  _delay_ms(20);

  if (nibble_dados) {
      DADOS_LCD = (DADOS_LCD & 0x0F) | 0x20;
  } else {
      DADOS_LCD = (DADOS_LCD & 0xF0) | 0x02;
  }

  pulse_E();

  // Comandos padrões do Display:
  cmd_LCD(0x28, 0); // interface de 4 bits 2 linhas
  cmd_LCD(0x08, 0); // desliga o display
  cmd_LCD(0x01, 0); // limpa todo o display
  cmd_LCD(0x0C, 0); // mensagem aparente cursor inativo não piscando
  cmd_LCD(0x80, 0); // inicializa cursor na primeira posição a esquerda - 1a linha 
}

// Exibe uma string no LCD
void show_LCD(const char *str) {
    while (*str) {
        cmd_LCD(*str++, 1);
    }
}

void show_LCD_number(int number) {
    char numStr[MAX_NUM_STR_LEN];  // Array para armazenar a string do número

    // Converte o número em uma string
    snprintf(numStr, MAX_NUM_STR_LEN, "%d", number);

    // Exibe a string no LCD
    show_LCD(numStr);
}

// Converte um número inteiro em uma string e exibe a string no LCD
void show_LCD_float(float number, int decimalPlaces) {
    char numStr[MAX_NUM_STR_LEN];  // Array para armazenar a string do número

    // Verifica se o número é negativo
    int isNegative = 0;
    if (number < 0) {
        isNegative = 1;
        number = -number;
    }

    // Converte a parte inteira para string
    int integerPart = (int)number;
    sprintf(numStr, "%d", integerPart);

    // Adiciona o ponto decimal
    strcat(numStr, ".");

    // Converte a parte decimal para string
    float decimalPart = number - integerPart;
    for (int i = 0; i < decimalPlaces; ++i) {
        decimalPart *= 10.0;
    }

    int decimalInt = (int)decimalPart;
    sprintf(numStr + strlen(numStr), "%d", decimalInt);

    // Adiciona o sinal negativo se necessário
    if (isNegative) {
        memmove(numStr + 1, numStr, strlen(numStr) + 1);
        numStr[0] = '-';
    }

    // Exibe a string no LCD
    show_LCD(numStr);
}

// Função para posicionar o cursor em uma posição específica
void posicionar_cursor(unsigned char linha, unsigned char coluna) {
    // Calcula a posição do cursor com base na linha e coluna
    unsigned char posicao_cursor;
    if (linha == 1) {
        posicao_cursor = 0x80 + (coluna - 1);
    } else if (linha == 2) {
        posicao_cursor = 0xC0 + (coluna - 1);
    } else {
        // Trate casos inválidos aqui, se necessário
        return;
    }

    // Envia o comando para posicionar o cursor na posição desejada
    cmd_LCD(posicao_cursor, 0);
}

// Tela principal: mostra a porcentagem da Umidade e Temperatura
void screen_1(){
  // Limpa a tela do Display
  if(enable_c == 1){
    cmd_LCD(0x01, 0);
    enable_c = 0;
  }

  // Enquanto o botão de seleção não for pressionado, exibir a tela de porcentagem da Umidade e Temperatura
  while(control_button_slt == 1){
    //getData(&humidity, &temperature);
    getDataDHT11(&humidity, &temperature);
    t = convertTemperatureDHT11(temperature);
    h = convertHumidityDHT11(humidity);
    Serial.print(h);
    armazenarDados(h, t);
    potValue = readPotentiometer();

    if(h < 30 && t > 60 && potValue < 1024/2){
      PORTH &= ~(1 << BUZZER);
      posicionar_cursor(1, 1);
      show_LCD("Umidade: ");
      posicionar_cursor(1, 10);
      show_LCD_float(h, 1);
      posicionar_cursor(1, 14);
      show_LCD(" %");
      posicionar_cursor(2, 1);
      show_LCD("Temp: ");
      posicionar_cursor(2, 7);
      show_LCD_float(t, 1);
      posicionar_cursor(2, 12);
      cmd_LCD(223, 1);
      posicionar_cursor(2, 13);
      show_LCD("C");
      posicionar_cursor(2, 16);
      cmd_LCD(0x40, 1);
      alert_led();
    }else if(potValue > 1024/2){
      PORTH |= (1 << BUZZER);
      PORTC |= (1 << led_red);
      cmd_LCD(0x01, 0);
      posicionar_cursor(1, 5);
      show_LCD("ALERTA!!");
      posicionar_cursor(2, 6);
      show_LCD("Fogo!!");
      _delay_ms(2000);
      cmd_LCD(0x01, 0);
    }else{
      PORTH &= ~(1 << BUZZER);
      posicionar_cursor(1, 1);
      show_LCD("Umidade: ");
      posicionar_cursor(1, 10);
      show_LCD_float(h, 1);
      posicionar_cursor(1, 14);
      show_LCD(" %");
      posicionar_cursor(2, 1);
      show_LCD("Temp: ");
      posicionar_cursor(2, 7);
      show_LCD_float(t, 1);
      posicionar_cursor(2, 12);
      cmd_LCD(223, 1);
      posicionar_cursor(2, 13);
      show_LCD("C   ");
      alert_led();
    }
  }
  mode_screen = 1;
  enable_c = 1;
}

// Tela 2: Configuração
void screen_2(){
  control_button_lft = 1;
  control_button_slt = 1;
  control_button_rgt = 1;
  // Limpa a tela do Display
  if(enable_c == 1){
    cmd_LCD(0x01, 0);
    enable_c = 0;
  }

  // Permanecer na tela de configuração enquanto n aperta-se outro botão
  while(mode_screen == 1){
    posicionar_cursor(1, 4);
    show_LCD("Configurar");
    posicionar_cursor(2, 5);
    show_LCD("1  2  3");
    
    if(control_button_lft == 0){
      mode = 1;
      mode_screen = 2;
      enable_c = 1;
    }else if(control_button_slt == 0){
      mode = 2;
      mode_screen = 2;
      enable_c = 1;
    }else if(control_button_rgt == 0){
      mode = 3;
      mode_screen = 2;
      enable_c = 1;
    }
  }
}

// Configurando a umidade
void screen_3(){
  if(enable_c == 1){
    cmd_LCD(0x01, 0);
    enable_c = 0;
  }
  while(mode_screen == 2){
    control_button_lft = 1;
    control_button_slt = 1;
    control_button_rgt = 1;
    posicionar_cursor(1, 4);
    show_LCD("Configurar");

    if(set_umi == 1){
      posicionar_cursor(2, 3);
      show_LCD("  Umidade  ");
    }else if(set_temp == 1){
      posicionar_cursor(2, 3);
      show_LCD("Temperatura");
    }
    
    if(control_button_slt == 0 && mode == 1 && set_umi == 1 && set_temp == 0){
      default_umi_max = 100;
      mode_screen = 3;
      enable_c = 1;
    }else if(control_button_slt == 0 && mode == 1 && set_umi == 0 && set_temp == 1){
      default_temp_max = 80;
      mode_screen = 3;
      enable_c = 1;
    }if(control_button_slt == 0 && mode == 2 && set_umi == 1){
      mode_screen = 3;
      enable_c = 1;
    }else if(control_button_slt == 0 && mode == 2 && set_temp == 1){
      mode_screen = 3;
      enable_c = 1;
    }if(control_button_slt == 0 && mode == 3 && set_umi == 1){
      default_umi_min = 0;
      mode_screen = 4;
      enable_c = 1;
    }else if(control_button_slt == 0 && mode == 3 && set_temp == 1){
      default_temp_min = -40;
      mode_screen = 4;
      enable_c = 1;
    }else if(control_button_rgt == 0){
      set_umi = 0;
      set_temp = 1;
    }else if(control_button_lft == 0){
      set_umi = 1;
      set_temp = 0;
    }
  }
}

// Configurando a Temperatura
void screen_4(){
  if(enable_c == 1){
    cmd_LCD(0x01, 0);
    enable_c = 0;
  }
  while(mode_screen == 3){
    control_button_lft = 1;
    control_button_slt = 1;
    control_button_rgt = 1;
    if(mode == 1 && set_umi == 1){
      posicionar_cursor(1, 3);
      show_LCD("Ajuste o min");
      posicionar_cursor(2, 1);
      show_LCD("Umidade: ");
      posicionar_cursor(2, 10);
      show_LCD_float(default_umi_min, 1);

      //control_button_slt = click_select();

      if(control_button_lft == 0){
        default_umi_min = default_umi_min - 1;
      }else if(control_button_rgt == 0){
        default_umi_min = default_umi_min + 1;
      }else if(control_button_slt == 0){
        mode_screen = 0;
        enable_c = 1;
        control_button_lft = 1;
        control_button_slt = 1;
        control_button_rgt = 1;
      }
    }else if(mode == 1 && set_temp == 1){
      posicionar_cursor(1, 3);
      show_LCD("Ajuste o min");
      posicionar_cursor(2, 1);
      show_LCD("Temperatura: ");
      posicionar_cursor(2, 14);
      show_LCD_float(default_temp_min, 1);
    
      //control_button_slt = click_select();

      if(control_button_lft == 0){
        default_temp_min = default_temp_min - 1;
      }else if(control_button_rgt == 0){
        default_temp_min = default_temp_min + 1;
      }else if(control_button_slt == 0){
        mode_screen = 0;
        enable_c = 1;
        control_button_lft = 1;
        control_button_slt = 1;
        control_button_rgt = 1;
      }
    }else if(mode == 2 && set_umi == 1){
      posicionar_cursor(1, 3);
      show_LCD("Ajuste o min");
      posicionar_cursor(2, 1);
      show_LCD("Umidade: ");
      posicionar_cursor(2, 10);
      show_LCD_float(default_umi_min, 1);

      //control_button_slt = click_select();

      if(control_button_lft == 0){
        default_umi_min = default_umi_min - 1;
      }else if(control_button_rgt == 0){
        default_umi_min = default_umi_min + 1;
      }else if(control_button_slt == 0){
        mode_screen = 4;
        enable_c = 1;
      }
    }else if(mode == 2 && set_temp == 1){
      posicionar_cursor(1, 3);
      show_LCD("Ajuste o min");
      posicionar_cursor(2, 1);
      show_LCD("Temperatura: ");
      posicionar_cursor(2, 14);
      show_LCD_float(default_temp_min, 1);
    
      //control_button_slt = click_select();

      if(control_button_lft == 0){
        default_temp_min = default_temp_min - 1;
      }else if(control_button_rgt == 0){
        default_temp_min = default_temp_min + 1;
      }else if(control_button_slt == 0){
        mode_screen = 4;
        enable_c = 1;
      }
    }
  }
}

void screen_5(){
  if(enable_c == 1){
    cmd_LCD(0x01, 0);
    enable_c = 0;
  }
  while(mode_screen == 4){
    control_button_lft = 1;
    control_button_slt = 1;
    control_button_rgt = 1;
    if(mode == 3 && set_umi == 1){
      posicionar_cursor(1, 3);
      show_LCD("Ajuste o max");
      posicionar_cursor(2, 1);
      show_LCD("Umidade: ");
      posicionar_cursor(2, 10);
      show_LCD_float(default_umi_max, 1);

      //control_button_slt = click_select();

      if(control_button_lft == 0){
        default_umi_max = default_umi_max - 1;
      }else if(control_button_rgt == 0){
        default_umi_max = default_umi_max + 1;
      }else if(control_button_slt == 0){
        mode_screen = 0;
        enable_c = 1;
        control_button_lft = 1;
        control_button_slt = 1;
        control_button_rgt = 1;
      }
    }else if(mode == 3 && set_temp == 1){
      posicionar_cursor(1, 3);
      show_LCD("Ajuste o max");
      posicionar_cursor(2, 1);
      show_LCD("Temperatura: ");
      posicionar_cursor(2, 14);
      show_LCD_float(default_temp_max, 1);
    
      //control_button_slt = click_select();

      if(control_button_lft == 0){
        default_temp_max = default_temp_max - 1;
      }else if(control_button_rgt == 0){
        default_temp_max = default_temp_max + 1;
      }else if(control_button_slt == 0){
        mode_screen = 0;
        enable_c = 1;
        control_button_lft = 1;
        control_button_slt = 1;
        control_button_rgt = 1;
      }
    }else if(mode == 2 && set_umi == 1){
      posicionar_cursor(1, 3);
      show_LCD("Ajuste o max");
      posicionar_cursor(2, 1);
      show_LCD("Umidade: ");
      posicionar_cursor(2, 10);
      show_LCD_float(default_umi_max, 1);

      //control_button_slt = click_select();

      if(control_button_lft == 0){
        default_umi_max = default_umi_max - 1;
      }else if(control_button_rgt == 0){
        default_umi_max = default_umi_max + 1;
      }else if(control_button_slt == 0){
        mode_screen = 0;
        enable_c = 1;
        control_button_lft = 1;
        control_button_slt = 1;
        control_button_rgt = 1;
      }
    }else if(mode == 2 && set_temp == 1){
      posicionar_cursor(1, 3);
      show_LCD("Ajuste o max");
      posicionar_cursor(2, 1);
      show_LCD("Temperatura: ");
      posicionar_cursor(2, 14);
      show_LCD_float(default_temp_max, 1);
    
      //control_button_slt = click_select();

      if(control_button_lft == 0){
        default_temp_max = default_temp_max - 1;
      }else if(control_button_rgt == 0){
        default_temp_max = default_temp_max + 1;
      }else if(control_button_slt == 0){
        mode_screen = 0;
        enable_c = 1;
        control_button_lft = 1;
        control_button_slt = 1;
        control_button_rgt = 1;
      }
    }
  }
}

void alert_led(){
  if(mode == 1 && (h < default_umi_min || t < default_temp_min)){
    PORTH |= (1 << BUZZER);
    PORTC |= (1 << led_red);
    PORTC &= ~(1 << led_yellow);
    PORTC &= ~(1 << led_green);
  }else if(mode == 1 && ((h >= default_umi_min && h <= default_umi_min*1.1) || (t >= default_temp_min && t <= default_temp_min*1.1))){
    PORTH &= ~(1 << BUZZER);
    PORTC &= ~(1 << led_red);
    PORTC |= (1 << led_yellow);
    PORTC &= ~(1 << led_green);
  }else if(mode == 1 && ((h > default_umi_min) && (t > default_temp_min))){
    PORTH &= ~(1 << BUZZER);
    PORTC &= ~(1 << led_red);
    PORTC &= ~(1 << led_yellow);
    PORTC |= (1 << led_green);
  }

  if(mode == 2 && ((h < default_umi_min || h > default_umi_max) || (t < default_temp_min || t > default_temp_max))){
    PORTH |= (1 << BUZZER);
    PORTC |= (1 << led_red);
    PORTC &= ~(1 << led_yellow);
    PORTC &= ~(1 << led_green);
  }else if(mode == 2 && (((h >= default_umi_min && h <= default_umi_min*1.1)||(h <= default_umi_max && h >= default_umi_max*0.9)) || ((t >= default_temp_min && t <= default_temp_min*1.1)||(t <= default_temp_max && t >= default_temp_max*0.9)))){
    PORTH &= ~(1 << BUZZER);
    PORTC &= ~(1 << led_red);
    PORTC |= (1 << led_yellow);
    PORTC &= ~(1 << led_green);
  }else if(mode == 2 && ((h > default_umi_min && h < default_umi_max) || (t > default_temp_min && t < default_temp_max))){
    PORTH &= ~(1 << BUZZER);
    PORTC &= ~(1 << led_red);
    PORTC &= ~(1 << led_yellow);
    PORTC |= (1 << led_green);
  }

  if(mode == 3 && ((h > default_umi_max) || (t > default_temp_max))){
    PORTH |= (1 << BUZZER);
    PORTC |= (1 << led_red);
    PORTC &= ~(1 << led_yellow);
    PORTC &= ~(1 << led_green);
  }else if(mode == 3 && ((h >= default_umi_max*0.9) || (t >= default_temp_max*0.9))) {
    PORTH &= ~(1 << BUZZER);
    PORTC &= ~(1 << led_red);
    PORTC |= (1 << led_yellow);
    PORTC &= ~(1 << led_green);
  }else if(mode == 3 && ((h < default_umi_max) || (t < default_temp_max))){
    PORTH &= ~(1 << BUZZER);
    PORTC &= ~(1 << led_red);
    PORTC &= ~(1 << led_yellow);
    PORTC |= (1 << led_green);
  }
}

uint16_t readPotentiometer() {
    // Configura o pino do potenciômetro como entrada
    DDRA &= ~(1 << POTENTIOMETER_PIN);
    
    // Realiza a leitura do valor analógico do potenciômetro
    ADCSRA |= (1 << ADPS2) | (1 << ADPS1); // Configura o pré-divisor do ADC para 64 (opcional)
    ADMUX = (1 << REFS0) | (POTENTIOMETER_PIN & 0x07); // Configura a referência e o pino de leitura
    ADCSRA |= (1 << ADEN); // Habilita o conversor AD
    ADCSRA |= (1 << ADSC); // Inicia a conversão
    while (ADCSRA & (1 << ADSC)); // Aguarda a conversão ser concluída
    uint16_t potValue = ADC; // Lê o valor convertido
    
    return potValue;
}

// O sistema está pronto para operar e é inicializado
int main() {
  start();
  Serial.begin(9600);
  posicionar_cursor(1, 4);
  show_LCD("InovaTech");
  posicionar_cursor(2, 4);
  show_LCD("Solutions");
  _delay_ms(2000);
  while(true){
    posicionar_cursor(1, 4);
    show_LCD("InovaTech");
    posicionar_cursor(2, 4);
    show_LCD("Solutions");
    if(mode_screen == 0){
      screen_1();
    }else if(mode_screen == 1){
      screen_2();
    }else if(mode_screen == 2){
      screen_3();
    }else if(mode_screen == 3){
      screen_4();
    }else if(mode_screen == 4){
      screen_5();
    }
  }
  return 0;
}

// Detecção do click select
ISR(INT0_vect) {
  _delay_ms(100);
  control_button_slt = 0;
  EIFR |= (1 << INTF0);  // Limpar sinal de interrupção INT0
}

// Detecção do click select
ISR(INT1_vect) {
  _delay_ms(100);
  control_button_lft = 0;
  EIFR |= (1 << INTF1);  // Limpar sinal de interrupção INT0
}

// Detecção do click select
ISR(INT2_vect) {
  _delay_ms(100);
  control_button_rgt = 0;
  EIFR |= (1 << INTF2);  // Limpar sinal de interrupção INT0
}
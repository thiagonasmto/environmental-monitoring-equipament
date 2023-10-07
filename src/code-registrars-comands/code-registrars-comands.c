// Incluisão das bibliotecas
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdlib.h>
#include "DHT.h"

// Configuração os pinos
#define RS PH3
#define E PH5
#define DADOS_LCD PORTA
#define nibble_dados 1
#define button_select PC0
#define button_left PC1
#define button_rigth PC2
#define led_red PC3
#define led_yellow PC4
#define led_green PC5
#define MAX_NUM_STR_LEN 12
#define DHTPIN 2 
#define DHTTYPE DHT22 

DHT dht(DHTPIN, DHTTYPE);

// Definindo as variáveis globais
bool enable_lft = 1;
bool enable_slt = 1;
bool enable_rgt = 1;
bool enable_c = 1;
bool control_button_lft = 0;
bool control_button_slt = 0;
bool control_button_rgt = 0;
bool set_umi = 1;
bool set_temp = 0;

int mode = 0;
int mode_screen = 0;
int default_umi_min = 40;
int default_umi_max = 40;
int default_temp_min = 20;
int default_temp_max = 20;

float h;
float t;

// Responsável por controlar as operações de escrita/leitura no display
void pulse_E() {
    _delay_ms(1);
    PORTH |= (1 << E);
    _delay_ms(1);
    PORTH &= ~(1 << E);
    _delay_ms(1);
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
  DDRH = 0b00101000;
  DDRC = 0b00111000;

  // DDRE &= ~(1 << DDE4);
  // PORTE &= ~(1 << PORTE4);

  // EICRB |= (1 << ISC41) | (1 << ISC40);   // Configurar para detecção de borda de descida
  // EIMSK |= (1 << INT4);                   // Habilitar a interrupção externa 4

  // // Habilitar interrupções globais
  // sei();

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

// Verifica o click do botão left
bool click_left(){
  if (PINC & (1 << button_left)) {
    if(enable_lft == 0){
      enable_lft = 1;
    }
    return 0;
  } else if(enable_lft == 1){
    enable_lft = 0;
    return 1;
  }
}

// Verifica o click do botão select
bool click_select() {
  if (PINC & (1 << button_select)) {
    if(enable_slt == 0){
      enable_slt = 1;
    }
    return 0;
  } else if(enable_slt == 1){
    enable_slt = 0;
    return 1;
  }
}

// Verifica o click do botão rigth
bool click_right(){
  if (PINC & (1 << button_rigth)) {
    if(enable_rgt == 0){
      enable_rgt = 1;
    }
    return 0;
  } else if(enable_rgt == 1){
    enable_rgt = 0;
    return 1;
  }
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
  while(!click_select()){
    _delay_ms(250);
    h = dht.readHumidity();
    // Temperature em Celsius (default)
    _delay_ms(250);
    t = dht.readTemperature();

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
    alert_led();
  }

  mode_screen = 1;
  enable_c = 1;
}

// Tela 2: Configuração
void screen_2(){
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
    
    if(click_left()){
      mode = 1;
      mode_screen = 2;
      enable_c = 1;
    }else if(click_select()){
      mode = 2;
      mode_screen = 2;
      enable_c = 1;
    }else if(click_right()){
      mode = 3;
      mode_screen = 2;
      enable_c = 1;
    }
  }
}

// Configurando a umidade
void screen_3(){
  // Serial.print("Modo selecionado: ");
  // Serial.println(mode);
  if(enable_c == 1){
    cmd_LCD(0x01, 0);
    enable_c = 0;
  }
  while(mode_screen == 2){
    posicionar_cursor(1, 4);
    show_LCD("Configurar");

    if(set_umi == 1){
      posicionar_cursor(2, 3);
      show_LCD("  Umidade  ");
      // Serial.print("set_umi: ");
      // Serial.println(set_umi);
    }else if(set_temp == 1){
      posicionar_cursor(2, 3);
      show_LCD("Temperatura");
      // Serial.print("set_temp: ");
      // Serial.println(set_temp);
    }

    control_button_slt = click_select();
    
    if(control_button_slt && mode == 1 && set_umi == 1 && set_temp == 0){
      default_umi_max = 100;
      mode_screen = 3;
      enable_c = 1;
    }else if(control_button_slt && mode == 1 && set_umi == 0 && set_temp == 1){
      default_temp_max = 80;
      mode_screen = 3;
      enable_c = 1;
    }if(control_button_slt && mode == 2 && set_umi == 1){
      mode_screen = 3;
      enable_c = 1;
    }else if(control_button_slt && mode == 2 && set_temp == 1){
      mode_screen = 3;
      enable_c = 1;
    }if(control_button_slt && mode == 3 && set_umi == 1){
      default_umi_min = 0;
      mode_screen = 4;
      enable_c = 1;
    }else if(control_button_slt && mode == 3 && set_temp == 1){
      default_temp_min = -40;
      mode_screen = 4;
      enable_c = 1;
    }else if(click_right()){
      set_umi = 0;
      set_temp = 1;
    }else if(click_left()){
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
    if(mode == 1 && set_umi == 1){
      posicionar_cursor(1, 3);
      show_LCD("Ajuste o min");
      posicionar_cursor(2, 1);
      show_LCD("Umidade: ");
      posicionar_cursor(2, 10);
      show_LCD_float(default_umi_min, 1);

      control_button_slt = click_select();

      if(click_left()){
        default_umi_min = default_umi_min - 1;
      }else if(click_right()){
        default_umi_min = default_umi_min + 1;
      }else if(control_button_slt){
        mode_screen = 0;
        enable_c = 1;
      }
    }else if(mode == 1 && set_temp == 1){
      posicionar_cursor(1, 3);
      show_LCD("Ajuste o min");
      posicionar_cursor(2, 1);
      show_LCD("Temperatura: ");
      posicionar_cursor(2, 14);
      show_LCD_float(default_temp_min, 1);
    
      control_button_slt = click_select();

      if(click_left()){
        default_temp_min = default_temp_min - 1;
      }else if(click_right()){
        default_temp_min = default_temp_min + 1;
      }else if(control_button_slt){
        mode_screen = 0;
        enable_c = 1;
      }
    }else if(mode == 2 && set_umi == 1){
      posicionar_cursor(1, 3);
      show_LCD("Ajuste o min");
      posicionar_cursor(2, 1);
      show_LCD("Umidade: ");
      posicionar_cursor(2, 10);
      show_LCD_float(default_umi_min, 1);

      control_button_slt = click_select();

      if(click_left()){
        default_umi_min = default_umi_min - 1;
      }else if(click_right()){
        default_umi_min = default_umi_min + 1;
      }else if(control_button_slt){
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
    
      control_button_slt = click_select();

      if(click_left()){
        default_temp_min = default_temp_min - 1;
      }else if(click_right()){
        default_temp_min = default_temp_min + 1;
      }else if(control_button_slt){
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
    if(mode == 3 && set_umi == 1){
      posicionar_cursor(1, 3);
      show_LCD("Ajuste o max");
      posicionar_cursor(2, 1);
      show_LCD("Umidade: ");
      posicionar_cursor(2, 10);
      show_LCD_float(default_umi_max, 1);

      control_button_slt = click_select();

      if(click_left()){
        default_umi_max = default_umi_max - 1;
      }else if(click_right()){
        default_umi_max = default_umi_max + 1;
      }else if(control_button_slt){
        mode_screen = 0;
        enable_c = 1;
      }
    }else if(mode == 3 && set_temp == 1){
      posicionar_cursor(1, 3);
      show_LCD("Ajuste o max");
      posicionar_cursor(2, 1);
      show_LCD("Temperatura: ");
      posicionar_cursor(2, 14);
      show_LCD_float(default_temp_max, 1);
    
      control_button_slt = click_select();

      if(click_left()){
        default_temp_max = default_temp_max - 1;
      }else if(click_right()){
        default_temp_max = default_temp_max + 1;
      }else if(control_button_slt){
        mode_screen = 0;
        enable_c = 1;
      }
    }else if(mode == 2 && set_umi == 1){
      posicionar_cursor(1, 3);
      show_LCD("Ajuste o max");
      posicionar_cursor(2, 1);
      show_LCD("Umidade: ");
      posicionar_cursor(2, 10);
      show_LCD_float(default_umi_max, 1);

      control_button_slt = click_select();

      if(click_left()){
        default_umi_max = default_umi_max - 1;
      }else if(click_right()){
        default_umi_max = default_umi_max + 1;
      }else if(control_button_slt){
        mode_screen = 0;
        enable_c = 1;
      }
    }else if(mode == 2 && set_temp == 1){
      posicionar_cursor(1, 3);
      show_LCD("Ajuste o max");
      posicionar_cursor(2, 1);
      show_LCD("Temperatura: ");
      posicionar_cursor(2, 14);
      show_LCD_float(default_temp_max, 1);
    
      control_button_slt = click_select();

      if(click_left()){
        default_temp_max = default_temp_max - 1;
      }else if(click_right()){
        default_temp_max = default_temp_max + 1;
      }else if(control_button_slt){
        mode_screen = 0;
        enable_c = 1;
      }
    }
  }
}

void alert_led(){
  Serial.println("Entrou nos leds");
  Serial.print("Modo: ");
  Serial.println(mode);
  if(mode == 1 && (h < default_umi_min || t < default_temp_min)){
    Serial.println("Entrou 1");
    PORTC |= (1 << led_red);
    PORTC &= ~(1 << led_yellow);
    PORTC &= ~(1 << led_green);
  }else if(mode == 1 && ((h >= default_umi_min && h <= default_umi_min*1.1) || (t >= default_temp_min && t <= default_temp_min*1.1))){
    Serial.println("Entrou 2");
    PORTC &= ~(1 << led_red);
    PORTC |= (1 << led_yellow);
    PORTC &= ~(1 << led_green);
  }else if(mode == 1 && ((h > default_umi_min) && (t > default_temp_min))){
    Serial.println("Entrou 3");
    PORTC &= ~(1 << led_red);
    PORTC &= ~(1 << led_yellow);
    PORTC |= (1 << led_green);
  }

  if(mode == 2 && ((h < default_umi_min || h > default_umi_max) || (t < default_temp_min || t > default_temp_max))){
    Serial.println("Entrou 4");
    PORTC |= (1 << led_red);
    PORTC &= ~(1 << led_yellow);
    PORTC &= ~(1 << led_green);
  }else if(mode == 2 && (((h >= default_umi_min && h <= default_umi_min*1.1)||(h <= default_umi_max && h >= default_umi_max*0.9)) || ((t >= default_temp_min && t <= default_temp_min*1.1)||(t <= default_temp_max && t >= default_temp_max*0.9)))){
    Serial.println("Entrou 5");
    PORTC &= ~(1 << led_red);
    PORTC |= (1 << led_yellow);
    PORTC &= ~(1 << led_green);
  }else if(mode == 2 && ((h > default_umi_min && h < default_umi_max) || (t > default_temp_min && t < default_temp_max))){
    Serial.println("Entrou 6");
    PORTC &= ~(1 << led_red);
    PORTC &= ~(1 << led_yellow);
    PORTC |= (1 << led_green);
  }

  if(mode == 3 && ((h > default_umi_max) || (t > default_temp_max))){
    Serial.println("Entrou 7");
    PORTC |= (1 << led_red);
    PORTC &= ~(1 << led_yellow);
    PORTC &= ~(1 << led_green);
  }else if(mode == 3 && ((h >= default_umi_max*0.9) || (t >= default_temp_max*0.9))) {
    Serial.println("Entrou 8");
    PORTC &= ~(1 << led_red);
    PORTC |= (1 << led_yellow);
    PORTC &= ~(1 << led_green);
  }else if(mode == 3 && ((h < default_umi_max) || (t < default_temp_max))){
    Serial.println("Entrou 9");
    PORTC &= ~(1 << led_red);
    PORTC &= ~(1 << led_yellow);
    PORTC |= (1 << led_green);
  }
}

// Detecção do click select
// ISR(INT4_vect) {
//   Serial.println("Entrou interrup");
//   PORTC |= (1 << led_yellow);
// }

// O sistema está pronto para operar e é inicializado
void setup(){
  Serial.begin(9600);
  dht.begin();
  start();
}

void loop() {
  click_left();
  click_select();
  click_right();

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
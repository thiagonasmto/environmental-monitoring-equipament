#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdlib.h>

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

bool enable_b = 1;
bool enable_c = 1;
bool control_button_lft = 0;
bool control_button_slt = 0;
bool control_button_rgt = 0;

int mode_screen = 0;
int default_umi = 40;
int default_temp = 20;

void pulse_E() {
    _delay_ms(1);
    PORTH |= (1 << E);
    _delay_ms(1);
    PORTH &= ~(1 << E);
    _delay_ms(1);
}

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
  cmd_LCD(0x28, 0); //interface de 4 bits 2 linhas
  cmd_LCD(0x08, 0); //desliga o display
  cmd_LCD(0x01, 0); //limpa todo o display
  cmd_LCD(0x0C, 0); //mensagem aparente cursor inativo não piscando
  cmd_LCD(0x80, 0); //inicializa cursor na primeira posição a esquerda - 1a linha 
}

void show_LCD(const char *str) {
    while (*str) {
        cmd_LCD(*str++, 1);
    }
}

void printNumberOnLCD(int number) {
    char numStr[MAX_NUM_STR_LEN];  // Array para armazenar a string do número

    // Converte o número em uma string
    snprintf(numStr, MAX_NUM_STR_LEN, "%d", number);

    // Exibe a string no LCD
    show_LCD(numStr);
}

//Verifica o click do botão left
bool click_left(){
  if (PINC & (1 << button_left)) {
    // Botão pressionado, desliga o LED
    PORTC &= ~(1 << led_red);
    return 0;
  } else {
    // Botão não pressionado, liga o LED
    PORTC |= (1 << led_red);
    return 1;
  }
}

//Verifica o click do botão select
bool click_select() {
  if (PINC & (1 << button_select)) {
    // Botão não pressionado, desliga o LED
    PORTC &= ~(1 << led_yellow);
    return 0;
  } else {
    // Botão pressionado e enable_b é 1, liga o LED
    PORTC |= (1 << led_yellow);
    return 1;
  }
  return 0;
}

//Verifica o click do botão rigth
bool click_rigth(){
  if (PINC & (1 << button_rigth)) {
    // Botão pressionado, desliga o LED
    PORTC &= ~(1 << led_green);
    return 0;
  } else {
    // Botão não pressionado, liga o LED
    PORTC |= (1 << led_green);
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

void screen_1(){
  if(enable_c == 1){
    cmd_LCD(0x01, 0);
    enable_c = 0;
  }
  while(!click_select()){
    posicionar_cursor(1, 1);
    show_LCD("Umidade:");
    posicionar_cursor(2, 1);
    show_LCD("Temp:");
  }
  Serial.println("Saiu do while");
  mode_screen = 1;
  enable_c = 1;
}

void screen_2(){
  if(enable_c == 1){
    cmd_LCD(0x01, 0);
    enable_c = 0;
  }
  while(mode_screen == 1){
    posicionar_cursor(1, 4);
    show_LCD("Configurar");
    posicionar_cursor(2, 5);
    show_LCD("1  2  3");
    control_button_lft = click_left();
    control_button_slt = click_select();
    control_button_rgt = click_rigth();
    if(control_button_lft){
      mode_screen = 2;
      enable_c = 1;
    }else if(control_button_slt){
      mode_screen = 6;
      enable_c = 1;
    }else if(control_button_rgt){
      mode_screen = 3;
      enable_c = 1;
    }
  }
  Serial.println("Saiu do while 2");
}

void screen_3(){
  if(enable_c == 1){
    cmd_LCD(0x01, 0);
    enable_c = 0;
  }
  while(mode_screen == 2){
    posicionar_cursor(1, 4);
    show_LCD("Configurar");
    posicionar_cursor(2, 5);
    show_LCD("Umidade");
    control_button_lft = click_left();
    control_button_slt = click_select();
    control_button_rgt = click_rigth();
    if(control_button_slt){
      mode_screen = 4;
      enable_c = 1;
    }else if(control_button_rgt){
      mode_screen = 3;
      enable_c = 1;
    }
  }
  Serial.println("Saiu do while 3");
}

void screen_4(){
  if(enable_c == 1){
    cmd_LCD(0x01, 0);
    enable_c = 0;
  }
  while(mode_screen == 3){
    posicionar_cursor(1, 4);
    show_LCD("Configurar");
    posicionar_cursor(2, 3);
    show_LCD("Temperatura");
    control_button_lft = click_left();
    control_button_slt = click_select();
    control_button_rgt = click_rigth();
    if(control_button_slt){
      mode_screen = 5;
      enable_c = 1;
    }else if(control_button_lft){
      mode_screen = 2;
      enable_c = 1;
    }
  }
  Serial.println("Saiu do while 4");
}

void screen_5(){
  if(enable_c == 1){
    cmd_LCD(0x01, 0);
    enable_c = 0;
  }
  while(mode_screen == 4){
    posicionar_cursor(1, 3);
    show_LCD("Ajuste o min");
    posicionar_cursor(2, 1);
    show_LCD("Umidade: ");
    posicionar_cursor(2, 10);
    printNumberOnLCD(default_umi);
    control_button_lft = click_left();
    control_button_slt = click_select();
    control_button_rgt = click_rigth();
    if(control_button_lft){
      default_umi = default_umi - 1;
    }else if(control_button_rgt){
      default_umi = default_umi + 1;
    }else if(control_button_slt){
      mode_screen = 0;
      enable_c = 1;
    }
  }
  _delay_ms(250);
  Serial.println("Saiu do while 5");
}

void screen_6(){
  if(enable_c == 1){
    cmd_LCD(0x01, 0);
    enable_c = 0;
  }
  while(mode_screen == 5){
    posicionar_cursor(1, 3);
    show_LCD("Ajuste o min");
    posicionar_cursor(2, 1);
    show_LCD("Temperatura: ");
    posicionar_cursor(2, 14);
    printNumberOnLCD(default_temp);
    control_button_lft = click_left();
    control_button_slt = click_select();
    control_button_rgt = click_rigth();
    if(control_button_lft){
      default_temp = default_temp - 1;
    }else if(control_button_rgt){
      default_temp = default_temp + 1;
    }else if(control_button_slt){
      mode_screen = 0;
      enable_c = 1;
    }
  }
  _delay_ms(250);
  Serial.println("Saiu do while 6");
}

// Detecção do click select
// ISR(INT4_vect) {
//   Serial.println("Entrou interrup");
//   PORTC |= (1 << led_yellow);
// }

void setup(){
  Serial.begin(9600);
  start();
}

void loop() {
  click_left();
  click_select();
  click_rigth();
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
  }else if(mode_screen == 5){
    screen_6();
  }
}

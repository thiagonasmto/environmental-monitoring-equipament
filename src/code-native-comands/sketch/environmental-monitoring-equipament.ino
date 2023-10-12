#include "DHT.h"
#define DHTPIN 2     // Pino digital sensor DHT

// Escolha o tipo de sensor!
//# define DHTTYPE DHT11     // DHT 11
# define DHTTYPE DHT22   // DHT 22 (AM2302), AM2321
//# define DHTTYPE DHT21   // DHT 21 (AM2301)

DHT dht(DHTPIN, DHTTYPE);

#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// Definir o endereço do LCD para 0x27 para um display de 16 caracteres e 2 linhas
LiquidCrystal_I2C lcd(0x27, 16, 2);

int L_Red = 8;          //Dispositivos de alerta visual - Vermelho
int L_Green = 10;       //Dispositivos de alerta visual - Verde
int L_Yellow = 9;       //Dispositivos de alerta visual - Amarelo

int button_left = 5;    //Botão direcional para esquerda
int button_select = 6;  //Botão de seleção
int button_rigth = 7;   //Botão direcional para direita

bool statusButton_left = 0;    //variável de leitura botão esquerdo
bool statusButton_select = 0;  //variável de leitura botão de seleção
bool statusButton_rigth = 0;   //variável de leitura botão direito

bool E = 1;               //Enable para os botões
int click_select = 0;    //Variável de controle para verificar seleção (Temperatura ou Umidade, Modos - 1, 2 ou 3, finalização)
bool enable_clear = 1;    //Enable para apagar o LCD

int default_umi = 40;
int value_umi_min = 40;
int value_umi_max = 40;
int value_temp_min = 24;
int value_temp_max = 24;

bool select_temp = 0;
bool select_umi = 0;

int set_mode = 3;

//bool able = 0;

/*
int deb(bool statusButton_left, bool statusButton_select, bool statusButton_rigth, bool Enable){
  if(statusButton_left == 0 && statusButton_select == 0 && statusButton_rigth == 0 && Enable == 0){
    Enable = 1;
  }
  return Enable;
}
*/

void setup() {
  pinMode(L_Red, OUTPUT);
  pinMode(L_Green, OUTPUT);
  pinMode(L_Yellow, OUTPUT);

  pinMode(button_left, INPUT);
  pinMode(button_select, INPUT);
  pinMode(button_rigth, INPUT);

<<<<<<< HEAD
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

// Converte um número inteiro em uma string e exibe a string no LCD
void printNumberOnLCD(int number) {
    char numStr[MAX_NUM_STR_LEN];  // Array para armazenar a string do número

    // Converte o número em uma string
    snprintf(numStr, MAX_NUM_STR_LEN, "%d", number);

    // Exibe a string no LCD
    show_LCD(numStr);
}

// Verifica o click do botão left
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

// Verifica o click do botão select
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

// Verifica o click do botão rigth
bool click_right(){
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

// Tela principal: mostra a porcentagem da Umidade e Temperatura
void screen_1(){
  // Limpa a tela do Display
  if(enable_c == 1){
    cmd_LCD(0x01, 0);
    // enable_c = 0;
  }

  // Enquanto o botão de seleção não for pressionado, exibir a tela de porcentagem da Umidade e Temperatura
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
    
    if(click_left() && enable_c==1){
      mode_screen = 2;
      screen_3();
    }else if(click_select() && enable_c==1){
      mode_screen = 5;
      screen_6();
    }else if(click_right() && enable_c==1){
      mode_screen = 3;
      screen_4();
    }
    enable_c = 1;
  }
  Serial.println("Saiu do while 2");
}

// Configurando a umidade
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
    
    if(click_select() && enable_c==1){
      mode_screen = 4;
    }else if(click_right() && enable_c==1){
      mode_screen = 3;
      screen_4();
    }
    enable_c = 1;
  }
  Serial.println("Saiu do while 3");
}

// Configurando a Temperatura
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
    
    if(click_select() && enable_c==1){
      mode_screen = 5;
    }else if(click_right() && enable_c==1){
      mode_screen = 2;
      screen_3();
    }
    enable_c = 1;
  }
  Serial.println("Saiu do while 4");
}

// Ajuste da Umidade mínima
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
    
    if(click_left()){
      default_umi = default_umi - 1;
    }else if(click_right()){
      default_umi = default_umi + 1;
    }else if(click_select() && enable_c==1){
      mode_screen = 0;
    }
    enable_c = 1;
  }

  _delay_ms(250);
  Serial.println("Saiu do while 5");
}

// Ajuste da Temperatura mínima
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
    
    if(click_left()){
      default_temp = default_temp - 1;
    }else if(click_right()){
      default_temp = default_temp + 1;
    }else if(click_select() && enable){
      mode_screen = 0;
    }
    enable_c = 1;
  }

  _delay_ms(250);
  Serial.println("Saiu do while 6");
}

// Detecção do click select
// ISR(INT4_vect) {
//   Serial.println("Entrou interrup");
//   PORTC |= (1 << led_yellow);
// }

// O sistema está pronto para operar e é inicializado
void setup(){
=======
>>>>>>> be4f4acfaab5b895e2311fab31a7dc4320d60a44
  Serial.begin(9600);
  Serial.println(F("DHTxx teste!"));

  dht.begin();
  lcd.begin(16,2,1);
}

void loop() {
  // Aguarde alguns segundos entre as medições.
  //bdelay(2000);
  // A leitura da temperatura ou umidade leva cerca de 250 milissegundos!
  // O sensor pode ter um atraso de até 2 segundos para a leitura
  delay(250);
  float h = dht.readHumidity();
  // Temperature em Celsius (default)
  delay(250);
  float t = dht.readTemperature();

  // Verifique se alguma leitura falhou e tenta novamente.
  if (isnan(h) || isnan(t)) {
    lcd.printstr("Falha de leitura do sensor DHT!");
    return;
  }

  // Compute heat index in Celsius (isFahreheit = false)
  float hic = dht.computeHeatIndex(t, h, false);

  //ALERTAS VISUAIS
  if(set_mode == 1 && (h < value_umi_min || t < value_temp_min)){
    digitalWrite(L_Red, HIGH);
    digitalWrite(L_Yellow, LOW);
    digitalWrite(L_Green, LOW);
  }else if(set_mode == 1 && ((h >= value_umi_min && h <= value_umi_min*1.1) || (t >= value_temp_min && t <= value_temp_min*1.1))){
    digitalWrite(L_Red, LOW);
    digitalWrite(L_Yellow, HIGH);
    digitalWrite(L_Green, LOW);
  }else if(set_mode == 1 && ((h > value_umi_min) && (t > value_temp_min))){
    digitalWrite(L_Red, LOW);
    digitalWrite(L_Yellow, LOW);
    digitalWrite(L_Green, HIGH);
  }

  if(set_mode == 2 && ((h < value_umi_min || h > value_umi_max) || (t < value_temp_min || t > value_temp_max))){
    digitalWrite(L_Red, HIGH);
    digitalWrite(L_Yellow, LOW);
    digitalWrite(L_Green, LOW);
  }else if(set_mode == 2 && (((h >= value_umi_min && h <= value_umi_min*1.1)||(h <= value_umi_max && h >= value_umi_max*0.9)) || ((t >= value_temp_min && t <= value_temp_min*1.1)||(t <= value_temp_max && t >= value_temp_max*0.9)))){
    digitalWrite(L_Red, LOW);
    digitalWrite(L_Yellow, HIGH);
    digitalWrite(L_Green, LOW);
  }else if(set_mode == 2 && ((h > value_umi_min && h < value_umi_max) || (t > value_temp_min && t < value_temp_max))){
    digitalWrite(L_Red, LOW);
    digitalWrite(L_Yellow, LOW);
    digitalWrite(L_Green, HIGH);
  }

  if(set_mode == 3 && ((h > value_umi_max) || (t > value_temp_max))){
    digitalWrite(L_Red, HIGH);
    digitalWrite(L_Yellow, LOW);
    digitalWrite(L_Green, LOW);
  }else if(set_mode == 3 && ((h >= value_umi_max*0.9) || (t >= value_temp_max*0.9))) {
    digitalWrite(L_Red, LOW);
    digitalWrite(L_Yellow, HIGH);
    digitalWrite(L_Green, LOW);
  }else if(set_mode == 3 && ((h < value_umi_max) || (t < value_temp_max))){
    digitalWrite(L_Red, LOW);
    digitalWrite(L_Yellow, LOW);
    digitalWrite(L_Green, HIGH);
  }

  Serial.println(set_mode);
  
  Serial.print(F("Umidade:  "));
  Serial.print(h);
  Serial.print(F("%  Temperatura: "));
  Serial.print(t);
  Serial.println(F("°C "));
  
  lcd.setBacklight(HIGH);

  lcd.setCursor(0, 0);
  lcd.printstr("Umidade: ");
  lcd.setCursor(10, 0);
  lcd.print(round(h));
  lcd.setCursor(12, 0);
  lcd.print(F(" %"));
  //delay(3000);

  lcd.setCursor(0, 1);
  lcd.printstr("Temp: ");
  lcd.setCursor(7, 1);
  lcd.print(round(t));
  lcd.setCursor(9, 1);
  lcd.write(32);        // Caracter espaço
  lcd.write(223);       // Caracter °
  lcd.print(F("C"));
  //delay(3000);

  statusButton_left = digitalRead(button_left);
  statusButton_select = digitalRead(button_select);
  statusButton_rigth = digitalRead(button_rigth);

/*
  if(statusButton_left == 1 && E == 1){
    Serial.println("buttonRed HIGH");
    digitalWrite(L_Red, HIGH);
    E = 0;
  }
*/

  if(statusButton_select == 1 && E == 1){
    E = 0;
    int finish = 1;
    bool select = 0;
    lcd.clear();
    while(finish == 1){
      statusButton_left = digitalRead(button_left);
      statusButton_select = digitalRead(button_select);
      statusButton_rigth = digitalRead(button_rigth);

      if(select == 0 && click_select == 0){
        lcd.setCursor(0, 0);
        lcd.printstr("Selecione: ");
        lcd.setCursor(0, 1);
        lcd.printstr("Umidade    ");
      }else if(select == 1 && click_select == 0){
        lcd.setCursor(0, 0);
        lcd.printstr("Selecione:");
        lcd.setCursor(0, 1);
        lcd.printstr("Temperatura");
      }else if(click_select == 1){
        if(enable_clear == 1){
          Serial.println("Limpou LCD");
          lcd.clear();
          enable_clear = 0;
        }
        lcd.setCursor(0, 0);
        lcd.printstr("Set o modo:");
        lcd.setCursor(0, 1);
        lcd.printstr("1  2  3");
        delay(250);
        E = 1;

        while(click_select == 1){
          //Serial.println(click_select);
          statusButton_left = digitalRead(button_left);
          statusButton_select = digitalRead(button_select);
          statusButton_rigth = digitalRead(button_rigth);

          if(statusButton_left == 1 && statusButton_select == 0 && statusButton_rigth == 0 && E == 1){
            //Selecionou o modo 1
            Serial.println("Selecionou modo 1");
            E = 0;
            click_select = 2;
            lcd.clear();
            set_mode = 1;

            if(statusButton_left == 0 && statusButton_select == 0 && statusButton_rigth == 0 && E == 0){
              //Enable botões
              E = 1;
            }
            Serial.println(click_select);
          }else if(statusButton_left == 0 && statusButton_select == 1 && statusButton_rigth == 0 && E == 1){
            //Selecionou o modo 2
            Serial.println("Selecionou modo 2");
            E = 0;
            click_select = 3;
            lcd.clear();
            set_mode = 2;

            if(statusButton_left == 0 && statusButton_select == 0 && statusButton_rigth == 0 && E == 0){
              //Enable botões
              E = 1;
            }
          }else if(statusButton_left == 0 && statusButton_select == 0 && statusButton_rigth == 1 && E == 1){
            //Selecionou o modo 3
            Serial.println("Selecionou modo 3");
            E = 0;
            click_select = 4;
            lcd.clear();
            set_mode = 3;

            if(statusButton_left == 0 && statusButton_select == 0 && statusButton_rigth == 0 && E == 0){
              //Enable botões
              E = 1;
            }
          }
        }
      }else if(click_select == 2){
        //Serial.println("Modo: Valor mínimo");
        lcd.setCursor(0, 0);
        lcd.printstr("Set o minimo");
        lcd.setCursor(0, 1);
        if(select_umi == 1){
          lcd.print(value_umi_min);
          if(statusButton_left == 1 && statusButton_select == 0 && statusButton_rigth == 0 && E == 1 && value_umi_min > -40){
            E = 0;
            value_umi_min = value_umi_min-1;
            if(statusButton_left == 0 && statusButton_select == 0 && statusButton_rigth == 0 && E == 0){
              //Enable botões
              E = 1;
            }
          }else if(statusButton_left == 0 && statusButton_select == 0 && statusButton_rigth == 1 && E == 1 && value_umi_min < 80){
            E = 0;
            value_umi_min = value_umi_min+1;
            if(statusButton_left == 0 && statusButton_select == 0 && statusButton_rigth == 0 && E == 0){
              //Enable botões
              E = 1;
            }
          }else if(statusButton_left == 0 && statusButton_select == 1 && statusButton_rigth == 0 && E == 1){
            E = 0;
            finish = 0;
            select = 0;
            click_select = 0;
            select_umi = 0;
            if(statusButton_left == 0 && statusButton_select == 0 && statusButton_rigth == 0 && E == 0){
              //Enable botões
              E = 1;
            }
          }
        }else if(select_temp == 1){
          lcd.print(value_temp_min);
          if(statusButton_left == 1 && statusButton_select == 0 && statusButton_rigth == 0 && E == 1 && value_temp_min > -40){
            E = 0;
            value_temp_min = value_temp_min-1;
            if(statusButton_left == 0 && statusButton_select == 0 && statusButton_rigth == 0 && E == 0){
              //Enable botões
              E = 1;
            }
          }else if(statusButton_left == 0 && statusButton_select == 0 && statusButton_rigth == 1 && E == 1 && value_temp_min < 80){
            E = 0;
            value_temp_min = value_temp_min+1;
            if(statusButton_left == 0 && statusButton_select == 0 && statusButton_rigth == 0 && E == 0){
              //Enable botões
              E = 1;
            }
          }else if(statusButton_left == 0 && statusButton_select == 1 && statusButton_rigth == 0 && E == 1){
            E = 0;
            finish = 0;
            select = 0;
            click_select = 0;
            select_temp = 0;
            if(statusButton_left == 0 && statusButton_select == 0 && statusButton_rigth == 0 && E == 0){
              //Enable botões
              E = 1;
            }
          }
        }
      }else if(click_select == 3){
        //Serial.println("Modo: Valor mínimo");
        lcd.setCursor(0, 0);
        lcd.printstr("Set o minimo");
        lcd.setCursor(0, 1);
        if(select_umi == 1){
          lcd.print(value_umi_min);
          if(statusButton_left == 1 && statusButton_select == 0 && statusButton_rigth == 0 && E == 1 && value_umi_min > -40){
            E = 0;
            value_umi_min = value_umi_min-1;
            if(statusButton_left == 0 && statusButton_select == 0 && statusButton_rigth == 0 && E == 0){
              //Enable botões
              E = 1;
            }
          }else if(statusButton_left == 0 && statusButton_select == 0 && statusButton_rigth == 1 && E == 1 && value_umi_min < 80){
            E = 0;
            value_umi_min = value_umi_min+1;
            if(statusButton_left == 0 && statusButton_select == 0 && statusButton_rigth == 0 && E == 0){
              //Enable botões
              E = 1;
            }
          }else if(statusButton_left == 0 && statusButton_select == 1 && statusButton_rigth == 0 && E == 1){
            E = 0;
            click_select = 4;
            if(statusButton_left == 0 && statusButton_select == 0 && statusButton_rigth == 0 && E == 0){
              //Enable botões
              E = 1;
            }
          }
        }else if(select_temp == 1){
          lcd.print(value_temp_min);
          if(statusButton_left == 1 && statusButton_select == 0 && statusButton_rigth == 0 && E == 1 && value_temp_min > -40){
            E = 0;
            value_temp_min = value_temp_min-1;
            if(statusButton_left == 0 && statusButton_select == 0 && statusButton_rigth == 0 && E == 0){
              //Enable botões
              E = 1;
            }
          }else if(statusButton_left == 0 && statusButton_select == 0 && statusButton_rigth == 1 && E == 1 && value_temp_min < 80){
            E = 0;
            value_temp_min = value_temp_min+1;
            if(statusButton_left == 0 && statusButton_select == 0 && statusButton_rigth == 0 && E == 0){
              //Enable botões
              E = 1;
            }
          }else if(statusButton_left == 0 && statusButton_select == 1 && statusButton_rigth == 0 && E == 1){
            E = 0;
            click_select = 4;
            if(statusButton_left == 0 && statusButton_select == 0 && statusButton_rigth == 0 && E == 0){
              //Enable botões
              E = 1;
            }
          }
        }
      }else if(click_select == 4){
        //Serial.println("Modo: Valor máximo");
        lcd.setCursor(0, 0);
        lcd.printstr("Set o maximo");
        lcd.setCursor(0, 1);
        if(select_umi == 1){
          lcd.print(value_umi_max);
          if(statusButton_left == 1 && statusButton_select == 0 && statusButton_rigth == 0 && E == 1 && value_umi_max > -40){
            E = 0;
            value_umi_max = value_umi_max-1;
            if(statusButton_left == 0 && statusButton_select == 0 && statusButton_rigth == 0 && E == 0){
              //Enable botões
              E = 1;
            }
          }else if(statusButton_left == 0 && statusButton_select == 0 && statusButton_rigth == 1 && E == 1 && value_umi_max  < 80){
            E = 0;
            value_umi_max = value_umi_max+1;
            if(statusButton_left == 0 && statusButton_select == 0 && statusButton_rigth == 0 && E == 0){
              //Enable botões
              E = 1;
            }
          }else if(statusButton_left == 0 && statusButton_select == 1 && statusButton_rigth == 0 && E == 1){
            E = 0;
            finish = 0;
            select = 0;
            click_select = 0;
            select_umi = 0;
            if(statusButton_left == 0 && statusButton_select == 0 && statusButton_rigth == 0 && E == 0){
              //Enable botões
              E = 1;
            }
          }
        }else if(select_temp == 1){
          lcd.print(value_temp_max);
          if(statusButton_left == 1 && statusButton_select == 0 && statusButton_rigth == 0 && E == 1 && value_temp_max > -40){
            E = 0;
            value_temp_max = value_temp_max-1;
            if(statusButton_left == 0 && statusButton_select == 0 && statusButton_rigth == 0 && E == 0){
              //Enable botões
              E = 1;
            }
          }else if(statusButton_left == 0 && statusButton_select == 0 && statusButton_rigth == 1 && E == 1 && value_temp_max  < 80){
            E = 0;
            value_temp_max = value_temp_max+1;
            if(statusButton_left == 0 && statusButton_select == 0 && statusButton_rigth == 0 && E == 0){
              //Enable botões
              E = 1;
            }
          }else if(statusButton_left == 0 && statusButton_select == 1 && statusButton_rigth == 0 && E == 1){
            E = 0;
            finish = 0;
            select = 0;
            click_select = 0;
            select_temp = 0;
            if(statusButton_left == 0 && statusButton_select == 0 && statusButton_rigth == 0 && E == 0){
              //Enable botões
              E = 1;
            }
          }          
        }
      }
      
      if(statusButton_left == 0 && statusButton_select == 0 && statusButton_rigth == 0 && E == 0){
        //Enable botões
        E = 1;
      }

      if(statusButton_rigth == 1 && statusButton_select == 0 && statusButton_left == 0 && E == 1 && select == 0){
        //Set Temepratura como escolha
        E = 0;
        select = 1;

        if(statusButton_left == 0 && statusButton_select == 0 && statusButton_rigth == 0 && E == 0){
          //Enable botões
          E = 1;
        }
      }else if(statusButton_rigth == 0 && statusButton_select == 0 && statusButton_left == 1 && E == 1 && select == 1){
        //Set Umidade como escolha
        E = 0;
        select = 0;

        if(statusButton_left == 0 && statusButton_select == 0 && statusButton_rigth == 0 && E == 0){
          //Enable botões
          E = 1;
        }
      }else if(statusButton_rigth == 0 && statusButton_select == 1 && statusButton_left == 0 && E == 1 && select == 0 && click_select == 0){
        //Seleciona Umidade para ajuste de parâmetros
        Serial.println("Selecionou Umidade");
        E = 0;
        click_select = 1;
        select_umi = 1;
        Serial.print("select_umi = ");
        Serial.println(select_umi);
        enable_clear = 1;

        if(statusButton_left == 0 && statusButton_select == 0 && statusButton_rigth == 0 && E == 0){
          //Enable botões
          E = 1;
        }
      }else if(statusButton_rigth == 0 && statusButton_select == 1 && statusButton_left == 0 && E == 1 && select == 1 && click_select == 0){
        //Seleciona Umidade para ajuste de parâmetros
        Serial.println("Selecionou Temperatura");
        E = 0;
        click_select = 1;
        select_temp = 1;
        Serial.print("select_temp = ");
        Serial.println(select_temp);
        enable_clear = 1;

        if(statusButton_left == 0 && statusButton_select == 0 && statusButton_rigth == 0 && E == 0){
          //Enable botões
          E = 1;
        }
      }
    }
  }

/*
  if(statusButton_rigth == 1 && E == 1){
    lcd.clear();
    digitalWrite(L_Green, HIGH);
    E = 0;
  }
  
*/

  if(statusButton_left == 0 && statusButton_select == 0 && statusButton_rigth == 0 && E == 0){
    E = 1;
  }
}
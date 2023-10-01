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
    Serial.println("mode 1: h < min");
    digitalWrite(L_Red, HIGH);
    digitalWrite(L_Yellow, LOW);
    digitalWrite(L_Green, LOW);
  }else if(set_mode == 1 && ((h >= value_umi_min && h <= value_umi_min*1.1) || (t >= value_temp_min && t <= value_temp_min*1.1))){
    Serial.println("mode 1: h > min*1.1");
    digitalWrite(L_Red, LOW);
    digitalWrite(L_Yellow, HIGH);
    digitalWrite(L_Green, LOW);
  }else if(set_mode == 1 && ((h > value_umi_min) && (t > value_temp_min))){
    Serial.println("mode 1: h > min");
    digitalWrite(L_Red, LOW);
    digitalWrite(L_Yellow, LOW);
    digitalWrite(L_Green, HIGH);
  }

  if(set_mode == 2 && ((h < value_umi_min || h > value_umi_max) || (t < value_temp_min || t > value_temp_max))){
    Serial.println("Entrou 1");
    digitalWrite(L_Red, HIGH);
    digitalWrite(L_Yellow, LOW);
    digitalWrite(L_Green, LOW);
  }else if(set_mode == 2 && (((h >= value_umi_min && h <= value_umi_min*1.1)||(h <= value_umi_max && h >= value_umi_max*0.9)) || ((t >= value_temp_min && t <= value_temp_min*1.1)||(t <= value_temp_max && t >= value_temp_max*0.9)))){
    Serial.println("Entrou 2");
    digitalWrite(L_Red, LOW);
    digitalWrite(L_Yellow, HIGH);
    digitalWrite(L_Green, LOW);
  }else if(set_mode == 2 && ((h > value_umi_min && h < value_umi_max) || (t > value_temp_min && t < value_temp_max))){
    Serial.println("Entrou 3");
    digitalWrite(L_Red, LOW);
    digitalWrite(L_Yellow, LOW);
    digitalWrite(L_Green, HIGH);
  }

  if(set_mode == 3 && ((h > value_umi_max) || (t > value_temp_max))){
    Serial.println("mode 3: h > max");
    digitalWrite(L_Red, HIGH);
    digitalWrite(L_Yellow, LOW);
    digitalWrite(L_Green, LOW);
  }else if(set_mode == 3 && ((h >= value_umi_max*0.9) || (t >= value_temp_max*0.9))) {
    Serial.println("mode 3: h > faixa");
    digitalWrite(L_Red, LOW);
    digitalWrite(L_Yellow, HIGH);
    digitalWrite(L_Green, LOW);
  }else if(set_mode == 3 && ((h < value_umi_max) || (t < value_temp_max))){
    Serial.println("mode 3: h < max");
    digitalWrite(L_Red, LOW);
    digitalWrite(L_Yellow, LOW);
    digitalWrite(L_Green, HIGH);
  }

  Serial.println(set_mode);
  
  Serial.print(F("Umidade: "));
  Serial.print(h);
  Serial.print(F("%  Temperatura: "));
  Serial.print(t);
  Serial.println(F("°C "));
  
  lcd.setBacklight(HIGH);

  lcd.setCursor(0, 0);
  lcd.printstr("Humidade: ");
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
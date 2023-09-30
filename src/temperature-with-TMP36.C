// Definindo os registradores do ADC
#define ADMUX_REG   (*(volatile uint8_t*) 0x7C)
#define ADCSRA_REG  (*(volatile uint8_t*) 0x7A)
#define ADCH_REG    (*(volatile uint8_t*) 0x79)
#define ADCL_REG    (*(volatile uint8_t*) 0x78)

// Definindo os registradores do Digital I/O
#define PORTB_REG   (*(volatile uint8_t*) 0x25)
#define DDRB_REG    (*(volatile uint8_t*) 0x24)

// Definindo os pinos usados
#define A0_PIN      0 // Pino analógico A0
#define LED_PIN     13

void setup() {
  // Configura o LED como saída
  DDRB_REG |= (1 << DDB5);
  // Inicializa o Serial para depuração
  Serial.begin(9600);
}

void loop() {
  // Configura o pino A0 como entrada
  ADMUX_REG = (1 << REFS0) | A0_PIN;
  
  // Ativa o ADC, configura a taxa de amostragem e inicia a conversão
  ADCSRA_REG = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0) | (1 << ADSC);
  
  // Aguarda a conversão ser concluída
  while (ADCSRA_REG & (1 << ADSC));

  // Combina os registros ADCL e ADCH para obter o valor de 10 bits
  uint16_t adcValue = (ADCH_REG << 8) | ADCL_REG;

  // Converte o valor ADC em temperatura em graus Celsius
  float temperaturaC = (adcValue * 5.0 / 1023.0 - 0.5) * 100.0;

  // Imprime a temperatura no monitor serial
  Serial.print("Temperatura: ");
  Serial.print(temperaturaC);
  Serial.println(" graus Celsius");

  // Define um limite de temperatura para acionar o LED
  float limiteTemperatura = 25.0;

  // Verifica se a temperatura ultrapassa o limite
  if (temperaturaC > limiteTemperatura) {
    // Acende o LED
    PORTB_REG |= (1 << PORTB5);
  } else {
    // Desliga o LED
    PORTB_REG &= ~(1 << PORTB5);
  }

  delay(1000);  // Aguarda 1 segundo antes de realizar a próxima leitura
}

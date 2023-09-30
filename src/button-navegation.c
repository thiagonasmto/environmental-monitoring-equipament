int main(void) {
    DDRD = 0b00011100;   // Configura ledPin (pino 4) como saída
    PORTD = 0b11100000;  // Configura o pino 3 como Pull-up 
  
    while (1) {
        if (PIND & 0b00100000) { // PIND - Registrador de leitura do estado do Port D
            PORTD = 0b11100000;  // Desliga o LED
        } else {
            PORTD = 0b11110000;  // Liga o LED
        }
      	if (PIND & 0b01000000) { // PIND - Registrador de leitura do estado do Port D
            PORTD = 0b11100000;  // Desliga o LED
        } else {
            PORTD = 0b11101000;  // Liga o LED
        }
      	if (PIND & 0b10000000) { // PIND - Registrador de leitura do estado do Port D
            PORTD = 0b11100000;  // Desliga o LED
        } else {
            PORTD = 0b11100100;  // Liga o LED
        }
    }
    return 0;
}
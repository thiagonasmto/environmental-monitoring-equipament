//================================================================================ //
// ACIONANDO UM DISPLAY DE CRISTAL LIQUIDO DE 16x2 //
// Interface de dados de 4 bits //
//================================================================================ //
#include "def_principais.h" //inclusão do arquivo com as principais definições
#include "LCD.h"
// definição para acessar a memória flash
prog_char mensagem[] = " DADOS DE 4BITS!\0"; // mensagem armazenada na memória flash
//-----------------------------------------------------------------------------------
int main(){
    DDRD = 0xFF;                  // PORTD como saída
    DDRB = 0xFF;                  // PORTB como saída
    inic_LCD_4bits();             // inicializa o LCD
    escreve_LCD(" INTERFACE DE"); // string armazenada na RAM
    cmd_LCD(0xC0, 0);             // desloca cursor para a segunda linha
    escreve_LCD_Flash(mensagem);  // string armazenada na flash
    for (;;){
    } // laço infinito, aqui vai o código principal
}
//===================================================================================
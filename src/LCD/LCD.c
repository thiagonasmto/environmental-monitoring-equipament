#include <avr/io.h>
#include <util/delay.h>

#define RS PH3
#define E PH5
#define DADOS_LCD PORTA
#define nibble_dados 1

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

    PORTH &= ~(1 << RS);
    PORTH &= ~(1 << E);

    _delay_ms(20);

    if (nibble_dados) {
        DADOS_LCD = (DADOS_LCD & 0x0F) | 0x20;
    } else {
        DADOS_LCD = (DADOS_LCD & 0xF0) | 0x02;
    }

    pulse_E();
    cmd_LCD(0x28, 0);
    cmd_LCD(0x08, 0);
    cmd_LCD(0x01, 0);
    cmd_LCD(0x0C, 0);
    cmd_LCD(0x80, 0);
}

void show_LCD(const char *str) {
    while (*str) {
        cmd_LCD(*str++, 1);
    }
}

int main() {
    start();
    show_LCD(" INTERFACE DE");
    cmd_LCD(0xC0, 0);

    while (1);

    return 0;
}

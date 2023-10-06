# Equipamento de Monitoriamento Ambiental

Com o objetivo do trabalho em mente, um código em C criado foi criado para controlar um display LCD usando um microcontrolador AVR. O código está configurado para utilizar um microcontrolador com portas de saída e temporização para controlar o display LCD.

## 🤖 Descrição do [código](src/LCD/LCD.c):
>1. Inclusão de bibliotecas
>2. Definição de constantes
>3. **Função pulse_E()** com a função de criar um pulso no pino E (Enable) para sincronizar os dados enviados ao LCD.
>4. **Função cmd_LCD(unsigned char c, char cd)** para enviar comandos ou dados ao LCD.
>    - 'c' é o byte de comando ou dado a ser enviado.
>    - 'cd' é um indicador que determina se você está enviando um comando (0) ou um dado (1) para o LCD.
>    - A função configura os pinos RS de acordo com 'cd' e, em seguida, envia os dados em formato de 4 bits ou 8 bits, dependendo da configuração de 'nibble_dados'.
>5. **Função start()** usada para configurar as portas necessárias como saídas e inicializa o LCD e inicializar o LCD com os comandos necessários, como a seleção do modo de 4 bits e outras configurações iniciais.
>6. <b>Função show_LCD (const char *str)</b> usada para exibição das sequências de caracteres no LCD.
>    - Ela chama cmd_LCD() para enviar cada caractere da string para o LCD até que o final da string seja alcançado.
>7. **Função main()** onde o programa executa as funções conforme a necessidade e entra em um loop infinito com 'while (1);', mantendo o LCD exibindo a mensagem.

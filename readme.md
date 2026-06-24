# Sobre

Assembler para o conjunto de instruções hipotético usado na disciplina de Arquitetura de Computadores do IFPR-Paranavaí.

# Compilando

Para compilar, apenas use o **make**.

# Running

O binário do compilador é o **pasm**.

# Sobre o conjunto de instruções

A especificação do conjunto de instruções pode ser consultada na aula que ministro, [neste link](https://drive.google.com/file/d/1Nmx3BCSIip_pgfKTOJiPLcYIl5CFdgP6/view?usp=drive_link).

Importante apenas ressaltar os seguintes pontos:

- 8 registradores de propósito geral

- Os registradores são de 16 bits

- Cada palavra (word) da memória também tem 16 bits, de forma que o endereço 0 (zero) referencia os bytes 0 e 1, o endereço 1 referencia os bytes 2 e 3, e assim por diante.

# Sobre o código gerado

O espaço de endereçamento de memória gerado inicia no endereço **0 (zero)** e vai até **n - 1**, sendo **n** o tamanho do binário em words.

O linker irá buscar pelo símbolo **_start**, que deverá referenciar a instrução inicial do código.

O início do código gerado ficará da seguinte forma:

- Endereço 0 (zero): conterá o valor 0 (zero)

- Endereço 1: conterá uma instrução jump para o símbolo **_start**

Dessa forma, um simulador para tal arquitetura deverá setar como endereço inicial do registrador **pc** o endereço **1**.
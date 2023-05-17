# Sobre

Assembler para o conjunto de instruções hipotético usado na disciplina de Arquitetura de Computadores do IFPR-Paranavaí.

# Compilando

Para compilar, apenas use o **make**.

# Running

O binário do compilador é o **pasm**.

# Sobre o conjunto de instruções

A especificação do conjunto de instruções pode ser consultada na aula que ministro, em **docs/12-instrucoes-codificacao.pdf**.

Importante apenas ressaltar os seguintes pontos:

- 8 registradores de propósito geral

- Os registradores são de 16 bits

- Cada palavra da memória também tem 16 bits, de forma que o endereço 0 (zero) referencia os bytes 0 e 1, o endereço 1 referencia os bytes 2 e 3, e assim por diante.

# Sobre o código gerado

O linker irá buscar pelo símbolo **_start**, que deverá referenciar a instrução inicial do código.

O inicial do código gerado ficará da seguinte forma:

- Endereço 0 (zero): conterá o valor 0 (zero)

- Endereço 1: conterá uma instrução jump para o símbolo **_start**

Dessa forma, um simulador para tal arquitetura deverá setar como endereço inicial do registrador **pc** o endereço **1**.
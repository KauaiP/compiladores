# Compilador COOL ➔ BRIL

![C](https://img.shields.io/badge/Language-C-00599C?style=for-the-badge&logo=c)
![COOL](https://img.shields.io/badge/Source-COOL-green?style=for-the-badge)
![BRIL](https://img.shields.io/badge/Target-BRIL-red?style=for-the-badge)
![Status](https://img.shields.io/badge/Status-Em_Desenvolvimento-yellow?style=for-the-badge)

Este projeto consiste no desenvolvimento de um compilador para a linguagem **COOL** (*Classroom Object Oriented Language*), escrito em **C**, com destino à representação intermediária **BRIL** (*Big Red Intermediate Language*).

Este trabalho faz parte da disciplina de **Compiladores** na **Universidade Federal Fluminense (UFF)**, campus Rio das Ostras - Ciência da Computação.

---

## Sobre o Projeto

O objetivo é transformar código de alto nível com suporte a objetos (COOL) em uma representação intermediária baseada em JSON (BRIL), permitindo a execução e otimização do código original.

### Arquitetura do Compilador
Clique nos tópicos para ver os detalhes da implementação:

1. [Análise Léxica](#1-análise-léxica)
2. [Análise Sintática](#2-análise-sintática)
3. [Análise Semântica](#3-análise-semântica)
4. [Geração de Código](#4-geração-de-código)

---

## Especificações Técnicas

### 1. Análise Léxica
Nesta etapa, o compilador lê o fluxo de caracteres do arquivo `.cl` e os agrupa em **Tokens**.
* **Ferramenta:** Flex (ou implementação manual em C).
* **Destaque:** Tratamento de strings complexas, comentários aninhados e identificadores (Type vs Object identifiers).

### 2. Análise Sintática
Os tokens são organizados em uma **Árvore Sintática Abstrata (AST)**, validando se a estrutura do código respeita a gramática da linguagem COOL.
* **Ferramenta:** Bison (ou Recursive Descent Parser).
* **Destaque:** Definição da precedência de operadores e estrutura de classes/herança.

### 3. Análise Semântica
A fase mais crítica no COOL. Aqui verificamos a validade lógica do programa.
* **Escopo:** Verificação de variáveis e métodos.
* **Sistema de Tipos:** Verificação de conformidade de tipos e o cálculo do *Least Upper Bound* (LUB) para condicionais.

### 4. Geração de Código
Tradução da AST para o formato **BRIL**.
* **Mapeamento:** Conversão de métodos de classes COOL para funções BRIL.
* **Saída:** Geração de um arquivo JSON compatível com o interpretador `brili`.

---

## Como Executar

```bash
# Clone o repositório
git clone [https://github.com/seu-usuario/seu-repositorio.git](https://github.com/seu-usuario/seu-repositorio.git)

# Entre na pasta
cd seu-repositorio

# Compile o projeto
make
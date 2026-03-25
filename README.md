# Compilador COOL ➔ BRIL

![C](https://img.shields.io/badge/Language-C-00599C?style=for-the-badge&logo=c)
![COOL](https://img.shields.io/badge/Source-COOL-green?style=for-the-badge)
![BRIL](https://img.shields.io/badge/Target-BRIL-red?style=for-the-badge)
![Status](https://img.shields.io/badge/Status-Em_Desenvolvimento-yellow?style=for-the-badge)

Este projeto consiste no desenvolvimento de um compilador para a linguagem **COOL** (*Classroom Object Oriented Language*), escrito em **C**, com destino à representação intermediária **BRIL** (*Big Red Intermediate Language*).

Este trabalho faz parte da disciplina de **Compiladores** na **Universidade Federal Fluminense (UFF)**, campus Rio das Ostras - Instituto de Ciência e Tecnologia (**ICT**)
Curso: *Ciência da Computação*

---

## Sobre o Projeto

O objetivo é transformar código de alto nível com suporte a objetos (COOL) em uma representação intermediária (BRIL), permitindo a execução e otimização do código original.

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
* **Ferramenta:** Implementação manual em C.
* **Destaque:** Tratamento de strings complexas, comentários aninhados e identificadores (Type vs Object identifiers).
* **Implementação:** *Texto a desenvolver*

### 2. Análise Sintática
Ainda será desenvolvido...

### 3. Análise Semântica
Ainda será desenvolvido...

### 4. Geração de Código
Ainda será desenvolvido...

---

## 👥 Autores

* **Kauai Pereira** - [GitHub](https://github.com/KauaiP)
* **Davi Fontes** - [GitHub](https://github.com/daviifm)

## Como Executar

```bash
# Clone o repositório
git clone [https://github.com/seu-usuario/seu-repositorio.git](https://github.com/seu-usuario/seu-repositorio.git)

# Entre na pasta
cd seu-repositorio

# Compile o projeto
make
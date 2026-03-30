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

## 👥 Autores

* **Kauai Pereira** - [GitHub](https://github.com/KauaiP)
* **Davi Fontes** - [GitHub](https://github.com/daviifm)
---

### Arquitetura do Compilador
Clique nos tópicos para ver os detalhes da implementação:

1. [Análise Léxica](#1-análise-léxica)
2. [Análise Sintática](#2-análise-sintática)
3. [Análise Semântica](#3-análise-semântica)
4. [Geração de Código](#4-geração-de-código)

---

## Especificações Técnicas

### 1. Análise Léxica
Nesta etapa, o compilador lê o fluxo de caracteres de um arquivo `.txt` que contém um código na linguagem **Cool** e os agrupa em **Tokens**.
* **Método escolhido:** Implementação manual em C.
* **Destaque:** Essa etapa, embora uma das mais simples, requer alguns conhecimentos específicos de **C**. Estamos falando de algumas funções e mecanismos não muito conhecidos mas que são interessantes para a construção de um analisador léxico. Vamos então destacar algumas dessas ferramentas que utilizamos para a análise léxica do compilador:

    - *strdup()*
        Essa é uma função muito interessante em **C** pois ela é uma das poucas funções dessa linguagem que fazem processos automatizados para o programador. Ao chamar *strdup()* ela automaticamente:
            1. Aloca memória dinamicamente (usa **malloc**) para o novo pedaço de texto.
            2. Copia exatamente **len** caracteres da origem para essa nova memória.
            3. Adiciona o caractere nulo (\0) ao final da nova string para que ela seja válida.
        ```c
        int len = 3
        char *nova_string = strdup("Olá mundo" + 5, len);
        ```
        *nova_string* = "mun"
        **OBS**:Lembrando que deve ser feito um **free(nova_string)** já que **strdup()** alocou o seu vetor dinâmicamente!!
        **OBS**: A função *strndup()* faz parte da extensão POSIX (Linux/macOS). Se você estiver compilando no Windows com um compilador muito rígido ou antigo, ele pode não reconhecer essa função.
--
        
    - *struct anônima*
        Em C, uma struct anônima é uma estrutura definida sem um nome de identificação (o "tag name" da struct). Elas são usadas principalmente dentro de outras estruturas ou uniões para organizar dados sem poluir o namespace do código com nomes de tipos que só fazem sentido naquele contexto específico.
        O objetivo aqui não é antecipar por agora informações detalhadas do código (essa parte é detinada a sessão de **implementação** logo adiante). Mas, para exemplificar o uso de struct anômima, vale analisar um trecho do arquivo lexer.c:
        ```c
        static struct { 
            const char *word; 
            TokenType type; 
        } KEYWORDS[] = {
            {"class",    CLASS},    {"else",   ELSE},
            {"fi",       FI},       {"if",     IF},
            {"in",       IN},       {"inherits",INHERITS},
            {"isvoid",   ISVOID},   {"let",    LET},
            {"loop",     LOOP},     {"pool",   POOL},
            {"then",     THEN},     {"while",  WHILE},
            {"case",     CASE},     {"esac",   ESAC},
            {"new",      NEW},      {"of",     OF},
            {"not",      NOT},
        };
        ```
        Essa *struct* está no meio do arquivo, sem definição de nome e com uma lista da prórpria struct (**KEYWORDS[]**) defnida logo em seguida. Parece que isso nem é C puramente, mas na verdade é. Struct anônima ajudou nosso código a ficar mais limpo e organizado, embora a sintaxe assuste um pouco. 
        Struct anônima tem outras diversas funcionalidades e usabilidades nos programas em **C**. Para fins de pesquisa e estudo recomendamos uma leitura mais aprofundada nas documentações de **C**.

---

* **Implementação:** A ideia principal do Analisador Léxico (e de todos os outros "componentes" do compilador) é de criarmos bibliotecas internas à pasta do programa (as famosas .h). Nesta etapa desenvolvemos o arquivo **lexer.h** e implementamos suas funções (e mais uma infinidade de outras funções auxiliares) no arquivo **lexer.c**.
A função principal de um Analisador Léxico é separar todo o arquivo .txt em **Tokens**. Tokens, para o nosso caso, se resumem em **{;}** **{.}** **{+}** **{-}** **{"...*string_literal*..."}** **{*keywords*}**... e muitos outros tokens da linguagem **Cool**. Por esse motivo vamos começar analisando a enumeração que fizemos no **lexer.h** com todos os tipos de **Token** em **Cool**:
```c
typedef enum {
    // Constants:
    INT, 
    STR, 
    BOOL,

    // Keywords:
    CLASS,
    ELSE,
    FALSE,
    FI,
    IF,
    IN,
    INHERITS,
    ISVOID,
    LET,
    LOOP,
    POOL,
    THEN,
    WHILE,
    CASE,
    ESAC,
    NEW,
    OF,
    NOT,
    TRUE,

    // Identifiers:
    TYPE_ID, //começam com maiúscula
    OBJECT_ID, // começam com minúscula

    // Precedence
    ASSIGN,         // <-
    LE,             // <=
    DARROW,         // =>
    LT,             // <
    EQ,             // =
    PLUS,           // +
    MINUS,          // -
    STAR,           // *
    SLASH,          // /
    DOT,            // .
    AT, // @
    TILDE,          // ~

    //literal de String
    STR_LIT,

    LBRACE,         // {
    RBRACE,         // }
    LPAREN,         // (
    RPAREN,         // )
    COLON,          // :
    SEMICOLON,      // ;
    COMMA,          // ,

    // Para controle do código:
    TOKEN_EOF,
    ERROR

}TokenType;
```
Toda essa seleção de Token foi definida através da documentação específica da sintaxe de **Cool**.
Depois de definir e classificar os Tokens estrturamos nosso projeto da seguinte forma:
 - Temos uma estrtuta do analisador léxico o qual é responsável por **armazenar todo o conteúdo do arquivo .txt**, a **linha a qual o analisador está analisando** e a **posição (o caracter) que ele está analisando**.
 - Temos uma estrtura do Token o qual é responsável por armazenar o **tipo do Token (um dos elementos da enumeração *TokenType* que definimos)**, o **valor to Token ("class" ou ";" ou "." ou "inherits"...)** e a **linha a qual o Token está**.
 - Com as estrtuturas definidas, nosso programa irá se utilizar do analisador léxico para iterar em todo o arquivo .txt e montar tokens através de diversas funções auxiliares.

 Aqui está a definição da estrtutura **Token** no *lexer.h*, assim como a definição do escopo da estrtutura **Lexer**. No código em seguida está a definição da estrtura **Lexer** no arquivo *lexer.c*
 *lexer.h*
 ```c
 typedef struct token
{
    TokenType type;
    char* value;
    int line;
}Token;

typedef struct lexer Lexer;
```
*lexer.c*
```c
typedef struct lexer{
    const char *src;
    int line;
    int pos;
}Lexer;
```
Em uma analogia não muito boa, a estrutura **Lexer** é uma *Máquina de Turing* a qual vai guardar todo o arquivo.txt em uma variável denominada *src* e se utilizará das variáveis *line* e *pos* como auxiliadores para iterar sobre *src*.
Com essas estruturas principais definidas, o que temos no resto de *lexer.h* são definições de escopos de funções necessárias para a criação e deleção das mesmas (*next_token()*, *free_token()*, *new_lexer()*, *free_lexer()*).

**OBS**:Daqui em diante explicaremos o toda a engrenagem por trás do nosso Analisador Léxico. Não entraremos detalhadamente em algumas funções pois o código está disponível para acesso aqui neste repositório.
--
No arquivo **lexer.c**, além de implementarmos as funções *next_token()*, *free_token()*, *new_lexer()*, *free_lexer()* nós definimos algumas funções clássicas da etapa de análise léxica. São elas:
 - *peek()*
    Essa função é responsável por "olhar" o caracter atual ao qual o **Lexer** está. Sua implementação consiste em:
    ```c
    static char peek(Lexer *l){

        return l->src[l->pos];
    }
    ```
    --
- *peek_next()*
   Semelhante a anterior, mas aqui estamos olhando para próximo caracter na ordem de análise do **Lexer**. Sua implementação consiste em:
   ```c
   static char peek_next(Lexer *l){ 

        return l->src[l->pos + 1];
    }
    ```
    --
- *advanced()*
   Essa função é para que a string src do **Lexer** seja avançada para o próximo caracter, fazemos isto iterando na variável *pos* da estrtutura **Lexer**.
   ```c
   static void advanced(Lexer *l){

        l->pos++;
        char c = l->src[l->pos];
        if (c == '\n')
        {
            l->line++;
        }
    }
    ```
    Além de avançar para o próximo caracter da sequência de src essa função verifica se o caracter, depois de avançado, é uma quebra de linha. Caso isso seja verdadeiro, temos uma adição na variável line da estrutura do **Lexer**.
    --
- *at_end()*
   Esta função cumpre um papel importante, sendo fundamental e a mais usada no resto do código. Embora sua implementação seja simples, ela é responsável por verificar se o caracter atual de src é '\0'. Isso significaria que estamos no final do arquivo.
   ```c
   static int at_end(Lexer *l){

    return l->src[l->pos] == '\0';
    }
    ```
   --
   *make_token()*
   Essa função não está definida no lexer.h e nem deve estar. O papel de *make_token()* é de criar um novo token para que esse seja retornado ao programa principal. São passados a ele todos os parâmetros necessários para se definir uma estrtutura **Token**.
   ```c
   static Token make_token(TokenType type, char *value, int line){

    Token t;
    t.type = type;
    t.value = value;
    t.line = line;
    return t;
    }
    ``` 
    --
 - *token_error()*
    Já essa função fica responsável por montar o chamamos nesse projeto de **erro de token**. Há diversos cenários em que podem haver erros de token. Um bom exemplo de erro de token acontece em blocos de comentários ("(*....*)") quando falta um desses caracteres que definem o bloco.
    ```c
    static Token token_error(const char *msg, int line){

    return make_token(ERROR, strdup(msg), line);
    }
    ```
    --

    O resto das funções do **lexer.c** se utilizam dessas funções especificadas para gerarem tokens e retorná-los ao programa principal. Elas estão comentadas em código.

### 2. Análise Sintática
Ainda será desenvolvido...

### 3. Análise Semântica
Ainda será desenvolvido...

### 4. Geração de Código
Ainda será desenvolvido...

---


## Como Executar

```bash
# Clone o repositório
git clone [https://github.com/seu-usuario/seu-repositorio.git](https://github.com/seu-usuario/seu-repositorio.git)

# Entre na pasta
cd seu-repositorio

# Compile o projeto
make
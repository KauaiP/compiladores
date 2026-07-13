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
---    

### 2. Análise Sintática
Nesta etapa, o compilador recebe o fluxo de **Tokens** gerado pelo Analisador Léxico e verifica se eles formam estruturas válidas na gramática de **Cool**, construindo uma **Árvore Sintática Abstrata (AST)** que representa o programa de forma hierárquica.

* **Método escolhido:** Recursive Descent Parser para estruturas da linguagem + Pratt Parser para expressões.

* **Destaque:** Essa etapa exigiu decisões de design mais complexas do que a léxica. Vamos destacar as ferramentas e conceitos de **C** que tornaram essa implementação possível:

    - *Union de structs*
        A AST precisava representar dezenas de tipos de nó diferentes — cada um com campos distintos. A solução em **C** foi utilizar uma `union` de `structs` anônimas dentro da struct principal do nó. Isso permite que cada tipo de nó tenha exatamente os campos que precisa, sem desperdiçar memória:
        ```c
        struct ASTNode {
            NodeKind kind;
            int line;
            union {
                struct { NodeList *classes; } program;
                struct { char *name; char *parent; NodeList *features; } class_;
                struct { ASTNode *expr_cond; ASTNode *expr_then; ASTNode *expr_else_; } if_;
                struct { ASTNode *left_expr; ASTNode *right_expr; } binary;
                // ...
            } data;
        };
        ```
        O campo `kind` indica qual variante da union está ativa. Acessar a variante errada é **undefined behavior** em C — o compilador não te protege disso, então é responsabilidade do programador garantir que `node->data.if_` só seja acessado quando `node->kind == NODE_IF`.

    - *Lista encadeada de nós (NodeList)*
        Estruturas como classes, métodos e blocos têm um número variável de filhos — não é possível usar arrays de tamanho fixo. A solução foi uma lista encadeada simples:
        ```c
        typedef struct NodeList {
            ASTNode *node;
            struct NodeList *next;
        } NodeList;
        ```
        Toda coleção de nós na AST — features de uma classe, argumentos de um dispatch, expressões de um bloco — é representada por uma `NodeList`. Isso simplifica muito a travessia recursiva nas etapas seguintes.

    - *Struct opaca (forward declaration)*
        Seguindo o mesmo padrão do Analisador Léxico, a struct do Parser é declarada como opaca no `.h` e definida apenas no `.c`:
        ```c
        // parser.h — interface pública
        typedef struct parser Parser;

        // parser.c — detalhe de implementação
        struct parser {
            Lexer *lexer;
            Token current;
            Token next;
        };
        ```
        Isso garante encapsulamento — quem inclui `parser.h` sabe o que o parser faz, mas não como ele funciona internamente.

    - *Shallow copy de Token e o problema do dangling pointer*
        A função `expect()` precisava retornar o token consumido para que o parser pudesse usar seu valor. A implementação ingênua copiava a struct por valor:
        ```c
        Token t = p->current;
        advance(p); // faz free do value de p->current
        return t;   // t.value aponta para memória liberada!
        ```
        Como a cópia é **shallow**, `t.value` e `p->current.value` apontam para o mesmo endereço. Quando `advance()` libera `p->current`, `t.value` vira um **dangling pointer**. A correção foi fazer `strdup` do value antes de avançar:
        ```c
        Token t = p->current;
        t.value = t.value ? strdup(t.value) : NULL;
        advance(p);
        return t;
        ```

* **Implementação:** O parser foi dividido em dois mecanismos complementares que trabalham juntos:

    O **Recursive Descent** cuida das estruturas fixas da linguagem — programa, classes, features e formals. Para cada regra gramatical há uma função dedicada: `parse_class()`, `parse_feature()`, `parse_formal()`. Essas funções usam `expect()` para consumir tokens obrigatórios e constroem os nós da AST com as construtoras definidas no `ast.c`.

    O **Pratt Parser** cuida das expressões, onde a precedência de operadores é o grande desafio. A função central é:
    ```c
    static ASTNode *parse_expression(Parser *p, int min_bp);
    ```
    Cada operador tem um **binding power** (bp) que representa sua precedência. O algoritmo é iterativo — chama `parse_nud()` para obter o lado esquerdo e entra num loop que consome operadores enquanto o bp do operador atual for maior que `min_bp`. Isso resolve naturalmente a recursão à esquerda que destruiria um parser descendente recursivo ingênuo.

    A tabela de precedências ficou assim:
    ```c
    static int binding_power(TokenType type) {
        switch (type) {
            case AT: case DOT:   return 80;
            case TILDE:          return 70;
            case ISVOID:         return 60;
            case STAR: case SLASH: return 50;
            case PLUS: case MINUS: return 40;
            case LT: case LE: case EQ: return 30;
            case NOT:            return 20;
            case ASSIGN:         return 10;
            default:             return 0;
        }
    }
    ```

---

### 3. Análise Semântica
Nesta etapa, o compilador percorre a AST construída pelo Analisador Sintático e verifica se o programa faz **sentido semântico** — tipos corretos, variáveis declaradas, herança válida, métodos existentes. Erros como atribuir uma `String` a uma variável `Int` ou chamar um método inexistente são detectados aqui.

* **Método escolhido:** Travessia recursiva da AST com duas estruturas auxiliares: `SymTable` (tabela de símbolos) e `ClassEnv` (ambiente de classes).

* **Destaque:** Essa foi a etapa mais rica em problemas sutis de gerenciamento de memória em **C**. Os destaques técnicos são:

    - *Tabela de símbolos com escopos encadeados*
        A `SymTable` é implementada como uma **pilha de escopos**. Cada escopo é uma lista encadeada de entradas (nome → tipo). Entrar em um escopo empilha um novo `Scope`; sair desempilha e libera:
        ```c
        typedef struct Scope {
            SymbolEntry *entries;
            struct Scope *parent;
        } Scope;
        ```
        A busca por uma variável sobe pelos escopos via o ponteiro `parent` até encontrar ou chegar em `NULL`. Isso implementa naturalmente a regra de que uma variável de escopo interno oculta uma de mesmo nome no escopo externo.

    - *O problema do ponteiro liberado e a solução com string interning*
        O bug mais insidioso desta etapa surgiu da interação entre a `SymTable` e o retorno de tipos em `check_expr`. Quando `symtable_exit_scope()` era chamado, ele liberava as strings de tipo das entradas do escopo. Porém, `check_expr` retornava ponteiros diretos para essas strings — que viravam lixo de memória assim que o escopo fechava.

        A solução foi implementar **string interning**: uma tabela global de strings únicas que vivem durante todo o programa:
        ```c
        const char *intern(const char *s) {
            for (int i = 0; i < count; i++)
                if (strcmp(table[i], s) == 0)
                    return table[i]; // ponteiro estável, sempre válido
            table[count] = strdup(s);
            return table[count++];
        }
        ```
        Com isso, todo retorno de tipo em `check_expr` passa pelo `intern()`. Não importa quando escopos são abertos ou fechados — os ponteiros de tipo nunca ficam inválidos porque apontam para a tabela de internação, liberada apenas no final do `main()`.

    - *Detecção de ciclos na hierarquia de herança*
        O COOL permite herança, mas ciclos como `class A inherits B` + `class B inherits A` são inválidos e precisam ser detectados. A solução usou um limite baseado no total de classes:
        ```c
        int steps = 0, total = /* total de classes */;
        while (current != NULL) {
            if (entry->parent == NULL) break; // chegou em Object — sem ciclo
            current = entry->parent;
            steps++;
            if (steps > total) return 1; // ciclo detectado
        }
        ```
        Se você subiu mais vezes do que o número total de classes sem chegar em `Object`, é matematicamente impossível que a hierarquia seja acíclica.

    - *LUB — Least Upper Bound*
        O tipo de um `if/then/else` em COOL é o ancestral comum mais próximo dos tipos dos dois ramos. Isso requer calcular o **LUB** de dois tipos na hierarquia de herança. O algoritmo coleta todos os ancestrais do primeiro tipo e sobe a cadeia do segundo até encontrar o primeiro ancestral em comum.

* **Implementação:** O analisador semântico opera em duas passagens sobre a AST:

    Na **primeira passagem**, `collect_classes()` percorre todos os `NODE_CLASS` do programa e registra cada classe no `ClassEnv` com seu nome e pai. Depois `collect_features()` percorre novamente e registra os métodos e atributos de cada classe.

    Na **segunda passagem**, `check_class()` percorre cada classe e chama `check_expr()` no corpo de cada método. A função `check_expr()` é o coração do analisador — um grande `switch` no `kind` do nó que verifica tipos e retorna o tipo resultante da expressão. Por exemplo, para um `NODE_PLUS` ela verifica se os dois operandos são `Int` e retorna `"Int"`. Para um `NODE_IF` ela calcula o LUB dos tipos dos dois ramos.

    Antes de qualquer passagem, os tipos primitivos de COOL são registrados manualmente no `ClassEnv`:
    ```c
    classenv_add_class(env, "Object", NULL);
    classenv_add_class(env, "Int",    "Object");
    classenv_add_class(env, "Bool",   "Object");
    classenv_add_class(env, "String", "Object");
    classenv_add_class(env, "IO",     "Object");
    ```
    Sem isso, qualquer classe sem `inherits` explícito teria seu pai (`Object`) não encontrado no ambiente, gerando falsos positivos de ciclo de herança.

---

### 4. Geração de Código
Nesta etapa, o compilador percorre a AST validada e gera código na linguagem intermediária **BRIL** (Big Red Intermediate Language). O BRIL é uma linguagem de representação de programas desenvolvida para fins educacionais, onde um programa é uma lista de funções e cada função é uma lista de instruções sobre variáveis temporárias.

* **Método escolhido:** Travessia recursiva da AST com geração de código no formato texto `.bril`, convertido para JSON com a ferramenta `bril2json` e executado com `brili`.

* **Destaque:** Essa etapa exigiu decisões de mapeamento entre dois mundos muito diferentes — o COOL orientado a objetos e o BRIL procedural. Os destaques técnicos são:

    - *Temporários e labels com contadores*
        O BRIL não tem expressões compostas — cada operação intermediária precisa de uma variável própria. Para gerar nomes únicos automaticamente, o gerador mantém dois contadores:
        ```c
        static char *new_tmp(CodeGen *cg) {
            char *name = malloc(32);
            snprintf(name, 32, "tmp%d", cg->tmp_count++);
            return name;
        }

        static char *new_label(CodeGen *cg) {
            char *name = malloc(32);
            snprintf(name, 32, "lbl%d", cg->label_count++);
            return name;
        }
        ```
        Cada chamada garante um nome único. O resultado de `1 + 2 * 3` vira:
        ```
        tmp0: int = const 2;
        tmp1: int = const 3;
        tmp2: int = mul tmp0 tmp1;
        tmp3: int = const 1;
        tmp4: int = add tmp3 tmp2;
        ```

    - *O problema da geração de argumentos dentro de chamadas*
        Um bug sutil surgiu ao gerar chamadas de função com argumentos. A abordagem ingênua emitia os argumentos dentro do `fprintf` da instrução `call`:
        ```c
        fprintf(out, "  %s: int = call @%s_%s", dest, class, method);
        // ERRADO: emit_expr dos args escreve novas instruções no meio da linha!
        emit_expr(arg1); // escreve "tmp5: int = const 7;\n" no arquivo
        ```
        O resultado era instruções quebradas no meio da linha do `call`. A solução foi avaliar **todos os argumentos antes** de emitir a instrução `call`, guardando os nomes dos temporários num array:
        ```c
        char *argnames[64];
        int argc = 0;
        while (args != NULL) {
            argnames[argc++] = emit_expr(cg, args->node); // avalia tudo primeiro
            args = args->next;
        }
        fprintf(out, "  %s: int = call @%s_%s", dest, class, method);
        for (int i = 0; i < argc; i++)
            fprintf(out, " %s", argnames[i]); // só então emite os nomes
        fprintf(out, ";\n");
        ```

    - *Inferência de tipos para dispatch dinâmico*
        O BRIL não tem orientação a objetos — cada método COOL vira uma função BRIL com convenção de nome `ClassName_methodName`. Para gerar a chamada correta de um dispatch como `c.soma(1, 2)`, o gerador precisa saber que `c` é do tipo `Calculadora` para gerar `@Calculadora_soma` e não `@Object_soma`.

        Isso requer duas funções complementares. `infer_type()` consulta a AST para descobrir o tipo de um nó **sem emitir código**. `track_var()` registra o tipo de cada variável declarada em `let` para que `infer_type()` possa consultá-la depois via `lookup_var_type()`:
        ```c
        static const char *infer_type(CodeGen *cg, ASTNode *node) {
            switch (node->kind) {
                case NODE_INT:  return "Int";
                case NODE_NEW:  return node->data.new_.type; // tipo exato
                case NODE_ID:   return lookup_var_type(cg, node->data.id.name);
                default:        return "Object";
            }
        }
        ```

    - *Convenção de `self` como ponteiro opaco*
        O BRIL não tem objetos nativos. A solução adotada foi representar `self` como um `int` opaco — uma simplificação válida para o escopo do projeto. Todo método gerado recebe `self: int` como primeiro parâmetro, e o `@main` instancia `Main` com `self = 0`:
        ```c
        fprintf(out, "@main {\n");
        fprintf(out, "  self: int = const 0;\n");
        fprintf(out, "  result: int = call @%s_main self;\n", main_class);
        fprintf(out, "  print result;\n");
        fprintf(out, "}\n");
        ```

* **Implementação:** O gerador opera em uma única passagem sobre a AST, produzindo o arquivo `.bril` diretamente:

    `codegen_run()` é o ponto de entrada. Ele primeiro varre todas as classes para descobrir qual delas contém o método `main` — necessário para gerar o `@main` correto independente do nome da classe. Depois chama `emit_class()` para cada classe.

    `emit_class()` percorre as features de uma classe e chama `emit_method()` para cada método encontrado.

    `emit_method()` escreve o cabeçalho da função BRIL com todos os parâmetros formais tipados, chama `emit_expr()` no corpo do método, e fecha com `ret`.

    `emit_expr()` é o coração do gerador — um `switch` no `kind` do nó que emite as instruções BRIL correspondentes e retorna o nome do temporário que contém o resultado. Cada nó da AST tem um mapeamento direto:

    | Nó COOL | Instrução BRIL gerada |
    |---|---|
    | `NODE_INT(42)` | `tmpN: int = const 42;` |
    | `NODE_PLUS(a, b)` | `tmpN: int = add tmpA tmpB;` |
    | `NODE_IF` | `br cond .then .else` + labels |
    | `NODE_WHILE` | label de loop + `br` + `jmp` de volta |
    | `NODE_SELF_DISPATCH` | `tmpN: int = call @Classe_metodo self args...` |
    | `NODE_DISPATCH` | `tmpN: int = call @TipoObjeto_metodo obj args...` |


---


## Como Executar

```bash
# Clone o repositório
git clone [https://github.com/seu-usuario/seu-repositorio.git](https://github.com/seu-usuario/seu-repositorio.git)

# Entre na pasta
cd seu-repositorio

# Compile o projeto
# 1. compila o seu compilador
gcc -g *.c -o coolc

# 2. gera o .bril a partir do .cool
./coolc teste_if.cool > teste_if.bril

# 3. converte o .bril para JSON
bril2json < teste_if.bril > teste_if.json

# 4. executa com o interpretador
brili < teste_if.json
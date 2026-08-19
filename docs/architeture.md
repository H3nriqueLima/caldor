# Architecture

Diferente dos arquivos de docs/spec/, que documentam a linguagem Caldor em si (sintaxe, semântica), esse arquivo documenta a ferramenta em C que implementa ela. Como cada peça é organizada e por quê. Cresce conforme cada peça do roadmap for sendo escrita, começando pelo front-end.

## ast.h / ast.c

### ast.h — contrato

Header, não tem lógica, só a forma. Três coisas:

1. Enum de kind (Um valor por tipo de nó que existe nessa fase. Escopo limitado ao item 1 do roadmap, não a spec inteira):

```c
typedef enum {
    // literais
    AST_LITERAL_INT,
    AST_LITERAL_FLOAT,
    AST_LITERAL_BOOL,
    AST_LITERAL_STRING,
 
    // variável
    AST_IDENTIFIER,
 
    // operações
    AST_BINARY_OP,
    AST_UNARY_OP,
    AST_ASSIGNMENT,
 
    // controle de fluxo
    AST_IF,
    AST_WHILE,
    AST_FOR,
    AST_BLOCK,
 
    // declaração
    AST_LET_DECL,
    AST_FN_DECL,

    // chamada e retorno
    AST_CALL,
    AST_RETURN,
    AST_BREAK,
    AST_CONTINUE,
} AstKind;
```

  `struct`/`enum`/`match`/genérico/array/referência ficam de fora, são item 3/4 do roadmap, não existem ainda nessa árvore. Os 4 literais são separados (não um `AST_LITERAL` genérico) porque cada um guarda um valor de C diferente por baixo. Juntar todos exigiria um campo a mais só pra saber qual dos quatro é, informação que o enum já dá de graça. `AST_IF` é um kind só, usado tanto em posição de statement quanto de expression, o nó não muda, só o contexto onde ele aparece na árvore (02-syntax.md já trata os dois usos como a mesma construção).

2. Struct do nó -> Campo `kind` + `line` (linha do código-fonte, para mensagem de erro) + um `union` que muda de formato dependendo do kind:
```c
typedef struct AstNode AstNode; // forward declaration: nó aponta para nó.
 
struct AstNode {
    AstKind kind;
    int line;
    union {
        long long int_value;
        double float_value;
        bool bool_value;
        char* string_value; // AST_LITERAL_STRING e AST_IDENTIFIER
 
        struct { AstNode* left; AstNode* right; BinOp op; } binary_op;
        struct { AstNode* operand; UnOp op; } unary_op;
        struct { AstNode* target; AstNode* value; } assignment;
 
        struct { AstNode* condition; AstNode* then_block; AstNode* else_block; } if_expr;
        struct { AstNode* condition; AstNode* body; } while_stmt;
        struct { AstNode* init; AstNode* condition; AstNode* increment; AstNode* body; } for_stmt;
        struct { LinkedList* statements; } block;
 
        struct { char* name; char* type_name; bool is_mut; AstNode* value; } let_decl;
        struct { char* name; LinkedList* params; char* return_type; AstNode* body; } fn_decl;
 
        struct { char* callee_name; LinkedList* args; } call;
        struct { AstNode* value; } return_stmt; // pode ser NULL
 
        // AST_BREAK e AST_CONTINUE não entram no union, o kind sozinho já basta.
    } as;
};
```

Campo `kind` é quem diz qual "gaveta" do `union` está válida. Ler `as.if_expr` num nó que na verdade é `AST_LITERAL_INT` é lixo de memória. Toda função que percorre `AstNode*` começa com `switch (node->kind)` por causa disso. Campos aparecem visíveis no header (diferente do `Environment` do interpretador, que ficou opaco) porque tem dois consumidores de verdade: parser escreve, interpretador lê.

Operador (`BinOp`/`UnOp`) é enum próprio da AST:
 
```c
typedef enum {
    OP_ADD, OP_SUB, OP_MUL, OP_DIV, OP_MOD, OP_POW,
    OP_EQ, OP_NEQ, OP_LT, OP_GT, OP_LTE, OP_GTE,
    OP_AND, OP_OR,
    OP_BIT_AND, OP_BIT_OR, OP_BIT_XOR, OP_SHL, OP_SHR,
} BinOp;
 
typedef enum {
    OP_NEG, OP_NOT, OP_BIT_NOT,
} UnOp;
```

Motivo (ver tabela de decisões abaixo): mantém `ast.h` sem depender de `token.h`, e evita a AST carregar token de pontuação/palavra reservada que nunca vira operador de nó nenhum.

3. Protótipo das construtoras -> Uma função por kind (`ast_new_...`), o parser chama em vez de montar a struct na mão:

```c
AstNode* ast_new_literal_int(int line, long long value);
AstNode* ast_new_literal_float(int line, double value);
AstNode* ast_new_literal_bool(int line, bool value);
AstNode* ast_new_literal_string(int line, const char* value);
AstNode* ast_new_identifier(int line, const char* name);
 
AstNode* ast_new_binary_op(int line, AstNode* left, BinOp op, AstNode* right);
AstNode* ast_new_unary_op(int line, UnOp op, AstNode* operand);
AstNode* ast_new_assignment(int line, AstNode* target, AstNode* value);
 
AstNode* ast_new_if(int line, AstNode* condition, AstNode* then_block, AstNode* else_block);
AstNode* ast_new_while(int line, AstNode* condition, AstNode* body);
AstNode* ast_new_for(int line, AstNode* init, AstNode* condition, AstNode* increment, AstNode* body);
AstNode* ast_new_block(int line, LinkedList* statements);
 
AstNode* ast_new_let_decl(int line, const char* name, const char* type_name, bool is_mut, AstNode* value);
AstNode* ast_new_fn_decl(int line, const char* name, LinkedList* params, const char* return_type, AstNode* body);
 
AstNode* ast_new_call(int line, const char* callee_name, LinkedList* args);
AstNode* ast_new_return(int line, AstNode* value);
AstNode* ast_new_break(int line);
AstNode* ast_new_continue(int line);
 
void ast_destroy(AstNode* node);
void ast_print(const AstNode* node, int depth);
```
 
`line` primeiro em toda construtora. Preenche o campo `line` da struct. Parâmetro `AstNode*` que pode ser `NULL` não aparece marcado no tipo (C não tem isso), precisa de comentário/doc separado: `else_block` de `ast_new_if`, `value` de `ast_new_return`, `type_name` de `ast_new_let_decl` são os três casos disso nessa lista. Parâmetro de string recebido é `const char*` (só leitura, a construtora copia para dentro), mas o campo salvo na struct é `char*` sem `const`, é a cópia própria do nó, sujeita a ser liberada depois pelo destruidor.

### ast.c — fábrica

1. Construtoras -> Cada uma aloca, seta o `kind` certo, preenche os campos daquele kind, devolve o ponteiro. Uma função por kind evita o bug clássico de esquecer de preencher campo ao montar struct na mão.

2. Destruidor recursivo -> Desce a árvore até a folha, libera filho primeiro, depois o próprio nó. É onde a decisão de memória manual do `vision-roadmap.md` vira código de verdade. Sem GC automático nessa fase, esse destruidor é quem libera.

3. Pretty-print -> Anda a árvore imprimindo cada nó indentado por profundidade. Permite testar o parser isolado, sem interpretador nenhum (roda o parser, imprime, confere se a forma bate com o esperado. Antes de qualquer coisa rodar de verdade).

### Decisões já fechadas

| Decisão | Escolha | Por quê |
|---|---|---|
| Lista de filhos de tamanho variável (statements de um bloco, parâmetros de `fn`, argumentos de chamada) | Reaproveita a `LinkedList` da minha libcds em vez de escrever tipo de lista novo para AST. | Já guarda `void*`, já testada. Não faz sentido duplicar. |
| Posse de string (nome de identificador, conteúdo de literal string) | Nó guarda cópia própria da string, alocada no momento da construção, não ponteiro para o texto original do código-fonte. | Consequência direta -> o destruidor recursivo também libera essas strings, não só os nós filhos (sem isso vazaria toda string alocada durante o parsing). |
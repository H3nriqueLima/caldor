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

  `struct`/`enum`/`match`/genérico/array/referência ficam de fora, são item 3/4 do roadmap, não existem ainda nessa árvore. Os 4 literais são separados (não um `AST_LITERAL` genérico) porque cada um guarda um valor de C diferente por baixo. Juntar todos exigiria um campo a mais só para saber qual dos quatro é, informação que o enum já dá de graça. `AST_IF` é um kind só, usado tanto em posição de statement quanto de expression, o nó não muda, só o contexto onde ele aparece na árvore (02-syntax.md já trata os dois usos como a mesma construção).

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

3. Protótipo dos constructors -> Uma função por kind (`ast_new_...`), o parser chama em vez de montar a struct na mão:

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
 
`line` primeiro em toda constructor. Preenche o campo `line` da struct. Parâmetro `AstNode*` que pode ser `NULL` não aparece marcado no tipo (C não tem isso), precisa de comentário/doc separado: `else_block` de `ast_new_if`, `value` de `ast_new_return`, `type_name` de `ast_new_let_decl` são os três casos disso nessa lista. Parâmetro de string recebido é `const char*` (só leitura, a construtora copia para dentro), mas o campo salvo na struct é `char*` sem `const`, é a cópia própria do nó, sujeita a ser liberada depois pelo destructor.

### ast.c — fábrica

1. Constructors -> Uma por kind, seguindo o mesmo padrão sempre: aloca `sizeof(AstNode)`, se `malloc` falhar devolve `NULL` direto (mesma convenção da libcds, `ll_create` também devolve `NULL` se malloc falhar, não trata isso como erro fatal escondido), senão seta `kind`, `line` e os campos daquele kind específico.
 
Cópia de string usa uma função própria, `dup_string` (`malloc` + `memcpy`). Construtora que copia string e a cópia falha (`dup_string` devolve `NULL` por `malloc` ter falhado) libera o que já tinha sido alocado antes de devolver `NULL`, não deixa nó pela metade solto.
 
Apareceu uma peça que não tava prevista na teoria original, `fn_decl.params` guarda parâmetro (nome + tipo), que não é `AstNode`, não é expressão, não tem "kind". Precisou de um tipo à parte:

```c
typedef struct {
    char* name;
    char* type_name;
} AstParam;
 
AstParam* ast_param_create(const char* name, const char* type_name);
void ast_param_destroy(AstParam* param);
```

Segue a mesma regra de posse de string que o resto, `ast_param_create` copia, `ast_param_destroy` libera.

2. Destructor recursivo -> `switch (node->kind)`, cada caso libera o que aquele kind especificamente possui: string copiada (`AST_LITERAL_STRING`/`AST_IDENTIFIER`/nome de `let`/`fn`/`call`), filho via chamada recursiva de `ast_destroy`, ou lista via `ll_destroy_with_values` da libcds passando `ast_destroy`/`ast_param_destroy` como função de liberar cada valor, evita escrever um loop manual para cada lista (`block.statements`, `fn_decl.params`, `call.args`). `ast_destroy(NULL)` não faz nada (checagem logo na entrada da função), o que cobre de graça os campos opcionais (`else_block` de `if` sem `else`, `value` de `return` sem valor).
 
`switch` não tem `default` de propósito, se um kind novo entrar no enum e alguém esquecer de tratar aqui, o compilador (`-Wswitch`) avisa que faltou caso. Um `default` silenciaria esse aviso.

3. Pretty-print -> `ast_print` recursivo, indentado por profundidade. Mesma lógica do destructor, mas imprimindo em vez de liberar. Para percorrer lista (`block.statements`, `fn_decl.params`, `call.args`) sem remover nada dela, usa `ll_for_each` da libcds com um contexto pequeno (`{ int depth; }`) empacotando a profundidade, já que o callback de `ll_for_each` só recebe `(valor, context)`.

### Decisões já fechadas

| Decisão | Escolha | Por quê |
|---|---|---|
| Lista de filhos de tamanho variável (statements de um bloco, parâmetros de `fn`, argumentos de chamada) | Reaproveita a `LinkedList` da minha libcds em vez de escrever tipo de lista novo para AST. | Já guarda `void*`, já testada. Não faz sentido duplicar. |
| Posse de string (nome de identificador, conteúdo de literal string) | Nó guarda cópia própria da string, alocada no momento da construção, não ponteiro para o texto original do código-fonte. | Consequência direta -> o destructor recursivo também libera essas strings, não só os nós filhos (sem isso vazaria toda string alocada durante o parsing). |
| Operador de nó (`BinOp`/`UnOp`) desacoplado de `TokenType` do lexer | Enum próprio da AST em vez de reusar `TokenType`. Quem traduz token para operador é o parser, no momento de montar o nó. | `ast.h` fica sem depender de `token.h`, mantém a ordem de implementação (`ast.h` antes do lexer) válida. Evita a AST carregar token que nunca vira operador (`TOKEN_SEMICOLON`, palavra reservada de declaração, etc). |
| Parâmetro de função (`fn_decl.params`) | Tipo próprio `AstParam` (nome + tipo), não `AstNode`. | Parâmetro não é expressão, não tem "kind", só apareceu como gap ao escrever `ast.c` de verdade, não fazia parte da teoria original do `ast.h`. |

## token.h / token.c
 
Peça seguinte do front-end, entre o texto fonte e o parser.
 
### token.h — a forma
 
Enum `TokenType` -> diferente do `AstKind`, cobre o 01-lexical-grammar.md inteiro de uma vez, não só o item 1 do roadmap. Reconhecer um token não exige saber tratar ele depois, o lexer devolve o token de `struct`/`enum`/`match` mesmo sem o parser ainda saber o que fazer com isso. Quem decide "não trato isso ainda" é o parser, mais adiante. Cobrir tudo agora evita voltar no lexer a cada item novo do roadmap.
 
Dois token especiais além dos óbvios (palavra reservada, operador, literal): `TOKEN_EOF` (fim do texto, sem isso o parser não sabe quando parar de pedir token) e `TOKEN_ERROR` (caractere que não bate com regra nenhuma, tipo `@` solto. Vira token de erro em vez de travar o programa).
 
Struct `Token` -> `type`, `line`, e o lexema (o pedaço de texto reconhecido). Token guarda cópia própria do lexema, mesma lógica de posse de string do `ast.h`. Evita lidar com ponteiro para dentro do buffer do arquivo original depois que ele já não existe mais. Literal numérico (`int`/`float`) já sai do lexer convertido para valor de verdade, não como texto. O lexer já andou dígito por dígito reconhecendo o padrão, não faz sentido o parser refazer esse trabalho.
 
### token.c — construção e debug
 
`token_create` -> uma função só, não uma por tipo de token (diferente do `ast.c`): a diferença de formato entre os tipos de token é pequena (só o valor numérico muda em alguns casos), não justifica uma construtora por tipo.
 
Destructor -> só libera o lexema copiado. Não libera o `Token` em si, porque ele nunca é alocado no heap. É valor pequeno, devolvido por cópia de função (`lexer_next_token`), não por ponteiro.
 
`token_type_to_string` -> mesmo papel do `binop_to_string`/`unop_to_string` do `ast.c`, útil para mensagem de erro do parser mais adiante.
 
### Decisões já fechadas
 
| Decisão | Escolha | Por quê |
|---|---|---|
| Posse do lexema | `Token` guarda cópia própria (aloca e copia), não ponteiro para o texto original do arquivo fonte. | Parser converte token em nó de AST e descarta o token logo em seguida, não precisa se preocupar se o buffer do arquivo original ainda existe. Custa uma alocação a mais por token, evita lidar com substring sem terminador nulo apontando para o meio do arquivo. |
| Lexer sob demanda, não lista pronta | `lexer_next_token` devolve um token de cada vez, quando o parser pede, não gera lista completa de token antes de começar a parsear. | Mesmo jeito que o parser-expressoes-infixa já funciona na prática (parser recursivo descendente pedindo o próximo símbolo conforme precisa). `Token` fica valor pequeno, sem `malloc` para o token em si, só para o lexema de dentro dele. |
 
 ### Gap conhecido
 
`token_create` devolve `Token` por valor, não por ponteiro (decisão acima) -> diferente das constructors de `AstNode*` no `ast.c`, que devolvem `NULL` quando `malloc`/`dup_string` falha. Não tem "valor nulo" equivalente para um `Token` por valor: se `dup_string(lexeme)` falhar por falta de memória, o `Token` volta mesmo assim, só que com `lexeme == NULL` silenciosamente, sem sinalizar que aquilo foi uma falha, indistinguível de um cenário que nem devia existir (`Token` sempre recebe um lexeme de verdade, nunca `NULL` de propósito).
 
Não resolvido ainda, porque quem chama `token_create` é o `lexer.c`, que não existe. Fica em aberto até lá. Possível saída: o lexer detectar `token.lexeme == NULL` depois de criar e devolver um `TOKEN_ERROR` no lugar, mas isso é decisão de quando o lexer for escrito, não antes.
 
## util.h / util.c
 
Utilitário pequeno compartilhado entre partes diferentes do projeto -> hoje só `dup_string`. Nasceu quando essa função apareceu duplicada em `ast.c` e `token.c` ao mesmo tempo (as duas precisam copiar string do mesmo jeito). Antes da segunda duplicação, cada `.c` tinha a própria cópia, duplicar uma função de 6 linhas uma vez não justificava criar arquivo novo; na segunda vez, passou a valer a pena.
 
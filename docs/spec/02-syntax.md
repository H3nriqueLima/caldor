# Syntax

Gramática formal, como a sequência de tokens do 01-lexical-grammar.md vira AST. Notação EBNF.

## Notação

| Símbolo | Significado |
|---|---|
| `::=` | "é definido como" |
| `\|` | alternativa |
| `[ x ]` | `x` opcional (0 ou 1 vez) |
| `{ x }` | `x` repetido (0 ou mais vezes) |
| `( x \| y )` | agrupamento, para alternativa dentro de uma regra maior |
| `"texto"` | terminal literal (token exato) |
| `IDENT`, `NUMBER`, `STRING` | terminal vindo direto do lexer |

## Programa

```
program ::= { declaration } ;
```

## Declarações

```
declaration ::= let_decl | fn_decl | struct_decl | trait_decl | impl_decl | enum_decl | statement ;

let_decl    ::= "let", [ "mut" ], IDENT, [ ":", type ], "=", expression, ";" ;

fn_decl     ::= "fn", IDENT, [ type_params ], "(", [ param_list ], ")", [ "->", type ], block ;
param_list  ::= param, { ",", param } ;
param       ::= IDENT, ":", type ;

struct_decl ::= "struct", IDENT, [ type_params ], "{", { field }, "}" ;
field       ::= IDENT, ":", type, ";" ;

trait_decl  ::= "trait", IDENT, [ type_params ], "{", { fn_signature }, "}" ;
fn_signature::= "fn", IDENT, "(", [ param_list ], ")", [ "->", type ], ";" ;

impl_decl   ::= "impl", IDENT, [ "for", IDENT ], "{", { fn_decl }, "}" ;

enum_decl   ::= "enum", IDENT, [ type_params ], "{", variant_list, "}" ;
variant_list::= variant, { ",", variant } ;
variant     ::= IDENT, [ "(", type_list, ")" ] ;

type_params ::= "<", type_param, { ",", type_param }, ">" ;
type_param  ::= IDENT, [ ":", trait_bound ] ;
trait_bound ::= IDENT, { "+", IDENT } ;

type        ::= [ "&" ], IDENT, [ "<", type_list, ">" ], { "[", "]" } ;
type_list   ::= type, { ",", type } ;
```

`impl Struct { ... }` declara método próprio. `impl Trait for Struct { ... }` implementa o contrato do trait para o struct, é onde "composição em vez de herança" (a decisão está em vision-roadmap.md) vira sintaxe concreta.

`let` é imutável por padrão. `let mut x = 1;` libera reatribuição, `let x = 1;` não permite. A checagem de reatribuição em variável não-`mut` é semântica (03-semantics.md), aqui a gramática só permite o `mut` opcional.

`type` cobre `&T` (referência), `T[]`/`T[][]` (array, aninhável) e `Nome<T, U>` (instanciação genérica) além do nome simples. `type_params` (`<T: Trait>`, `+` para mais de um bound) só aparece em posição de declaração, nunca em `expression`. Por isso não colide com `<`/`>` de comparação: o parser já sabe que está em modo "tipo" quando chega ali, não em modo "expressão".

## Statements

```
statement    ::= expr_stmt | if_expr | while_stmt | for_stmt
               | return_stmt | break_stmt | continue_stmt | block ;

expr_stmt    ::= expression, ";" ;

if_expr      ::= "if", "(", expression, ")", block, [ "else", ( if_expr | block ) ] ;

while_stmt   ::= "while", "(", expression, ")", block ;

for_stmt     ::= "for", "(", let_decl, expression, ";", expression, ")", block ;

return_stmt  ::= "return", [ expression ], ";" ;
break_stmt   ::= "break", ";" ;
continue_stmt::= "continue", ";" ;

block        ::= "{", { declaration }, "}" ;
```

`for` é C-style: inicialização, condição, incremento, não `for x in lista`. Isso fica para quando existir iterador definido na stdlib, sem stdlib ainda não tem o que iterar formalmente.

`if` é expression, não statement. Tem valor, o valor do braço executado é o da última expressão do bloco escolhido, sem `;` no fim dela. Se tiver `;`, ou o bloco terminar vazio, o valor é `unit` (a regra de valor de bloco é formalizada no 03-semantics.md, aqui é só sintaxe). Aparece solto, em posição de `statement`, sem exigir `;` depois do `}` (mesma convenção do bloco de função/struct, que também não leva `;`), e também dentro de expressão maior via `primary`, para casos como `let x = if (cond) { 1 } else { 2 };`.

`else` continua opcional na gramática mesmo com `if` sendo expression. `if` sem `else` tem valor `unit`, então só é válido nas posições onde o valor é descartado (statement). Usar sem `else` dentro de `let x = if (...) { 1 };` é erro de tipo, não de sintaxe.

`while`/`for` continuam statement puro, não produzem valor. Uma feature de `break valor` deixaria isso menos estranho, mas ainda não foi decidida. Combinar as duas coisas junto seria especular além do que já foi fechado.

## Expressões, precedência

Estende direto a cadeia do parser-expressoes (expressão, termo, potência, fator), agora com todos os operadores do 01-lexical-grammar.md encaixados. Ordem abaixo é da menor para maior precedência, cada regra só desce para próxima quando não acha o operador do seu próprio nível:

```
assignment      ::= logical_or, [ "=", assignment ] ;

logical_or      ::= logical_and, { "or", logical_and } ;
logical_and     ::= equality, { "and", equality } ;

equality        ::= comparison, { ( "==" | "!=" ), comparison } ;
comparison      ::= bitwise_or, { ( "<" | ">" | "<=" | ">=" ), bitwise_or } ;

bitwise_or      ::= bitwise_xor, { "|", bitwise_xor } ;
bitwise_xor     ::= bitwise_and, { "xor", bitwise_and } ;
bitwise_and     ::= shift, { "&", shift } ;
shift           ::= additive, { ( "<<" | ">>" ), additive } ;

additive        ::= multiplicative, { ( "+" | "-" ), multiplicative } ;
multiplicative  ::= power, { ( "*" | "/" | "%" ), power } ;

power           ::= unary, [ "^", power ] ;
unary           ::= ( "-" | "not" | "~" | "&" | "*" ), unary | cast ;
cast            ::= postfix, [ "as", type ] ;

postfix         ::= primary, { call_suffix | member_suffix | index_suffix } ;
call_suffix     ::= "(", [ arg_list ], ")" ;
member_suffix   ::= ".", IDENT ;
index_suffix    ::= "[", expression, "]" ;

primary         ::= NUMBER | STRING | "true" | "false" | "null"
                   | IDENT | if_expr | match_expr | struct_literal
                   | enum_literal | array_literal
                   | "(", expression, ")" ;

struct_literal  ::= IDENT, "{", [ field_init_list ], "}" ;
field_init_list ::= field_init, { ",", field_init } ;
field_init      ::= IDENT, ":", expression ;

enum_literal    ::= IDENT, ".", IDENT, [ "(", arg_list, ")" ] ;

array_literal   ::= "[", [ expression, { ",", expression } ], "]" ;

match_expr      ::= "match", expression, "{", { match_arm }, "}" ;
match_arm       ::= pattern, "=>", ( expression | block ), [ "," ] ;
pattern         ::= IDENT, ".", IDENT, [ "(", bind_list, ")" ]
                   | IDENT ;
bind_list       ::= IDENT, { ",", IDENT } ;

arg_list        ::= expression, { ",", expression } ;
```

`power` continua associativo à direita, no mesmo lugar da cadeia, isso não mudou desde o parser-expressoes-infixa. `assignment` é associativo à direita (`a = b = 1` é `a = (b = 1)`), todo o resto é associativo à esquerda.

A ordem de bit a bit entre comparação e aritmética segue a convenção do C: `|` mais solto que `xor`, que é mais solto que `&`, que é mais solto que shift. Os lógicos por palavra reservada entraram no lugar onde `&&`/`||` estariam num C comum.

`assignment` valida em cima de `logical_or` (não de `primary`) de propósito. O lado esquerdo precisa ser um lvalue válido (identificador, member_access ou index), essa checagem é semântica, não sintática, quem rejeita é o 03-semantics.md.

`unary` ganhou `&`/`*`, reaproveitados do bit a bit e resolvidos por posição (01-lexical-grammar.md). `cast` fica entre `unary` e `postfix`: `-x as float` é `-(x as float)`, `x.campo as int` é `(x.campo) as int`.

`call`/`member_access`/indexação viraram um `postfix` só, encadeável (`foo().bar[0].baz()` é uma sequência de sufixos em cima do mesmo `primary`). É mais fiel a como um parser recursivo descendente resolve isso na prática do que três regras soltas.

`struct_literal` cria uma ambiguidade real em `if (Ponto { x: 1 } == p)`: o parser não sabe se `{` é literal de struct ou abertura do bloco do `if`. Resolvida como o Rust resolve, literal de struct cru não é permitido direto na condição de `if`/`while`/`for` sem parêntese. `if ((Ponto { x: 1 }) == p)` funciona, sem os parênteses extras é erro de sintaxe.

`match` é expression igual `if`, o valor é o do braço escolhido. `pattern ::= IDENT` sozinho cobre dois casos: se o identificador bate com o nome de uma variante sem dado (`None`), casa por igualdade; senão é binding, captura qualquer valor e dá nome a ele (inclusive `_`, que por convenção captura e descarta, já que `_` é identificador válido desde o 01-lexical-grammar.md). Exaustividade, cobrir toda variante do enum ou ter um braço `_`, é checagem semântica (03-semantics.md), a gramática aceita `match` incompleto, quem rejeita é o type checker.
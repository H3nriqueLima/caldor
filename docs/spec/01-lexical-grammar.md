# 01 — Lexical grammar

Como o texto fonte vira token. Antes da AST, antes da gramática, essa é a camada mais baixa, reaproveitando o que já foi resolvido no parser-expressoes-infixa para números e operadores, estendendo para o resto que uma linguagem de verdade precisa (identificador, palavra reservada, string).

## Espaço em branco e comentários

Espaço, tab e quebra de linha são ignorados fora de string/comentário, não têm significado sintático (decisão já fechada: chaves, não indentação).

| Forma | Exemplo |
|---|---|
| Linha | `// comentário até o fim da linha` |
| Bloco | `/* comentário \n em várias linhas */` |

Comentário de bloco não aninha. `/* /* */ */` fecha no primeiro `*/`.

## Identificadores

```
identifier = (letter | "_"), { letter | digit | "_" };
letter     = "a".."z" | "A".."Z";
digit      = "0".."9";
```

Case-sensitive. `total` e `Total` são identificadores diferentes. Não pode começar com dígito. `_` sozinho já é um identificador válido por essa regra (letra-ou-underscore seguido de zero repetições), não precisou de token novo para virar o coringa de `match` em 02-syntax.md, é reaproveitamento direto.

## Palavras reservadas

Não podem ser usadas como identificador:

| Categoria | Palavras |
|---|---|
| Declaração | `let`, `mut`, `fn`, `struct`, `trait`, `impl`, `enum` |
| Controle de fluxo | `if`, `else`, `while`, `for`, `return`, `break`, `continue`, `match` |
| Literal | `true`, `false`, `null` |
| Lógico | `and`, `or`, `not` |
| Bit a bit | `xor` |
| Conversão | `as` |
| Modo de compilação | `safe`, `unsafe` |

`fn` em vez de `function` por ser mais curto, consistente com o resto da sintaxe enxuta já decidida (chaves, `;` obrigatório). O exemplo de LOIA no vision-roadmap.md usa `function` por escrever pseudocódigo daquela ideia, não é a sintaxe final da Fase 1.

## Literais

| Tipo | Forma | Exemplo |
|---|---|---|
| Inteiro | dígitos, sem ponto | `42` |
| Decimal | dígitos, ponto, dígitos | `3.5` |
| String | entre aspas duplas, `\"`, `\\`, `\n` como escape | `"texto"` |
| Booleano | palavra reservada | `true`, `false` |
| Nulo | palavra reservada | `null` |

Número negativo (`-5`) não é literal léxico, é o literal `5` precedido do operador unário `-`, resolvido na gramática/parser, igual já era no parser-expressoes-infixa.

## Operadores e delimitadores

| Categoria | Símbolos |
|---|---|
| Aritmético | `+` `-` `*` `/` `%` `^` |
| Comparação | `==` `!=` `<` `>` `<=` `>=` |
| Bit a bit | `&` `\|` `~` `<<` `>>` (XOR é a palavra reservada `xor`, não símbolo) |
| Atribuição | `=` |
| Agrupamento | `(` `)` `{` `}` `[` `]` |
| Separador | `,` `;` `:` |
| Acesso | `.` |

`^` é potência, associativo à direita, herdado direto do parser-expressoes-infixa, não é XOR bit a bit. `&` e `|` puderam ficar livres para bit a bit sem ambiguidade justamente porque os lógicos (`and`/`or`) já são palavra reservada, não símbolo, se um dia a linguagem adotasse `&&`/`||` para lógico, essa folga desaparecia.

`&` e `*` têm segundo significado por posição, em posição de tipo ou de unário, `&T`/`&expr` é referência e `*expr` é desreferência (02-syntax.md), em vez de bit a bit/multiplicação. É a mesma resolução por contexto que C usa há décadas para o mesmo par de símbolos, não é ambiguidade nova.

## Erros léxicos

Token que não casa com nenhuma regra acima (caractere solto tipo `@`, `#`, `$` fora de string) é erro léxico, rejeitado antes de chegar no parser. Mesmo princípio que já existia no parser-expressoes-infixa para caractere inválido.
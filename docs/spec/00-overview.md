# Overview

Essa pasta é a especificação da linguagem, o que cada construção significa formalmente. O vision-roadmap.md carrega o porquê das decisões, aqui fica o quê e o como.

## Escopo

Cobre só a Fase 1, a linguagem tradicional. LOIA fica de fora, é ideia especulativa demais pra merecer spec formal ainda. Quando a Fase 2 virar implementação de verdade, ganha spec própria.

## Status

Rascunho. Cada arquivo reflete o estado atual da implementação, não uma versão fechada. Vai mudar conforme o interpretador for escrito e algum detalhe que parecia óbvio no papel se revelar ambíguo na prática.

## Terminologia usada no resto da spec

| Termo | Significado |
|---|---|
| Lexema | Sequência de caracteres reconhecida como uma unidade (ex: `123`, `+`, `nomevar`) |
| Token | Lexema classificado por tipo (`NUMBER`, `PLUS`, `IDENTIFIER`) |
| AST | Árvore de sintaxe abstrata, saída do parser |
| Statement | Instrução que executa por efeito, não produz valor usável (`while`, declaração) |
| Expression | Construção que produz um valor (`if` incluído, ver 02-syntax.md) |
| Lvalue | Lado esquerdo válido de atribuição, identificador, acesso a campo ou índice de array, nunca um literal |
| Shadowing | Redeclarar um nome num escopo aninhado, escondendo o de fora até o bloco fechar (redeclarar no mesmo escopo é erro, isso não) |

## Como ler o resto da spec

1. 01-lexical-grammar.md, como o texto vira token. Regras de lexema, números, identificadores, palavras reservadas, operadores, comentários.
2. 02-syntax.md, como token vira AST. Gramática formal em EBNF, uma regra por construção da linguagem.
3. 03-semantics.md, o que cada construção faz quando executa. Regras de tipo, escopo, avaliação.

## Referência cruzada

Decisão de design, por que estático, por que chaves etc, fica em vision-roadmap.md. Essa spec só formaliza o resultado, não repete o argumento.
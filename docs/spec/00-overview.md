# 00 — Overview

Essa pasta é a especificação da linguagem, o que cada construção significa, formalmente. Diferente do vision-roadmap.md, que carrega o porquê das decisões, aqui é só o quê e o como (referência, não justificativa).

## Escopo

Cobre só a Fase 1 (linguagem tradicional). LOIA não entra aqui, quando a Fase 2 virar implementação de verdade, ganha spec própria, não faz sentido documentar formalmente algo que ainda é ideia especulativa.

## Status

Rascunho. Cada arquivo aqui reflete o estado atual da implementação, não uma versão final fechada. Vai mudar conforme o interpretador for sendo escrito e algum detalhe que parecia óbvio no papel se revelar ambíguo na prática.

## Terminologia usada no resto da spec

| Termo | Significado |
|---|---|
| Lexema | Sequência de caracteres reconhecida como uma unidade (ex: `123`, `+`, `nomevar`) |
| Token | Lexema classificado por tipo (`NUMBER`, `PLUS`, `IDENTIFIER`) |
| AST | Árvore de sintaxe abstrata, saída do parser |
| Statement | Instrução que executa por efeito, não produz valor usável (`while`, declaração) |
| Expression | Construção que produz um valor (`if` incluído, ver 02-syntax.md) |
| Lvalue | Lado esquerdo válido de atribuição. Identificador, acesso a campo ou índice de array, nunca um literal |
| Shadowing | Redeclarar um nome num escopo aninhado, escondendo o de fora até o bloco fechar (não é erro, redeclarar no mesmo escopo é) |

## Como ler o resto da spec

1. 01-lexical-grammar.md → como o texto vira token. Regras de lexema: números, identificadores, palavras reservadas, operadores, comentários.
2. 02-syntax.md → como token vira AST. Gramática formal (EBNF), uma regra por construção da linguagem.
3. 03-semantics.md → o que cada construção faz quando executa. Regras de tipo, escopo, avaliação.

## Referência cruzada

Decisão de design (por que estático, por que chaves, etc.) fica em vision-roadmap.md. Essa spec só formaliza o resultado da decisão, não repete o argumento.
# Caldor
Linguagem de programação própria, em desenvolvimento.

## O que é?
Uma linguagem multi-paradigma (imperativo, procedural, funcional, orientação a objetos), pensada para funcionar tanto em mid-level (acesso a hardware, controle de memória, estilo C/C++) quanto em high-level. O front-end (lexer, parser, AST) é único e alimenta dois backends de execução, interpretado e bytecode com VM própria.

## Por que existe?
Projeto pessoal de estudo de design de linguagens e implementação de interpretadores/compiladores. É a evolução do parser-expressao-infixa, mesma técnica de parsing recursivo descendente, agora numa linguagem de verdade em vez de só expressões aritméticas.

A ideia de fundo é não obrigar a escolher entre abstração e controle. A mesma linguagem cobre o struct/trait de alto nível e o malloc/free explícito de baixo nível, dependendo do modo escolhido.

Tem uma fase futura mais especulativa, uma extensão chamada LOIA (Linguagem Orientada a Intenção e Ambiguidade), onde trechos de código podem ser especificados por contrato (requires/ensures) em vez de implementação explícita. Um modelo de IA embutido no build resolveria isso e depois congelaria em código determinístico. Só faz sentido depois da linguagem base estar madura, então fica documentada mas não é o foco agora.

## Status
Fase inicial. Ainda não existe lexer, parser nem interpretador funcionando, só a estrutura do repositório. O plano completo, com decisões de design em aberto, roadmap e os problemas conhecidos da fase LOIA, está em docs/vision-roadmap.md.

## Como rodar (ainda a desenvolver)
```
mkdir build && cd build
cmake ..
cmake --build .
```

Requer CMake 3.15+ e um compilador C11.

## Estrutura
```
caldor/
├── include/caldor/   # headers públicos
├── src/              # implementação
├── docs/             # visão do projeto e a especificação da linguagem
├── examples/         # programas de exemplo em Caldor (.cldr)
└── tests/            # testes
```
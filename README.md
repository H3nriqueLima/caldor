# Caldor
Linguagem de programação própria, em desenvolvimento.

## O que é?
Uma linguagem multi-paradigma (imperativo, procedural, funcional, orientação a objetos), pensada para funcionar tanto em mid-level (acesso a hardware, controle de memória, no estilo C/C++) quanto em high-level. Mesmo front-end (lexer, parser, AST) alimentando dois backends de execução → interpretado e bytecode + VM própria.

## Por que existe?
Projeto pessoal de estudo de design de linguagens e implementação de interpretadores/compiladores, é a evolução natural do meu projeto parser-expressao-infixa (mesma técnica de parsing recursivo descendente, agora aplicada a uma linguagem de verdade em vez de só expressões aritméticas).

A ideia de fundo é não forçar escolha entre abstração e controle. A mesma linguagem cobre o struct/trait de alto nível e o malloc/free explícito de baixo nível, dependendo do modo.

A fase futura especulativa → uma extensão chamada LOIA (Linguagem Orientada a Intenção e Ambiguidade), onde trechos de código podem ser especificados por contrato (requires/ensures) em vez de implementação explícita, resolvidos por um modelo de IA embutido no build e depois congelados em código determinístico. Só faz sentido depois da linguagem base estar madura, então fica documentada mas não é o foco agora.

## Status
Fase inicial. Ainda não existe lexer, parser nem interpretador funcionando, só a estrutura do repositório. O plano completo (decisões de design em aberto, roadmap, problemas conhecidos da fase LOIA) está em docs/vision-roadmap.md.

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
├── docs/             # visão do projeto e depois a especificação da linguagem
├── examples/         # programas de exemplo em Caldor (.cldr)
└── tests/            # testes
```
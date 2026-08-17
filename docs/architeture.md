# Architecture

Diferente dos arquivos de docs/spec/, que documentam a linguagem Caldor em si (sintaxe, semântica), esse arquivo documenta a ferramenta em C que implementa ela. Como cada peça é organizada e por quê. Cresce conforme cada peça do roadmap for sendo escrita, começando pelo front-end.

## ast.h / ast.c

### ast.h — contrato

Header, não tem lógica, só a forma. Três coisas:

1. Enum de kind (Um valor por tipo de nó que existe nessa fase. Escopo limitado ao item 1 do roadmap, não a spec inteira): literal (int/float/bool/string), identificador, operação binária, operação unária, atribuição, `if`, `while`, `for`, bloco, `let`, `fn`, chamada, `return`, `break`, `continue`. 
  `struct`/`enum`/`match`/genérico/array/referência ficam de fora do enum, são item 3/4, não existem ainda nessa árvore.

2. Struct do nó -> Campo `kind` + o espaço que muda de formato dependendo do kind (um literal guarda valor, um binary guarda operador e dois ponteiros pra outros nós). Campos aparecem no header, visíveis. Diferente do `Environment` do interpretador, que ficou opaco por não ter consumidor de fora. Aqui tem dois consumidores reais (parser escreve, interpretador lê), esconder atrapalharia em vez de ajudar.

3. Protótipo das construtoras -> Uma função por kind (`ast_new_...`), implementadas no `.c`. O parser chama essas funções em vez de montar a struct na mão toda hora.

### ast.c — fábrica

1. Construtoras -> Cada uma aloca, seta o `kind` certo, preenche os campos daquele kind, devolve o ponteiro. Uma função por kind evita o bug clássico de esquecer de preencher campo ao montar struct na mão.

2. Destruidor recursivo -> Desce a árvore até a folha, libera filho primeiro, depois o próprio nó. É onde a decisão de memória manual do `vision-roadmap.md` vira código de verdade. Sem GC automático nessa fase, esse destruidor é quem libera.

3. Pretty-print -> Anda a árvore imprimindo cada nó indentado por profundidade. Permite testar o parser isolado, sem interpretador nenhum (roda o parser, imprime, confere se a forma bate com o esperado. Antes de qualquer coisa rodar de verdade).

### Decisões já fechadas

| Decisão | Escolha | Por quê |
|---|---|---|
| Lista de filhos de tamanho variável (statements de um bloco, parâmetros de `fn`, argumentos de chamada) | Reaproveita a `LinkedList` da minha libcds em vez de escrever tipo de lista novo para AST. | Já guarda `void*`, já testada. Não faz sentido duplicar. |
| Posse de string (nome de identificador, conteúdo de literal string) | Nó guarda cópia própria da string, alocada no momento da construção, não ponteiro para o texto original do código-fonte. | Consequência direta -> o destruidor recursivo também libera essas strings, não só os nós filhos (sem isso vazaria toda string alocada durante o parsing). |
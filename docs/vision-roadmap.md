# Visão e roadmap

Projeto dividido em duas fases. A fase 2 só existe no papel por enquanto, depende da fase 1 estar pronta para fazer sentido, não tem onde encaixar contrato/IA numa linguagem que ainda não tem parser, tipos nem execução.

## Princípios de design

Servem para decidir features novas sem contradizer o que já foi decidido:

1. Controle explícito não é sacrificado por abstração, malloc/free continuam expostos mesmo com GC opcional ligado.
2. Implementação mínima antes de recurso avançado, LLVM só entra depois do bytecode+VM já rodando de verdade.
3. IA não é necessária para rodar programa comum, LOIA é opt-in, nunca faz parte do caminho padrão de execução.
4. Um front-end só alimenta os dois jeitos de rodar, interpretado e bytecode+VM saem do mesmo lexer/parser/AST, não são dois projetos separados.

## Fase 1, linguagem tradicional

Multi-paradigma (imperativo, procedural, funcional, OOP), rodando tanto em mid-level (ponteiros, controle de memória, estilo C/C++) quanto em high-level (abstrações, coleções, produtividade). Interpretada e compilada, não uma escolha única.

### Decisões de design

Registro do que já foi decidido, mais o que ainda falta fechar:

| Decisão | Escolha | Por quê | Status |
|---|---|---|---|
| Tipagem | Estática, forte, inferência local (`let` infere, assinatura de função sempre explícita). | Pega erro em tempo de compilação sem exigir anotação em toda linha. Full Hindley-Milner fica de fora, caro demais para o que a linguagem precisa. | Definido |
| Memória | Manual por padrão (malloc/free expostos), GC (mark-and-sweep) como modo opcional via flag de compilação (`safe` / `unsafe`). | Dá controle real de hardware no núcleo. Ownership/borrow checker descartado por enquanto, é o item mais caro de implementar da lista, fica para uma v2 da linguagem, se fizer sentido em algum momento. | Definido |
| Paradigma | Multi-paradigma. OOP via composição (`struct` + `trait`/`interface`), não herança clássica. | Funções como cidadão de primeira classe cobrem o lado funcional. Composição evita o acoplamento que herança clássica causa. | Definido |
| Sintaxe | Chaves (C-like), ponto-e-vírgula obrigatório. | Parser mais simples e previsível, sem lidar com indentação significativa, sem ambiguidade de onde o statement termina. | Definido |
| Alvo de compilação | Bytecode + VM stack-based própria | Reaproveita o front-end (lexer/parser/AST) que já está em construção, sem precisar aprender uma API de codegen de terceiros. É como Python, Lua e a JVM funcionam por dentro, LLVM entra só depois disso rodar de verdade. | Definido |
| Mutabilidade | Imutável por padrão, `let mut` libera reatribuição. | Mutação vira sinal explícito no código, não acontece por acidente. Mesmo espírito do `safe`/`unsafe` já decidido para memória. | Definido |
| Referência/ponteiro | `&T` no tipo, `&expr` empresta, `*expr` desreferencia. | `&` e `*` já eram token reservado (bit a bit / multiplicação), resolvido por posição, mesma técnica que C usa há décadas. | Definido |
| Conversão explícita (cast) | `expr as Type`, palavra reservada `as`. | Não confunde com chamada de função (`int(x)` pareceria call). | Definido |
| Literal de struct | `Nome { campo: valor, ... }`, proibido cru dentro de condição de `if`/`while`/`for` (precisa parêntese). | Ambíguo com abertura de bloco sem a restrição, mesma regra que Rust adota pelo mesmo motivo. | Definido |
| Array | `T[]` dinâmico, literal `[1, 2, 3]`, indexação `arr[i]`. | Construção embutida do compilador, não depende de genérico existir, dá para fechar antes. | Definido |
| Genéricos | Type params `<T: Trait>` em fn/struct/trait/enum, sempre inferido (nunca explícito na chamada), monomorfização. | Sem argumento explícito em posição de expressão, `<`/`>` de genérico nunca colide com comparação, o parser já sabe que tá em modo tipo. | Definido |
| Enum / tipo soma | `enum Nome<T> { Variante(T), ... }`, construção `Nome.Variante(args)`. | Necessário para `Result`/`Option` existirem, não dá para representar "ou uma coisa ou outra, com dado dentro" só com struct. | Definido |
| Pattern matching | `match` como expression, exaustivo (cobre toda variante ou tem `_`). | Sem isso, enum existiria mas o dado de dentro da variante seria inacessível, e falha exaustiva vira erro de compilação em vez de passar quieto. | Definido |
| Despacho de trait | Estático por padrão (resolvido em tempo de compilação). | Despacho dinâmico (vtable/`dyn Trait`) só importa para coleção heterogênea, feature própria, não bloqueia nada do resto. | Definido (estático) / dinâmico em aberto |
| Tratamento de erros | `Result<T, E>`/`Option<T>`, enum embutido do compilador, sem exceção. | Exceção é fluxo de controle escondido (`throw` pula não-localmente), contradiz o princípio de controle explícito. | Definido |
| Backend nativo | LLVM | Caminho avançado depois do bytecode+VM já rodando de verdade, não bloqueia o roadmap principal. | Planejado |

### Mid-level vs high-level

Mid-level: ponteiros/referências explícitas, controle de alocação, talvez inline assembly, sem GC obrigatório.

High-level: stdlib rica (strings, coleções, I/O, rede), GC opcional, sintaxe mais permissiva. High-level de verdade sempre vai precisar de libs externas (JSON, HTTP), nenhuma linguagem reimplementa tudo, a diferença é oferecer FFI e sistema de módulos para isso ser possível.

### Decisões de implementação
 
Diferente da tabela acima, essas são sobre o interpretador/compilador em C, a ferramenta que roda o CaldOR, não sobre a linguagem em si. Não vão para `docs/spec/` porque não descrevem sintaxe nem semântica do CaldOR.
 
| Decisão | Escolha | Por quê |
|---|---|---|
| Propagação de erro no interpretador (C) | Valor sentinela (tipo `Result` em C), função de avaliação devolve resultado normal ou erro, cada chamada checa e propaga. Macro (`TRY(...)`) reduz o boilerplate de checagem repetida. | `longjmp` pula sem rodar `free()` no caminho, vaza memória bem na peça que devia ser exemplo de controle explícito. E é fluxo de controle escondido, mesmo motivo que já descartou exceção na linguagem (tabela acima), usar `longjmp` para implementar o interpretador contradiria essa decisão só que por baixo dos panos, sem ninguém perceber. |

### Roadmap de implementação

Sequência direta a partir do parser de expressões já feito:

1. Interpretador simples, variáveis, if/while, funções, tree-walking sobre AST.
2. Sistema de tipos, checagem antes de rodar, não só execução.
3. Arrays, structs/records, OOP via `struct` + `trait`/`interface`.
4. Genéricos, enum e pattern matching, design já fechado (`docs/spec`), falta só implementar.
5. Tratamento de erros, `Result`/`Option`, já formalizado como enum embutido, entra assim que o item 4 estiver rodando.
6. Bytecode + VM stack-based, trocando o tree-walking. É como Python/Lua/JVM funcionam por dentro.
7. Sistema de módulos e stdlib mínima.
8. Codegen nativo via LLVM, saindo de bytecode para binário de verdade.

## Fase 2, LOIA (Linguagem Orientada a Intenção e Ambiguidade)

```
function processarDocumento(doc: PDF) -> DadosExtraidos
    requires doc.paginas > 0
    ensures result.cpf.isValido()
{
    // corpo resolvido por IA em tempo de build
}
```

Programador declara contrato (requires/ensures), um modelo de IA resolve a implementação durante o build, testes validam, e a versão aprovada é congelada em código determinístico e cacheada.

### Mitigações já pensadas

| Problema | Mitigação |
|---|---|
| Não-determinismo | Congelamento + cache semântico por hash |
| Risco em produção | Modo debug (IA ativa) vs production (só código congelado) |
| Latência/custo em runtime | Resolução em tempo de compilação, não runtime |
| IA precisar rodar em runtime mesmo assim | SLM local (1B–3B params) embutido, GPU/NPU local |
| Debug difícil | Separar falha de contrato (humano) de falha de implementação (IA) |

### Problemas que ainda não têm solução

1. Contrato fraco deixa passar solução errada. `ensures result.cpf.isValido()` não garante que o CPF veio do documento, dava para devolver um CPF fixo válido sem nunca ler `doc` e passar no contrato mesmo assim. É specification gaming, o mesmo problema do reward hacking em RL, só que fora do contexto de RL. Resolver de verdade exige contrato mais rico, property-based testing, invariante sobre a relação entrada-saída, não só sobre a forma da saída.

2. Congelar resolve determinismo, não corretude. Um bug que passou nos testes acaba congelado com mais confiança do que merece, porque agora tá "validado e cacheado" e ninguém revisa de novo. Pode ser pior que não-determinismo, que pelo menos avisa que tá instável.

3. Invalidar o cache semântico já é problema difícil sozinho. O que conta como "mesma intenção"? Contrato muda uma vírgula, é a mesma função? Modelo de IA atualiza, cache antigo ainda vale? Precisa versionar contrato + versão do modelo + hash do código junto, não só hash do contrato.

4. Nada disso é ideia nova. Design by Contract já resolvia require/ensure sem IA nenhuma, lá nos anos 80, na linguagem Eiffel, com verificação em runtime ou prova estática. Ferramentas como Dafny, F* e TLA+ provam que uma implementação satisfaz um contrato usando SMT solver, sem gerar código, só verificando. E gerar implementação a partir de spec é literalmente um campo de pesquisa ativo (Sketch, síntese neurosimbólica), longe de resolvido de forma geral.

## Compilador vs interpretador

Interpretador primeiro (tree-walking, depois bytecode+VM), ensina AST e semântica com menos fricção. Compilação vem depois reaproveitando o mesmo front-end (lexer, parser, AST, checagem de tipos), só troca o back-end de execução.
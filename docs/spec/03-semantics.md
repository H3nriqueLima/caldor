# Semantics

O que cada construção do 02-syntax.md significa quando executa.

## Sistema de tipos

| Tipo | Largura | Observação |
|---|---|---|
| `int` | 64 bits, sinalizado | equivalente a `i64` |
| `float` | 64 bits, IEEE 754 | equivalente a `f64`, double |
| `bool` | — | `true` / `false` |
| `string` | — | imutável (ver "Mutabilidade") |
| `unit` | — | valor de bloco/`if` que não retorna nada útil, não tem literal próprio |

Struct é tipagem nominal. Dois structs com os mesmos campos mas nomes diferentes são tipos diferentes, mesmo com estrutura idêntica. Não tem tipagem estrutural nessa versão.

`let` sempre infere do valor inicial (02-syntax.md já exige `=` no `let_decl`, não dá para declarar variável sem inicializar). Anotação de tipo, quando presente, só é conferida contra o tipo inferido, não converte o valor: `let x: float = 5;` é erro de compilação, não vira `5.0` sozinho.

## Mutabilidade

Imutável por padrão. `let x = 1;` não pode ser reatribuída, `x = 2;` depois disso é erro de compilação. `let mut x = 1;` libera reatribuição. Mesmo raciocínio do `safe`/`unsafe` de memória, o comportamento mais permissivo exige palavra a mais escrita, nunca é o caminho silencioso.

`string` sendo "imutável" na tabela acima é sobre o conteúdo do buffer, não dá para mudar caractere no meio de uma string existente, independente de a variável que a guarda ser `mut` ou não. São dois níveis diferentes de mutabilidade, o valor em si e o que a variável aponta, igual em qualquer linguagem que separa os dois.

## Escopo

Léxico, por bloco. Cada `{ }` abre escopo novo. Redeclarar o mesmo nome dentro do mesmo escopo é erro de compilação. Redeclarar em escopo aninhado é permitido e faz shadowing, o de dentro esconde o de fora até o bloco fechar:

```
let x = 1;
{
    let x = 2;   // ok, sombra o x de fora
    // x aqui vale 2
}
// x aqui volta a valer 1
```

Parâmetro de função vive no mesmo escopo do corpo. Declarar `let` com o mesmo nome de um parâmetro logo na primeira linha do corpo conta como mesmo escopo, e é erro, não shadowing.

## Avaliação numérica

- `int` e `float` misturados em operação aritmética: `int` promove para `float` (nunca o contrário, não existe truncamento implícito de `float` para `int`).
- Divisão `int / int` trunca em direção a zero, igual C (`7 / 2` é `3`, não `3.5`). Se qualquer operando for `float`, a divisão é em ponto flutuante.
- Divisão por zero é sempre erro de execução, `int` ou `float`, não produz `inf`/`nan` estilo IEEE. Mesma regra que já existia no parser-expressoes, mantida por consistência.
- Overflow de `int` é checado por padrão (erro de execução), condizente com o modo `safe`. Em modo `unsafe`, faz wraparound sem checagem, reaproveitando a mesma flag de compilação que já existe para memória (vision-roadmap.md) em vez de criar uma segunda chave só para isso.

## Funções

Parâmetro é por valor (cópia) por padrão, inclusive struct. Parâmetro do tipo `&T` passa referência em vez de copiar: `fn soma(p: &Ponto)` recebe a mesma instância, `fn soma(p: Ponto)` recebe cópia. `&expr` empresta (pega referência), `*expr` desreferencia, não existe aritmética de ponteiro (`ponteiro + 1` é erro de tipo, diferente de C), só acesso ao valor apontado.

Função sem `-> type` retorna `unit` implicitamente. `return expr;` numa função `unit` é erro de tipo. `return;` sem valor numa função com `-> type` declarado também é erro.

Recursão é permitida sem restrição, nenhuma regra de análise de tamanho de pilha nessa fase.

## Conversão de tipo (cast)

`expr as Type` converte quando existe regra definida. Não é reinterpretação de bits, é conversão de valor:

| De | Para | Comportamento |
|---|---|---|
| `int` | `float` | exato, sem perda |
| `float` | `int` | trunca em direção a zero (`3.9 as int` é `3`, `-3.9 as int` é `-3`) |
| `int`/`float` | `bool` | não existe, erro de tipo, sem "0 é falso" implícito |
| qualquer coisa | `string` | não existe ainda. Formatação/`toString` fica para quando a stdlib mínima (roadmap item 7) existir |

Cast entre struct/trait ainda não foi decidido, depende do despacho de trait (abaixo), que também não fecha o caso geral.

## Structs e traits

`impl Trait for Struct` exige implementar toda função declarada em `fn_signature` no trait. Se faltar uma, é erro de compilação (contrato incompleto).

`Nome { campo: valor, ... }` cria uma instância, todo campo do `struct_decl` precisa aparecer no literal, em qualquer ordem (não é posicional). Faltar campo é erro de compilação, não vira valor default silencioso.

Despacho de método de trait é estático por padrão, resolvido em tempo de compilação, sem vtable. Isso significa que hoje um trait só serve como restrição em `impl` (`impl Trait for Struct`), não como tipo de variável (`let x: MeuTrait = ...` não existe ainda). Trait object com despacho dinâmico fica pro roadmap item 4, junto de genéricos, porque as duas coisas normalmente andam juntas (`Vec<dyn Trait>` etc).

## Array

`T[]` é homogêneo (todo elemento do mesmo tipo `T`) e de tamanho dinâmico, cresce, não é array fixo estilo C. `arr[i]` fora do intervalo é sempre erro de execução, mesmo em modo `unsafe`. Não é comportamento indefinido tipo os ponteiros, índice fora do array é uma decisão separada da de overflow aritmético, não tem relação com gerência de memória. `array_literal` vazio (`[]`) precisa de contexto para inferir o tipo do elemento: `let x: int[] = [];` funciona, `let x = [];` sozinho é erro, porque não dá para inferir tipo de nada.

## Genéricos

Monomorfização: cada uso concreto de um tipo genérico (`Caixa<int>`, `Caixa<string>`) vira uma cópia especializada do código em tempo de compilação, nenhum boxing, nenhuma informação de tipo carregada em runtime, condizente com o despacho estático já decidido. O custo é tamanho de binário (cada instanciação gera código próprio), não velocidade.

`<T: Trait>` restringe: dentro do corpo, só dá para chamar em valor do tipo `T` o que o `Trait` promete. Sem bound, `T` só aceita operação genérica de qualquer tipo (atribuição, passagem de parâmetro), nada específico.

Tipo de parâmetro genérico é sempre inferido do uso (argumento passado, campo do literal), nunca escrito explícito na chamada. Se a inferência falhar, ambígua ou impossível a partir do que foi passado, é erro de compilação, não existe fallback silencioso para `unit` ou parecido.

## Enum

Tipagem nominal, igual struct: `enum Cor { Vermelho, Verde }` e outro enum com variantes de mesmo nome são tipos diferentes. Variante com dado (`Some(T)`) guarda o valor junto da tag, sem herança nem campo compartilhado entre variantes.

`Nome.Variante(args)` (`enum_literal`) constrói. Número e tipo de argumento têm que bater exatamente com o que `variant` declarou, variante sem parêntese na declaração (`None`) não aceita `()` na construção nem o contrário.

## Match

Expression, o valor é o do braço escolhido, todo braço precisa ter o mesmo tipo (mesma regra do `if`/`else`).

Exaustividade é obrigatória: todo `match` sobre um enum precisa cobrir toda variante, ou ter um braço final `_` (ou binding solto tipo `outro => ...`) cobrindo o resto. Faltar variante sem coringa é erro de compilação, não é warning, porque deixar passar quieto abriria brecha para o mesmo tipo de bug que motivou escolher `Result`/`Option` em vez de exceção: falha silenciosa não tratada.

`Nome.Variante(a, b) => expr` dentro do `pattern` liga `a`/`b` ao conteúdo da variante para o escopo daquele braço só, desaparece fora dele, mesma regra de escopo por bloco de sempre.

## `Result`/`Option`, formalizado

Com genérico + enum + match fechados, deixam de ser conceito do vision-roadmap.md e viram tipo embutido de verdade:

```
enum Option<T> { Some(T), None }
enum Result<T, E> { Ok(T), Err(E) }
```

```
fn dividir(a: int, b: int) -> Result<int, string> {
    if (b == 0) {
        return Result.Err("divisão por zero");
    }
    return Result.Ok(a / b);
}

match dividir(10, 0) {
    Result.Ok(valor) => print(valor),
    Result.Err(msg) => print(msg),
}
```

É `enum_decl` + `enum_literal` + `match_expr` normais, sem sintaxe nova, só que os dois tipos vêm definidos pelo compilador em vez de escritos pelo programador. Mesma situação de `int`/`float`, que são "primitivo" mas não têm token especial, é só um `IDENT` que o compilador já reconhece.

## Gaps conhecidos

Sobrou só o que é feature separada de verdade, não sub-item escondido de outra decisão:

| Gap | Por quê continua em aberto |
|---|---|
| Trait object / despacho dinâmico (`dyn Trait`) | Não bloqueia `Result`/`Option` (que são enum, não trait), só importa para coleção heterogênea (`Vec<dyn Trait>`), feature própria, não sub-item de outra coisa |
| Cast entre struct/trait | Depende do item acima estar fechado |
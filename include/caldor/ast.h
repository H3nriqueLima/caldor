#pragma once

#include <stdbool.h>
#include <libcds/linked_list.h>

/*
AST (Árvore de Sintaxe Abstrata) do Caldor.

Escopo dessa versão: Só o que o item 1 do roadmap precisa (interpretador simples). 
Sem struct/enum/match/genérico/array/referência ainda. Esses são item 3/4, e simplesmente não têm kind aqui embaixo. 
Se um dia procurar algum desses e não achar, não é bug, é escopo mesmo.

Ideia geral: Todo nó é uma etiqueta ("kind", dizendo o que ele é) + uma caixa de tamanho variável (o "union" logo abaixo)
guardando os pedaços que formam aquele nó específico.
Um binary_op guarda dois ponteiros para outro nó e um operador; um literal guarda só um valor; um bloco guarda uma lista de outros nós.
*/

/*
Um valor por tipo de nó que existe nessa fase. 
AST_IF é usando tanto como statement solto quando como expression dentro de outra coisa 
(tipo `let x = if (...) {...} else {...};`), é o mesmo kind nos dois casos, o que muda é só onde ele aparece dentro da árvore,
não o kind em si (ver 02-syntax.md para o motivo completo).
*/
typedef enum {
	// literais: cada um guarda um valor de C diferente por baixo (ver union mais abaixo), por isso são 4 kind separados e não um só.
	AST_LITERAL_INT,
	AST_LITERAL_FLOAT,
	AST_LITERAL_BOOL,
	AST_LITERAL_STRING,

	// uso de uma variável já declarada, tipo `x` sozinho numa expressão.
	AST_IDENTIFIER,

	AST_BINARY_OP, // a + b, a == b, a and b, etc —> um operador no meio.
	AST_UNARY_OP, // -a, not a, ~a —> um operador só, um operando só.
	AST_ASSIGNMENT, // a = expr (reatribuição, não é o `let` que declara).

	AST_IF, // if sem else, else_block do union fica NULL.
	AST_WHILE,
	AST_FOR,
	AST_BLOCK, // `{ ... }` —> guarda uma lista de outros nós dentro.

	AST_LET_DECL, // `let` ou `let mut`, sempre com valor inicial (nunca "let x;" sozinho).
	AST_FN_DECL, // declaração de função inteira: nome + parâmetro + corpo.

	AST_CALL, // chamar uma função, tipo `soma(1, 2)`.
	AST_RETURN, // pode ou não carregar valor (`return;` vs `return x;`).
	AST_BREAK, // sem dado nenhum associado, não entra no union lá embaixo.
	AST_CONTINUE, // mesma coisa, kind sozinho já basta.
} AstKind;

/*
Operador de operação binária/unária. É um enum próprio, bem menor, só com o que faz sentido dentro de um nó de árvore.
Quem traduz token para isso aqui é o parser, no momento de montar o nó (ex: token TOKEN_PLUS vira OP_ADD).
Motivo de separar: ast.h não depende de token.h, e a AST não carrega lixo de token de pontuação/palavra reservada que
nunca vira operador.
*/
typedef enum {
	OP_ADD, OP_SUB, OP_MUL, OP_DIV, OP_MOD, OP_POW, 
	OP_EQ, OP_NEQ, OP_LT, OP_GT, OP_LTE, OP_GTE,
	OP_AND, OP_OR,
	OP_BIT_AND, OP_BIT_OR, OP_BIT_XOR, OP_SHL, OP_SHR,
} BinOp;

typedef enum {
	OP_NEG, // - (negativo unário, tipo 55).
	OP_NOT, // not (negação lógica).
	OP_BIT_NOT // - (negação bit a bit).
} UnOp;

/*
forward declaration -> precisa disso porque AstNode aponta para AstNode (um binary_op tem dois AstNode* dentro dele). 
O compilador precisa saber que o nome "AstNode" existe antes da struct terminar de ser definida, senão dá erro de tipo
desconhecido.
*/
typedef struct AstNode AstNode;

struct AstNode {
	AstKind kind; // diz qual "gaveta" do union abaixo é a válida para ler.
	int line; // linha do código-fonte onde esse nó apareceu, para mensagem de erro.

	/*
	CUIDADO: só o campo do union que corresponde ao `kind` acima é válido. 
	Ler `node->as.if_expr` num nó que na verdade é AST_LITERAL_INT é ler lixo de memória 
	(comportamento indefinido, não vai nem necessariamente dar erro visível, pode só devolver número aleatório). 
	Por isso toda função que mexe em AstNode* começa com switch (node->kind) antes de tocar em node->as.
	*/
	union {
		// literais, um valor direto cada.
		long long int_value;
		double float_value;
		bool bool_value;
		char* string_value; // serve para AST_LITERAL_STRING e AST_IDENTIFIER (nome da variável).

		// operações.
		struct { AstNode* left; AstNode* right; BinOp op; } binary_op;
		struct { AstNode* operand; UnOp op; } unary_op;
		struct { AstNode* target; AstNode* value; } assignment; // target = value.

		// controle de fluxo.
		struct { AstNode* condition; AstNode* then_block; AstNode* else_block; } if_expr; // else_block pode ser NULL (if sem else).
		struct { AstNode* condition; AstNode* body; } while_stmt;
		struct { AstNode* init; AstNode* condition; AstNode* increment; AstNode* body; } for_stmt; // C-style -> for (init; condition; increment).
		struct { LinkedList* statements; } block; // lista de AstNode*, um por statement dentro do bloco.

		// declaração.
		struct { char* name; char* type_name; bool is_mut; AstNode* value; } let_decl; // type_name pode ser NULL = tipo inferido do value.
		struct { char* name; LinkedList* params; char* return_type; AstNode* body; } fn_decl; // params é lista de AstParam* (ver mais abaixo). return_type pode ser NULL = retorna unit.

		// chamada e retorno.
		struct { char* callee_name; LinkedList* args; } call; // args é lista de AstNode* (expressões).
		struct { AstNode* value; } return_stmt; // value pode ser NULL, "return;" sem valor.

		// AST_BREAK e AST_CONTINUE não têm entrada aqui, não precisam de dado nenhum além do próprio kind, então não ocupam espaço.
	} as;
};

/*
Parâmetro de função (o "a: int" dentro de `fn soma(a: int, b: int)`).
Não é AstNode, não é uma expressão, não produz valor, não tem kind. 
É só nome + tipo, um tipinho à parte. fn_decl.params (acima) guarda uma LinkedList de AstParam*, um por parâmetro da função.
*/
typedef struct {
	char* name;
	char* type_name;
} AstParam;

AstParam* ast_param_create(const char* name, const char* type_name);
void ast_param_destroy(AstParam* param); // libera name, type_name e o próprio AstParam.

/*
Constructors —> uma por kind. Todas seguem o mesmo padrão:
- recebem `line` primeiro, para preencher o campo line da struct.
- alocam o AstNode, preenchem o kind certo e os campos daquele kind.
- devolvem NULL se a alocação falhar em algum ponto (malloc do nó em si, ou malloc de alguma string que precisou ser copiada)
- string recebida é `const char*` (a construtora só lê, não guarda o ponteiro recebido). 
O que fica salvo dentro do nó é uma CÓPIA própria, alocada por dentro da construtora. 
Isso é o que permite o parser passar um texto temporário e esquecer dele depois.

Parâmetro que pode legitimamente vir NULL (C não marca isso no tipo, então fica só documentado aqui mesmo):
- ast_new_if: else_block (if sem else).
- ast_new_return: value (return sem valor).
- ast_new_let_decl: type_name (tipo inferido, sem anotação).
- ast_new_fn_decl: return_type (função sem "-> tipo", retorna unit).
*/
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

/*
ast_destroy -> desce a árvore inteira recursivamente e libera tudo. 
(Cada nó filho antes do próprio nó pai, cada string copiada, cada lista (block.statements, fn_decl.params, call.args)). 
Chamar isso na raiz da árvore libera a árvore inteira de uma vez. ast_destroy(NULL) não faz nada (checagem logo na entrada), 
o que cobre de graça os campos opcionais tipo else_block/return_stmt.value quando são NULL.

ast_print -> imprime a árvore inteira no stdout, indentada por profundidade 
(depth começa em 0 na raiz, +1 a cada nível pra baixo).
Serve pra testar o PARSER sozinho, sem precisar do interpretador ainda, roda o parser, 
chama ast_print na árvore que ele produziu, olha se a forma impressa bate com o que era esperado para aquele trecho de código.
*/
void ast_destroy(AstNode* node); 
void ast_print(const AstNode* node, int depth);
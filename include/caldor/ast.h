#pragma once

#include <stdbool.h>
#include <libcds/linked_list.h>

// fase 1 do roadmap só, sem struct/enum/match/genérico/array/referência ainda.
typedef enum {
	AST_LITERAL_INT,
	AST_LITERAL_FLOAT,
	AST_LITERAL_BOOL,
	AST_LITERAL_STRING,

	AST_IDENTIFIER,

	AST_BINARY_OP,
	AST_UNARY_OP,
	AST_ASSIGNMENT,

	AST_IF, // statement e expression são o mesmo kind, muda só o contexto na árvore.
	AST_WHILE,
	AST_FOR,
	AST_BLOCK,

	AST_LET_DECL,
	AST_FN_DECL,

	AST_CALL,
	AST_RETURN,
	AST_BREAK, // sem dado nenhum associado, não entra no union lá embaixo.
	AST_CONTINUE, // mesma coisa.
} AstKind;

typedef enum {
	OP_ADD, OP_SUB, OP_MUL, OP_DIV, OP_MOD, OP_POW, 
	OP_EQ, OP_NEQ, OP_LT, OP_GT, OP_LTE, OP_GTE,
	OP_AND, OP_OR,
	OP_BIT_AND, OP_BIT_OR, OP_BIT_XOR, OP_SHL, OP_SHR,
} BinOp;

typedef enum {
	OP_NEG,
	OP_NOT,
	OP_BIT_NOT
} UnOp;

typedef struct AstNode AstNode; // nó aponta para nó, precisa existir antes de terminar de se definir.

struct AstNode {
	AstKind kind; // decide qual campo do union abaixo é válido, todo o resto é lixo de memória.
	int line; // linha do código-fonte, para mensagem de erro.

	union {
		long long int_value;
		double float_value;
		bool bool_value;
		char* string_value; // serve para AST_LITERAL_STRING e AST_IDENTIFIER (nome).

		struct { AstNode* left; AstNode* right; BinOp op; } binary_op;
		struct { AstNode* operand; UnOp op; } unary_op;
		struct { AstNode* target; AstNode* value; } assignment;

		struct { AstNode* condition; AstNode* then_block; AstNode* else_block; } if_expr; // else_block pode ser NULL.
		struct { AstNode* condition; AstNode* body; } while_stmt;
		struct { AstNode* init; AstNode* condition; AstNode* increment; AstNode* body; } for_stmt;
		struct { LinkedList* statements; } block;

		struct { char* name; char* type_name; bool is_mut; AstNode* value; } let_decl; // type_name pode ser NULL, tipo inferido.
		struct { char* name; LinkedList* params; char* return_type; AstNode* body; } fn_decl;

		struct { char* callee_name; LinkedList* args; } call;
		struct { AstNode* value; } return_stmt; // value pode ser NULL, return sem valor.
	} as;
};

// Todas recebem line primeiro para preencher o campo acima. Parâmetro de string é const char* (só leitura), o nó guarda cópia própria dela.
AstNode* ast_new_literal_int(int line, long long value);
AstNode* ast_new_literal_float(int line, double value);
AstNode* ast_new_literal_bool(int line, bool value);
AstNode* ast_new_literal_string(int line, const char* value);

AstNode* ast_new_identifier(int line, const char* name);

AstNode* ast_new_binary_op(int line, AstNode* left, BinOp op, AstNode* right);
AstNode* ast_new_unary_op(int line, UnOp op, AstNode* operand);
AstNode* ast_new_assignment(int line, AstNode* target, AstNode* value);

AstNode* ast_new_if(int line, AstNode* condition, AstNode* then_block, AstNode* else_block);
AstNode* ast_new_while(int line, AstNode* condition, AstNode* value);
AstNode* ast_new_for(int line, AstNode* init, AstNode* condition, AstNode* increment, AstNode* body);
AstNode* ast_new_block(int line, LinkedList* statements);

AstNode* ast_new_let_decl(int line, const char* name, const char* type_name, bool is_mut, AstNode* value);
AstNode* ast_new_fn_decl(int line, const char* name, LinkedList* params, const char* return_type, AstNode* body);

AstNode* ast_new_call(int line, const char* callee_name, LinkedList* args);
AstNode* ast_new_return(int line, AstNode* value);
AstNode* ast_new_break(int line);
AstNode* ast_new_continue(int line);

void ast_destroy(AstNode* node); // desce a árvore, libera filho antes do pai, libera string copiada também.
void ast_print(const AstNode* node, int depth); // indentado por profundidade, para testar o parser sem interpretador.
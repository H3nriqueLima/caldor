#include <caldor/ast.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Dono da cópia é sempre o nó (ou o AstParam) que chamou isso, nunca quem passou o ponteiro original.
static char* dup_string(const char* s) {
	if (!s) return NULL;

	size_t len = strlen(s) + 1;
	char* copy = malloc(len);
	if (copy) memcpy(copy, s, len);

	return copy;
}

// Constructors.

AstNode* ast_new_literal_int(int line, long long value) {
	AstNode* node = malloc(sizeof(AstNode));
	if (!node) return NULL;

	node->kind = AST_LITERAL_INT;
	node->line = line;
	node->as.int_value = value;

	return node;
}

AstNode* ast_new_literal_float(int line, double value) {
	AstNode* node = malloc(sizeof(AstNode));
	if (!node) return NULL;

	node->kind = AST_LITERAL_FLOAT;
	node->line = line;
	node->as.float_value = value;

	return node;
}

AstNode* ast_new_literal_bool(int line, bool value) {
	AstNode* node = malloc(sizeof(AstNode));
	if (!node) return NULL;

	node->kind = AST_LITERAL_BOOL;
	node->line = line;
	node->as.bool_value = value;

	return node;
}

AstNode* ast_new_literal_string(int line, const char* value) {
	AstNode* node = malloc(sizeof(AstNode));
	if (!node) return NULL;

	node->kind = AST_LITERAL_STRING;
	node->line = line;
	node->as.string_value = value;
	if (!node->as.string_value) {
		free(node);

		return NULL;
	}

	return node;
}

AstNode* ast_new_identifier(int line, const char* name) {
	AstNode* node = malloc(sizeof(AstNode));
	if (!node) return NULL;

	node->kind = AST_IDENTIFIER;
	node->line = line;
	node->as.string_value = dup_string(name);
	if (!node->as.string_value) {
		free(node);

		return NULL;
	}

	return node;
}

AstNode* ast_new_binary_op(int line, AstNode* left, BinOp op, AstNode* right) {
	AstNode* node = malloc(sizeof(AstNode));
	if (!node) return NULL;

	node->kind = AST_BINARY_OP;
	node->line = line;
	node->as.binary_op.left = left;
	node->as.binary_op.op = op;
	node->as.binary_op.right = right;

	return node;
}

AstNode* ast_new_unary_op(int line, UnOp op, AstNode* operand) {
	AstNode* node = malloc(sizeof(AstNode));
	if (!node) return NULL;

	node->kind = AST_UNARY_OP;
	node->line = line;
	node->as.unary_op.operand = operand;
	node->as.unary_op.op = op;

	return node;
}

AstNode* ast_new_assignment(int line, AstNode* target, AstNode* value) {
	AstNode* node = malloc(sizeof(AstNode));
	if (!node) return NULL;

	node->kind = AST_ASSIGNMENT;
	node->line = line;
	node->as.assignment.target = target;
	node->as.assignment.value = value;

	return node;
}

AstNode* ast_new_if(int line, AstNode* condition, AstNode* then_block, AstNode* else_block) {
	AstNode* node = malloc(sizeof(AstNode));
	if (!node) return NULL;

	node->kind = AST_IF;
	node->line = line;
	node->as.if_expr.condition = condition;
	node->as.if_expr.then_block = then_block;
	node->as.if_expr.else_block = else_block; // NULL aqui já cobre if sem else.

	return node;
}

AstNode* ast_new_while(int line, AstNode* condition, AstNode* body) {
	AstNode* node = malloc(sizeof(AstNode));
	if (!node) return NULL;

	node->kind = AST_WHILE;
	node->line = line;
	node->as.while_stmt.condition = condition;
	node->as.while_stmt.body = body;

	return node;
}

AstNode* ast_new_for(int line, AstNode* init, AstNode* condition, AstNode* increment, AstNode* body) {
	AstNode* node = malloc(sizeof(AstNode));
	if (!node) return NULL;

	node->kind = AST_FOR;
	node->line = line;
	node->as.for_stmt.init = init;
	node->as.for_stmt.condition = condition;
	node->as.for_stmt.increment = increment;
	node->as.for_stmt.body = body;

	return node;
}

AstNode* ast_new_block(int line, LinkedList* statements) {
	AstNode* node = malloc(sizeof(AstNode));
	if (!node) return NULL;

	node->kind = AST_BLOCK;
	node->line = line;
	node->as.block.statements = statements;

	return node;
}

AstNode* ast_new_let_decl(int line, const char* name, const char* type_name, bool is_mut, AstNode* value) {
	AstNode* node = malloc(sizeof(AstNode));
	if (!node) return NULL;

	node->kind = AST_LET_DECL;
	node->line = line;

	node->as.let_decl.name = dup_string(name);
	if (!node->as.let_decl.name) {
		free(node);

		return NULL;
	}

	node->as.let_decl.type_name = dup_string(type_name); // dup_string(NULL) já devolve NULLm cobre "tipo inferido".
	if (type_name && !node->as.let_decl.type_name) {
		free(node->as.let_decl.name);
		free(node);

		return NULL;
	}

	node->as.let_decl.is_mut = is_mut;
	node->as.let_decl.value = value;

	return node;
}

AstNode* ast_new_fn_decl(int line, const char* name, LinkedList* params, const char* return_type, AstNode* body) {
	AstNode* node = malloc(sizeof(AstNode));
	if (!node) return NULL;

	node->kind = AST_FN_DECL;
	node->line = line;

	node->as.fn_decl.name = dup_string(name);
	if (!node->as.fn_decl.name) {
		free(node);

		return NULL;
	}

	node->as.fn_decl.return_type = dup_string(return_type); // NULL aqui = fn sem "-> type", retorna unit.
	if (return_type && !node->as.fn_decl.return_type) {
		free(node->as.fn_decl.name);
		free(node);

		return NULL;
	}

	node->as.fn_decl.params = params;
	node->as.fn_decl.body = body;

	return node;
}

AstNode* ast_new_call(int line, const char* callee_name, LinkedList* args) {
	AstNode* node = malloc(sizeof(AstNode));
	if (!node) return NULL;

	node->kind = AST_CALL;
	node->line = line;

	node->as.call.callee_name = dup_string(callee_name);
	if (!node->as.call.callee_name) { 
		free(node); 

		return NULL; 
	}

	node->as.call.args = args;

	return node;
}

AstNode* ast_new_return(int line, AstNode* value) {
	AstNode* node = malloc(sizeof(AstNode));
	if (!node) return NULL;

	node->kind = AST_RETURN;
	node->line = line;
	node->as.return_stmt.value = value;

	return node;
}

AstNode* ast_new_break(int line) {
	AstNode* node = malloc(sizeof(AstNode));
	if (!node) return NULL;

	node->kind = AST_BREAK;
	node->line = line;

	return node;
}

AstNode* ast_new_continue(int line) {
	AstNode* node = malloc(sizeof(AstNode));
	if (!node) return NULL;

	node->kind = AST_CONTINUE;
	node->line = line;

	return node;
}

AstParam* ast_param_create(const char* name, const char* type_name) {
	AstParam* param = malloc(sizeof(AstParam));
	if (!param) return NULL;

	param->name = dup_string(name);
	if (!param->name) { 
		free(param); 
		
		return NULL; 
	}

	param->type_name = dup_string(type_name);
	if (type_name && !param->type_name) {
		free(param->name);
		free(param);

		return NULL;
	}

	return param;
}

void ast_param_destroy(AstParam* param) {
	if (!param) return;

	free(param->name);
	free(param->type_name);
	free(param);
}

// Destructors.

void ast_destroy(AstNode* node) {
	if (!node) return;

	switch (node->kind) {
		case AST_LITERAL_INT:
		case AST_LITERAL_FLOAT:
		case AST_LITERAL_BOOL:
		case AST_BREAK:
		case AST_CONTINUE:
			break; // nada além do próprio node para liberar.

		case AST_LITERAL_STRING:
		case AST_IDENTIFIER:
			free(node->as.string_value);
			break;

		case AST_BINARY_OP:
			ast_destroy(node->as.binary_op.left);
			ast_destroy(node->as.binary_op.right);
			break;

		case AST_UNARY_OP:
			ast_destroy(node->as.unary_op.operand);
			break;

		case AST_ASSIGNMENT:
			ast_destroy(node->as.assignment.target);
			ast_destroy(node->as.assignment.value);
			break;

		case AST_IF:
			ast_destroy(node->as.if_expr.condition);
			ast_destroy(node->as.if_expr.then_block);
			ast_destroy(node->as.if_expr.else_block); // ast_destroy(NULL) não faz nada, if sem else cai aqui.
			break;

		case AST_WHILE:
			ast_destroy(node->as.while_stmt.condition);
			ast_destroy(node->as.while_stmt.body);
			break;

		case AST_FOR:
			ast_destroy(node->as.for_stmt.init);
			ast_destroy(node->as.for_stmt.condition);
			ast_destroy(node->as.for_stmt.increment);
			ast_destroy(node->as.for_stmt.body);
			break;

		case AST_BLOCK:
			if (node->as.block.statements) {
				ll_destroy_with_values(node->as.block.statements, (LLFreeValueFn)ast_destroy);
			}
			break;

		case AST_LET_DECL:
			free(node->as.let_decl.name);
			free(node->as.let_decl.type_name); // free(NULL) é no-op, cobre "sem anotação".
			ast_destroy(node->as.let_decl.value);
			break;

		case AST_FN_DECL:
			free(node->as.fn_decl.name);
			free(node->as.fn_decl.return_type);
			if (node->as.fn_decl.params) {
				ll_destroy_with_values(node->as.fn_decl.params, (LLFreeValueFn)ast_param_destroy);
			}
			ast_destroy(node->as.fn_decl.body);
			break;

		case AST_CALL:
			free(node->as.call.callee_name);
			if (node->as.call.args) {
				ll_destroy_with_values(node->as.call.args, (LLFreeValueFn)ast_destroy);
			}
			break;

		case AST_RETURN:
			ast_destroy(node->as.return_stmt.value); // NULL quando é "return;" sem valor.
			break;
	}

	free(node);
}

// Pretty-print.

static void print_indent(int depth) {
	for (int i = 0; i < depth; i++) {
		printf("  ");
	}
}

static const char* binop_to_string(BinOp op) {
	switch (op) {
		case OP_ADD: return "+";
		case OP_SUB: return "-";
		case OP_MUL: return "*";
		case OP_DIV: return "/";
		case OP_MOD: return "%";
		case OP_POW: return "^";
		case OP_EQ: return "==";
		case OP_NEQ: return "!=";
		case OP_LT: return "<";
		case OP_GT: return ">";
		case OP_LTE: return "<=";
		case OP_GTE: return ">=";
		case OP_AND: return "and";
		case OP_OR: return "or";
		case OP_BIT_AND: return "&";
		case OP_BIT_OR: return "|";
		case OP_BIT_XOR: return "xor";
		case OP_SHL: return "<<";
		case OP_SHR: return ">>";
	}

	return "?";
}

static const char* unop_to_string(UnOp op) {
	switch (op) {
		case OP_NEG: return "-";
		case OP_NOT: return "not";
		case OP_BIT_NOT: return "~";
	}

	return "?";
}

// Contexto para o ll_for_each. Callback só recebe (valor, context), depth precisa vir empacotado.
typedef struct {
	int depth;
} PrintCtx;

static void print_stmt_cb(void* value, void* context) {
	PrintCtx* ctx = context;
	ast_print((AstNode*)value, ctx->depth);
}

static void print_param_cb(void* value, void* context) {
	AstParam* param = value;
	PrintCtx* ctx = context;

	print_indent(ctx->depth);
	printf("param %s: %s\n", param->name, param->type_name ? param->type_name : "?");
}

void ast_print(const AstNode* node, int depth) {
	if (!node) return;
	print_indent(depth);

	switch (node->kind) {
		case AST_LITERAL_INT:
			printf("literal int %lld (linha %d)\n", node->as.int_value, node->line);
			break;

		case AST_LITERAL_FLOAT:
			printf("literal float %g (linha %d)\n", node->as.float_value, node->line);
			break;

		case AST_LITERAL_BOOL:
			printf("literal bool %s (linha %d)\n", node->as.bool_value ? "true" : "false", node->line);
			break;

		case AST_LITERAL_STRING:
			printf("literal string \"%s\" (linha %d)\n", node->as.string_value, node->line);
			break;

		case AST_IDENTIFIER:
			printf("identifier %s (linha %d)\n", node->as.string_value, node->line);
			break;

		case AST_BINARY_OP:
			printf("binary_op %s (linha %d)\n", binop_to_string(node->as.binary_op.op), node->line);
			ast_print(node->as.binary_op.left, depth + 1);
			ast_print(node->as.binary_op.right, depth + 1);
			break;

		case AST_UNARY_OP:
			printf("unary_op %s (linha %d)\n", unop_to_string(node->as.unary_op.op), node->line);
			ast_print(node->as.unary_op.operand, depth + 1);
			break;

		case AST_ASSIGNMENT:
			printf("assignment (linha %d)\n", node->line);
			ast_print(node->as.assignment.target, depth + 1);
			ast_print(node->as.assignment.value, depth + 1);
			break;

		case AST_IF:
			printf("if (linha %d)\n", node->line);
			ast_print(node->as.if_expr.condition, depth + 1);
			ast_print(node->as.if_expr.then_block, depth + 1);
			if (node->as.if_expr.else_block) {
				ast_print(node->as.if_expr.else_block, depth + 1);
			}
			break;

		case AST_WHILE:
			printf("while (linha %d)\n", node->line);
			ast_print(node->as.while_stmt.condition, depth + 1);
			ast_print(node->as.while_stmt.body, depth + 1);
			break;

		case AST_FOR:
			printf("for (linha %d)\n", node->line);
			ast_print(node->as.for_stmt.init, depth + 1);
			ast_print(node->as.for_stmt.condition, depth + 1);
			ast_print(node->as.for_stmt.increment, depth + 1);
			ast_print(node->as.for_stmt.body, depth + 1);
			break;

		case AST_BLOCK: {
			printf("block (linha %d)\n", node->line);
			PrintCtx ctx = { depth + 1 };

			if (node->as.block.statements) {
				ll_for_each(node->as.block.statements, print_stmt_cb, &ctx);
			}
			break;
		}

		case AST_LET_DECL:
			printf("let%s %s%s%s (linha %d)\n",
				node->as.let_decl.is_mut ? " mut" : "",
				node->as.let_decl.name,
				node->as.let_decl.type_name ? ": " : "",
				node->as.let_decl.type_name ? node->as.let_decl.type_name : "",
				node->line);
			ast_print(node->as.let_decl.value, depth + 1);
			break;

		case AST_FN_DECL: {
			printf("fn %s (linha %d)\n", node->as.fn_decl.name, node->line);

			if (node->as.fn_decl.params) {
				PrintCtx ctx = { depth + 1 };
				ll_for_each(node->as.fn_decl.params, print_param_cb, &ctx);
			}

			ast_print(node->as.fn_decl.body, depth + 1);
			break;
		}

		case AST_CALL: {
			printf("call %s (linha %d)\n", node->as.call.callee_name, node->line);
			PrintCtx ctx = { depth + 1 };

			if (node->as.call.args) {
				ll_for_each(node->as.call.args, print_stmt_cb, &ctx);
			}
			break;
		}

		case AST_RETURN:
			printf("return (linha %d)\n", node->line);

			if (node->as.return_stmt.value) {
				ast_print(node->as.return_stmt.value, depth + 1);
			}
			break;

		case AST_BREAK:
			printf("break (linha %d)\n", node->line);
			break;

		case AST_CONTINUE:
			printf("continue (linha %d)\n", node->line);
			break;
		}
}
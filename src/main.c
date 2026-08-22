#include "caldor/ast.h"
#include "caldor/token.h"

#include <stdio.h>

/*
main.c provisório. Não é o main.c final do roadmap (esse vai ler arquivo .cldr e encadear lexer -> parser -> interpretador, 
nenhum dos três existe ainda). 
Esse aqui só monta uma AST na mão, para confirmar que ast.c, token.c e util.c compilam e funcionam juntos antes de escrever 
o resto do front-end.
 */
int main(void) {
	// monta a árvore de "1 + 2 * 3" na mão, sem lexer nem parser.
	AstNode* dois = ast_new_literal_int(1, 2);
	AstNode* tres = ast_new_literal_int(1, 3);
	AstNode* mul = ast_new_binary_op(1, dois, OP_MUL, tres);
	AstNode* um = ast_new_literal_int(1, 1);
	AstNode* soma = ast_new_binary_op(1, um, OP_ADD, mul);

	printf("ast_print: 1 + 2 * 3\n");
	ast_print(soma, 0);

	ast_destroy(soma); // libera a árvore inteira recursivamente, uma chamada só.

	printf("\ntoken_type_to_string\n");
	printf("%s\n", token_type_to_string(TOKEN_PLUS));
	printf("%s\n", token_type_to_string(TOKEN_IDENTIFIER));

	printf("\ntoken_create / token_destroy\n");
	TokenValue value;
	value.int_value = 42;

	Token tok = token_create(TOKEN_INT, 1, "42", value);
	printf("token %s, lexeme=%s, valor=%lld\n", token_type_to_string(tok.type), tok.lexeme, tok.value.int_value);
	token_destroy(&tok);

	printf("\nbuild ok\n");

	return 0;
}
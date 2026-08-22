#include <caldor/token.h>
#include <caldor/util.h>

#include <stdlib.h>
#include <string.h>

/*
token_create: monta um Token e devolve por VALOR (não por ponteiro, diferente das constructors de AstNode no ast.c). 
Isso segue a decisão de lexer sob demanda -> cada Token é pequeno, não precisa viver no heap, só o lexeme de dentro 
dele precisa.

`value` é passado pronto de fora -> quem chama decide se aquele token é numérico ou não. 
Para token que não é TOKEN_INT/TOKEN_FLOAT, passa um TokenValue zerado (tipo (TokenValue){0}), 
o campo simplesmente não significa nada nesses casos e nunca deveria ser lido.

GAP EM ABERTO: dup_string(lexeme) pode falhar (malloc sem memória) e devolver NULL. Como Token volta por valor, 
não tem um jeito de "essa função falhou, devolve NULL" igual as constructors do ast.c fazem com AstNode*, 
um Token sempre "existe", só que com lexeme == NULL nesse caso de falha. Ninguém trata esse caso ainda porque o lexer
(que é quem chama isso) também não foi escrito ainda. Fica registrado aqui para não esquecer, quando o lexer existir, 
decide se isso vira um "token de erro" (TOKEN_ERROR) devolvido no lugar, ou outra saída.
*/
Token token_create(TokenType type, int line, const char* lexeme, TokenValue value) {
	Token token;
	token.type = type;
	token.line = line;
	token.lexeme = dup_string(lexeme);
	token.value = value;

	return token;
}

/*
token_destroy: libera só o lexeme copiado -> o Token em si nunca foi alocado no heap (foi devolvido por valor lá em cima),
então não tem "free(token)" aqui, só free do que tinha dentro dele. Zera o ponteiro depois de liberar por precaução, 
se alguém chamar token_destroy duas vezes no mesmo Token por engano, a segunda chamada vira free(NULL), que não faz nada, 
em vez de free duplo (que é comportamento indefinido de verdade, esse sim é perigoso).
*/
void token_destroy(Token* token) {
	if (!token) return;

	free(token->lexeme);
	token->lexeme = NULL;
}

/*
token_type_to_string: só serve para debug e mensagem de erro do parser mais adiante (tipo "esperava ';', encontrei IDENTIFIER" 
em vez de "esperava ';', encontrei 7", sem isso só teria o número cru do enum, ilegível). 
Sem "default:" no switch, igual o padrão já usado no ast.c -> se um token novo entrar no TokenType e essa função esquecer de
tratar ele aqui, o compilador avisa que faltou case.
*/
const char* token_type_to_string(TokenType type) {
	switch (type) {
		case TOKEN_INT: return "INT";
		case TOKEN_FLOAT: return "FLOAT";
		case TOKEN_STRING: return "STRING";
		case TOKEN_TRUE: return "TRUE";
		case TOKEN_FALSE: return "FALSE";
		case TOKEN_NULL: return "NULL";

		case TOKEN_IDENTIFIER: return "IDENTIFIER";

		case TOKEN_LET: return "LET";
		case TOKEN_MUT: return "MUT";
		case TOKEN_FN: return "FN";
		case TOKEN_STRUCT: return "STRUCT";
		case TOKEN_TRAIT: return "TRAIT";
		case TOKEN_IMPL: return "IMPL";
		case TOKEN_ENUM: return "ENUM";

		case TOKEN_IF: return "IF";
		case TOKEN_ELSE: return "ELSE";
		case TOKEN_WHILE: return "WHILE";
		case TOKEN_FOR: return "FOR";
		case TOKEN_RETURN: return "RETURN";
		case TOKEN_BREAK: return "BREAK";
		case TOKEN_CONTINUE: return "CONTINUE";
		case TOKEN_MATCH: return "MATCH";

		case TOKEN_AND: return "AND";
		case TOKEN_OR: return "OR";
		case TOKEN_NOT: return "NOT";

		case TOKEN_XOR: return "XOR";

		case TOKEN_AS: return "AS";

		case TOKEN_SAFE: return "SAFE";
		case TOKEN_UNSAFE: return "UNSAFE";

		case TOKEN_PLUS: return "+";
		case TOKEN_MINUS: return "-";
		case TOKEN_STAR: return "*";
		case TOKEN_SLASH: return "/";
		case TOKEN_PERCENT: return "%";
		case TOKEN_CARET: return "^";

		case TOKEN_EQ_EQ: return "==";
		case TOKEN_BANG_EQ: return "!=";
		case TOKEN_LT: return "<";
		case TOKEN_GT: return ">";
		case TOKEN_LT_EQ: return "<=";
		case TOKEN_GT_EQ: return ">=";

		case TOKEN_AMP: return "&";
		case TOKEN_PIPE: return "|";
		case TOKEN_TILDE: return "~";
		case TOKEN_LT_LT: return "<<";
		case TOKEN_GT_GT: return ">>";

		case TOKEN_EQ: return "=";

		case TOKEN_LPAREN: return "(";
		case TOKEN_RPAREN: return ")";
		case TOKEN_LBRACE: return "{";
		case TOKEN_RBRACE: return "}";
		case TOKEN_LBRACKET: return "[";
		case TOKEN_RBRACKET: return "]";

		case TOKEN_COMMA: return ",";
		case TOKEN_SEMICOLON: return ";";
		case TOKEN_COLON: return ":";

		case TOKEN_DOT: return ".";

		case TOKEN_ARROW: return "->";
		case TOKEN_FAT_ARROW: return "=>";

		case TOKEN_EOF: return "EOF";
		case TOKEN_ERROR: return "ERROR";
	}

	return "?"; // nunca deveria chegar aqui se todo TokenType foi coberto acima.
}
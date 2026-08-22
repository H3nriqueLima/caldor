#pragma once

#include <stdbool.h>

/*
Vocabulário do lexer -> todo tipo de token que o texto-fonte do CaldOR pode virar, mais a struct que representa um token
individual já reconhecido.

Diferente do ast.h (que só cobre o item 1 do roadmap), esse arquivo cobre o 01-lexical-grammar.md inteiro de uma vez, 
inclusive palavra reservada que o parser ainda não sabe tratar (struct, enum, match...). 
Motivo: reconhecer um token não exige saber o que fazer com ele depois. O lexer só devolve o token; 
quem decide "ainda não trato isso" é o parser, mais adiante. Cobrir tudo agora evita ter que voltar no lexer toda vez que 
um item novo do roadmap chegar.

Um valor por tipo de token reconhecível. Agrupado na mesma ordem das tabelas do 01-lexical-grammar.md, para facilitar 
comparar os dois lado a lado se precisar conferir se falta alguma coisa.
*/
typedef enum {
	/*
	literais -> token cuja "identidade" já é o próprio valor. 
	TOKEN_INT e TOKEN_FLOAT ainda carregam um valor numérico de verdade dentro do Token (ver TokenValue mais abaixo). 
	TOKEN_TRUE/FALSE/NULL não precisam de valor associado, o tipo do token já diz tudo.
	*/
	TOKEN_INT,
	TOKEN_FLOAT,
	TOKEN_STRING,
	TOKEN_TRUE,
	TOKEN_FALSE,
	TOKEN_NULL,

	/*
	uso de nome -> variável, função, tipo, o que for. O lexer não sabe (nem precisa saber) se aquele nome já foi declarado 
	antes, só reconhece que é um identificador válido.
	*/
	TOKEN_IDENTIFIER,

	// declaração.
	TOKEN_LET,
	TOKEN_MUT,
	TOKEN_FN,
	TOKEN_STRUCT,
	TOKEN_TRAIT,
	TOKEN_IMPL,
	TOKEN_ENUM,
	
	// controle de fluxo.
	TOKEN_IF,
	TOKEN_ELSE,
	TOKEN_WHILE,
	TOKEN_FOR,
	TOKEN_RETURN,
	TOKEN_BREAK,
	TOKEN_CONTINUE,
	TOKEN_MATCH,

	/*
	lógico, como palavra reservada (and/or/not), não símbolo (&&/||/!) -> é essa escolha que deixa & e | livres para bit a bit
	mais abaixo, sem colisão nenhuma entre os dois usos.
	*/
	TOKEN_AND,
	TOKEN_OR,
	TOKEN_NOT,

	/*
	bit a bit como palavra -> só o xor. O resto do bit a bit (& | ~ << >>) é símbolo, não palavra reservada. 
	xor precisou virar palavra porque o símbolo ^ já tava ocupado pelo operador de potência.
	*/
	TOKEN_XOR,

	// conversão explícita de tipo, "expr as Type".
	TOKEN_AS,

	// flag de compilação safe/unsafe -> memória manual vs GC opcional.
	TOKEN_SAFE,
	TOKEN_UNSAFE,

	// aritmético.
	TOKEN_PLUS,
	TOKEN_MINUS,
	TOKEN_STAR,
	TOKEN_SLASH,
	TOKEN_PERCENT,
	TOKEN_CARET, // ^ é potência aqui, não XOR (reforçando o motivo do TOKEN_XOR acima.

	// comparação.
	TOKEN_EQ_EQ,
	TOKEN_BANG_EQ,
	TOKEN_LT,
	TOKEN_GT,
	TOKEN_LT_EQ,
	TOKEN_GT_EQ,

	/*
	bit a bit, como símbolo. & e * também têm um segundo significado fora daqui (referência e desreferência, "&expr"/"*expr"), 
	resolvido por posição no parser, não pelo lexer. Para o lexer, & sempre vira esse TOKEN_AMP aqui, é o parser que decide o
	que aquele token significa dependendo de onde ele apareceu.
	*/
	TOKEN_AMP,
	TOKEN_PIPE,
	TOKEN_TILDE,
	TOKEN_LT_LT,
	TOKEN_GT_GT,

	// atribuição, "a = expr". Não confundir com TOKEN_EQ_EQ (comparação).
	TOKEN_EQ,

	// agrupamento.
	TOKEN_LPAREN,
	TOKEN_RPAREN,
	TOKEN_LBRACE,
	TOKEN_RBRACE,
	TOKEN_LBRACKET,
	TOKEN_RBRACKET,

	// separador.
	TOKEN_COMMA,
	TOKEN_SEMICOLON,
	TOKEN_COLON,

	// acesso a campo/método, "a.campo". Também usado para literal de enum, tipo "Option.Some(5)". Mesmo token, contexto diferente decide.
	TOKEN_DOT,

	/*
	GAP que só apareceu escrevendo esse arquivo: "->" e "=>" nunca ganharam entrada formal na tabela do 01-lexical-grammar.md,
	só apareceram implícitos no 02-syntax.md (fn_decl usa "->" para o tipo de retorno, match_arm usa "=>"). 
	Documentados aqui, mas o 01-lexical-grammar.md ainda não foi atualizado com eles, fica como pendência registrada no
	architecture.md.
	*/
	TOKEN_ARROW, // ->
	TOKEN_FAT_ARROW, // =>

	// especiais, não vêm do 01-lexical-grammar.md. São invenção do lexer, não do texto-fonte em si.
	TOKEN_EOF, // fim do texto. Sem isso, o parser não tem como saber quando parar de pedir "me dá o próximo token".

	TOKEN_ERROR // caractere que não bate com regra nenhuma (tipo '@' solto, ou string sem fechar aspas), em vez do lexer travar o programa inteiro, ele devolve isso, e quem recebe decide como reportar.
} TokenType;

/*
Guarda o valor já convertido de um literal numérico. Separado da struct Token de propósito, assim token_create pode ser uma
função só (em vez de uma por tipo, como as constructors do ast.c), quem chama passa um TokenValue zerado para qualquer token 
que não seja TOKEN_INT/TOKEN_FLOAT, e o campo simplesmente não significa nada nesses casos (nunca deveria ser lido).

O motivo de o valor já vir convertido (em vez do lexer só devolver o texto "123" e deixar o parser converter depois): 
o lexer já andou dígito por dígito reconhecendo aquilo como número, seria retrabalho bobo o parser refazer essa conversão 
de novo.
*/
typedef union {
	long long int_value; // só significa algo quando type == TOKEN_INT.
	double float_value; // só significa algo quando type == TOKEN_FLOAT.
} TokenValue;

/*
Um token já reconhecido pelo lexer. Diferente de AstNode (que é sempre ponteiro, alocado no heap), Token é pensado para ser 
um valor pequeno, comum passar/devolver por cópia direta. Só o `lexeme` de dentro dele vive no heap, o resto (type, line,
value) é dado simples.
*/
typedef struct {
	TokenType type;
	int line; // linha do código-fonte onde esse token apareceu.
	char* lexeme; // cópia do texto reconhecido ("123", "soma", "+"...). Não é  ponteiro para o meio do buffer do arquivo original, mesma lógica de posse de string que o ast.h já usa.
	TokenValue value; // Só importa se type for TOKEN_INT ou TOKEN_FLOAT.
} Token;

/*
token_create: monta e devolve um Token POR VALOR (não por ponteiro). `value` é passado pronto por quem chama, 
para token que não é numérico, passa qualquer TokenValue zerado, tipo (TokenValue){0}.

Internamente copia `lexeme` (não guarda o ponteiro recebido) mas atenção: se essa cópia falhar por falta de memória, 
o Token ainda assim "existe" com lexeme == NULL, sem sinalizar que aquilo foi uma falha (diferente das constructors de
AstNode*, que devolvem NULL quando dá errado, não tem "NULL" equivalente para um valor que não é ponteiro). 
Gap conhecido, documentado também no architecture.md, só resolve quando o lexer.c existir de verdade.
*/
Token token_create(TokenType type, int line, const char* lexeme, TokenValue value);

/*
token_destroy: libera só o `lexeme` copiado. Não libera o Token em si -> ele nunca foi alocado no heap (foi devolvido por 
valor lá em cima), só o texto de dentro dele precisa ser devolvido para o sistema.
*/
void token_destroy(Token* token);

/*
Traduz o enum para texto legível, usado em mensagem de erro do parser (tipo "esperava ';', encontrei IDENTIFIER") em vez 
de expor o número cru do enum, que não ajuda ninguém a debugar.
*/
const char* token_type_to_string(TokenType type);
#pragma once

#include <stddef.h>

/*
util.h -> funções pequenas usadas por mais de uma parte do projeto. 
Nasceu quando dup_string apareceu duplicada em ast.c e token.c ao mesmo tempo, antes disso, cada .c tinha a própria cópia, 
porque duplicar uma função de 6 linhas uma única vez não justificava criar um arquivo novo só pra isso. 
Na segunda duplicação, passou a valer a pena. Cresce do mesmo jeito, só quando algo pequeno se repetir de verdade em mais de 
um lugar, não se anota aqui por antecipação.
 
dup_string: aloca espaço novo (malloc) e copia o texto de `s` para dentro, incluindo o terminador nulo.
dup_string(NULL) devolve NULL direto, sem tentar alocar nada, usado de propósito em toda parte que tem campo opcional 
(tipo type_name de let_decl, ou lexeme de um token) para não precisar de if separado toda vez que for chamar.
Também devolve NULL se a alocação falhar de verdade (malloc sem memória disponível), quem chama e recebe NULL de volta precisa
saber diferenciar os dois casos: `s` era NULL desde o início (nada de errado, campo opcional vazio) vs `s` não era NULL mas 
faltou memória para copiar (erro de verdade, precisa ser tratado).
 */
char* dup_string(const char* s);
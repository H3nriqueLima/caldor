#include <caldor/util.h>

#include <stdlib.h>
#include <string.h>

char* dup_string(const char* s) {
	if (!s) return NULL;

	size_t len = strlen(s) + 1; // +1 para o terminador nulo.
	char* copy = malloc(len);
	if (copy) memcpy(copy, s, len);

	return copy; // NULL se malloc falhou, mesmo valor que dup_string(NULL) devolveria, ver aviso no util.h.
}
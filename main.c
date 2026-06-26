#include "parser.h"

void test_file(const char* fname){
	Token* tokens = lexical_analysis(fname);
	printf("testing file: %s...\n", fname);
	Token* p = tokens;
	while(p != NULL && p->lexeme != NULL){
		printf("Token: %s, Type: %s\n", p->lexeme, stringFromToken(p->type));
		free(p->lexeme);
		p++;
	}
	// free(tokens);
}

int main(){
	test_file("./test.json");
	test_file("./tests/test1.json");
	test_file("./tests/test2.json");
	test_file("./tests/test3.json");
	return 0;
}

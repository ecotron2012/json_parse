#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <wchar.h>
#include <ctype.h>

// inspired by:
// https://youtu.be/Kqp9a91sjX4?si=9_I33RbzsT-WzVKY

typedef enum {
	INVALID,
	NULL_TYPE,
	OBJ_BEGIN,
	OBJ_END,
	STRING,
	CHAR,
	PUNCTUATOR,
	NUMBER,
	BOOLEAN,
	COMMA,
	ARR_BEGIN,
	ARR_END,
	FILE_EOF
} TokenType;

typedef struct {
	char* lexeme;
	TokenType type;
} Token;

typedef struct {
	
} Lexer;

Token get_string_token(FILE* fp){
	char buf[2];
	char lex[2048];
	Token token;
	strcat(lex, "\"");
	while(fgets(buf, sizeof buf, fp) != NULL){
		char curr = buf[0];
		strcat(lex, &curr);
		if(curr == '\"'){
			token.lexeme = lex;
			token.type = STRING;
			return token;
		}
	}
	// if we read the entire file and no closing brackets are found,
	// mark as invalid
	token.lexeme = lex;
	token.type = INVALID;
	return token;
}

typedef enum {
	START,
	INTEGER,
	DOUBLE,
	SCIENTIFIC_NOTATION,
	__VALID_END_STATES,
	INTEGER_WITH_DOT,
	INITIAL_MINUS_SIGN,
	SCIENTIFIC_E,
	SCIENTIFIC_E_WITH_SIGN,
	__LAST_STATE
} NUMERIC_FSM_STATES_t;

typedef struct {
	NUMERIC_FSM_STATES_t state;
	char *tokens;
	NUMERIC_FSM_STATES_t next_state;
} NUMERIC_FSM_INFO_t;

// check if the token is a number using FSMs
Token get_numeric_token(FILE* fp, char first){
	NUMERIC_FSM_INFO_t state_machine[] = {
		{START, "1234567890", INTEGER},
		{START, "-", INITIAL_MINUS_SIGN},
		{INITIAL_MINUS_SIGN, "1234567890", INTEGER},
		{INTEGER, "1234567890", INTEGER},
		{INTEGER, "e", SCIENTIFIC_E},
		{INTEGER, ".", INTEGER_WITH_DOT},
		{INTEGER_WITH_DOT, "1234567890", DOUBLE},
		{DOUBLE, "1234567890", DOUBLE},
		{DOUBLE, "e", SCIENTIFIC_E},
		{SCIENTIFIC_E, "1234567890", SCIENTIFIC_NOTATION},
		{SCIENTIFIC_E, "-+", SCIENTIFIC_E_WITH_SIGN},
		{SCIENTIFIC_E_WITH_SIGN, "1234567890", SCIENTIFIC_NOTATION},
		{SCIENTIFIC_NOTATION, "1234567890", SCIENTIFIC_NOTATION},
		{__LAST_STATE, "", __LAST_STATE}
		
	};
	NUMERIC_FSM_STATES_t current_state = START;
	NUMERIC_FSM_INFO_t *p = state_machine;
	NUMERIC_FSM_INFO_t *found;

	char buf[2];
	char lex[2048];
	Token token;
	int has_dot = 0;
	while(p->state != __LAST_STATE){
		if (strchr(p->tokens, first) != NULL){
			found = state_machine;
			break;
		}
		p++;
	}
	if(found != NULL){
		current_state = p->next_state;
	}else{
		printf("Invalid character: %s", &first);
		token.lexeme = &first;
		token.type = INVALID;
		return token;
	}
	strcat(lex, &first);
	while(fgets(buf, sizeof buf, fp) != NULL){
		char curr = buf[0];
		// TODO: check for delimiter characters before state processing
		// 
		//
		p = state_machine;
		while(p->state != __LAST_STATE){
			if (strchr(p->tokens, curr) != NULL){
				found = state_machine;
				break;
			}
			p++;
		}
		strcat(lex, &curr);
		if(found != NULL){
			current_state = p->next_state;
		}else{
			printf("Invalid character: %s", &first);
			token.lexeme = lex;
			token.type = INVALID;
			return token;
		}
	}

	// if we read the entire file and no closing brackets are found,
	// mark as invalid
	token.lexeme = lex;
	token.type = INVALID;
	return token;
}

Token* lexical_analysis(char* fname){
	Token* tokens = (Token*)malloc(2048 * sizeof(*tokens));

	// Check for malloc Failure
	if (tokens == NULL) {
		fprintf(stderr, "Memory allocation failed\n");
		return NULL;
	}

	int curr_token_idx = 0;
	FILE* fp = fopen(fname, "r");
	if (!fp){
		perror("File opening failed");
		return NULL;
	}

	// read character by character
	char buf[2];
	char keyval_pairs[1024];
	while(fgets(buf, sizeof buf, fp) != NULL){
		char curr = buf[0];
		switch (curr) {
			case '{':
				tokens[curr_token_idx].lexeme = buf;
				tokens[curr_token_idx].type = OBJ_BEGIN;
				printf("Found beggining bracket");
				break;
			case '}':
				tokens[curr_token_idx].lexeme = buf;
				tokens[curr_token_idx].type = OBJ_END;
				printf("Found end bracket");
				break;
			case '[':
				tokens[curr_token_idx].lexeme = buf;
				tokens[curr_token_idx].type = ARR_BEGIN;
				printf("Found beggining array bracket");
				break;
			case ']':
				tokens[curr_token_idx].lexeme = buf;
				tokens[curr_token_idx].type = ARR_END;
				printf("Found end array bracket");
				break;
			case ':':
				tokens[curr_token_idx].lexeme = buf;
				tokens[curr_token_idx].type = PUNCTUATOR;
				printf("Found colon");
				break;
			case ',':
				tokens[curr_token_idx].lexeme = buf;
				tokens[curr_token_idx].type = COMMA;
				printf("Found comma");
				break;
			case '\"':
				tokens[curr_token_idx] = get_string_token(fp);
				printf("Found double-quote string");
				break;
			// skip the trailing whitespaces
			case ' ':
				printf("Skipping whitespace");
				break;
			case EOF:
				tokens[curr_token_idx].lexeme = buf;
				tokens[curr_token_idx].type = FILE_EOF;
				printf("Found EOF");
				break;
			default:
				if(isdigit(curr) || strcmp(&curr, "-")){
					// TODO: handle cases for numbers and strings
				}
				printf("Unkown character");
				break;
		}
		curr_token_idx++;
	}
	fclose(fp);
	printf("key-value pairs: %s\n", keyval_pairs);
	return tokens;
}

int main(){
	const char* fname = "./test.json";
	int is_ok = EXIT_FAILURE;

	FILE* fp = fopen(fname, "r");
	if (!fp){
		perror("File opening failed");
		return is_ok;
	}

	// read character by character
	char buf[2];
	char keyval_pairs[1024];
	char prev;
	char* first = fgets(buf, sizeof buf, fp);
	if(*first == '{'){
		printf("first character: %s\n", first);
	}
	else{
		perror("File is not a valid JSON");
		return 1;
	}
	while(fgets(buf, sizeof buf, fp) != NULL){
		if(buf[0] != '\n' && buf[0] != ' ' && buf[0] != '\0'){
			prev = buf[0];
			if( prev != '}'){
				strcat(keyval_pairs, &prev);
			}
		}
	}
	printf("key-value pairs: %s\n", keyval_pairs);
	if( prev == '}'){
		printf("found last character: %s\n", &prev);
	}
	else{
		perror("File is not a valid JSON");
		return 1;
	}

	// tokenize the key-value pairs
	char* token = strtok(keyval_pairs, ",");
	while(token != NULL){
		printf("token: %s \n", token);
		token = strtok(NULL, " , ");
		char* key = strtok(token, ":");
		// check if the key is a string
		if (check_str_delim(key)){
			key = strtok(NULL, ":");
			// TODO:
			// check if value is either string, int, bool or JSON
		}else{
			perror("File is not a valid JSON");
			return 1;
		}
	}

	fclose(fp);
	return is_ok;
}

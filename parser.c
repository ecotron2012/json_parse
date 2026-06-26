#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <wchar.h>
#include <ctype.h>
#include "parser.h"

// inspired by:
// https://youtu.be/Kqp9a91sjX4?si=9_I33RbzsT-WzVKY

Token get_string_token(FILE* fp){
	char copy[2];
	char lex[2048] = "";
	Token token;
	strcat(lex, "\"");
	int curr;
	while((curr = fgetc(fp)) != EOF){
		copy[0] = curr;
		copy[1] = '\0';
		strcat(lex, copy);
		printf("Current string: %s\n", lex);
		if(curr == '\"'){
			char *copy = malloc(strlen(lex) + 1);
			strcpy(copy, lex);
			token.lexeme = copy;
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

Token get_bool_null_token(FILE* fp, char first){
	char copy[2];
	char lex[2048] = "";
	Token token;
	copy[0] = first;
	copy[1] = '\0';
	strcat(lex, copy);
	int curr;
	while((curr = fgetc(fp)) != EOF){
		if(strchr(",}:]", curr)){
			ungetc(curr, fp);
			char *copy2 = malloc(strlen(lex) + 1);
			strcpy(copy2, lex);
			token.lexeme = copy2;
			if(strcmp(lex, "true") || strcmp(lex, "false")){
				token.type = BOOLEAN;
			} else if(strcmp(lex, "null")) {
				token.type = NULL_TYPE;
			} else{
				token.type = INVALID;
			}
			return token;
		}
		copy[0] = curr;
		copy[1] = '\0';
		strcat(lex, copy);
		printf("Current string: %s\n", lex);
	}
	// if we read the entire file and no closing brackets are found,
	// mark as invalid
	char *copy2 = malloc(strlen(lex) + 1);
	strcpy(copy2, lex);
	token.lexeme = copy2;
	token.type = INVALID;
	return token;
}

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
	NUMERIC_FSM_INFO_t *found = NULL;

	char lex[255] = "";
	Token token;
	char copy[2];
	while(p->state != __LAST_STATE){
		if (p->state == current_state){
			if (strchr(p->tokens, first) != NULL){
				printf("Found transition from initial state\n");
				found = state_machine;
				break;
			}
		}
		p++;
	}
	if(found != NULL){
		printf("Transitioning to the next state\n");
		current_state = p->next_state;
	}else{
		printf("Invalid character: %c\n", first);
		copy[0] = first;
		copy[1] = '\0';
		strcat(lex, copy);
		char *copy2 = malloc(strlen(lex) + 1);
		strcpy(copy2, lex);
		token.lexeme = copy2;
		token.type = INVALID;
		return token;
	}
	copy[0] = first;
	copy[1] = '\0';
	printf("First strcat: %s\n", copy);
	strcat(lex, copy);
	printf("After strcat: %s\n", lex);
	int curr;
	while((curr = fgetc(fp)) != EOF){
		found = NULL;
		printf("current char: %c\n", curr);
		
		// skip any trailing newlines
		if(strchr("\n", curr)){ continue;}

		// check for delimiter characters before state processing
		if(strchr(",}:]", curr )){
			ungetc(curr, fp);
			char *copy2 = malloc(strlen(lex) + 1);
			strcpy(copy2, lex);
			token.lexeme = copy2;
			break;
		}
		//
		p = state_machine;
		while(p->state != __LAST_STATE){
			if (p->state == current_state){
				if (strchr(p->tokens, (char)curr) != NULL){
					printf("found matching transition with tokens %s to %s\n", p->tokens, stringFromState(p->next_state));
					found = state_machine;
					break;
				}
			}
			p++;
		}
		if(found != NULL){
			current_state = p->next_state;
			copy[0] = curr;
			copy[1] = '\0';
			strcat(lex, copy);
		}else{
			printf("Invalid character: %c\n", curr);
			copy[0] = curr;
			copy[1] = '\0';
			strcat(lex, copy);
			printf("Final invalid token: %s\n", lex);
			char *copy2 = malloc(strlen(lex) + 1);
			strcpy(copy2, lex);
			token.lexeme = copy2;
			token.type = INVALID;
			return token;
		}
	}
	if(current_state > __VALID_END_STATES){
		token.type = NUMBER;
	}else{
		token.type = INVALID;
	}
	return token;
}

Token* lexical_analysis(const char* fname){
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
	int curr;
	while((curr = fgetc(fp)) != EOF){
		switch (curr) {
			case '{':
				tokens[curr_token_idx].lexeme = malloc(2);

				tokens[curr_token_idx].lexeme[0] = curr;
				tokens[curr_token_idx].lexeme[1] = '\0';

				tokens[curr_token_idx].type = OBJ_BEGIN;
				curr_token_idx++;
				printf("Found beggining bracket\n");
				break;
			case '}':
				tokens[curr_token_idx].lexeme = malloc(2);

				tokens[curr_token_idx].lexeme[0] = curr;
				tokens[curr_token_idx].lexeme[1] = '\0';
				tokens[curr_token_idx].type = OBJ_END;
				curr_token_idx++;
				printf("Found end bracket\n");
				break;
			case '[':
				tokens[curr_token_idx].lexeme = malloc(2);

				tokens[curr_token_idx].lexeme[0] = curr;
				tokens[curr_token_idx].lexeme[1] = '\0';
				tokens[curr_token_idx].type = ARR_BEGIN;
				curr_token_idx++;
				printf("Found beggining array bracket\n");
				break;
			case ']':
				tokens[curr_token_idx].lexeme = malloc(2);

				tokens[curr_token_idx].lexeme[0] = curr;
				tokens[curr_token_idx].lexeme[1] = '\0';
				tokens[curr_token_idx].type = ARR_END;
				curr_token_idx++;
				printf("Found end array bracket\n");
				break;
			case ':':
				tokens[curr_token_idx].lexeme = malloc(2);

				tokens[curr_token_idx].lexeme[0] = curr;
				tokens[curr_token_idx].lexeme[1] = '\0';
				tokens[curr_token_idx].type = PUNCTUATOR;
				curr_token_idx++;
				printf("Found colon\n");
				break;
			case ',':
				tokens[curr_token_idx].lexeme = malloc(2);

				tokens[curr_token_idx].lexeme[0] = curr;
				tokens[curr_token_idx].lexeme[1] = '\0';
				tokens[curr_token_idx].type = COMMA;
				curr_token_idx++;
				printf("Found comma\n");
				break;
			case '\"':
				printf("Found double-quote string\n");
				tokens[curr_token_idx] = get_string_token(fp);
				curr_token_idx++;
				break;
			// skip the trailing whitespaces
			case ' ':
				printf("Skipping whitespace\n");
				break;
			case '\n':
				printf("Found file ending newline\n");
				break;
			case EOF:
				printf("Found EOF\n");
				break;
			default:
				char aux[2];
				aux[0] = curr;
				aux[1] = '\0';
				if(isdigit(curr) || strcmp(aux, "-") == 0){
					printf("checking if valid numeric token\n");
					tokens[curr_token_idx] = get_numeric_token(fp, curr);
					curr_token_idx++;
				} else if(strchr("tfn", curr)){
					// TODO: Handle checking for true, false or null
					printf("checking if boolean or null\n");
					tokens[curr_token_idx] = get_bool_null_token(fp, curr);
					curr_token_idx++;
				}else{
					tokens[curr_token_idx].lexeme = malloc(2);

					tokens[curr_token_idx].lexeme[0] = curr;
					tokens[curr_token_idx].lexeme[1] = '\0';
					tokens[curr_token_idx].type = INVALID;
					curr_token_idx++;
					printf("Unkown character\n");
				}
				break;
		}
	}
	fclose(fp);
	return tokens;
}

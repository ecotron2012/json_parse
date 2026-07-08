#ifndef JSONPARSE_H

#define JSONPARSE_H

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

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
  char *lexeme;
  TokenType type;
} Token;

typedef enum {
  START,
  INTEGER_WITH_DOT,
  INITIAL_MINUS_SIGN,
  SCIENTIFIC_E,
  SCIENTIFIC_E_WITH_SIGN,
  __VALID_END_STATES,
  INTEGER,
  DOUBLE,
  SCIENTIFIC_NOTATION,
  __LAST_STATE
} NUMERIC_FSM_STATES_t;

typedef struct {
  NUMERIC_FSM_STATES_t state;
  char *tokens;
  NUMERIC_FSM_STATES_t next_state;
} NUMERIC_FSM_INFO_t;

static const char *stringFromToken(TokenType t) {
  const char *strings[] = {"INVALID", "NULL_TYPE", "OBJ_BEGIN",  "OBJ_END",
                           "STRING",  "CHAR",      "PUNCTUATOR", "NUMBER",
                           "BOOLEAN", "COMMA",     "ARR_BEGIN",  "ARR_END",
                           "FILE_EOF"};

  return strings[t];
}

static const char *stringFromState(TokenType t) {
  const char *strings[] = {"START",
                           "INTEGER_WITH_DOT",
                           "INITIAL_MINUS_SIGN",
                           "SCIENTIFIC_E",
                           "SCIENTIFIC_E_WITH_SIGN",
                           "__VALID_END_STATES",
                           "INTEGER",
                           "DOUBLE",
                           "SCIENTIFIC_NOTATION",
                           "__LAST_STATE"};

  return strings[t];
}

typedef struct {
  char *state_name;
  TokenType *expected_next_tokens;
  int token_amt;
} SYNTACTIC_FSM_INFO_t;

typedef struct {

} Lexer;

int syntactic_fsm_linear_search(TokenType *arr, int n, TokenType key);

Token get_string_token(FILE *fp);
Token get_bool_null_token(FILE *fp, char first);
Token get_numeric_token(FILE *fp, char first);

Token *lexical_analysis(const char *fname);

int syntactic_analysis(Token *tokens, int stop_at_closing_bracket);
int check_valid_array(Token *tokens);

int parse(const char *fname);

#endif // JSONPARSE_H

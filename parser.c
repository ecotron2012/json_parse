#include "parser.h"
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

int syntactic_fsm_linear_search(TokenType *arr, int n, TokenType key) {

  // Starting the loop and looking for the key in arr
  for (int i = 0; i < n; i++) {

    // If key is found, return key
    if (arr[i] == key) {
      return i;
    }
  }

  // If key is not found, return some value to indicate
  // end
  return -1;
}

// inspired by:
// https://youtu.be/Kqp9a91sjX4?si=9_I33RbzsT-WzVKY

Token get_string_token(FILE *fp) {
  char copy[2];
  char lex[2048] = "";
  Token token;
  strcat(lex, "\"");
  int curr;
  while ((curr = fgetc(fp)) != EOF) {
    copy[0] = curr;
    copy[1] = '\0';
    strcat(lex, copy);
    printf("Current string: %s\n", lex);
    if (curr == '\"') {
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

Token get_bool_null_token(FILE *fp, char first) {
  char copy[2];
  char lex[2048] = "";
  Token token;
  copy[0] = first;
  copy[1] = '\0';
  strcat(lex, copy);
  int curr;
  while ((curr = fgetc(fp)) != EOF) {
    if (strchr(",}:]", curr)) {
      ungetc(curr, fp);

      char *copy2 = malloc(strlen(lex) + 1);
      strcpy(copy2, lex);
      token.lexeme = copy2;

      if (strcmp(lex, "true") == 0 || strcmp(lex, "false") == 0) {
        token.type = BOOLEAN;
      } else if (strcmp(lex, "null") == 0) {
        token.type = NULL_TYPE;
      } else {
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
Token get_numeric_token(FILE *fp, char first) {
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
  while (p->state != __LAST_STATE) {
    if (p->state == current_state) {
      if (strchr(p->tokens, first) != NULL) {
        printf("Found transition from initial state\n");
        found = state_machine;
        break;
      }
    }
    p++;
  }
  if (found != NULL) {
    printf("Transitioning to the next state\n");
    current_state = p->next_state;
  } else {
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
  while ((curr = fgetc(fp)) != EOF) {
    found = NULL;
    printf("current char: %c\n", curr);

    // skip any trailing newlines
    if (strchr("\n", curr)) {
      continue;
    }

    // check for delimiter characters before state processing
    if (strchr(",}:]", curr)) {
      ungetc(curr, fp);
      char *copy2 = malloc(strlen(lex) + 1);
      strcpy(copy2, lex);
      token.lexeme = copy2;
      break;
    }
    //
    p = state_machine;
    while (p->state != __LAST_STATE) {
      if (p->state == current_state) {
        if (strchr(p->tokens, (char)curr) != NULL) {
          printf("found matching transition with tokens %s to %s\n", p->tokens,
                 stringFromState(p->next_state));
          found = state_machine;
          break;
        }
      }
      p++;
    }
    if (found != NULL) {
      current_state = p->next_state;
      copy[0] = curr;
      copy[1] = '\0';
      strcat(lex, copy);
    } else {
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

  if (current_state > __VALID_END_STATES) {
    token.type = NUMBER;
  } else {
    token.type = INVALID;
  }

  return token;
}

Token *lexical_analysis(const char *fname) {
  Token *tokens = (Token *)malloc(2048 * sizeof(*tokens));

  // Check for malloc Failure
  if (tokens == NULL) {
    fprintf(stderr, "Memory allocation failed\n");
    return NULL;
  }

  int curr_token_idx = 0;
  FILE *fp = fopen(fname, "r");
  if (!fp) {
    perror("File opening failed");
    return NULL;
  }

  // read character by character
  int curr;
  while ((curr = fgetc(fp)) != EOF) {
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
      if (isdigit(curr) || strcmp(aux, "-") == 0) {
        printf("checking if valid numeric token\n");
        tokens[curr_token_idx] = get_numeric_token(fp, curr);
        curr_token_idx++;
      } else if (strchr("tfn", curr)) {
        // TODO: Handle checking for true, false or null
        printf("checking if boolean or null\n");
        tokens[curr_token_idx] = get_bool_null_token(fp, curr);
        curr_token_idx++;
      } else {
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

int syntactic_analysis(Token *tokens, int stop_at_closing_bracket) {
  Token *p = tokens;
  // variable that represent states similar to a finite state machine
  // step == 0 -> start of the program
  // step == 1 -> program has opening bracket
  // step == 2 -> found valid key
  // step == 3 -> found colon
  // step == 4 -> found valid value
  // step == 5 -> found comma
  // step == 6 -> found closing bracket
  int step = 0;
  int count = 0;
  SYNTACTIC_FSM_INFO_t state_machine[] = {
      {"start", (TokenType[]){OBJ_BEGIN}},
      {"obj_begin", (TokenType[]){STRING, OBJ_END}},
      {"key", (TokenType[]){PUNCTUATOR}},
      {"colon", (TokenType[]){STRING, NUMBER, BOOLEAN, NULL_TYPE}},
      {"value", (TokenType[]){COMMA, OBJ_END}},
      {"comma", (TokenType[]){STRING, OBJ_END}},
      {"end", (TokenType[]){}},
  };

  // the state machine represents the current value, while p represents
  // the next step
  while (p != NULL && p->lexeme != NULL) {
    printf("Checking next expected next tokens...\n");
    printf("step value: %d\n", step);
    TokenType *next_tokens = state_machine[step].expected_next_tokens;
    char state_name[strlen(state_machine[step].state_name) + 1];
    strcpy(state_name, state_machine[step].state_name);
    printf("Got the next tokens.\n");

    if (strcmp(state_name, "obj_begin") == 0) {
      // modify to not allow nested objects inside internal JSON
      if (p->type != OBJ_END && stop_at_closing_bracket == 1) {
        state_machine[1].expected_next_tokens = (TokenType[]){STRING};
      }
    }

    if (strcmp(state_name, "end") == 0) {
      if (stop_at_closing_bracket == 1) {
        printf("Exiting at first detected closing bracket after %d traversed "
               "tokens...\n",
               count);
        return count;
      } else {
        if (p != NULL) {
          printf("Invalid syntax: JSON object has trailing text after "
                 "closing\n");
          return -1;
        }
      }
    }

    if (p->type == INVALID) {
      printf("Syntax error: Invalid token type is present\n");
      return -1;
    }

    // handle edge cases
    // recursively check if json value is valid before continuing
    if (strcmp(state_name, "colon") == 0) {
      if (p->type == OBJ_BEGIN) {
        printf("Checking inner JSON validity...\n");
        int valid = syntactic_analysis(p, 1);
        if (valid <= 0) {
          printf("Syntax error: JSON value has an invalid structure\n");
          return -1;
        }
        printf("Advancing %d steps from current position with token: %s\n",
               valid, stringFromToken(p->type));
        step = 4;
        p += valid;
        continue;
      } else if (p->type == ARR_BEGIN) {
        printf("Checking inner array validity...\n");
        int valid = check_valid_array(p);
        if (valid <= 0) {
          printf("Syntax error: Array value has an invalid structure\n");
          return -1;
        }
        printf("Advancing %d steps from current position with token: %s\n",
               valid, stringFromToken(p->type));
        step = 4;
        p += valid;
        continue;
      }
    }

    count++;

    // check if the token is a valid next step
    if (syntactic_fsm_linear_search(next_tokens, sizeof(next_tokens),
                                    p->type) != -1) {
      if (strcmp(state_name, "value") == 0) {
        if (p->type == COMMA) {
          step = (step + 1) % 7;
        } else if (p->type == OBJ_END) {
          step = (step + 2) % 7;
        } else {
          printf("Syntax error\n");
          return -1;
        }
      } else if (strcmp(state_name, "comma") == 0) {
        if (p->type == OBJ_END) {
          step = (step + 1) % 7;
        } else if (p->type == STRING) {
          step = 2;
        } else {
          printf("Syntax error\n");
          return -1;
        }
      } else {
        if (p->type == OBJ_END) {
          step = 6;
        } else {
          step = (step + 1) % 7;
        }
      }
    } else {
      printf("Error: JSON doesn't have a valid token type, instead have token "
             "type: %s\n",
             stringFromToken(p->type));
      return -1;
    }
    p++;
  }

  printf("Exiting (exit on inner brackets = % d) JSON check with %d steps...\n",
         stop_at_closing_bracket, step);
  return count;
}

int check_valid_array(Token *tokens) {
  Token *p = tokens;
  int count = 0;
  int step = 0;
  SYNTACTIC_FSM_INFO_t state_machine[] = {
      {"start", (TokenType[]){ARR_BEGIN}},
      {"arr_begin", (TokenType[]){STRING, NUMBER, BOOLEAN, NULL_TYPE, ARR_END}},
      {"value", (TokenType[]){COMMA, ARR_END}},
      {"comma", (TokenType[]){STRING, NUMBER, BOOLEAN, NULL_TYPE}},
      {"end", (TokenType[]){}},
  };

  while (p != NULL && p->lexeme != NULL) {
    count++;
    printf("Checking next expected next tokens in array current step %s...\n",
           state_machine[step].state_name);
    TokenType *next_tokens = state_machine[step].expected_next_tokens;
    char state_name[strlen(state_machine[step].state_name) + 1];
    strcpy(state_name, state_machine[step].state_name);
    printf("Got the next tokens in array.\n");

    if (p->type == INVALID) {
      printf("Syntax error: Invalid token type is present\n");
      return -1;
    }

    // handle edge case
    // recursively check if json or array value is valid before continuing
    if (strcmp(state_name, "arr_begin") == 0 ||
        strcmp(state_name, "comma") == 0) {
      if (p->type == OBJ_BEGIN) {
        printf("Checking inner JSON validity...\n");
        int valid = syntactic_analysis(p, 1);
        if (valid <= 0) {
          printf("Syntax error: JSON value has an invalid structure\n");
          return -1;
        }
        printf("Advancing %d steps\n", valid + 1);
        count += valid;
        p += valid + 1;
      } else if (p->type == ARR_BEGIN) {
        printf("Checking inner array validity...\n");
        int valid = check_valid_array(p);
        if (valid <= 0) {
          printf("Syntax error: Array value has an invalid structure\n");
          return -1;
        }
        printf("Advancing %d steps\n", valid + 1);
        count += valid;
        p += valid + 1;
      }
    }

    if (strcmp(state_name, "end") == 0) {
      printf("Returning after finding end bracket with %d steps\n", count);
      return count;
    }

    // check if the token is a valid next step
    if (syntactic_fsm_linear_search(next_tokens, sizeof(next_tokens),
                                    p->type) != -1) {
      if (strcmp(state_name, "value") == 0) {
        if (p->type == COMMA) {
          step = (step + 1) % 5;
        } else if (p->type == ARR_END) {
          step = (step + 2) % 5;
        } else {
          printf("Syntax error\n");
          return -1;
        }
      } else if (strcmp(state_name, "comma") == 0) {
        if (p->type == STRING || p->type == NUMBER || p->type == BOOLEAN ||
            p->type == NULL_TYPE) {
          step = 2;
        } else {
          printf("Syntax error\n");
          return -1;
        }
      } else {
        if (p->type == ARR_END) {
          step = 4;
        } else {
          step = (step + 1) % 5;
        }
      }
    } else {
      printf("Error: Array doesn't have a valid token type, instead have token "
             "type: %s\n",
             stringFromToken(p->type));
      return -1;
    }
    p++;
  }
  printf("Exiting array check...\n");
  return count;
}

int parse(const char *fname) {
  printf("testing file: %s...\n", fname);
  Token *tokens = lexical_analysis(fname);
  Token *p = tokens;
  int valid = syntactic_analysis(tokens, 0);

  while (p != NULL && p->lexeme != NULL) {
    free(p->lexeme);
    p++;
  }

  printf("Result of syntactic analysis: %d\n", valid);
  if (valid >= 0)
    return 0;
  else
    return 1;
}

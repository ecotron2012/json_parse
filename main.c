#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <wchar.h>

// Checks if the string is delimited by quotes (inside the string)
// for example, the string ""key"" is delimited by quotes, but "key" is not.
int check_str_delim(char* s){
	char first = s[0];
	char last = s[-2];
	if (strcmp(&first, "\"") && strcmp(&last,"\"")){
		return 1;
	}
	return 0;
}

int check_if_bool(char* s){
	return 0;
}

int check_if_num(char* s){
	return 1;
}

int check_if_json(char* s){
	return 1;
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

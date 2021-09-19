#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void lexer_parser(char* in, char* tokens[]) {
  int len = strlen(in);
  char* cmd = strdup(in);
  char* ptr;
  ptr = strtok(cmd, "\n ");
  int i = 0;
  while(ptr != NULL) {
    tokens[i] = ptr;
    i++;
    ptr = strtok(NULL, "\n ");
  }
  tokens[i] = NULL;
}

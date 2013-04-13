#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "string.h"

const int INITIAL_STRING_LENGTH = 1000;

String *new_string(){
  String *r_string = malloc(sizeof(String));
  r_string->buffer = malloc(INITIAL_STRING_LENGTH);
  memset(r_string->buffer, 0, INITIAL_STRING_LENGTH);
  r_string->size = INITIAL_STRING_LENGTH;
  r_string->current_length = 0;
  return r_string; 
}

void free_string(String *str){
  free(str->buffer);
  free(str);
}

int cat_string(String *str, char *to_add, int length){
  int new_length = strlen(to_add);
  if(str->current_length + new_length > str->size ){
    fprintf(stderr, "reallocating\n");
    if((str->buffer = realloc(str->buffer, str->size * 2)) == NULL){
      perror("realloc");
      exit(1);
    }
    str->size = str->size * 2;
  }

  str->current_length += new_length;
  strncat(str->buffer, to_add, length);
  return length;
}

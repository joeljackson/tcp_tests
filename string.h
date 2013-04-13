#ifndef _JJ_STRING_H
#define _JJ_STRING_H

typedef struct String{
  int current_length;
  int size;
  char *buffer;
} String;

String *new_string();
void free_string(String *str);
int cat_string(String *str, char *to_add, int length);

#endif

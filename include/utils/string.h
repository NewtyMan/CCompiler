#ifndef CCOMPILER_STRING_H
#define CCOMPILER_STRING_H
int string_cmp(char *a, char *b);
int string_len(char *s);
void string_cpy(char *dst, char *src);
int string_atoi(char *str, int base);
char *string_substr(char *str, int start, int end);
#endif //CCOMPILER_STRING_H

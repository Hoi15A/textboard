#pragma once

#include <math.h>
#define startswith(x,y) (strncmp(x,y,strlen(y)) == 0)
#define streq(x,y) (strcmp(x,y) == 0)
#define digits(x) (x==0?1:(floor(log10(abs(x)))+1))
#define max(x,y) (x>y?x:y)

char *clone_str(const char *x);
void decode_uri(char *dest, const char *src);
char *encode_html(const char *src);

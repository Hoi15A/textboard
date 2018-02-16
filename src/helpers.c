#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define HTML_ESCAPE_LENGTH 14

char *clone_str(const char *x) {
    char *str = malloc(strlen(x) + 1);
    strcpy(str, x);
    return str;
}

static int numberOfBytesInChar(const unsigned char val) {
    if(val < 128) return 1;
    else if (val < 224) return 2;
    else if (val < 240) return 3;
    return 4;
}

void decode_uri(char *dest, const char *src) {
    for(size_t i = 0, j = 0; src[i];) {
        if(src[i] == '%') {
            i++;
            const size_t prev = i;
            const char
                c1 = i < strlen(src) ? src[i++] : 0,
                c2 = i < strlen(src) ? src[i++] : 0;
            if(isxdigit(c1) && isxdigit(c2)) {
                
                const char hex[3] = { c1, c2, 0 };
                dest[j++] = strtol(hex, NULL, 16);
                
            } else {
                dest[j++] = '%';
                i = prev;
            }
        } else if(src[i] == '+') {
            dest[j++] = ' ';
            i++;
        } else {
            dest[j++] = src[i++];
        }
    }
}

char *encode_html(const char *src) {
    size_t length = strlen(src)+1; // +null terminator
    char *dest = malloc(length);
    if(dest == NULL) return NULL;
    memset(dest, 0, length);
    size_t i = 0, j = 0;
    while(src[i]) {
        int chars = numberOfBytesInChar((unsigned char)src[i]);
        if(chars > 1) { // magic bitwise fuckery
            // https://stackoverflow.com/a/10017544
            // https://stackoverflow.com/a/6240819
            unsigned long bits = 0;
            unsigned char first = (unsigned char)src[i++];
            // clear first's n bits
            for(int k = 0; k <= chars+1; k++)
                first &= ~(0x80 >> k);
            // concat first -> bits
            bits = (bits << (8-(chars+1))) | first;
            for(int k = 0; k < chars-1; k++) {
                unsigned char hexchar = (unsigned char)src[i];
                if((char)hexchar == 0) { // besure the string ain't terminated
                    bits = 0;
                    break;
                }
                i++;
                // clear first 2 bits
                hexchar &= ~0x80; // 1
                hexchar &= ~0x40; // 0
                // concat hexchar -> bits
                bits = (bits << 6) | hexchar;
            }
            
            if(bits) {
                char entity[16];
                snprintf(entity, 16, "&#x%lx;", bits);
                length += strlen(entity)-chars;
                dest = realloc(dest, length);
                for(size_t x = 0; x < strlen(entity); x++)
                    dest[j+x] = entity[x];
                j += strlen(entity);
            } else
                i++;
        }
        // don't escape these characters
        else if(isalnum(src[i]) || // [A-Za-z0-9]
            (32 <= src[i] && src[i] <= 47 && src[i] != 44) || // [space] to / without ,
            // (makes converting from/to csv A LOT fucking easier
            (src[i] == 61 || src[i] == 63 || src[i] == 64) || // =, ?, @
            (123 <= src[i] && src[i] <= 126)) { // { to ~
            dest[j++] = src[i++];
        }
        // break line
        else if(src[i] == '\n') {
            length += strlen("<br>")-1; // replaces 1 character with 1 string
            dest = realloc(dest, length);
            dest[j++] = '<';
            dest[j++] = 'b';
            dest[j++] = 'r';
            dest[j++] = '>';
            i++;
        }
        // escape the others
        else {
            char num[HTML_ESCAPE_LENGTH];
            snprintf(num, HTML_ESCAPE_LENGTH, "&#%i;", src[i++]);
            
            length += strlen(num)-1; // replaces 1 character with 1 string
            dest = realloc(dest, length);
            for(size_t x = 0; x < strlen(num); x++)
                dest[j+x] = num[x];
            j += strlen(num);
        }
    }
    dest[j] = 0;
    return dest;
}

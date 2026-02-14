#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

typedef enum {
    INT,
} TypeLiteral;

typedef enum {
    EXIT,
} TypeKeyword;

typedef enum {
    OPEN_PAREN,
    CLOSED_PAREN,
    SEMI_COLON,
} TypeSeparator;

typedef struct {
    TypeKeyword type;
} TokenKeyword;

typedef struct {
    TypeLiteral type;
    int value;
} TokenLiteral;

typedef struct {
    TypeSeparator type;
} TokenSeparator;

TokenLiteral generate_number(char *buf,int *idx) {
    int val = 0;
    while (buf[*idx]!='\0' && isdigit(buf[*idx])) {
        val *= 10;
        val += (buf[*idx] - '0');
        (*idx)++;
    }
    TokenLiteral token;
    token.value = val;
    token.type = INT;
    return token;
}

TokenSeparator get_separator(char c ) {
    TokenSeparator token;
    if (c == '(') {
        token.type = OPEN_PAREN;
    } else if (c == ')') {
        token.type = CLOSED_PAREN;
    } else if (c == ';') {
        token.type = SEMI_COLON;
    } else {
        printf("ERROR: Unknown Separator Type '%c'\n", c);
    }
    return token;
}

TokenKeyword get_keyword(char *buf,int *idx ) {
    char keyword[50] = {0};
    int i = 0;
    TokenKeyword token;
    while (buf[*idx]!='\0' && isalpha(buf[*idx])) {
        if (i < 49) keyword[i++] = buf[*idx];
        (*idx)++;
    }
    if (strcmp(keyword, "exit") == 0) {
        token.type = EXIT;
    } else {
        printf("ERROR: Unknown Keyword '%s'\n", keyword);
    }
    return token;
}

int lexel(FILE *file) {
    int size =  0 ; 
    fseek(file, 0 , SEEK_END) ; 

    size =  ftell(file);
    rewind(file) ; 
    char *buf = malloc(sizeof(char) * (size+1)) ; 
    if(!buf){
        perror("malloc"); 
        fclose(file); 
        return 1 ; 
    }
    size_t read_size =  fread(buf , sizeof(char), size , file ) ; 
    if(read_size!=size){
        perror("fread") ; 
        fclose(file) ; 
        return 1 ; 
    }
    buf[size] = '\0';
    int idx = 0 ; 
    while (idx < size) {
        if (isdigit(buf[idx])) {
            TokenLiteral t = generate_number(buf, &idx);
            printf("Token INT: %d\n", t.value);
        } else if (isalpha(buf[idx])) {
            TokenKeyword t = get_keyword(buf, &idx);
            if(t.type == EXIT) printf("Token Keyword: EXIT\n");
        } else if (isspace(buf[idx])) {
            idx++;
        } else {
            TokenSeparator t = get_separator(buf[idx]);
            if(t.type == OPEN_PAREN) printf("Token Separator: (\n");
            else if(t.type == CLOSED_PAREN) printf("Token Separator: )\n");
            else if(t.type == SEMI_COLON) printf("Token Separator: ;\n");
            idx++;
        }
    }
    free(buf) ; 
    return 0  ; 
}

int main() {
    FILE *file;
    file = fopen("test.ab", "r");
    if (!file) {
        perror("Failed to open file");
        return 1;
    }
    int val = lexel(file);
    if(val != 0){
        printf("ENCOUNTERED an error") ; 
        return 1 ; 
    }
    fclose(file);
    return 0;
}
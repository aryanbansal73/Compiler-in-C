// src/lexer/lexer.c
#include "lexer.h"
#include <ctype.h>
#include <string.h>
#include <stdlib.h>

typedef struct {
    const char * word ; 
    TokenType type ; 
} Keyword ; 
static const Keyword keywords[] = {
    {"as",       TOKEN_AS},
    {"bool",     TOKEN_BOOL},
    {"break",    TOKEN_BREAK},
    {"continue", TOKEN_CONTINUE},
    {"else",     TOKEN_ELSE},
    {"enum",     TOKEN_ENUM},
    {"f32",      TOKEN_F32},
    {"f64",      TOKEN_F64},
    {"false",    TOKEN_FALSE},
    {"fn",       TOKEN_FN},
    {"for",      TOKEN_FOR},
    {"i16",      TOKEN_I16},
    {"i32",      TOKEN_I32},
    {"i64",      TOKEN_I64},
    {"i8",       TOKEN_I8},
    {"if",       TOKEN_IF},
    {"import",   TOKEN_IMPORT},
    {"let",      TOKEN_LET},
    {"match",    TOKEN_MATCH},
    {"mut",      TOKEN_MUT},
    {"null",      TOKEN_NULL},
    {"return",   TOKEN_RETURN},
    {"str",      TOKEN_STR},
    {"struct",   TOKEN_STRUCT},
    {"true",     TOKEN_TRUE},
    {"u16",      TOKEN_U16},
    {"u32",      TOKEN_U32},
    {"u64",      TOKEN_U64},
    {"u8",       TOKEN_U8},
    {"void",     TOKEN_VOID},
    {"while",    TOKEN_WHILE},
};
#define KEYWORD_COUNT (sizeof(keywords) / sizeof(keywords[0]) )

void lexer_init(Lexer *lex ,  const char *source  , const char * filename ){
    lex->source =  source ; 
    lex->current =  source ; 
    lex->col = 1 ; 
    lex->line = 1 ; 
    lex->has_peeked =  0 ; 
    lex->offset = 0 ; 
    lex->file =  filename ; 
}

static inline char peek_char(Lexer *lex ){
    return *lex->current ; 
}

static inline char peek_next (Lexer *lex ){
    if(*(lex->current) == '\0')return '\0';
    return lex->current[1];
}

static inline char advance(Lexer *lex){
    char c =  *(lex->current++) ; 
    lex->offset++;
    if(c=='\n'){
        lex->line++;
        lex->col = 1; 
    }
    else{
        lex->col++;
    }
    return c ; 
}

static inline bool match(Lexer *lex , char expected){
    if(*lex->current ==  expected){
        advance(lex) ; 
        return true ; 
    }
    return false ; 
}

static inline bool is_at_end(Lexer *lex){
    return *lex->current == "\0";
}

static inline bool is_ident_char(char c) {
    return isalnum((unsigned char)c) || c == '_';
}

static Token make_token(Lexer *lex ,TokenType type ,  const char *start , int length ,  SrcLoc loc ){
    Token tok ;
    tok.type =  type ; 
    tok.length =  length ; 
    tok.start= start ; 
    tok.loc =  loc ; 
    tok.int_val = 0 ; 
    return tok ; 
}

static Token error_token(Lexer *lex, const char *msg) {
    SrcLoc loc = { lex->file, lex->line, lex->col, lex->offset };
    Token tok;
    tok.type    = TOKEN_ERR;
    tok.start   = msg;
    tok.length  = (int)strlen(msg);
    tok.loc     = loc;
    tok.int_val = 0;
    return tok;
}

static TokenType lookup_keyword(const char * start , int length ){
    int l = 0 , h = KEYWORD_COUNT-1 ; 
    while(l<=h){
        int mid = (l+h)/2 ; 
        int comp =  strncmp(start ,  keywords[mid].word , length ); 
        if(comp ==0){
            if(keywords[mid].word[length] == '\0'){
                return  keywords[mid].type ; 
            }
            comp = -1 ; 
        }
        if(comp<0){
            h = mid -1 ; 
        }
        else{
            h = mid+1 ;
        }
    }
    return TOKEN_INDENT ; 
}

static void skip_whitespace(Lexer *lex) {
    for (;;) {
        char c = peek_char(lex);
        switch (c) {
            case ' ': case '\t': case '\r': case '\n':
                advance(lex);
                break;

            case '/':
                if (peek_next(lex) == '/') {
                    while (!is_at_end(lex) && peek_char(lex) != '\n') {
                        advance(lex);
                    }
                    break;
                }
                if (peek_next(lex) == '*') {
                    advance(lex);
                    advance(lex);
                    int depth = 1;
                    while (!is_at_end(lex) && depth > 0) {
                        if (peek_char(lex) == '/' && peek_next(lex) == '*') {
                            advance(lex); advance(lex);
                            depth++;
                        } else if (peek_char(lex) == '*' && peek_next(lex) == '/') {
                            advance(lex); advance(lex);
                            depth--;
                        } else {
                            advance(lex);
                        }
                    }
                    break;
                }
                return;
            default:
                return;
        }
    }
}

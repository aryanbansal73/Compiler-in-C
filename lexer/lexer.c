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

static inline bool is_ident_start(char c) {
    return isalpha((unsigned char)c) || c == '_';
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
    return TOKEN_IDENT_LITERAL ; 
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

static Token lex_number(Lexer *lex) {
    const char *start =  lex->current ; 
    SrcLoc loc =  {lex->file , lex->line , lex->col,lex->offset} ; 
    bool is_float = 0 ; 
    if(peek_char(lex) == '0' && (peek_next(lex)=='x' || peek_next(lex) == 'X') ){
        advance(lex) ; 
        advance(lex) ; 
        if(!isxdigit((unsigned char)peek_char(lex))){
            return error_token(lex , "Expected hex digits after '0x'" ); 
        }
        while(isxdigit((unsigned char)peek_char(lex)) || peek_char =='_'){
            advance(lex) ; 
        }
        Token token =  make_token(lex, TOKEN_INT_LITERAL ,  start ,  (int)(lex->current - start ) , loc);
        token.int_val =  strtoll(start , NULL , 16) ; 
        return token ; 
    }
    if(peek_char(lex) == '0' && (peek_next(lex)=='b'  ||peek_next(lex)=='B')){
        advance(lex) ; 
        advance(lex) ; 
        if(peek_char(lex)!='0' && peek_char(lex)!='1'){
            return error_token(lex , "Expected binary digits after '0b");
        }
        while(peek_char(lex)=='0' || peek_char(lex)=='1' || peek_char(lex) == '_'){
            advance(lex);
        }
        Token token =  make_token(lex, TOKEN_INT_LITERAL ,  start ,  (int)(lex->current - start ) , loc);
        token.int_val =  strtoll(start+2  , NULL , 2) ; 
        return token ; 
    }
    while (isdigit((unsigned char)peek_char(lex)) || peek_char(lex) == '_') {
        advance(lex);
    }
    if (peek_char(lex) == '.' && isdigit((unsigned char)peek_next(lex))) {
        is_float = true;
        advance(lex); // consume '.'
        while (isdigit((unsigned char)peek_char(lex)) || peek_char(lex) == '_') {
            advance(lex);
        }
    }
    if (peek_char(lex)=='e'||peek_char(lex)=='E'){
        is_float = 1 ; 
        advance(lex);
        if(peek_char(lex)=='+'||peek_char(lex) =='-'){
            advance(lex);
        }
        if(!isdigit((unsigned char)peek_char(lex))){
            return error_token(lex , "Expected digits in exponent") ;
        }
        while(isdigit((unsigned char)(peek_char(lex))))advance(lex);
    }
    int length =  (int)(lex->current -  start );
    if(is_float){
        Token tok =  make_token(lex,TOKEN_FLOAT_LITERAL , start , length , loc) ; 
        tok.float_val =  strtod(start , NULL) ;
        return tok ; 
    }
    else{
        Token tok =  make_token(lex,TOKEN_INT_LITERAL ,start , length , loc ) ; 
        tok.int_val = strtoll(start ,  NULL  , 10) ; 
        return tok ;
    }
}

static Token lex_string (Lexer *lex) {
    SrcLoc loc =  {lex->file , lex->line , lex->col , lex->offset } ; 
    advance(lex) ; // consume  '"'
    const char *start = lex->current ; 
    if(!is_at_end(peek_char(lex)) && peek_char !='"'){
        if(peek_char(lex) == '\\'){
            advance(lex);
            if(is_at_end(lex)){
                return error_token(lex , "unterminated escape character ");
            }
        }
        if (peek_char(lex) == '\n') {
            return error_token(lex, "unterminated string (newline in string literal)");
        }
        advance(lex);
    }
     if (is_at_end(lex)) {
        return error_token(lex, "Unterminated string literal");
    }

    int length = (int)(lex->current - start);
    advance(lex); // consume  '"'

    return make_token(lex, TOKEN_STRING_LITERAL, start, length, loc);
}

static Token lex_indentifier(Lexer *lex){
    const char *start = lex->current ; 
    SrcLoc loc = {lex->file , lex->line , lex->col , lex->offset} ; 
    while (is_ident_char(peek_char(lex))){
        advance(lex);
    }
    int length =  (int)(lex->current - start );
    TokenType type  = lookup_keyword(start , length ); 
    return make_token(lex , type , start , length , loc ) ; 
}

Token lexer_next_token(Lexer *lex){
    if(lex->has_peeked){
        lex->has_peeked =  false ; 
        return lex->peeked; 
    }
    skip_whitespace(lex) ; 
    SrcLoc loc = {lex->file , lex->line , lex->col , lex->offset} ;
    if(is_at_end(lex) ){
        return  make_token(lex , TOKEN_EOF , lex->current , 0 , loc) ;
    }
    char c = peek_char(lex) ; 
    if(isdigit(c)) {
        return lex_number(lex) ; 
    }
    if(is_ident_start(c)){
        return lex_indentifier(lex) ; 
    }
    if(c=='"'){
        return lex_string(lex) ; 
    }
    if (c== '\''){
        advance(lex) ; 
        const char * start = lex->current ; 
        if(peek_char(lex) == '\\'){
            advance(lex) ; 
        }
        advance(lex) ; 
        if(peek_char(lex )!='\''){
            return error_token(lex, "Unterminated character literal");
        }
        int len = (int)(lex->current -  start ) ; 
        advance(lex) ; 
        return make_token(lex , TOKEN_CHAR_LITERAL , start , len , loc ) ; 
    }
    advance(lex);
    switch(c){
    // Single-char tokens
        case '(': return make_token(lex, TOKEN_LPAREN,    &c, 1, loc);
        case ')': return make_token(lex, TOKEN_RPAREN,    &c, 1, loc);
        case '{': return make_token(lex, TOKEN_LBRACE,    &c, 1, loc);
        case '}': return make_token(lex, TOKEN_RBRACE,    &c, 1, loc);
        case '[': return make_token(lex, TOKEN_LBRACKET,  &c, 1, loc);
        case ']': return make_token(lex, TOKEN_RBRACKET,  &c, 1, loc);
        case ';': return make_token(lex, TOKEN_SEMICOLON, &c, 1, loc);
        case ',': return make_token(lex, TOKEN_COMMA,     &c, 1, loc);
        case '~': return make_token(lex, TOKEN_TILDE,     &c, 1, loc);
        case '^': return make_token(lex, TOKEN_CARET,     &c, 1, loc);
        case '%': return make_token(lex, TOKEN_PERCENT,   &c, 1, loc);
        
        case '.': return make_token(lex, TOKEN_DOT, &c, 1, loc);

        case ':':
            if (match(lex, ':')) return make_token(lex, TOKEN_DOUBLE_COLON, lex->current - 2, 2, loc);
            return make_token(lex, TOKEN_COLON, &c, 1, loc);

        case '+':
            if (match(lex, '=')) return make_token(lex, TOKEN_PLUS_ASSIGN, lex->current - 2, 2, loc);
            return make_token(lex, TOKEN_PLUS, &c, 1, loc);

        case '-':
            if (match(lex, '>')) return make_token(lex, TOKEN_ARROW, lex->current - 2, 2, loc);
            if (match(lex, '=')) return make_token(lex, TOKEN_MINUS_ASSIGN, lex->current - 2, 2, loc);
            return make_token(lex, TOKEN_MINUS, &c, 1, loc);

        case '*':
            if (match(lex, '=')) return make_token(lex, TOKEN_STAR_ASSIGN, lex->current - 2, 2, loc);
            return make_token(lex, TOKEN_STAR, &c, 1, loc);

        case '/':
            if (match(lex, '=')) return make_token(lex, TOKEN_SLASH_ASSIGN, lex->current - 2, 2, loc);
            return make_token(lex, TOKEN_SLASH, &c, 1, loc);

        case '=':
            if (match(lex, '=')) return make_token(lex, TOKEN_EQ, lex->current - 2, 2, loc);
            if (match(lex, '>')) return make_token(lex, TOKEN_FAT_ARROW, lex->current - 2, 2, loc);
            return make_token(lex, TOKEN_ASSIGN, &c, 1, loc);

        case '!':
            if (match(lex, '=')) return make_token(lex, TOKEN_NEQ, lex->current - 2, 2, loc);
            return make_token(lex, TOKEN_BANG, &c, 1, loc);

        case '<':
            if (match(lex, '=')) return make_token(lex, TOKEN_LEQ, lex->current - 2, 2, loc);
            if (match(lex, '<')) return make_token(lex, TOKEN_LSHIFT, lex->current - 2, 2, loc);
            return make_token(lex, TOKEN_LT, &c, 1, loc);

        case '>':
            if (match(lex, '=')) return make_token(lex, TOKEN_GEQ, lex->current - 2, 2, loc);
            if (match(lex, '>')) return make_token(lex, TOKEN_RSHIFT, lex->current - 2, 2, loc);
            return make_token(lex, TOKEN_GT, &c, 1, loc);

        case '&':
            if (match(lex, '&')) return make_token(lex, TOKEN_AND, lex->current - 2, 2, loc);
            return make_token(lex, TOKEN_AMP, &c, 1, loc);

        case '|':
            if (match(lex, '|')) return make_token(lex, TOKEN_OR, lex->current - 2, 2, loc);
            return make_token(lex, TOKEN_PIPE, &c, 1, loc);

        default :
        return error_token(lex, "Unexpected character");
    }
}
Token lexer_peek(Lexer *lex) {
    if (!lex->has_peeked) {
        lex->peeked = lexer_next_token(lex);
        lex->has_peeked = true;
    }
    return lex->peeked;
}
const char *token_type_str(TokenType type) {
    static const char *names[] = {
        [TOKEN_INT_LITERAL]    = "INT_LIT",
        [TOKEN_FLOAT_LITERAL]  = "FLOAT_LIT",
        [TOKEN_STRING_LITERAL] = "STRING_LIT",
        [TOKEN_CHAR_LITERAL]   = "CHAR_LIT",
        [TOKEN_IDENT_LITERAL]  = "IDENTIFIER",
        [TOKEN_AS]        = "as",
        [TOKEN_BOOL]      = "bool",
        [TOKEN_BREAK]     = "break",
        [TOKEN_CONTINUE]  = "continue",
        [TOKEN_ELSE]      = "else",
        [TOKEN_ENUM]      = "enum",
        [TOKEN_F32]       = "f32",
        [TOKEN_F64]       = "f64",
        [TOKEN_FALSE]     = "false",
        [TOKEN_FN]        = "fn",
        [TOKEN_FOR]       = "for",
        [TOKEN_I8]        = "i8",
        [TOKEN_I16]       = "i16",
        [TOKEN_I32]       = "i32",
        [TOKEN_I64]       = "i64",
        [TOKEN_U8]        = "u8",
        [TOKEN_U16]       = "u16",
        [TOKEN_U32]       = "u32",
        [TOKEN_U64]       = "u64",
        [TOKEN_IF]        = "if",
        [TOKEN_IMPORT]    = "import",
        [TOKEN_LET]       = "let",
        [TOKEN_MATCH]     = "match",
        [TOKEN_MUT]       = "mut",
        [TOKEN_NULL]      = "null",
        [TOKEN_RETURN]    = "return",
        [TOKEN_STR]       = "str",
        [TOKEN_STRUCT]    = "struct",
        [TOKEN_TRUE]      = "true",
        [TOKEN_VOID]      = "void",
        [TOKEN_WHILE]     = "while",
        [TOKEN_LPAREN]    = "(",
        [TOKEN_RPAREN]    = ")",
        [TOKEN_LBRACE]    = "{",
        [TOKEN_RBRACE]    = "}",
        [TOKEN_LBRACKET]  = "[",
        [TOKEN_RBRACKET]  = "]",
        [TOKEN_SEMICOLON] = ";",
        [TOKEN_COMMA]     = ",",
        [TOKEN_DOT]       = ".",
        [TOKEN_COLON]     = ":",
        [TOKEN_DOUBLE_COLON] = "::",
        [TOKEN_PLUS]      = "+",
        [TOKEN_MINUS]     = "-",
        [TOKEN_STAR]      = "*",
        [TOKEN_SLASH]     = "/",
        [TOKEN_PERCENT]   = "%",
        [TOKEN_ASSIGN]    = "=",
        [TOKEN_PLUS_ASSIGN]  = "+=",
        [TOKEN_MINUS_ASSIGN] = "-=",
        [TOKEN_STAR_ASSIGN]  = "*=",
        [TOKEN_SLASH_ASSIGN] = "/=",
        [TOKEN_EQ]        = "==",
        [TOKEN_NEQ]       = "!=",
        [TOKEN_LT]        = "<",
        [TOKEN_LEQ]       = "<=",
        [TOKEN_GT]        = ">",
        [TOKEN_GEQ]       = ">=",
        [TOKEN_LSHIFT]    = "<<",
        [TOKEN_RSHIFT]    = ">>",
        [TOKEN_AMP]       = "&",
        [TOKEN_AND]       = "&&",
        [TOKEN_PIPE]      = "|",
        [TOKEN_OR]        = "||",
        [TOKEN_CARET]     = "^",
        [TOKEN_TILDE]     = "~",
        [TOKEN_BANG]      = "!",
        [TOKEN_ARROW]     = "->",
        [TOKEN_FAT_ARROW] = "=>",
        [TOKEN_EOF]       = "EOF",
        [TOKEN_ERR]       = "ERROR",
    };
    if (type >= 0 && type < TOKEN_COUNT && names[type]) {
        return names[type];
    }
    return "UNKNOWN";
}
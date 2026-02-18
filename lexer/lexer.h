#ifndef LEXER_H
#define LEXER_H
#include <stddef.h>
#include <stdbool.h>
#include <cstdint>
typedef struct{
    const char * file ; 
    int line ; 
    int col ; 
    int offset ; 
} SrcLoc ; 
typedef struct  
{
    SrcLoc  start ; 
    SrcLoc end ; 
} Span ; 

// Token Types 
typedef enum {
    TOKEN_INT , 
    TOKEN_FLOAT , 
    TOKEN_STRING , 
    TOKEN_CHAR , 
    TOKEN_INDENT, 


    TOKEN_FN  , 
    TOKEN_LET, 
    TOKEN_MUT , 
    TOKEN_IF,
    TOKEN_ELSE, 
    TOKEN_WHILE,
    TOKEN_FOR,
    TOKEN_RETURN  , 
    TOKEN_STRUCT , 
    TOKEN_ENUM , 
    TOKEN_TRUE ,
    TOKEN_FALSE , 
    TOKEN_IMPORT ,
    TOKEN_AS,
    TOKEN_BREAK ,
    TOKEN_NULL,
    TOKEN_CONTINUE,
    TOKEN_MATCH,


    TOKEN_I8, TOKEN_I16, TOKEN_I32,TOKEN_I64,
    TOKEN_U8,TOKEN_U16,TOKEN_U32,TOKEN_U64 , 
    TOKEN_F32,TOKEN_F64 , 
    TOKEN_BOOL , 
    TOKEN_VOID , 
    TOKEN_STR ,


    TOKEN_PLUS,
    TOKEN_MINUS,
    TOKEN_STAR,
    TOKEN_SLASH,
    TOKEN_PERCENT,
    TOKEN_AMP,
    TOKEN_PIPE,
    TOKEN_CARET,
    TOKEN_TILDE,
    TOKEN_BANG,
    TOKEN_LSHIFT,
    TOKEN_RSHIFT,
    
    
    TOKEN_EQ,
    TOKEN_NEW,
    TOKEN_LT,TOKEN_GT,
    TOKEN_LEQ,TOKEN_GEQ,

    TOKEN_AND,
    TOKEN_OR,

    TOKEN_ASSIGN,
    TOKEN_PLUS_ASSIGN,
    TOKEN_MINUS_ASSIGN,
    TOKEN_STAR_ASSIGN,
    TOKEN_SLASH_ASSIGN,

    TOKEN_LPAREN,
    TOKEN_RPAREN,
    TOKEN_LBRACE,
    TOKEN_RBRACE,
    TOKEN_LBRACKET,
    TOKEN_RBRACKET,
    TOKEN_SEMICOLON,
    TOKEN_COLON,
    TOKEN_COMMA,
    TOKEN_DOT,
    TOKEN_ARROW,
    TOKEN_FAT_ARROW,
    TOKEN_DOUBLE_COLUMN,

    TOKEN_EOF,
    TOKEN_ERR,

    TOKEN_COUNT

} TokenType ; 

typedef struct { 
    TokenType  type ; 
    const char *start ; 
    int length ; 
    SrcLoc  loc ; 
    union {
        int64_t int_val ;
        double float_val ; 
    } ;
}Token ; 
typedef struct {
    const char *source  ; 
    const char *current; 
    const char *file ; 
    int line ; 
    int  col ; 
    int offset ;
    Token peeked  ;
    bool has_peeked ; 
}Lexer ; 


void  lexer_init(Lexer *lex, const char *source, const char *filename);
Token lexer_next_token(Lexer *lex);
Token lexer_peek(Lexer *lex);
const char *token_type_str(TokenType type);
#endif
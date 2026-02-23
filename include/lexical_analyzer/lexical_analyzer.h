#ifndef CCOMPILER_LEXICAL_ANALYZER_H
#define CCOMPILER_LEXICAL_ANALYZER_H
#include <utils/dynamic_array.h>
#include <utils/file_buffer.h>

typedef enum token_type {
    TOKEN_TYPE_NONE,
    TOKEN_TYPE_UNKNOWN,

    TOKEN_TYPE_PREPROCESSOR_DIRECTIVE,

    TOKEN_TYPE_STRING_LITERAL,
    TOKEN_TYPE_CHARACTER_LITERAL,
    TOKEN_TYPE_ENUM_LITERAL,

    TOKEN_TYPE_NUMBER_DEC,
    TOKEN_TYPE_NUMBER_OCT,
    TOKEN_TYPE_NUMBER_HEX,
    TOKEN_TYPE_NUMBER_FLOAT,

    TOKEN_TYPE_COMMENT,
    TOKEN_TYPE_WHITESPACE,

    TOKEN_TYPE_IDENTIFIER,

    TOKEN_TYPE_KEYWORD_BREAK,               // break
    TOKEN_TYPE_KEYWORD_CASE,                // case
    TOKEN_TYPE_KEYWORD_CHAR,                // char
    TOKEN_TYPE_KEYWORD_CONST,               // const
    TOKEN_TYPE_KEYWORD_CONTINUE,            // continue
    TOKEN_TYPE_KEYWORD_DEFAULT,             // default
    TOKEN_TYPE_KEYWORD_DO,                  // do
    TOKEN_TYPE_KEYWORD_DOUBLE,              // double
    TOKEN_TYPE_KEYWORD_ELSE,                // else
    TOKEN_TYPE_KEYWORD_ENUM,                // enum
    TOKEN_TYPE_KEYWORD_FLOAT,               // float
    TOKEN_TYPE_KEYWORD_FOR,                 // for
    TOKEN_TYPE_KEYWORD_IF,                  // if
    TOKEN_TYPE_KEYWORD_INT,                 // int
    TOKEN_TYPE_KEYWORD_LONG,                // long
    TOKEN_TYPE_KEYWORD_RETURN,              // return
    TOKEN_TYPE_KEYWORD_SHORT,               // short
    TOKEN_TYPE_KEYWORD_SIGNED,              // signed
    TOKEN_TYPE_KEYWORD_STRUCT,              // struct
    TOKEN_TYPE_KEYWORD_SWITCH,              // switch
    TOKEN_TYPE_KEYWORD_TYPEDEF,             // typedef
    TOKEN_TYPE_KEYWORD_UNSIGNED,            // unsigned
    TOKEN_TYPE_KEYWORD_VOID,                // void
    TOKEN_TYPE_KEYWORD_WHILE,               // while

    TOKEN_TYPE_OPERATOR_SUM,                // +
    TOKEN_TYPE_OPERATOR_INC,                // ++
    TOKEN_TYPE_OPERATOR_SUM_ASSIGN,         // +=
    TOKEN_TYPE_OPERATOR_SUB,                // -
    TOKEN_TYPE_OPERATOR_DEC,                // --
    TOKEN_TYPE_OPERATOR_SUB_ASSIGN,         // -=
    TOKEN_TYPE_OPERATOR_BIT_AND,            // &
    TOKEN_TYPE_OPERATOR_LOG_AND,            // &&
    TOKEN_TYPE_OPERATOR_BIT_AND_ASSIGN,     // &=
    TOKEN_TYPE_OPERATOR_BIT_OR,             // |
    TOKEN_TYPE_OPERATOR_LOG_OR,             // ||
    TOKEN_TYPE_OPERATOR_BIT_OR_ASSIGN,      // |=
    TOKEN_TYPE_OPERATOR_MUL,                // *
    TOKEN_TYPE_OPERATOR_MUL_ASSIGN,         // *=
    TOKEN_TYPE_OPERATOR_DIV,                // /
    TOKEN_TYPE_OPERATOR_DIV_ASSIGN,         // /=
    TOKEN_TYPE_OPERATOR_MOD,                // %
    TOKEN_TYPE_OPERATOR_MOD_ASSIGN,         // %=
    TOKEN_TYPE_OPERATOR_XOR,                // ^
    TOKEN_TYPE_OPERATOR_XOR_ASSIGN,         // ^=
    TOKEN_TYPE_OPERATOR_ASSIGN,             // =
    TOKEN_TYPE_OPERATOR_EQUAL,              // ==
    TOKEN_TYPE_OPERATOR_NOT,                // !
    TOKEN_TYPE_OPERATOR_NOT_EQUAL,          // !=
    TOKEN_TYPE_OPERATOR_GT,                 // >
    TOKEN_TYPE_OPERATOR_GTE,                // >=
    TOKEN_TYPE_OPERATOR_BIT_RSHIFT,         // >>
    TOKEN_TYPE_OPERATOR_BIT_RSHIFT_ASSIGN,  // >>=
    TOKEN_TYPE_OPERATOR_LT,                 // <
    TOKEN_TYPE_OPERATOR_LTE,                // <=
    TOKEN_TYPE_OPERATOR_BIT_LSHIFT,         // <<
    TOKEN_TYPE_OPERATOR_BIT_LSHIFT_ASSIGN,  // <<=
    TOKEN_TYPE_OPERATOR_TILDE,              // ~

    TOKEN_TYPE_PUNCTUATOR_LPAREN,       // (
    TOKEN_TYPE_PUNCTUATOR_RPAREN,       // )
    TOKEN_TYPE_PUNCTUATOR_LBRACKET,     // [
    TOKEN_TYPE_PUNCTUATOR_RBRACKET,     // ]
    TOKEN_TYPE_PUNCTUATOR_LBRACE,       // {
    TOKEN_TYPE_PUNCTUATOR_RBRACE,       // }
    TOKEN_TYPE_PUNCTUATOR_COMMA,        // ,
    TOKEN_TYPE_PUNCTUATOR_DOT,          // .
    TOKEN_TYPE_PUNCTUATOR_QUESTION,     // ?
    TOKEN_TYPE_PUNCTUATOR_COLON,        // :
    TOKEN_TYPE_PUNCTUATOR_SEMICOLON,    // ;
} token_type_t;

static char *token_type_labels[81] = {
    "None",
    "Unknown",

    "PreDirective",

    "StringLiteral",
    "CharacterLiteral",
    "EnumLiteral",

    "NumberDec",
    "NumberOct",
    "NumberHex",
    "NumberFloat",

    "Comment",
    "Whitespace",

    "Identifier",

    "break",
    "case",
    "char",
    "const",
    "continue",
    "default",
    "do",
    "double",
    "else",
    "enum",
    "float",
    "for",
    "if",
    "int",
    "long",
    "return",
    "short",
    "signed",
    "struct",
    "switch",
    "typedef",
    "unsigned",
    "void",
    "while",

    "+",
    "++",
    "+=",
    "-",
    "--",
    "-=",
    "&",
    "&&",
    "&=",
    "|",
    "||",
    "|=",
    "*",
    "*=",
    "/",
    "/=",
    "%",
    "%=",
    "^",
    "^=",
    "=",
    "==",
    "!",
    "!=",
    ">",
    ">=",
    ">>",
    ">>=",
    "<",
    "<=",
    "<<",
    "<<=",
    "~",

    "(",
    ")",
    "[",
    "]",
    "{",
    "}",
    ",",
    ".",
    "?",
    ":",
    ";"
};

typedef struct position {
    int char_start;
    int char_end;
    int line_start;
    int line_end;
} position_t;

typedef struct lexeme {
    token_type_t token_type;
    position_t position;
    char *token;
} lexeme_t;

// External functions
dynamic_array_t* lexical_analysis(file_buffer_t*);
void lexical_analyzer_print(dynamic_array_t*);
void lexical_analyzer_free(dynamic_array_t*);
#endif //CCOMPILER_LEXICAL_ANALYZER_H

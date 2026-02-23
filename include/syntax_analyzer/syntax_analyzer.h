#ifndef CCOMPILER_SYNTAX_ANALYZER_H
#define CCOMPILER_SYNTAX_ANALYZER_H
#include <lexical_analyzer/lexical_analyzer.h>
#include <utils/dynamic_array.h>

typedef enum ast_type {
    AST_TYPE_ARRAY,
    AST_TYPE_ARRAY_ACCESS,
    AST_TYPE_ARRAY_DECLARATOR,
    AST_TYPE_ASM,
    AST_TYPE_BINARY,
    AST_TYPE_BREAK,
    AST_TYPE_CASE,
    AST_TYPE_COMPOUND_STATEMENT,
    AST_TYPE_CONSTANT,
    AST_TYPE_CONTINUE,
    AST_TYPE_DECLARATION,
    AST_TYPE_DECLARATOR,
    AST_TYPE_DEFAULT,
    AST_TYPE_DO_WHILE,
    AST_TYPE_EMPTY,
    AST_TYPE_ENUM,
    AST_TYPE_ENUMERATOR,
    AST_TYPE_FOR,
    AST_TYPE_FUNCTION,
    AST_TYPE_FUNCTION_CALL,
    AST_TYPE_IF_ELSE,
    AST_TYPE_KEYWORD,
    AST_TYPE_PARAMETER,
    AST_TYPE_POINTER,
    AST_TYPE_POSTFIX,
    AST_TYPE_RETURN,
    AST_TYPE_SPECIFIER,
    AST_TYPE_STRING_LITERAL,
    AST_TYPE_STRUCT,
    AST_TYPE_SWITCH,
    AST_TYPE_TYPEDEF,
    AST_TYPE_TERNARY,
    AST_TYPE_UNARY,
    AST_TYPE_WHILE,
} ast_type_t;

typedef struct ast_node {
    position_t position;
    ast_type_t type;
    void *value;
} ast_node_t;

typedef struct ast_array {
    dynamic_array_t *array;
} ast_array_t;

typedef struct ast_array_access {
    ast_node_t *array;
    ast_node_t *expression;
} ast_array_access_t;

typedef struct ast_array_declarator {
    ast_node_t *array;
    ast_node_t *size;
    ast_node_t *initializer;
} ast_array_declarator_t;

typedef struct ast_binary {
    token_type_t operator;
    ast_node_t *left;
    ast_node_t *right;
} ast_binary_t;

typedef struct ast_case {
    ast_node_t *expression;
    ast_node_t *statement;
} ast_case_t;

typedef struct ast_compound_statement {
    ast_node_t *declarations;
    ast_node_t *statements;
} ast_compound_statement_t;

typedef struct ast_constant {
    token_type_t type;
    char *constant;
} ast_constant_t;

typedef struct ast_declaration {
    ast_node_t *specifiers;
    ast_node_t *declarators;
} ast_declaration_t;

typedef struct ast_declarator {
    ast_node_t *pointer;
    ast_node_t *identifier;
    ast_node_t *array;
    ast_node_t *initializer;
} ast_declarator_t;

typedef struct ast_default {
    ast_node_t *statement;
} ast_default_t;

typedef struct ast_do_while {
    ast_node_t *expression;
    ast_node_t *statement;
} ast_do_while_t;

typedef struct ast_enum {
    ast_node_t *identifier;
    ast_node_t *enumerators;
} ast_enum_t;

typedef struct ast_enumerator {
    ast_node_t *identifier;
    ast_node_t *value;
} ast_enumerator_t;

typedef struct ast_for {
    ast_node_t *expression_low;
    ast_node_t *expression_high;
    ast_node_t *expression_step;
    ast_node_t *statement;
} ast_for_t;

typedef struct ast_function {
    ast_node_t *specifiers;
    ast_node_t *pointer;
    ast_node_t *identifier;
    ast_node_t *parameter_list;
    ast_node_t *compound_statement;
} ast_function_t;

typedef struct ast_function_call {
    ast_node_t *function;
    ast_node_t *arguments;
} ast_function_call_t;

typedef struct ast_if_else {
    ast_node_t *expression;
    ast_node_t *if_statement;
    ast_node_t *else_statement;
} ast_if_else_t;

typedef struct ast_jump_statement {
    ast_type_t jump_type;
    ast_node_t *expression;
} ast_jump_statement_t;

typedef struct ast_keyword {
    token_type_t keyword;
} ast_keyword_t;

typedef struct ast_parameter {
    ast_node_t *specifiers;
    ast_node_t *pointer;
    ast_node_t *identifier;
    ast_node_t *array;
} ast_parameter_t;

typedef struct ast_pointer {
    ast_node_t *child;
} ast_pointer_t;

typedef struct ast_postfix {
    token_type_t operator;
    ast_node_t *expression;
    ast_node_t *expression_right;
} ast_postfix_t;

typedef struct ast_return {
    ast_node_t *expression;
} ast_return_t;

typedef struct ast_specifier {
    token_type_t storage_class_specifier;
    token_type_t type_qualifier;
    ast_node_t *type_specifier;
    ast_node_t *typedef_name;
} ast_specifier_t;

typedef struct ast_string_literal {
    char *value;
    char *label;
} ast_string_literal_t;

typedef struct ast_struct {
    ast_node_t *identifier;
    ast_node_t *declarations;
} ast_struct_t;

typedef struct ast_switch {
    ast_node_t *expression;
    ast_node_t *statement;
} ast_switch_t;

typedef struct ast_ternary {
    ast_node_t *expression;
    ast_node_t *true_expression;
    ast_node_t *false_expression;
} ast_ternary_t;

typedef struct ast_typedef {
    ast_node_t *identifier;
    ast_node_t *declaration_specifier;
} ast_typedef_t;

typedef struct ast_unary {
    token_type_t operator;
    ast_node_t *expression;
} ast_unary_t;

typedef struct ast_while {
    ast_node_t *expression;
    ast_node_t *statement;
} ast_while_t;


ast_node_t* syntax_analysis(
    dynamic_array_t *lexemes,
    dynamic_array_t *string_literals
);

void syntax_analyzer_print(
    ast_node_t *ast_node,
    int indent
);

void syntax_analyzer_free(
    void *element
);
#endif //CCOMPILER_SYNTAX_ANALYZER_H

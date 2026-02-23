#ifndef CCOMPILER_SYNTAX_ANALYZER_HELPERS_H
#define CCOMPILER_SYNTAX_ANALYZER_HELPERS_H
#include <lexical_analyzer/lexical_analyzer.h>
#include "syntax_analyzer.h"

typedef struct syntax_analyzer_scope {
    struct syntax_analyzer_scope *parent;
    dynamic_array_t *type_definitions;
    dynamic_array_t *enumerators;
} syntax_analyzer_scope_t;

typedef struct syntax_analyzer_params {
    syntax_analyzer_scope_t *scope;
    dynamic_array_t *lexemes;
    int lexemes_index;
    dynamic_array_t *string_literals;
    position_t start_position, end_position;
} syntax_analyzer_params_t;


/********************************
 *     VALIDATION FUNCTIONS     *
 ********************************/

int validate_lexeme(
    syntax_analyzer_params_t *params,
    token_type_t token_type
);

int check_lexeme(
    syntax_analyzer_params_t *params,
    token_type_t token_type
);

lexeme_t* current_lexeme(
    syntax_analyzer_params_t *params
);

lexeme_t* previous_lexeme(
    syntax_analyzer_params_t *params
);

int is_typedef_name(
    syntax_analyzer_scope_t *scope,
    char *typedef_name
);

int is_enumerator_constant(
    syntax_analyzer_scope_t *scope,
    char *enumerator_name
);

syntax_analyzer_scope_t* new_syntax_analyzer_scope(
    syntax_analyzer_scope_t *parent
);

void syntax_analyzer_scope_free(
    syntax_analyzer_scope_t *scope
);


/*****************************
 *     FACTORY FUNCTIONS     *
 *****************************/

ast_node_t* new_ast_node(
    ast_type_t ast_type,
    void *element
);

ast_array_t* new_ast_array();

ast_function_t* new_ast_function(
    ast_node_t *specifiers,
    ast_node_t *pointer,
    ast_node_t *identifier,
    ast_node_t *parameter_list,
    ast_node_t *compound_statement
);

ast_case_t* new_ast_case(
    ast_node_t *expression,
    ast_node_t *statement
);

ast_default_t* new_ast_default(
    ast_node_t *statement
);

ast_switch_t* new_ast_switch(
    ast_node_t *expression,
    ast_node_t *statement
);

ast_if_else_t* new_ast_if_else(
    ast_node_t *expression,
    ast_node_t *if_statement,
    ast_node_t *else_statement
);

ast_while_t* new_ast_while(
    ast_node_t *expression,
    ast_node_t *statement
);

ast_do_while_t* new_ast_do_while(
    ast_node_t *expression,
    ast_node_t *statement
);

ast_for_t* new_ast_for(
    ast_node_t *expression_low,
    ast_node_t *expression_high,
    ast_node_t *expression_step,
    ast_node_t *statement
);

ast_enum_t* new_ast_enum(
    ast_node_t *identifier,
    ast_node_t *enumerator
);

ast_enumerator_t* new_ast_enumerator(
    ast_node_t *identifier,
    ast_node_t *value
);

ast_struct_t* new_ast_struct(
    ast_node_t *identifier,
    ast_node_t *declarations
);

ast_binary_t* new_ast_binary(
    token_type_t operator,
    ast_node_t *left,
    ast_node_t *right
);

ast_ternary_t* new_ast_ternary(
    ast_node_t *expression,
    ast_node_t *true_expression,
    ast_node_t *false_expression
);

ast_unary_t* new_ast_unary(
    token_type_t operator,
    ast_node_t *expression
);

ast_postfix_t* new_ast_postfix(
    token_type_t operator,
    ast_node_t *expression,
    ast_node_t *expression_right
);

ast_constant_t* new_ast_constant(
    token_type_t constant_type,
    char *value
);

ast_pointer_t* new_ast_pointer(ast_node_t *child);
ast_keyword_t* new_ast_keyword(token_type_t token_type);
ast_return_t* new_ast_return(ast_node_t *expression);

ast_parameter_t* new_ast_parameter(
    ast_node_t *specifiers,
    ast_node_t *pointer,
    ast_node_t *identifier,
    ast_node_t *array
);

ast_specifier_t* new_ast_specifier(
    token_type_t storage_class_specifier,
    token_type_t type_qualifier,
    ast_node_t *type_specifier,
    ast_node_t *typedef_name
);

ast_typedef_t* new_ast_typedef(
    ast_node_t *identifier,
    ast_node_t *declaration_specifier
);

ast_jump_statement_t* new_ast_jump_statement(
    ast_type_t jump_type,
    ast_node_t *expression
);

ast_function_call_t* new_ast_function_call(
    ast_node_t *function,
    ast_node_t *arguments
);

ast_array_access_t* new_ast_array_access(
    ast_node_t *array,
    ast_node_t *expression
);

ast_declaration_t* new_ast_declaration(
    ast_node_t *specifiers,
    ast_node_t *declarators
);

ast_declarator_t* new_ast_declarator(
    ast_node_t* pointer,
    ast_node_t* identifier,
    ast_node_t* array,
    ast_node_t* initializer
);

ast_compound_statement_t* new_ast_compound_statement(
    ast_node_t *declarations,
    ast_node_t *statements
);

ast_string_literal_t* new_ast_string_literal(
    char *value,
    int label_id
);

ast_array_declarator_t* new_ast_array_declarator(
    ast_node_t *array,
    ast_node_t *size
);


/*****************************
 *     PARSING FUNCTIONS     *
 *****************************/

ast_node_t* parse_statement(syntax_analyzer_params_t *params);
ast_node_t* parse_constant_expression(syntax_analyzer_params_t *params);
ast_node_t* parse_expression(syntax_analyzer_params_t *params);
ast_node_t* parse_declaration(syntax_analyzer_params_t *params);
ast_node_t* parse_declarator(syntax_analyzer_params_t *params);
ast_node_t* parse_compound_statement(syntax_analyzer_params_t *params);
ast_node_t* parse_specifier_qualifier_list(syntax_analyzer_params_t *params);
ast_node_t* parse_pointer(syntax_analyzer_params_t *params);
ast_node_t* parse_identifier(syntax_analyzer_params_t *params);
ast_node_t* parse_declaration_specifiers(syntax_analyzer_params_t *params);
ast_node_t* parse_assignment_expression(syntax_analyzer_params_t *params);
ast_node_t* parse_conditional_expression(syntax_analyzer_params_t *params);
ast_node_t* parse_constant(syntax_analyzer_params_t *params);
ast_node_t* parse_type_name(syntax_analyzer_params_t *params);
ast_node_t* parse_primary_expression(syntax_analyzer_params_t *params);
ast_node_t* parse_postfix_expression_2(syntax_analyzer_params_t *params, ast_node_t*);
ast_node_t* parse_enum_specifier(syntax_analyzer_params_t *params);
ast_node_t* parse_struct_specifier(syntax_analyzer_params_t *params);
ast_node_t* parse_function_declaration(syntax_analyzer_params_t *params, ast_node_t*, ast_node_t*, ast_node_t*);
ast_node_t* parse_root_variable_declaration(syntax_analyzer_params_t *params, ast_node_t*, ast_node_t*, ast_node_t*);
ast_node_t* parse_initializer(syntax_analyzer_params_t *params);
token_type_t parse_unary_operator(syntax_analyzer_params_t *params);
token_type_t parse_assignment_operator(syntax_analyzer_params_t *params);
ast_node_t* parse_declarator_2(syntax_analyzer_params_t *params);

#endif //CCOMPILER_SYNTAX_ANALYZER_HELPERS_H

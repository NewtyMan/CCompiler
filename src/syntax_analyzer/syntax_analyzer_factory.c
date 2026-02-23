#include <syntax_analyzer/syntax_analyzer.h>
#include <syntax_analyzer/syntax_analyzer_helpers.h>
#include <utils/string.h>
#include <stdlib.h>

syntax_analyzer_scope_t* new_syntax_analyzer_scope(syntax_analyzer_scope_t *parent) {
    syntax_analyzer_scope_t *scope = malloc(sizeof(syntax_analyzer_scope_t));
    scope->parent = parent;
    scope->enumerators = dynamic_array_new();
    scope->type_definitions = dynamic_array_new();
    return scope;
}

ast_node_t* new_ast_node(ast_type_t ast_type, void *element) {
    if (ast_type == AST_TYPE_ARRAY && ((ast_array_t*)element)->array->size == 0) {
        return NULL;
    }
    ast_node_t *ast_node = malloc(sizeof(ast_node_t));
    ast_node->type = ast_type;
    ast_node->value = element;
    return ast_node;
}

ast_array_t* new_ast_array() {
    ast_array_t *ast_array = malloc(sizeof(ast_array_t));
    ast_array->array = dynamic_array_new();
    return ast_array;
}

ast_array_access_t* new_ast_array_access(ast_node_t *array, ast_node_t *expression) {
    ast_array_access_t *ast_array_access = malloc(sizeof(ast_array_access_t));
    ast_array_access->array = array;
    ast_array_access->expression = expression;
    return ast_array_access;
}

ast_array_declarator_t* new_ast_array_declarator(ast_node_t *array, ast_node_t *size) {
    ast_array_declarator_t *ast_array_declarator = malloc(sizeof(ast_array_declarator_t));
    ast_array_declarator->array = array;
    ast_array_declarator->size = size;
    ast_array_declarator->initializer = NULL;
    return ast_array_declarator;
}

ast_binary_t* new_ast_binary(token_type_t operator, ast_node_t *left, ast_node_t *right) {
    ast_binary_t *ast_binary = malloc(sizeof(ast_binary_t));
    ast_binary->operator = operator;
    ast_binary->left = left;
    ast_binary->right = right;
    return ast_binary;
}

ast_case_t* new_ast_case(ast_node_t *expression, ast_node_t *statement) {
    ast_case_t *ast_case = malloc(sizeof(ast_case_t));
    ast_case->expression = expression;
    ast_case->statement = statement;
    return ast_case;
}

ast_compound_statement_t* new_ast_compound_statement(
    ast_node_t *declarations,
    ast_node_t *statements
) {
    ast_compound_statement_t *ast_compound_statement = malloc(sizeof(ast_compound_statement_t));
    ast_compound_statement->declarations = declarations;
    ast_compound_statement->statements = statements;
    return ast_compound_statement;
}

ast_constant_t* new_ast_constant(token_type_t constant_type, char *value) {
    ast_constant_t *ast_constant = malloc(sizeof(ast_constant_t));
    ast_constant->type = constant_type;
    ast_constant->constant = calloc(string_len(value) + 1, sizeof(char));
    string_cpy(ast_constant->constant, value);
    return ast_constant;
}

ast_declaration_t* new_ast_declaration(ast_node_t *specifiers, ast_node_t *declarators) {
    ast_declaration_t *ast_declaration = malloc(sizeof(ast_declaration_t));
    ast_declaration->specifiers = specifiers;
    ast_declaration->declarators = declarators;
    return ast_declaration;
}

ast_declarator_t* new_ast_declarator(
    ast_node_t *pointer,
    ast_node_t *identifier,
    ast_node_t *array,
    ast_node_t *initializer
) {
    ast_declarator_t *ast_declarator = malloc(sizeof(ast_declarator_t));
    ast_declarator->pointer = pointer;
    ast_declarator->identifier = identifier;
    ast_declarator->array = array;
    ast_declarator->initializer = initializer;
    return ast_declarator;
}

ast_default_t* new_ast_default(ast_node_t *statement) {
    ast_default_t *ast_default = malloc(sizeof(ast_default_t));
    ast_default->statement = statement;
    return ast_default;
}

ast_do_while_t* new_ast_do_while(ast_node_t *expression, ast_node_t *statement) {
    ast_do_while_t *ast_do_while = malloc(sizeof(ast_do_while_t));
    ast_do_while->expression = expression;
    ast_do_while->statement = statement;
    return ast_do_while;
}

ast_enum_t* new_ast_enum(ast_node_t *identifier, ast_node_t *enumerators) {
    ast_enum_t *ast_enum = malloc(sizeof(ast_enum_t));
    ast_enum->identifier = identifier;
    ast_enum->enumerators = enumerators;
    return ast_enum;
}

ast_enumerator_t* new_ast_enumerator(ast_node_t *identifier, ast_node_t *value) {
    ast_enumerator_t *ast_enumerator = malloc(sizeof(ast_enumerator_t));
    ast_enumerator->identifier = identifier;
    ast_enumerator->value = value;
    return ast_enumerator;
}

ast_for_t* new_ast_for(
    ast_node_t *expression_low,
    ast_node_t *expression_high,
    ast_node_t *expression_step,
    ast_node_t *statement
) {
    ast_for_t *ast_for = malloc(sizeof(ast_for_t));
    ast_for->expression_low = expression_low;
    ast_for->expression_high = expression_high;
    ast_for->expression_step = expression_step;
    ast_for->statement = statement;
    return ast_for;
}

ast_function_t* new_ast_function(
    ast_node_t *specifiers,
    ast_node_t *pointer,
    ast_node_t *identifier,
    ast_node_t *parameter_list,
    ast_node_t *compound_statement
) {
    ast_function_t *ast_function = malloc(sizeof(ast_function_t));
    ast_function->specifiers = specifiers;
    ast_function->pointer = pointer;
    ast_function->identifier = identifier;
    ast_function->parameter_list = parameter_list;
    ast_function->compound_statement = compound_statement;
    return ast_function;
}

ast_function_call_t* new_ast_function_call(ast_node_t *function, ast_node_t *arguments) {
    ast_function_call_t *ast_function_call = malloc(sizeof(ast_function_call_t));
    ast_function_call->function = function;
    ast_function_call->arguments = arguments;
    return ast_function_call;
}

ast_if_else_t* new_ast_if_else(ast_node_t *expression, ast_node_t *if_statement, ast_node_t *else_statement) {
    ast_if_else_t *ast_if_else = malloc(sizeof(ast_if_else_t));
    ast_if_else->expression = expression;
    ast_if_else->if_statement = if_statement;
    ast_if_else->else_statement = else_statement;
    return ast_if_else;
}

ast_keyword_t* new_ast_keyword(token_type_t token_type) {
    ast_keyword_t *ast_keyword = malloc(sizeof(ast_keyword_t));
    ast_keyword->keyword = token_type;
    return ast_keyword;
}

ast_parameter_t*  new_ast_parameter(
    ast_node_t *specifiers,
    ast_node_t *pointer,
    ast_node_t *identifier,
    ast_node_t *array
) {
    ast_parameter_t *ast_parameter = malloc(sizeof(ast_parameter_t));
    ast_parameter->specifiers = specifiers;
    ast_parameter->pointer = pointer;
    ast_parameter->identifier = identifier;
    ast_parameter->array = array;
    return ast_parameter;
}

ast_pointer_t* new_ast_pointer(ast_node_t *child) {
    ast_pointer_t *ast_pointer = malloc(sizeof(ast_pointer_t));
    ast_pointer->child = child;
    return ast_pointer;
}

ast_postfix_t* new_ast_postfix(
    token_type_t operator,
    ast_node_t *expression,
    ast_node_t *expression_right
) {
    ast_postfix_t *ast_postfix = malloc(sizeof(ast_postfix_t));
    ast_postfix->operator = operator;
    ast_postfix->expression = expression;
    ast_postfix->expression_right = expression_right;
    return ast_postfix;
}

ast_return_t* new_ast_return(ast_node_t *expression) {
    ast_return_t *ast_return = malloc(sizeof(ast_return_t));
    ast_return->expression = expression;
    return ast_return;
}

ast_specifier_t* new_ast_specifier(
    token_type_t storage_class_specifier,
    token_type_t type_qualifier,
    ast_node_t *type_specifier,
    ast_node_t *typedef_name
) {
    ast_specifier_t *ast_specifier = malloc(sizeof(ast_specifier_t));
    ast_specifier->storage_class_specifier = storage_class_specifier;
    ast_specifier->type_qualifier = type_qualifier;
    ast_specifier->type_specifier = type_specifier;
    ast_specifier->typedef_name = typedef_name;
    return ast_specifier;
}

ast_string_literal_t* new_ast_string_literal(char *value, int label_id) {
    ast_string_literal_t *ast_string_literal = malloc(sizeof(ast_string_literal_t));
    ast_string_literal->value = calloc(string_len(value) + 1, sizeof(char));
    string_cpy(ast_string_literal->value, value);
    ast_string_literal->label = calloc(8, sizeof(char));
    sprintf(ast_string_literal->label, "str%d", label_id);
    return ast_string_literal;
}

ast_struct_t* new_ast_struct(ast_node_t *identifier, ast_node_t *declarations) {
    ast_struct_t *ast_struct = malloc(sizeof(ast_struct_t));
    ast_struct->identifier = identifier;
    ast_struct->declarations = declarations;
    return ast_struct;
}

ast_switch_t* new_ast_switch(ast_node_t *expression, ast_node_t *statement) {
    ast_switch_t *ast_switch = malloc(sizeof(ast_switch_t));
    ast_switch->expression = expression;
    ast_switch->statement = statement;
    return ast_switch;
}

ast_typedef_t* new_ast_typedef(ast_node_t *identifier, ast_node_t *declaration_specifier) {
    ast_typedef_t *ast_typedef = malloc(sizeof(ast_typedef_t));
    ast_typedef->identifier = identifier;
    ast_typedef->declaration_specifier = declaration_specifier;
    return ast_typedef;
}

ast_ternary_t* new_ast_ternary(ast_node_t *expression, ast_node_t *true_expression, ast_node_t *false_expression) {
    ast_ternary_t *ast_ternary = malloc(sizeof(ast_ternary_t));
    ast_ternary->expression = expression;
    ast_ternary->true_expression = true_expression;
    ast_ternary->false_expression = false_expression;
    return ast_ternary;
}

ast_unary_t* new_ast_unary(token_type_t operator, ast_node_t *expression) {
    ast_unary_t *ast_unary = malloc(sizeof(ast_unary_t));
    ast_unary->operator = operator;
    ast_unary->expression = expression;
    return ast_unary;
}

ast_while_t* new_ast_while(ast_node_t *expression, ast_node_t *statement) {
    ast_while_t *ast_while = malloc(sizeof(ast_while_t));
    ast_while->expression = expression;
    ast_while->statement = statement;
    return ast_while;
}

ast_jump_statement_t* new_ast_jump_statement(ast_type_t jump_type, ast_node_t *expression) {
    ast_jump_statement_t *ast_jump_statement = malloc(sizeof(ast_jump_statement_t));
    ast_jump_statement->jump_type = jump_type;
    ast_jump_statement->expression = expression;
    return ast_jump_statement;
}

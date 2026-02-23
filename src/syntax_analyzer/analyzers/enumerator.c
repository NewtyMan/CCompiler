#include <syntax_analyzer/syntax_analyzer.h>
#include <syntax_analyzer/syntax_analyzer_helpers.h>

ast_node_t* parse_enumerator_2(
    syntax_analyzer_params_t *params,
    ast_node_t *identifier
) {
    if (check_lexeme(params, TOKEN_TYPE_OPERATOR_ASSIGN) == 0) {
        ast_enumerator_t *enumerator = new_ast_enumerator(identifier, NULL);
        return new_ast_node(AST_TYPE_ENUMERATOR, enumerator);
    }

    ast_node_t *constant_expression = parse_constant_expression(params);
    if (constant_expression == NULL) {
        return NULL;
    }

    ast_enumerator_t *enumerator = new_ast_enumerator(identifier, constant_expression);
    return new_ast_node(AST_TYPE_ENUMERATOR, enumerator);
}

ast_node_t* parse_enumerator(syntax_analyzer_params_t *params) {
    ast_node_t *identifier = parse_identifier(params);
    if (identifier == NULL) {
        return NULL;
    }

    ast_node_t *enumerator = parse_enumerator_2(params, identifier);
    if (enumerator == NULL) {
        syntax_analyzer_free(identifier);
        return NULL;
    }

    // Adds enumerator to array in parameters, so that we can later on identify it as enum constant
    ast_constant_t *identifier_constant = identifier->value;
    dynamic_array_add(params->scope->enumerators, identifier_constant->constant);

    return enumerator;
}

ast_node_t* parse_enumerator_list_2(
    syntax_analyzer_params_t *params,
    ast_array_t *array
) {
    if (check_lexeme(params, TOKEN_TYPE_PUNCTUATOR_COMMA) == 0) {
        return new_ast_node(AST_TYPE_ARRAY, array);
    }

    ast_node_t *enumerator = parse_enumerator(params);
    if (enumerator == NULL) {
        return new_ast_node(AST_TYPE_ARRAY, array);
    }

    dynamic_array_add(array->array, enumerator);
    return parse_enumerator_list_2(params, array);
}

ast_node_t* parse_enumerator_list(syntax_analyzer_params_t *params) {
    ast_array_t *array = new_ast_array();
    ast_node_t *enumerator = parse_enumerator(params);
    if (enumerator == NULL) {
        return new_ast_node(AST_TYPE_ARRAY, array);
    }

    dynamic_array_add(array->array, enumerator);
    return parse_enumerator_list_2(params, array);
}

ast_node_t* parse_enum_specifier_2(
    syntax_analyzer_params_t *params,
    ast_node_t *identifier
) {
    if (check_lexeme(params, TOKEN_TYPE_PUNCTUATOR_LBRACE) == 0) {
        ast_enum_t *ast_enum = new_ast_enum(identifier, NULL);
        return new_ast_node(AST_TYPE_ENUM, ast_enum);
    }

    ast_node_t *enumerator_list = parse_enumerator_list(params);
    if (validate_lexeme(params, TOKEN_TYPE_PUNCTUATOR_RBRACE) == 0) {
        syntax_analyzer_free(enumerator_list);
        return NULL;
    }

    ast_enum_t *ast_enum = new_ast_enum(identifier, enumerator_list);
    return new_ast_node(AST_TYPE_ENUM, ast_enum);
}

ast_node_t* parse_enum_specifier(syntax_analyzer_params_t *params) {
    if (check_lexeme(params, TOKEN_TYPE_KEYWORD_ENUM) == 0) {
        return NULL;
    }

    ast_node_t *identifier = parse_identifier(params);
    if (identifier == NULL) {
        return NULL;
    }

    ast_node_t *enum_specifier = parse_enum_specifier_2(params, identifier);
    if (enum_specifier == NULL) {
        syntax_analyzer_free(identifier);
        return NULL;
    }

    return enum_specifier;
}
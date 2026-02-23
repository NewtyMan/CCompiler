#include <syntax_analyzer/syntax_analyzer.h>
#include <syntax_analyzer/syntax_analyzer_helpers.h>

ast_node_t* parse_parameter_declaration_3(
    syntax_analyzer_params_t *params,
    ast_node_t *specifier_qualifier_list,
    ast_node_t *pointer,
    ast_node_t *identifier,
    ast_node_t *array
) {
    if (check_lexeme(params, TOKEN_TYPE_PUNCTUATOR_LBRACKET) == 1) {
        // Since array parameter declarations get passed as pointers, we ignore any constant values and transform the
        // array into a pointer declaration
        parse_constant_expression(params);
        if (validate_lexeme(params, TOKEN_TYPE_PUNCTUATOR_RBRACKET) == 0) {
            syntax_analyzer_free(specifier_qualifier_list);
            syntax_analyzer_free(pointer);
            syntax_analyzer_free(identifier);
            return NULL;
        }

        pointer = new_ast_node(AST_TYPE_POINTER, new_ast_pointer(pointer));
        return parse_parameter_declaration_3(params, specifier_qualifier_list, pointer, identifier, array);
    }

    ast_parameter_t *parameter = new_ast_parameter(specifier_qualifier_list, pointer, identifier, array);
    return new_ast_node(AST_TYPE_PARAMETER, parameter);
}

ast_node_t* parse_parameter_declaration_2(
    syntax_analyzer_params_t *params,
    ast_node_t *specifier_qualifier_list,
    ast_node_t *pointer
) {
    ast_node_t *identifier = parse_identifier(params);
    return parse_parameter_declaration_3(params, specifier_qualifier_list, pointer, identifier, NULL);
}

ast_node_t* parse_parameter_declaration(syntax_analyzer_params_t *params) {
    ast_node_t *specifier_qualifier_list = parse_specifier_qualifier_list(params);
    if (specifier_qualifier_list == NULL) {
        return NULL;
    }

    ast_node_t *pointer = parse_pointer(params);
    return parse_parameter_declaration_2(params, specifier_qualifier_list, pointer);
}

ast_node_t* parse_parameter_list_2(
    syntax_analyzer_params_t *params,
    ast_array_t *array
) {
    if (check_lexeme(params, TOKEN_TYPE_PUNCTUATOR_COMMA) == 0) {
        return new_ast_node(AST_TYPE_ARRAY, array);
    }

    ast_node_t *parameter_declaration = parse_parameter_declaration(params);
    if (parameter_declaration == NULL) {
        return NULL;
    }

    dynamic_array_add(array->array, parameter_declaration);
    return parse_parameter_list_2(params, array);
}

ast_node_t* parse_parameter_list(syntax_analyzer_params_t *params) {
    ast_array_t *array = new_ast_array();
    ast_node_t *parameter_declaration = parse_parameter_declaration(params);
    if (parameter_declaration == NULL) {
        return new_ast_node(AST_TYPE_ARRAY, array);
    }

    dynamic_array_add(array->array, parameter_declaration);
    return parse_parameter_list_2(params, array);
}

ast_node_t* parse_function_declaration_2(
    syntax_analyzer_params_t *params,
    ast_node_t *declaration_specifiers,
    ast_node_t *pointer,
    ast_node_t *identifier,
    ast_node_t *parameter_list
) {
    if (check_lexeme(params, TOKEN_TYPE_PUNCTUATOR_SEMICOLON) == 1) {
        syntax_analyzer_scope_t *scope = params->scope;
        params->scope = params->scope->parent;
        syntax_analyzer_scope_free(scope);

        ast_function_t *function = new_ast_function(declaration_specifiers, pointer, identifier, parameter_list, NULL);
        return new_ast_node(AST_TYPE_FUNCTION, function);
    }

    ast_node_t *compound_statement = parse_compound_statement(params);
    if (compound_statement == NULL) {
        syntax_analyzer_free(declaration_specifiers);
        syntax_analyzer_free(pointer);
        syntax_analyzer_free(identifier);
        syntax_analyzer_free(parameter_list);
        return NULL;
    }

    syntax_analyzer_scope_t *scope = params->scope;
    params->scope = params->scope->parent;
    syntax_analyzer_scope_free(scope);

    ast_function_t *function = new_ast_function(declaration_specifiers, pointer, identifier, parameter_list, compound_statement);
    return new_ast_node(AST_TYPE_FUNCTION, function);
}

ast_node_t* parse_function_declaration(
    syntax_analyzer_params_t *params,
    ast_node_t *declaration_specifiers,
    ast_node_t *pointer,
    ast_node_t *identifier
) {
    syntax_analyzer_scope_t *scope = new_syntax_analyzer_scope(params->scope);
    params->scope = scope;

    ast_node_t *parameter_list = parse_parameter_list(params);
    if (validate_lexeme(params, TOKEN_TYPE_PUNCTUATOR_RPAREN) == 0) {
        syntax_analyzer_free(parameter_list);
        return NULL;
    }

    return parse_function_declaration_2(params, declaration_specifiers, pointer, identifier, parameter_list);
}

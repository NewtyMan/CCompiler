#include <syntax_analyzer/syntax_analyzer.h>
#include <syntax_analyzer/syntax_analyzer_helpers.h>
#include <utils/logger.h>

ast_node_t* parse_type_name(syntax_analyzer_params_t *params) {
    return NULL;
}

ast_node_t* parse_pointer_2(syntax_analyzer_params_t *params, ast_node_t *pointer_node) {
    if (check_lexeme(params, TOKEN_TYPE_OPERATOR_MUL) == 0) {
        return pointer_node;
    }

    ast_pointer_t *pointer = new_ast_pointer(pointer_node);
    return parse_pointer_2(params, new_ast_node(AST_TYPE_POINTER, pointer));
}

ast_node_t* parse_pointer(syntax_analyzer_params_t *params) {
    if (check_lexeme(params, TOKEN_TYPE_OPERATOR_MUL) == 0) {
        return NULL;
    }

    ast_pointer_t *pointer = new_ast_pointer(NULL);
    return parse_pointer_2(params, new_ast_node(AST_TYPE_POINTER, pointer));
}

ast_node_t* parse_initializer_list_2(
    syntax_analyzer_params_t *params,
    ast_array_t *array
) {
    if (check_lexeme(params, TOKEN_TYPE_PUNCTUATOR_COMMA) == 0) {
        return new_ast_node(AST_TYPE_ARRAY, array);
    }

    ast_node_t *initializer = parse_initializer(params);
    if (initializer == NULL) {
        return NULL;
    }

    dynamic_array_add(array->array, initializer);
    return parse_initializer_list_2(params, array);
}

ast_node_t* parse_initializer_list(syntax_analyzer_params_t *params) {
    ast_array_t *array = new_ast_array();
    ast_node_t *initializer = parse_initializer(params);
    if (initializer == NULL) {
        return new_ast_node(AST_TYPE_ARRAY, array);
    }

    dynamic_array_add(array->array, initializer);
    return parse_initializer_list_2(params, array);
}

ast_node_t* parse_initializer_2(
    syntax_analyzer_params_t *params,
    ast_node_t *initializer_list
) {
    // Comma is an optional lexeme, so we don't validate the presence of it
    check_lexeme(params, TOKEN_TYPE_PUNCTUATOR_COMMA);

    if (validate_lexeme(params, TOKEN_TYPE_PUNCTUATOR_RBRACE) == 0) {
        syntax_analyzer_free(initializer_list);
        return NULL;
    }

    return initializer_list;
}

ast_node_t* parse_initializer(syntax_analyzer_params_t *params) {
    ast_node_t *assignment_expression = parse_assignment_expression(params);
    if (assignment_expression != NULL) {
        return assignment_expression;
    }

    if (check_lexeme(params, TOKEN_TYPE_PUNCTUATOR_LBRACE) == 1) {
        ast_node_t *initializer_list = parse_initializer_list(params);
        if (initializer_list == NULL) {
            return NULL;
        }
        return parse_initializer_2(params, initializer_list);
    }

    return NULL;
}

ast_node_t* parse_declarator_2(syntax_analyzer_params_t *params) {
    if (check_lexeme(params, TOKEN_TYPE_PUNCTUATOR_LBRACKET) == 1) {
        ast_node_t *constant_expression = parse_constant_expression(params);
        if (validate_lexeme(params, TOKEN_TYPE_PUNCTUATOR_RBRACKET) == 0) {
            logger_error("Array size declaration must be a constant integer value!");
            syntax_analyzer_free(constant_expression);
            return NULL;
        }

        ast_node_t *array_child = parse_declarator_2(params);
        ast_array_declarator_t *array_declarator = new_ast_array_declarator(array_child, constant_expression);
        return new_ast_node(AST_TYPE_ARRAY_DECLARATOR, array_declarator);
    }

    return NULL;
}

ast_node_t* parse_declarator(syntax_analyzer_params_t *params) {
    ast_node_t *pointer = parse_pointer(params);

    ast_node_t *identifier = parse_identifier(params);
    if (identifier == NULL) {
        syntax_analyzer_free(pointer);
        return NULL;
    }

    ast_node_t *array_declaration = parse_declarator_2(params);
    ast_declarator_t *declarator = new_ast_declarator(pointer, identifier, array_declaration, NULL);
    return new_ast_node(AST_TYPE_DECLARATOR, declarator);
}

ast_node_t* parse_init_declarator_2(
    syntax_analyzer_params_t *params,
    ast_node_t *declarator_node
) {
    if (check_lexeme(params, TOKEN_TYPE_OPERATOR_ASSIGN) == 0) {
        return declarator_node;
    }

    ast_node_t *initializer = parse_initializer(params);
    if (initializer == NULL) {
        syntax_analyzer_free(declarator_node);
        return NULL;
    }

    if (declarator_node->type == AST_TYPE_DECLARATOR) {
        ast_declarator_t *declarator = declarator_node->value;
        declarator->initializer = initializer;
    } else if (declarator_node->type == AST_TYPE_ARRAY_DECLARATOR) {
        ast_array_declarator_t *array_declarator = declarator_node->value;
        array_declarator->initializer = initializer;
    } else {
        printf("[ERROR] [SA] Unknown init declarator type: %s\n", token_type_labels[declarator_node->type]);
    }

    return declarator_node;
}

ast_node_t* parse_init_declarator(syntax_analyzer_params_t *params) {
    ast_node_t *declarator = parse_declarator(params);
    if (declarator == NULL) {
        return NULL;
    }

    return parse_init_declarator_2(params, declarator);
}

ast_node_t* parse_init_declarator_list_2(
    syntax_analyzer_params_t *params,
    ast_array_t *array
) {
    if (check_lexeme(params, TOKEN_TYPE_PUNCTUATOR_COMMA) == 0) {
        return new_ast_node(AST_TYPE_ARRAY, array);
    }

    ast_node_t *init_declarator = parse_init_declarator(params);
    if (init_declarator == NULL) {
        return NULL;
    }

    dynamic_array_add(array->array, init_declarator);
    return parse_init_declarator_list_2(params, array);
}

ast_node_t* parse_init_declarator_list(syntax_analyzer_params_t *params) {
    ast_array_t *array = new_ast_array();
    ast_node_t *init_declarator = parse_init_declarator(params);
    if (init_declarator == NULL) {
        return new_ast_node(AST_TYPE_ARRAY, array);
    }

    dynamic_array_add(array->array, init_declarator);
    return parse_init_declarator_list_2(params, array);
}

ast_node_t* parse_declaration(syntax_analyzer_params_t *params) {
    ast_node_t *declaration_specifiers = parse_declaration_specifiers(params);
    if (declaration_specifiers == NULL) {
        return NULL;
    }

    ast_node_t *init_declarator_list = parse_init_declarator_list(params);
    if (init_declarator_list == NULL) {
        syntax_analyzer_free(declaration_specifiers);
        return NULL;
    }

    if (validate_lexeme(params, TOKEN_TYPE_PUNCTUATOR_SEMICOLON) == 0) {
        syntax_analyzer_free(declaration_specifiers);
        syntax_analyzer_free(init_declarator_list);
        return NULL;
    }

    ast_declaration_t *declaration = new_ast_declaration(declaration_specifiers, init_declarator_list);
    return new_ast_node(AST_TYPE_DECLARATION, declaration);
}

ast_node_t* parse_root_variable_declaration_3(
    syntax_analyzer_params_t *params,
    ast_node_t *declaration_specifiers,
    ast_node_t *pointer,
    ast_node_t *identifier
) {
    if (check_lexeme(params, TOKEN_TYPE_PUNCTUATOR_SEMICOLON) == 1) {
        if (declaration_specifiers->type == AST_TYPE_TYPEDEF) {
            ast_typedef_t *ast_typedef = declaration_specifiers->value;
            ast_typedef->identifier = identifier;

            ast_constant_t *typedef_name = identifier->value;
            dynamic_array_add(params->scope->type_definitions, typedef_name->constant);
            return declaration_specifiers;
        }

        ast_array_t *array = new_ast_array();
        dynamic_array_add(array->array, new_ast_declarator(pointer, identifier, NULL, NULL));

        ast_declaration_t *declaration = new_ast_declaration(declaration_specifiers, new_ast_node(AST_TYPE_ARRAY, array));
        return new_ast_node(AST_TYPE_DECLARATION, declaration);
    }

    return NULL;
}

ast_node_t* parse_root_variable_declaration_2(
    syntax_analyzer_params_t *params,
    ast_node_t *declaration_specifiers,
    ast_node_t *pointer,
    ast_node_t *identifier
) {
    return parse_root_variable_declaration_3(params, declaration_specifiers, pointer, identifier);;
}

ast_node_t* parse_root_variable_declaration(
    syntax_analyzer_params_t *params,
    ast_node_t *declaration_specifiers,
    ast_node_t *pointer,
    ast_node_t *identifier
) {
    return parse_root_variable_declaration_2(params, declaration_specifiers, pointer, identifier);
}

#include <syntax_analyzer/syntax_analyzer.h>
#include <syntax_analyzer/syntax_analyzer_helpers.h>

ast_node_t* parse_struct_declarator_list_2(
    syntax_analyzer_params_t *params,
    ast_array_t *array
) {
    if (check_lexeme(params, TOKEN_TYPE_PUNCTUATOR_COMMA) == 0) {
        return new_ast_node(AST_TYPE_ARRAY, array);
    }

    ast_node_t *declarator = parse_declarator(params);
    if (declarator == NULL) {
        return NULL;
    }

    dynamic_array_add(array->array, declarator);
    return parse_struct_declarator_list_2(params, array);
}

ast_node_t* parse_struct_declarator_list(syntax_analyzer_params_t *params) {
    ast_array_t *array = new_ast_array();
    ast_node_t *declarator = parse_declarator(params);
    if (declarator == NULL) {
        return new_ast_node(AST_TYPE_ARRAY, array);
    }

    dynamic_array_add(array->array, declarator);
    return parse_struct_declarator_list_2(params, array);
}

ast_node_t* parse_struct_declaration(syntax_analyzer_params_t *params) {
    ast_node_t *specifier_qualifier_list = parse_specifier_qualifier_list(params);
    if (specifier_qualifier_list == NULL) {
        return NULL;
    }

    ast_node_t *struct_declarator_list = parse_struct_declarator_list(params);
    if (struct_declarator_list == NULL) {
        syntax_analyzer_free(specifier_qualifier_list);
        return NULL;
    }

    if (validate_lexeme(params, TOKEN_TYPE_PUNCTUATOR_SEMICOLON) == 0) {
        syntax_analyzer_free(specifier_qualifier_list);
        syntax_analyzer_free(struct_declarator_list);
        return NULL;
    }

    ast_declaration_t *declaration = new_ast_declaration(specifier_qualifier_list, struct_declarator_list);
    return new_ast_node(AST_TYPE_DECLARATION, declaration);
}

ast_node_t* parse_struct_declaration_list_2(
    syntax_analyzer_params_t *params,
    ast_array_t *array
) {
    ast_node_t *struct_declaration = parse_struct_declaration(params);
    if (struct_declaration != NULL) {
        dynamic_array_add(array->array, struct_declaration);
        return parse_struct_declaration_list_2(params, array);
    }

    return new_ast_node(AST_TYPE_ARRAY, array);
}

ast_node_t* parse_struct_declaration_list(syntax_analyzer_params_t *params) {
    ast_array_t *array = new_ast_array();
    ast_node_t *struct_declaration = parse_struct_declaration(params);
    if (struct_declaration == NULL) {
        return new_ast_node(AST_TYPE_ARRAY, array);
    }

    dynamic_array_add(array->array, struct_declaration);
    return parse_struct_declaration_list_2(params, array);
}

ast_node_t* parse_struct_specifier_2(
    syntax_analyzer_params_t *params,
    ast_node_t *identifier
) {
    if (check_lexeme(params, TOKEN_TYPE_PUNCTUATOR_LBRACE) == 0) {
        ast_struct_t *ast_struct = new_ast_struct(identifier, NULL);
        return new_ast_node(AST_TYPE_STRUCT, ast_struct);
    }

    ast_node_t *struct_declaration_list = parse_struct_declaration_list(params);
    if (validate_lexeme(params, TOKEN_TYPE_PUNCTUATOR_RBRACE) == 0) {
        syntax_analyzer_free(struct_declaration_list);
        return NULL;
    }

    ast_struct_t *ast_struct = new_ast_struct(identifier, struct_declaration_list);
    return new_ast_node(AST_TYPE_STRUCT, ast_struct);
}

ast_node_t* parse_struct_specifier(syntax_analyzer_params_t *params) {
    if (check_lexeme(params, TOKEN_TYPE_KEYWORD_STRUCT) == 0) {
        return NULL;
    }

    ast_node_t *identifier = parse_identifier(params);
    if (identifier == NULL) {
        return NULL;
    }

    ast_node_t *struct_specifier = parse_struct_specifier_2(params, identifier);
    if (struct_specifier == NULL) {
        syntax_analyzer_free(identifier);
        return NULL;
    }

    return struct_specifier;
}

#include <syntax_analyzer/syntax_analyzer.h>
#include <syntax_analyzer/syntax_analyzer_helpers.h>
#include <stdlib.h>

ast_node_t* parse_external_declaration(syntax_analyzer_params_t *params) {
    ast_node_t *declaration_specifiers = parse_declaration_specifiers(params);
    if (declaration_specifiers == NULL) {
        return NULL;
    }

    if (check_lexeme(params, TOKEN_TYPE_PUNCTUATOR_SEMICOLON) == 1) {
        return declaration_specifiers;
    }

    // Pointer is optional, so we don't enforce validation
    ast_node_t *pointer = parse_pointer(params);

    ast_node_t *identifier = parse_identifier(params);
    if (identifier == NULL) {
        return NULL;
    }

    if (check_lexeme(params, TOKEN_TYPE_PUNCTUATOR_LPAREN) == 1) {
        return parse_function_declaration(params, declaration_specifiers, pointer, identifier);
    }

    ast_node_t *variable = parse_root_variable_declaration(params, declaration_specifiers, pointer, identifier);
    if (variable != NULL) {
        return variable;
    }

    return NULL;
}

ast_node_t* parse_translation_unit_2(
    syntax_analyzer_params_t *params,
    ast_array_t *array
) {
    ast_node_t *external_declaration = parse_external_declaration(params);
    if (external_declaration == NULL) {
        return new_ast_node(AST_TYPE_ARRAY, array);
    }

    dynamic_array_add(array->array, external_declaration);
    return parse_translation_unit_2(params, array);
}

ast_node_t* parse_translation_unit(syntax_analyzer_params_t *params) {
    ast_array_t *array = new_ast_array();
    ast_node_t *external_declaration = parse_external_declaration(params);
    if (external_declaration == NULL) {
        return new_ast_node(AST_TYPE_ARRAY, array);
    }

    dynamic_array_add(array->array, external_declaration);
    return parse_translation_unit_2(params, array);
}

ast_node_t* syntax_analysis(dynamic_array_t *lexemes, dynamic_array_t *string_literals) {
    syntax_analyzer_params_t params;
    params.lexemes = lexemes;
    params.lexemes_index = 0;
    params.string_literals = string_literals;
    params.scope = malloc(sizeof(syntax_analyzer_params_t));

    params.scope->enumerators = dynamic_array_new();
    params.scope->type_definitions = dynamic_array_new();
    params.scope->parent = NULL;

    ast_node_t *ast_node = parse_translation_unit(&params);
    return ast_node;
}

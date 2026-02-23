#include <syntax_analyzer/syntax_analyzer.h>
#include <syntax_analyzer/syntax_analyzer_helpers.h>
#include <utils/string.h>
#include <stdlib.h>

token_type_t parse_assignment_operator(syntax_analyzer_params_t *params) {
    const int operator_count = 11;
    const token_type_t operators[] = {
        TOKEN_TYPE_OPERATOR_ASSIGN,
        TOKEN_TYPE_OPERATOR_MUL_ASSIGN,
        TOKEN_TYPE_OPERATOR_DIV_ASSIGN,
        TOKEN_TYPE_OPERATOR_MOD_ASSIGN,
        TOKEN_TYPE_OPERATOR_SUM_ASSIGN,
        TOKEN_TYPE_OPERATOR_SUB_ASSIGN,
        TOKEN_TYPE_OPERATOR_BIT_LSHIFT_ASSIGN,
        TOKEN_TYPE_OPERATOR_BIT_RSHIFT_ASSIGN,
        TOKEN_TYPE_OPERATOR_BIT_AND_ASSIGN,
        TOKEN_TYPE_OPERATOR_BIT_OR_ASSIGN,
        TOKEN_TYPE_OPERATOR_XOR_ASSIGN,
    };

    int i;
    for (i = 0; i < operator_count; i++) {
        if (check_lexeme(params, operators[i]) == 1) {
            return operators[i];
        }
    }

    return TOKEN_TYPE_NONE;
}

token_type_t parse_unary_operator(syntax_analyzer_params_t *params) {
    const int operator_count = 8;
    const token_type_t operators[] = {
        TOKEN_TYPE_OPERATOR_BIT_AND,
        TOKEN_TYPE_OPERATOR_MUL,
        TOKEN_TYPE_OPERATOR_SUM,
        TOKEN_TYPE_OPERATOR_SUB,
        TOKEN_TYPE_OPERATOR_TILDE,
        TOKEN_TYPE_OPERATOR_NOT,
        TOKEN_TYPE_OPERATOR_INC,
        TOKEN_TYPE_OPERATOR_DEC
    };

    int i;
    for (i = 0; i < operator_count; i++) {
        if (check_lexeme(params, operators[i]) == 1) {
            return operators[i];
        }
    }

    return TOKEN_TYPE_NONE;
}

ast_node_t* parse_constant(syntax_analyzer_params_t *params) {
    if (
        check_lexeme(params, TOKEN_TYPE_NUMBER_DEC) == 1 ||
        check_lexeme(params, TOKEN_TYPE_NUMBER_HEX) == 1 ||
        check_lexeme(params, TOKEN_TYPE_NUMBER_OCT) == 1 ||
        check_lexeme(params, TOKEN_TYPE_NUMBER_FLOAT) == 1
    ) {
        lexeme_t *lexeme = previous_lexeme(params);
        ast_constant_t *constant = new_ast_constant(lexeme->token_type, lexeme->token);
        return new_ast_node(AST_TYPE_CONSTANT, constant);
    }

    if (check_lexeme(params, TOKEN_TYPE_CHARACTER_LITERAL) == 1) {
        // With character literals, we want to omit the leading and ending apostrophe
        lexeme_t *lexeme = previous_lexeme(params);
        char *token = string_substr(lexeme->token, 1, string_len(lexeme->token) - 2);
        ast_constant_t *constant = new_ast_constant(lexeme->token_type, token);
        ast_node_t *node = new_ast_node(AST_TYPE_CONSTANT, constant);
        free(token);
        return node;
    }

    return NULL;
}

ast_node_t* parse_typedef_name(syntax_analyzer_params_t *params) {
    lexeme_t *lexeme = current_lexeme(params);
    if (lexeme != NULL && is_typedef_name(params->scope, lexeme->token) == 1) {
        ast_node_t *identifier = parse_identifier(params);
        return identifier;
    }

    return NULL;
}

ast_node_t* parse_type_specifier(syntax_analyzer_params_t *params) {
    if (
        check_lexeme(params, TOKEN_TYPE_KEYWORD_VOID) == 1 ||
        check_lexeme(params, TOKEN_TYPE_KEYWORD_CHAR) == 1 ||
        check_lexeme(params, TOKEN_TYPE_KEYWORD_INT) == 1 ||
        check_lexeme(params, TOKEN_TYPE_KEYWORD_FLOAT) == 1
    ) {
        lexeme_t *lexeme = previous_lexeme(params);
        ast_keyword_t *keyword = new_ast_keyword(lexeme->token_type);
        return new_ast_node(AST_TYPE_KEYWORD, keyword);
    }

    ast_node_t *struct_specifier = parse_struct_specifier(params);
    if (struct_specifier != NULL) {
        return struct_specifier;
    }

    ast_node_t *enum_specifier = parse_enum_specifier(params);
    if (enum_specifier != NULL) {
        return enum_specifier;
    }

    ast_node_t *typedef_name = parse_typedef_name(params);
    if (typedef_name != NULL) {
        return typedef_name;
    }

    return NULL;
}

ast_node_t* parse_specifier_qualifier_list(syntax_analyzer_params_t *params) {
    if (check_lexeme(params, TOKEN_TYPE_KEYWORD_CONST) == 1) {
        ast_node_t *type_specifier = parse_type_specifier(params);
        if (type_specifier == NULL) {
            return NULL;
        }

        ast_specifier_t *specifier = new_ast_specifier(TOKEN_TYPE_NONE, TOKEN_TYPE_KEYWORD_CONST, type_specifier, NULL);
        return new_ast_node(AST_TYPE_SPECIFIER, specifier);
    }

    ast_node_t *type_specifier = parse_type_specifier(params);
    if (type_specifier == NULL) {
        return NULL;
    }

    ast_specifier_t *specifier = new_ast_specifier(TOKEN_TYPE_NONE, TOKEN_TYPE_NONE, type_specifier, NULL);
    return new_ast_node(AST_TYPE_SPECIFIER, specifier);
}

ast_node_t* parse_declaration_specifiers_helper(
    token_type_t storage_class_specifier,
    token_type_t type_qualifier,
    ast_node_t *type_specifier
) {
    switch (type_specifier->type) {
        case AST_TYPE_ENUM:;
            ast_enum_t *ast_enum = type_specifier->value;
            if (ast_enum->enumerators != NULL) {
                return type_specifier;
            }
            break;
        case AST_TYPE_STRUCT:;
            ast_struct_t *ast_struct = type_specifier->value;
            if (ast_struct->declarations != NULL) {
                return type_specifier;
            }
            break;
        default:
            break;
    }

    ast_specifier_t *specifier = new_ast_specifier(storage_class_specifier, type_qualifier, type_specifier, NULL);
    return new_ast_node(AST_TYPE_SPECIFIER, specifier);
}

ast_node_t* parse_declaration_specifiers(syntax_analyzer_params_t *params) {
    if (check_lexeme(params, TOKEN_TYPE_KEYWORD_TYPEDEF) == 1) {
        ast_node_t *type_specifier = parse_type_specifier(params);
        if (type_specifier == NULL) {
            return NULL;
        }

        ast_node_t *declaration_specifier = parse_declaration_specifiers_helper(TOKEN_TYPE_KEYWORD_TYPEDEF, TOKEN_TYPE_NONE, type_specifier);
        ast_typedef_t *ast_typedef = new_ast_typedef(NULL, declaration_specifier);
        return new_ast_node(AST_TYPE_TYPEDEF, ast_typedef);
    }

    if (check_lexeme(params, TOKEN_TYPE_KEYWORD_CONST) == 1) {
        ast_node_t *type_specifier = parse_type_specifier(params);
        if (type_specifier == NULL) {
            return NULL;
        }

        return parse_declaration_specifiers_helper(TOKEN_TYPE_NONE, TOKEN_TYPE_KEYWORD_CONST, type_specifier);
    }

    ast_node_t *type_specifier = parse_type_specifier(params);
    if (type_specifier == NULL) {
        return NULL;
    }

    return parse_declaration_specifiers_helper(TOKEN_TYPE_NONE, TOKEN_TYPE_NONE, type_specifier);
}

ast_node_t* parse_identifier(syntax_analyzer_params_t *params) {
    if (check_lexeme(params, TOKEN_TYPE_IDENTIFIER) == 0) {
        return NULL;
    }

    const lexeme_t *lexeme = previous_lexeme(params);
    if (lexeme != NULL && is_enumerator_constant(params->scope, lexeme->token) == 1) {
        ast_constant_t *constant = new_ast_constant(TOKEN_TYPE_ENUM_LITERAL, lexeme->token);
        return new_ast_node(AST_TYPE_CONSTANT, constant);
    }

    ast_constant_t *constant = new_ast_constant(TOKEN_TYPE_IDENTIFIER, lexeme->token);
    return new_ast_node(AST_TYPE_CONSTANT, constant);
}

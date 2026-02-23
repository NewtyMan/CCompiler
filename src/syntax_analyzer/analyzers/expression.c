#include <syntax_analyzer/syntax_analyzer.h>
#include <syntax_analyzer/syntax_analyzer_helpers.h>
#include <stdlib.h>

int string_label_count = 1;

ast_node_t* parse_argument_expression_list_2(
    syntax_analyzer_params_t *params,
    ast_array_t *array
) {
    if (check_lexeme(params, TOKEN_TYPE_PUNCTUATOR_COMMA) == 0) {
        return new_ast_node(AST_TYPE_ARRAY, array);
    }

    ast_node_t *assignment_expression = parse_assignment_expression(params);
    if (assignment_expression == NULL) {
        return NULL;
    }

    dynamic_array_add(array->array, assignment_expression);
    return parse_argument_expression_list_2(params, array);
}

ast_node_t* parse_argument_expression_list(syntax_analyzer_params_t *params) {
    ast_array_t *array = new_ast_array();
    ast_node_t *assignment_expression = parse_assignment_expression(params);
    if (assignment_expression == NULL) {
        return new_ast_node(AST_TYPE_ARRAY, array);
    }

    dynamic_array_add(array->array, assignment_expression);
    return parse_argument_expression_list_2(params, array);
}

ast_node_t* parse_primary_expression_2(syntax_analyzer_params_t *params) {
    ast_node_t *expression = parse_expression(params);
    if (expression != NULL) {
        if (validate_lexeme(params, TOKEN_TYPE_PUNCTUATOR_RPAREN) == 0) {
            syntax_analyzer_free(expression);
            return NULL;
        }

        return expression;
    }

    ast_node_t *type_name = parse_type_name(params);
    if (type_name != NULL) {
        if (validate_lexeme(params, TOKEN_TYPE_PUNCTUATOR_RPAREN) == 0) {
            syntax_analyzer_free(type_name);
            return NULL;
        }
        // X: Casting
    }

    return NULL;
}


ast_node_t* parse_primary_expression(syntax_analyzer_params_t *params) {
    ast_node_t *identifier = parse_identifier(params);
    if (identifier != NULL) {
        return identifier;
    }

    ast_node_t *constant = parse_constant(params);
    if (constant != NULL) {
        return constant;
    }

    if (check_lexeme(params, TOKEN_TYPE_STRING_LITERAL) == 1) {
        const lexeme_t *lexeme = previous_lexeme(params);
        ast_string_literal_t *ast_string_literal = new_ast_string_literal(lexeme->token, string_label_count++);
        dynamic_array_add(params->string_literals, ast_string_literal);
        return new_ast_node(AST_TYPE_STRING_LITERAL, ast_string_literal);
    }

    if (check_lexeme(params, TOKEN_TYPE_PUNCTUATOR_LPAREN) == 1) {
        return parse_primary_expression_2(params);
    }

    return NULL;
}

ast_node_t* parse_postfix_expression_3(
    syntax_analyzer_params_t *params,
    ast_node_t *left
) {
    ast_node_t *argument_expression_list = parse_argument_expression_list(params);
    if (validate_lexeme(params, TOKEN_TYPE_PUNCTUATOR_RPAREN) == 0) {
        syntax_analyzer_free(argument_expression_list);
        syntax_analyzer_free(left);
        return NULL;
    }

    ast_function_call_t *function_call = new_ast_function_call(left, argument_expression_list);
    return new_ast_node(AST_TYPE_FUNCTION_CALL, function_call);
}


ast_node_t* parse_postfix_expression_2(
    syntax_analyzer_params_t *params,
    ast_node_t *left
) {
    // X: DOT, ACCESS
    if (check_lexeme(params, TOKEN_TYPE_PUNCTUATOR_LBRACKET) == 1) {
        ast_node_t *expression = parse_expression(params);
        if (expression == NULL) {
            syntax_analyzer_free(left);
            return NULL;
        }

        if (validate_lexeme(params, TOKEN_TYPE_PUNCTUATOR_RBRACKET) == 0) {
            syntax_analyzer_free(left);
            syntax_analyzer_free(expression);
            return NULL;
        }

        ast_array_access_t *array_access = new_ast_array_access(left, expression);
        return parse_postfix_expression_2(params, new_ast_node(AST_TYPE_ARRAY_ACCESS, array_access));
    }

    if (check_lexeme(params, TOKEN_TYPE_PUNCTUATOR_LPAREN) == 1) {
        return parse_postfix_expression_3(params, left);
    }

    if (check_lexeme(params, TOKEN_TYPE_OPERATOR_INC) == 1) {
        ast_postfix_t *postfix = new_ast_postfix(TOKEN_TYPE_OPERATOR_INC, left, NULL);
        return parse_postfix_expression_2(params, new_ast_node(AST_TYPE_POSTFIX, postfix));
    }

    if (check_lexeme(params, TOKEN_TYPE_OPERATOR_DEC) == 1) {
        ast_postfix_t *postfix = new_ast_postfix(TOKEN_TYPE_OPERATOR_DEC, left, NULL);
        return parse_postfix_expression_2(params, new_ast_node(AST_TYPE_POSTFIX, postfix));
    }

    if (check_lexeme(params, TOKEN_TYPE_PUNCTUATOR_DOT) == 1) {
        ast_node_t *identifier = parse_identifier(params);
        if (identifier == NULL) {
            syntax_analyzer_free(left);
            return NULL;
        }

        ast_postfix_t *postfix = new_ast_postfix(TOKEN_TYPE_PUNCTUATOR_DOT, left, identifier);
        return parse_postfix_expression_2(params, new_ast_node(AST_TYPE_POSTFIX, postfix));
    }

    return left;
}

ast_node_t* parse_postfix_expression(syntax_analyzer_params_t *params) {
    ast_node_t *primary_expression = parse_primary_expression(params);
    if (primary_expression == NULL) {
        return NULL;
    }

    return parse_postfix_expression_2(params, primary_expression);
}

ast_node_t* parse_unary_expression(syntax_analyzer_params_t *params) {
    const token_type_t unary_operator = parse_unary_operator(params);
    if (unary_operator != TOKEN_TYPE_NONE) {
        ast_node_t *unary_expression = parse_unary_expression(params);
        if (unary_expression == NULL) {
            return NULL;
        }

        ast_unary_t *unary = new_ast_unary(unary_operator, unary_expression);
        return new_ast_node(AST_TYPE_UNARY, unary);
    }

    // TODO: Add support for sizeof built-in operator
    return parse_postfix_expression(params);
}

ast_node_t* parse_multiplicative_expression_2(
    syntax_analyzer_params_t *params,
    ast_node_t *left
) {
    const lexeme_t *lexeme = current_lexeme(params);
    if (
        check_lexeme(params, TOKEN_TYPE_OPERATOR_MUL) == 0  &&
        check_lexeme(params, TOKEN_TYPE_OPERATOR_DIV) == 0  &&
        check_lexeme(params, TOKEN_TYPE_OPERATOR_MOD) == 0
    ) {
        return left;
    }

    ast_node_t *unary_expression = parse_unary_expression(params);
    if (unary_expression == NULL) {
        syntax_analyzer_free(left);
        return NULL;
    }

    ast_binary_t *binary = new_ast_binary(lexeme->token_type, left, unary_expression);
    return parse_multiplicative_expression_2(params, new_ast_node(AST_TYPE_BINARY, binary));
}

ast_node_t* parse_multiplicative_expression(syntax_analyzer_params_t *params) {
    ast_node_t *unary_expression = parse_unary_expression(params);
    if (unary_expression == NULL) {
        return NULL;
    }

    return parse_multiplicative_expression_2(params, unary_expression);
}


ast_node_t* parse_additive_expression_2(
    syntax_analyzer_params_t *params,
    ast_node_t *left
) {
    if (
        check_lexeme(params, TOKEN_TYPE_OPERATOR_SUM) == 0 &&
        check_lexeme(params, TOKEN_TYPE_OPERATOR_SUB) == 0
    ) {
        return left;
    }

    const lexeme_t *lexeme = previous_lexeme(params);
    ast_node_t *multiplicative_expression = parse_multiplicative_expression(params);
    if (multiplicative_expression == NULL) {
        syntax_analyzer_free(left);
        return NULL;
    }

    ast_binary_t *binary = new_ast_binary(lexeme->token_type, left, multiplicative_expression);
    return parse_additive_expression_2(params, new_ast_node(AST_TYPE_BINARY, binary));
}


ast_node_t* parse_additive_expression(syntax_analyzer_params_t *params) {
    ast_node_t *multiplicative_expression = parse_multiplicative_expression(params);
    if (multiplicative_expression == NULL) {
        return NULL;
    }

    return parse_additive_expression_2(params, multiplicative_expression);
}

ast_node_t* parse_shift_expression_2(
    syntax_analyzer_params_t *params,
    ast_node_t *left
) {
    if (
        check_lexeme(params, TOKEN_TYPE_OPERATOR_BIT_LSHIFT) == 0 &&
        check_lexeme(params, TOKEN_TYPE_OPERATOR_BIT_RSHIFT) == 0
    ) {
        return left;
    }

    const lexeme_t *lexeme = previous_lexeme(params);
    ast_node_t *additive_expression = parse_additive_expression(params);
    if (additive_expression == NULL) {
        syntax_analyzer_free(left);
        return NULL;
    }

    ast_binary_t *binary = new_ast_binary(lexeme->token_type, left, additive_expression);
    return parse_shift_expression_2(params, new_ast_node(AST_TYPE_BINARY, binary));
}


ast_node_t* parse_shift_expression(syntax_analyzer_params_t *params) {
    ast_node_t *additive_expression = parse_additive_expression(params);
    if (additive_expression == NULL) {
        return NULL;
    }

    return parse_shift_expression_2(params, additive_expression);
}


ast_node_t* parse_relational_expression_2(
    syntax_analyzer_params_t *params,
    ast_node_t *left
) {
    if (
        check_lexeme(params, TOKEN_TYPE_OPERATOR_LT) == 0  &&
        check_lexeme(params, TOKEN_TYPE_OPERATOR_GT) == 0 &&
        check_lexeme(params, TOKEN_TYPE_OPERATOR_LTE) == 0 &&
        check_lexeme(params, TOKEN_TYPE_OPERATOR_GTE) == 0
    ) {
        return left;
    }

    const lexeme_t *lexeme = previous_lexeme(params);
    ast_node_t *shift_expression = parse_shift_expression(params);
    if (shift_expression == NULL) {
        syntax_analyzer_free(left);
        return NULL;
    }

    ast_binary_t *binary = new_ast_binary(lexeme->token_type, left, shift_expression);
    return parse_relational_expression_2(params, new_ast_node(AST_TYPE_BINARY, binary));
}

ast_node_t* parse_relational_expression(syntax_analyzer_params_t *params) {
    ast_node_t *shift_expression = parse_shift_expression(params);
    if (shift_expression == NULL) {
        return NULL;
    }

    return parse_relational_expression_2(params, shift_expression);
}

ast_node_t* parse_equality_expression_2(
    syntax_analyzer_params_t *params,
    ast_node_t *left
) {
    if (
        check_lexeme(params, TOKEN_TYPE_OPERATOR_EQUAL) == 0 &&
        check_lexeme(params, TOKEN_TYPE_OPERATOR_NOT_EQUAL) == 0
    ) {
        return left;
    }

    const lexeme_t *lexeme = previous_lexeme(params);
    ast_node_t *relational_expression = parse_relational_expression(params);
    if (relational_expression == NULL) {
        syntax_analyzer_free(left);
        return NULL;
    }

    ast_binary_t *binary = new_ast_binary(lexeme->token_type, left, relational_expression);
    return parse_equality_expression_2(params, new_ast_node(AST_TYPE_BINARY, binary));
}

ast_node_t* parse_equality_expression(syntax_analyzer_params_t *params) {
    ast_node_t *relational_expression = parse_relational_expression(params);
    if (relational_expression == NULL) {
        return NULL;
    }

    return parse_equality_expression_2(params, relational_expression);
}


ast_node_t* parse_and_expression_2(
    syntax_analyzer_params_t *params,
    ast_node_t *left
) {
    if (check_lexeme(params, TOKEN_TYPE_OPERATOR_BIT_AND) == 0) {
        return left;
    }

    ast_node_t *equality_expression = parse_equality_expression(params);
    if (equality_expression == NULL) {
        syntax_analyzer_free(left);
        return NULL;
    }

    ast_binary_t *binary = new_ast_binary(TOKEN_TYPE_OPERATOR_BIT_AND, left, equality_expression);
    return parse_and_expression_2(params, new_ast_node(AST_TYPE_BINARY, binary));
}


ast_node_t* parse_and_expression(syntax_analyzer_params_t *params) {
    ast_node_t *equality_expression = parse_equality_expression(params);
    if (equality_expression == NULL) {
        return NULL;
    }

    return parse_and_expression_2(params, equality_expression);
}


ast_node_t* parse_exclusive_or_expression_2(
    syntax_analyzer_params_t *params,
    ast_node_t *left
) {
    if (check_lexeme(params, TOKEN_TYPE_OPERATOR_XOR) == 0) {
        return left;
    }

    ast_node_t *and_expression = parse_and_expression(params);
    if (and_expression == NULL) {
        syntax_analyzer_free(left);
        return NULL;
    }

    ast_binary_t *binary = new_ast_binary(TOKEN_TYPE_OPERATOR_XOR, left, and_expression);
    return parse_exclusive_or_expression_2(params, new_ast_node(AST_TYPE_BINARY, binary));
}

ast_node_t* parse_exclusive_or_expression(syntax_analyzer_params_t *params) {
    ast_node_t *and_expression = parse_and_expression(params);
    if (and_expression == NULL) {
        return NULL;
    }

    return parse_exclusive_or_expression_2(params, and_expression);
}

ast_node_t *parse_inclusive_or_expression_2(
    syntax_analyzer_params_t *params,
    ast_node_t *left
) {
    if (check_lexeme(params, TOKEN_TYPE_OPERATOR_BIT_OR) == 0) {
        return left;
    }

    ast_node_t *exclusive_or_expression = parse_exclusive_or_expression(params);
    if (exclusive_or_expression == NULL) {
        syntax_analyzer_free(left);
        return NULL;
    }

    ast_binary_t *binary = new_ast_binary(TOKEN_TYPE_OPERATOR_BIT_OR, left, exclusive_or_expression);
    return parse_inclusive_or_expression_2(params, new_ast_node(AST_TYPE_BINARY, binary));
}

ast_node_t *parse_inclusive_or_expression(syntax_analyzer_params_t *params) {
    ast_node_t *exclusive_or_expression = parse_exclusive_or_expression(params);
    if (exclusive_or_expression == NULL) {
        return NULL;
    }

    return parse_inclusive_or_expression_2(params, exclusive_or_expression);
}


ast_node_t *parse_logical_and_expression_2(
    syntax_analyzer_params_t *params,
    ast_node_t *left
) {
    if (check_lexeme(params, TOKEN_TYPE_OPERATOR_LOG_AND) == 0) {
        return left;
    }

    ast_node_t *inclusive_or_expression = parse_inclusive_or_expression(params);
    if (inclusive_or_expression == NULL) {
        syntax_analyzer_free(left);
        return NULL;
    }

    ast_binary_t *binary = new_ast_binary(TOKEN_TYPE_OPERATOR_LOG_AND, left, inclusive_or_expression);
    return parse_logical_and_expression_2(params, new_ast_node(AST_TYPE_BINARY, binary));
}


ast_node_t *parse_logical_and_expression(syntax_analyzer_params_t *params) {
    ast_node_t *inclusive_or_expression = parse_inclusive_or_expression(params);
    if (inclusive_or_expression == NULL) {
        return NULL;
    }

    return parse_logical_and_expression_2(params, inclusive_or_expression);
}

ast_node_t *parse_logical_or_expression_2(
    syntax_analyzer_params_t *params,
    ast_node_t *left
) {
    if (check_lexeme(params, TOKEN_TYPE_OPERATOR_LOG_OR) == 0) {
        return left;
    }

    ast_node_t *logical_and_expression = parse_logical_and_expression(params);
    if (logical_and_expression == NULL) {
        syntax_analyzer_free(left);
        return NULL;
    }

    ast_binary_t *binary = new_ast_binary(TOKEN_TYPE_OPERATOR_LOG_OR, left, logical_and_expression);
    return parse_logical_or_expression_2(params, new_ast_node(AST_TYPE_BINARY, binary));
}

ast_node_t *parse_logical_or_expression(syntax_analyzer_params_t *params) {
    ast_node_t *logical_and_expression = parse_logical_and_expression(params);
    if (logical_and_expression == NULL) {
        return NULL;
    }

    return parse_logical_or_expression_2(params, logical_and_expression);
}

ast_node_t *parse_conditional_expression_2(
    syntax_analyzer_params_t *params,
    ast_node_t *logical_or_expression
) {
    if (check_lexeme(params, TOKEN_TYPE_PUNCTUATOR_QUESTION) == 0) {
        return logical_or_expression;
    }

    ast_node_t *true_expression = parse_expression(params);
    if (true_expression == NULL) {
        return NULL;
    }

    if (validate_lexeme(params, TOKEN_TYPE_PUNCTUATOR_COLON) == 0) {
        syntax_analyzer_free(true_expression);
        return NULL;
    }

    ast_node_t *false_expression = parse_conditional_expression(params);
    if (false_expression == NULL) {
        syntax_analyzer_free(true_expression);
        return NULL;
    }

    ast_ternary_t *ternary = new_ast_ternary(logical_or_expression, true_expression, false_expression);
    return new_ast_node(AST_TYPE_TERNARY, ternary);
}

ast_node_t *parse_conditional_expression(syntax_analyzer_params_t *params) {
    ast_node_t *logical_or_expression = parse_logical_or_expression(params);
    if (logical_or_expression == NULL) {
        return NULL;
    }

    ast_node_t *conditional_expression = parse_conditional_expression_2(params, logical_or_expression);
    if (conditional_expression == NULL) {
        syntax_analyzer_free(logical_or_expression);
        return NULL;
    }

    return conditional_expression;
}

ast_node_t *parse_constant_expression(syntax_analyzer_params_t *params) {
    return parse_conditional_expression(params);
}

ast_node_t *parse_assignment_expression_2(
    syntax_analyzer_params_t *params,
    ast_node_t *left
) {
    const token_type_t assignment_operator = parse_assignment_operator(params);
    if (assignment_operator == TOKEN_TYPE_NONE) {
        return left;
    }

    ast_node_t *right = parse_assignment_expression(params);
    if (right == NULL) {
        syntax_analyzer_free(left);
        return NULL;
    }

    ast_binary_t *binary = new_ast_binary(assignment_operator, left, right);
    return new_ast_node(AST_TYPE_BINARY, binary);
}

ast_node_t *parse_assignment_expression(syntax_analyzer_params_t *params) {
    ast_node_t *conditional_expression = parse_conditional_expression(params);
    if (conditional_expression == NULL) {
        return NULL;
    }

    return parse_assignment_expression_2(params, conditional_expression);
}

ast_node_t* parse_expression_2(
    syntax_analyzer_params_t *params,
    ast_array_t *array
) {
    if (check_lexeme(params, TOKEN_TYPE_PUNCTUATOR_COMMA) == 0) {
        return new_ast_node(AST_TYPE_ARRAY, array);
    }

    ast_node_t *assignment_expression = parse_assignment_expression(params);
    if (assignment_expression == NULL) {
        return NULL;
    }

    dynamic_array_add(array->array, assignment_expression);
    return parse_expression_2(params, array);
}

ast_node_t* parse_expression(syntax_analyzer_params_t *params) {
    ast_array_t *array = new_ast_array();
    ast_node_t *assignment_expression = parse_assignment_expression(params);
    if (assignment_expression == NULL) {
        return new_ast_node(AST_TYPE_ARRAY, array);
    }

    dynamic_array_add(array->array, assignment_expression);
    return parse_expression_2(params, array);
}

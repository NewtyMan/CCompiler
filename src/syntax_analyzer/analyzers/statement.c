#include <syntax_analyzer/syntax_analyzer.h>
#include <syntax_analyzer/syntax_analyzer_helpers.h>
#include <stdlib.h>

ast_node_t *parse_jump_statement_2(syntax_analyzer_params_t *params) {
    ast_node_t *expression = parse_expression(params);
    if (validate_lexeme(params, TOKEN_TYPE_PUNCTUATOR_SEMICOLON) == 0) {
        syntax_analyzer_free(expression);
        return NULL;
    }
    ast_return_t *return_value = new_ast_return(expression);
    return new_ast_node(AST_TYPE_RETURN, return_value);
}

ast_node_t *parse_jump_statement(syntax_analyzer_params_t *params) {
    if (check_lexeme(params, TOKEN_TYPE_KEYWORD_CONTINUE) == 1) {
        if (validate_lexeme(params, TOKEN_TYPE_PUNCTUATOR_SEMICOLON) == 0) {
            return NULL;
        }
        return new_ast_node(AST_TYPE_CONTINUE, NULL);
    }

    if (check_lexeme(params, TOKEN_TYPE_KEYWORD_BREAK) == 1) {
        if (validate_lexeme(params, TOKEN_TYPE_PUNCTUATOR_SEMICOLON) == 0) {
            return NULL;
        }
        return new_ast_node(AST_TYPE_BREAK, NULL);
    }

    if (check_lexeme(params, TOKEN_TYPE_KEYWORD_RETURN) == 1) {
        return parse_jump_statement_2(params);
    }

    return NULL;
}

ast_node_t* parse_labeled_statement(syntax_analyzer_params_t *params) {
    if (check_lexeme(params, TOKEN_TYPE_KEYWORD_CASE) == 1) {
        ast_node_t *constant_expression = parse_constant_expression(params);
        if (constant_expression == NULL) {
            return NULL;
        }

        if (validate_lexeme(params, TOKEN_TYPE_PUNCTUATOR_COLON) == 0) {
            syntax_analyzer_free(constant_expression);
            return NULL;
        }

        ast_node_t *statement = parse_statement(params);
        if (statement == NULL) {
            syntax_analyzer_free(constant_expression);
            return NULL;
        }

        ast_case_t *ast_case = new_ast_case(constant_expression, statement);
        return new_ast_node(AST_TYPE_CASE, ast_case);
    }

    if (check_lexeme(params, TOKEN_TYPE_KEYWORD_DEFAULT) == 1) {
        if (validate_lexeme(params, TOKEN_TYPE_PUNCTUATOR_COLON) == 0) {
            return NULL;
        }

        ast_node_t *statement = parse_statement(params);
        if (statement == NULL) {
            return NULL;
        }

        ast_default_t *ast_default = new_ast_default(statement);
        return new_ast_node(AST_TYPE_DEFAULT, ast_default);
    }

    return NULL;
}

ast_node_t* parse_expression_statement(syntax_analyzer_params_t *params) {
    // X:V
    ast_node_t *expression = parse_expression(params);
    if (check_lexeme(params, TOKEN_TYPE_PUNCTUATOR_SEMICOLON) == 0) {
        syntax_analyzer_free(expression);
        return NULL;
    }

    if (expression == NULL) {
        return new_ast_node(AST_TYPE_EMPTY, NULL);
    }

    return expression;
}

ast_node_t* parse_compound_statement_3(
    syntax_analyzer_params_t *params,
    ast_array_t *declarations,
    ast_array_t *statements
) {
    ast_node_t *statement = parse_statement(params);
    if (statement != NULL) {
        dynamic_array_add(statements->array, statement);
        return parse_compound_statement_3(params, declarations, statements);
    }

    if (validate_lexeme(params, TOKEN_TYPE_PUNCTUATOR_RBRACE) == 0) {
        syntax_analyzer_free(new_ast_node(AST_TYPE_ARRAY, declarations));
        syntax_analyzer_free(new_ast_node(AST_TYPE_ARRAY, statements));
        return NULL;
    }

    syntax_analyzer_scope_t *scope = params->scope;
    params->scope = params->scope->parent;
    syntax_analyzer_scope_free(scope);

    ast_compound_statement_t *ast_compound_statement = new_ast_compound_statement(
        new_ast_node(AST_TYPE_ARRAY, declarations),
        new_ast_node(AST_TYPE_ARRAY, statements)
    );
    return new_ast_node(AST_TYPE_COMPOUND_STATEMENT, ast_compound_statement);
}

ast_node_t* parse_compound_statement_2(
    syntax_analyzer_params_t *params,
    ast_array_t *declarations
) {
    ast_node_t *declaration = parse_declaration(params);
    if (declaration != NULL) {
        dynamic_array_add(declarations->array, declaration);
        return parse_compound_statement_2(params, declarations);
    }

    ast_array_t *statements = new_ast_array();
    return parse_compound_statement_3(params, declarations, statements);
}

ast_node_t* parse_compound_statement(syntax_analyzer_params_t *params) {
    if (check_lexeme(params, TOKEN_TYPE_PUNCTUATOR_LBRACE) == 0) {
       return NULL;
    }

    syntax_analyzer_scope_t *scope = new_syntax_analyzer_scope(params->scope);
    params->scope = scope;

    ast_array_t *declarations = new_ast_array();
    return parse_compound_statement_2(params, declarations);
}

ast_node_t *parse_selection_statement_3(
    syntax_analyzer_params_t *params,
    ast_node_t *expression,
    ast_node_t *if_statement
) {
    if (check_lexeme(params, TOKEN_TYPE_KEYWORD_ELSE) == 0) {
        ast_if_else_t *ast_if_else = new_ast_if_else(expression, if_statement, NULL);
        return new_ast_node(AST_TYPE_IF_ELSE, ast_if_else);
    }

    ast_node_t *else_statement = parse_statement(params);
    if (else_statement == NULL) {
        syntax_analyzer_free(expression);
        syntax_analyzer_free(if_statement);
        return NULL;
    }

    ast_if_else_t *ast_if_else = new_ast_if_else(expression, if_statement, else_statement);
    return new_ast_node(AST_TYPE_IF_ELSE, ast_if_else);
}

ast_node_t *parse_selection_statement_2(syntax_analyzer_params_t *params) {
    ast_node_t *statement = NULL;

    statement = parse_expression_statement(params);
    if (statement != NULL) {
        return statement;
    }

    statement = parse_jump_statement(params);
    if (statement != NULL) {
        return statement;
    }

    statement = parse_compound_statement(params);
    if (statement != NULL) {
        return statement;
    }

    return NULL;
}

ast_node_t *parse_selection_statement(syntax_analyzer_params_t *params) {
    if (check_lexeme(params, TOKEN_TYPE_KEYWORD_SWITCH) == 1) {
        if (validate_lexeme(params, TOKEN_TYPE_PUNCTUATOR_LPAREN) == 0) {
            return NULL;
        }

        ast_node_t *expression = parse_expression(params);
        if (expression == NULL) {
            return NULL;
        }

        if (validate_lexeme(params, TOKEN_TYPE_PUNCTUATOR_RPAREN) == 0) {
            syntax_analyzer_free(expression);
            return NULL;
        }

        ast_node_t *statement = parse_statement(params);
        if (statement == NULL) {
            syntax_analyzer_free(expression);
            return NULL;
        }

        ast_switch_t *ast_switch = new_ast_switch(expression, statement);
        return new_ast_node(AST_TYPE_SWITCH, ast_switch);
    }

    if (check_lexeme(params, TOKEN_TYPE_KEYWORD_IF) == 1) {
        if (validate_lexeme(params, TOKEN_TYPE_PUNCTUATOR_LPAREN) == 0) {
            return NULL;
        }

        ast_node_t *expression = parse_expression(params);
        if (expression == NULL) {
            return NULL;
        }

        if (validate_lexeme(params, TOKEN_TYPE_PUNCTUATOR_RPAREN) == 0) {
            syntax_analyzer_free(expression);
            return NULL;
        }

        ast_node_t *if_statement = parse_selection_statement_2(params);
        if (if_statement == NULL) {
            syntax_analyzer_free(expression);
            return NULL;
        }

        return parse_selection_statement_3(params, expression, if_statement);
    }

    return NULL;
}

ast_node_t *parse_iteration_statement(syntax_analyzer_params_t *params) {
    if (check_lexeme(params, TOKEN_TYPE_KEYWORD_WHILE) == 1) {
        if (validate_lexeme(params, TOKEN_TYPE_PUNCTUATOR_LPAREN) == 0) {
            return NULL;
        }

        ast_node_t *expression = parse_expression(params);
        if (expression == NULL) {
            return NULL;
        }

        if (validate_lexeme(params, TOKEN_TYPE_PUNCTUATOR_RPAREN) == 0) {
            syntax_analyzer_free(expression);
            return NULL;
        }

        ast_node_t *statement = parse_statement(params);
        if (statement == NULL) {
            syntax_analyzer_free(expression);
            return NULL;
        }

        ast_while_t *ast_while = new_ast_while(expression, statement);
        return new_ast_node(AST_TYPE_WHILE, ast_while);
    }

    if (check_lexeme(params, TOKEN_TYPE_KEYWORD_DO) == 1) {
        ast_node_t *statement = parse_statement(params);
        if (statement == NULL) {
            return NULL;
        }

        if (validate_lexeme(params, TOKEN_TYPE_KEYWORD_WHILE) == 0) {
            syntax_analyzer_free(statement);
            return NULL;
        }

        if (validate_lexeme(params, TOKEN_TYPE_PUNCTUATOR_LPAREN) == 0) {
            syntax_analyzer_free(statement);
            return NULL;
        }

        ast_node_t *expression = parse_expression(params);
        if (expression == NULL) {
            syntax_analyzer_free(statement);
            return NULL;
        }

        if (validate_lexeme(params, TOKEN_TYPE_PUNCTUATOR_RPAREN) == 0) {
            syntax_analyzer_free(statement);
            syntax_analyzer_free(expression);
            return NULL;
        }

        if (validate_lexeme(params, TOKEN_TYPE_PUNCTUATOR_SEMICOLON) == 0) {
            syntax_analyzer_free(statement);
            syntax_analyzer_free(expression);
            return NULL;
        }

        ast_do_while_t *do_while = new_ast_do_while(expression, statement);
        return new_ast_node(AST_TYPE_DO_WHILE, do_while);
    }

    if (check_lexeme(params, TOKEN_TYPE_KEYWORD_FOR) == 1) {
        if (validate_lexeme(params, TOKEN_TYPE_PUNCTUATOR_LPAREN) == 0) {
            return NULL;
        }

        ast_node_t *expression_low = parse_expression(params);
        if (validate_lexeme(params, TOKEN_TYPE_PUNCTUATOR_SEMICOLON) == 0) {
            syntax_analyzer_free(expression_low);
            return NULL;
        }

        ast_node_t *expression_high = parse_expression(params);
        if (validate_lexeme(params, TOKEN_TYPE_PUNCTUATOR_SEMICOLON) == 0) {
            syntax_analyzer_free(expression_low);
            syntax_analyzer_free(expression_high);
            return NULL;
        }

        ast_node_t *expression_step = parse_expression(params);
        if (validate_lexeme(params, TOKEN_TYPE_PUNCTUATOR_RPAREN) == 0) {
            syntax_analyzer_free(expression_low);
            syntax_analyzer_free(expression_high);
            syntax_analyzer_free(expression_step);
            return NULL;
        }

        ast_node_t *statement = parse_statement(params);
        if (statement == NULL) {
            syntax_analyzer_free(expression_low);
            syntax_analyzer_free(expression_high);
            syntax_analyzer_free(expression_step);
            return NULL;
        }

        ast_for_t *ast_for = new_ast_for(expression_low, expression_high, expression_step, statement);
        return new_ast_node(AST_TYPE_FOR, ast_for);
    }

    return NULL;
}

ast_node_t* parse_statement(syntax_analyzer_params_t *params) {
    ast_node_t *statement = NULL;

    statement = parse_labeled_statement(params);
    if (statement != NULL) {
        return statement;
    }

    statement = parse_expression_statement(params);
    if (statement != NULL) {
        return statement;
    }

    statement = parse_compound_statement(params);
    if (statement != NULL) {
        return statement;
    }

    statement = parse_selection_statement(params);
    if (statement != NULL) {
        return statement;
    }

    statement = parse_iteration_statement(params);
    if (statement != NULL) {
        return statement;
    }

    statement = parse_jump_statement(params);
    if (statement != NULL) {
        return statement;
    }

    return NULL;
}

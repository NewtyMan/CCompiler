#include <semantic_analyzer/semantic_analyzer.h>
#include <syntax_analyzer/syntax_analyzer.h>
#include <syntax_analyzer/syntax_analyzer_visitor.h>
#include <symbol_table/symbol_table.h>
#include <utils/logger.h>

int semantic_analysis_type_callback(ast_type_t type, void *element, void *params_ptr);

typedef struct semantic_analysis_type_params {
    symbol_table_symbol_t *symbol_table;
    symbol_table_type_t *return_type;
    symbol_table_type_t *declaration_specifiers_type;
    int return_code;
} semantic_analysis_type_params_t;


 /************************
 *   UTILITY FUNCTIONS   *
 ************************/

int is_base_type_compatible(symbol_table_base_type_t a, symbol_table_base_type_t b) {
    switch (a) {
        case SYMBOL_TABLE_BASE_TYPE_VOID:
            return b == SYMBOL_TABLE_BASE_TYPE_VOID;
        case SYMBOL_TABLE_BASE_TYPE_CHAR:
            return b == SYMBOL_TABLE_BASE_TYPE_CHAR ||
                b == SYMBOL_TABLE_BASE_TYPE_INT;
        case SYMBOL_TABLE_BASE_TYPE_ENUM:
            return b == SYMBOL_TABLE_BASE_TYPE_ENUM;
        case SYMBOL_TABLE_BASE_TYPE_INT:
            return b == SYMBOL_TABLE_BASE_TYPE_INT ||
                b == SYMBOL_TABLE_BASE_TYPE_FLOAT ||
                b == SYMBOL_TABLE_BASE_TYPE_CHAR;
        case SYMBOL_TABLE_BASE_TYPE_FLOAT:
            return b == SYMBOL_TABLE_BASE_TYPE_FLOAT ||
                b == SYMBOL_TABLE_BASE_TYPE_INT;
        case SYMBOL_TABLE_BASE_TYPE_STRUCT:
            return b == SYMBOL_TABLE_BASE_TYPE_STRUCT;
        case SYMBOL_TABLE_BASE_TYPE_STRING:
            return b == SYMBOL_TABLE_BASE_TYPE_STRING;
    }
    return 0;
}

int is_type_compatible(symbol_table_type_t *a, symbol_table_type_t *b) {
    symbol_table_base_type_t a_base_type = symbol_table_get_base_type(a);
    symbol_table_base_type_t b_base_type = symbol_table_get_base_type(b);

    // Pointer are compatible between each other only if they are of the same base type (exception is void)
    if (a->type == SYMBOL_TABLE_DATA_TYPE_POINTER && b->type == SYMBOL_TABLE_DATA_TYPE_POINTER) {
        if (a_base_type == SYMBOL_TABLE_BASE_TYPE_VOID || b_base_type == SYMBOL_TABLE_BASE_TYPE_VOID) {
            return 1;
        }
        return a_base_type == b_base_type;
    }

    // Normal declarations have softer restrictions on type equality
    return is_base_type_compatible(a_base_type, b_base_type);
}

int is_type_compatible_with_base_type(symbol_table_type_t *a, symbol_table_base_type_t base_type) {
    if (a->type != SYMBOL_TABLE_DATA_TYPE_BASE_TYPE) {
        return 0;
    }

    return is_base_type_compatible(a->base_type, base_type);
}


 /************************
 *   VISITOR FUNCTIONS   *
 ************************/

void semantic_analysis_type_parse_array_access(ast_array_access_t *ast_array_access, semantic_analysis_type_params_t *params) {
    syntax_analyzer_visit(semantic_analysis_type_callback, ast_array_access->expression, params);
    if (is_type_compatible_with_base_type(params->return_type, SYMBOL_TABLE_BASE_TYPE_INT) == 0) {
        logger_error("array index must be compatible with integer type");
        params->return_code = 1;
    }

    syntax_analyzer_visit(semantic_analysis_type_callback, ast_array_access->array, params);
    params->return_type = params->return_type->next;
}

void semantic_analysis_type_parse_binary(ast_binary_t *ast_binary, semantic_analysis_type_params_t *params) {
    syntax_analyzer_visit(semantic_analysis_type_callback, ast_binary->left, params);
    symbol_table_type_t *left_type = params->return_type;
    if (left_type->type == SYMBOL_TABLE_DATA_TYPE_BASE_TYPE && left_type->base_type == SYMBOL_TABLE_BASE_TYPE_VOID) {
        logger_error("void type is not allowed in binary operations");
        params->return_code = 1;
    }

    syntax_analyzer_visit(semantic_analysis_type_callback, ast_binary->right, params);
    symbol_table_type_t *right_type = params->return_type;
    if (right_type->type == SYMBOL_TABLE_DATA_TYPE_BASE_TYPE && right_type->base_type == SYMBOL_TABLE_BASE_TYPE_VOID) {
        logger_error("void type is not allowed in binary operations");
        params->return_code = 1;
    }

    if (is_type_compatible(left_type, right_type) == 0) {
        logger_error("binary operator types are not compatible");
        params->return_code = 1;
    }
}

void semantic_analysis_type_parse_case(ast_case_t *ast_case, semantic_analysis_type_params_t *params) {
    syntax_analyzer_visit(semantic_analysis_type_callback, ast_case->expression, params);
    if (is_type_compatible_with_base_type(params->return_type, SYMBOL_TABLE_BASE_TYPE_INT) == 0) {
        logger_error("expression type in case statement must be compatible with integer type");
        params->return_code = 1;
    }

    syntax_analyzer_visit(semantic_analysis_type_callback, ast_case->statement, params);
}

void semantic_analysis_type_parse_compound_statement(ast_compound_statement_t *ast_compound_statement, semantic_analysis_type_params_t *params) {
    symbol_table_symbol_t *symbol = symbol_table_find_node(params->symbol_table, ast_compound_statement);

    params->symbol_table = symbol;
    syntax_analyzer_visit(semantic_analysis_type_callback, ast_compound_statement->declarations, params);
    syntax_analyzer_visit(semantic_analysis_type_callback, ast_compound_statement->statements, params);
    params->symbol_table = params->symbol_table->parent;
}


void semantic_analysis_type_parse_constant(ast_constant_t *constant, semantic_analysis_type_params_t *params) {
    switch (constant->type) {
        case TOKEN_TYPE_NUMBER_DEC:
        case TOKEN_TYPE_NUMBER_HEX:
        case TOKEN_TYPE_NUMBER_OCT:
            params->return_type = symbol_table_new_base_type(SYMBOL_TABLE_BASE_TYPE_INT);
            break;
        case TOKEN_TYPE_NUMBER_FLOAT:
            params->return_type = symbol_table_new_base_type(SYMBOL_TABLE_BASE_TYPE_FLOAT);
            break;
        case TOKEN_TYPE_CHARACTER_LITERAL:
            params->return_type = symbol_table_new_base_type(SYMBOL_TABLE_BASE_TYPE_INT);
            break;
        case TOKEN_TYPE_STRING_LITERAL:
            params->return_type = symbol_table_new_base_type(SYMBOL_TABLE_BASE_TYPE_STRING);
            break;
        case TOKEN_TYPE_IDENTIFIER:;
            symbol_table_symbol_t *symbol = symbol_table_find(params->symbol_table, constant->constant);
            switch (symbol->symbol_type) {
                case SYMBOL_TABLE_SYMBOL_TYPE_DECLARATION:
                case SYMBOL_TABLE_SYMBOL_TYPE_PARAMETER:;
                    symbol_table_declaration_t *declaration_symbol = symbol->element;
                    params->return_type = declaration_symbol->base_type;
                    break;
                case SYMBOL_TABLE_SYMBOL_TYPE_FUNCTION:;
                    symbol_table_function_t *function_symbol = symbol->element;
                    params->return_type = function_symbol->base_type;
                    break;
                default:
                    logger_error("unsupported symbol identifier type: %d", symbol->symbol_type);
                    params->return_code = 1;
                    break;
            }
        default:
            logger_error("unsupported constant type: %s", token_type_labels[constant->type]);
            params->return_code = 1;
            break;
    }
}

void semantic_analysis_type_parse_declaration(ast_declaration_t *ast_declaration, semantic_analysis_type_params_t *params) {
    syntax_analyzer_visit(semantic_analysis_type_callback, ast_declaration->specifiers, params);
    params->declaration_specifiers_type = params->return_type;
}

void semantic_analysis_type_parse_declarator(ast_declarator_t *ast_declarator, semantic_analysis_type_params_t *params) {
    if (ast_declarator->initializer == NULL) {
        return;
    }

    syntax_analyzer_visit(semantic_analysis_type_callback, ast_declarator->initializer, params);
    if (is_type_compatible(params->declaration_specifiers_type, params->return_type) == 0) {
        logger_error("declarator initialized with an incompatible type");
        params->return_code = 1;
    }
}

void semantic_analysis_type_parse_do_while(ast_do_while_t *ast_do_while, semantic_analysis_type_params_t *params) {
    syntax_analyzer_visit(semantic_analysis_type_callback, ast_do_while->expression, params);
    if (is_type_compatible_with_base_type(params->return_type, SYMBOL_TABLE_BASE_TYPE_INT) == 0) {
        logger_error("do-while expression must be compatible with integer type");
        params->return_code = 1;
    }

    syntax_analyzer_visit(semantic_analysis_type_callback, ast_do_while->statement, params);
}

void semantic_analysis_type_parse_enumerator(ast_enumerator_t *ast_enumerator, semantic_analysis_type_params_t *params) {
    params->return_type = symbol_table_new_base_type(SYMBOL_TABLE_BASE_TYPE_INT);
}

void semantic_analysis_type_parse_function(ast_function_t *ast_function, semantic_analysis_type_params_t *params) {
    ast_constant_t *function_name = ast_function->identifier->value;
    symbol_table_symbol_t *symbol = symbol_table_find(params->symbol_table, function_name->constant);
    symbol_table_function_t *function_symbol = symbol->element;

    params->symbol_table = symbol;
    syntax_analyzer_visit(semantic_analysis_type_callback, ast_function->compound_statement, params);
    if (function_symbol->base_type->base_type != SYMBOL_TABLE_BASE_TYPE_VOID && is_type_compatible(function_symbol->base_type, params->return_type) == 0) {
        logger_error("return function does not match function definition");
        params->return_code = 1;
    }
    params->symbol_table = params->symbol_table->parent;
}


void semantic_analysis_type_parse_function_call(ast_function_call_t *ast_function_call, semantic_analysis_type_params_t *params) {
    ast_constant_t *function_name = ast_function_call->function->value;
    symbol_table_function_t *function_symbol = symbol_table_find(params->symbol_table, function_name->constant)->element;

    ast_array_t *argument_array = ast_function_call->arguments->value;

    int i;
    for (i = 0; function_symbol->parameters->size; i++) {
        symbol_table_symbol_t *symbol = dynamic_array_get(function_symbol->parameters, i);
        symbol_table_declaration_t *parameter_symbol = symbol->element;

        ast_node_t *argument_node = dynamic_array_get(argument_array->array, i);
        syntax_analyzer_visit(semantic_analysis_type_callback, argument_node, params);

        if (is_type_compatible(parameter_symbol->base_type, params->return_type) == 0) {
            logger_error("function call argument not compatible with parameter declaration type");
            params->return_code = 1;
        }
    }

    params->return_type = function_symbol->base_type;
}

void semantic_analysis_type_parse_if_else(ast_if_else_t *ast_if_else, semantic_analysis_type_params_t *params) {
    syntax_analyzer_visit(semantic_analysis_type_callback, ast_if_else->expression, params);
    if (is_type_compatible_with_base_type(params->return_type, SYMBOL_TABLE_BASE_TYPE_INT) == 0) {
        logger_error("if condition expression must be compatible with integer type");
        params->return_code = 1;
    }

    syntax_analyzer_visit(semantic_analysis_type_callback, ast_if_else->if_statement, params);
    syntax_analyzer_visit(semantic_analysis_type_callback, ast_if_else->else_statement, params);
}

void semantic_analysis_type_parse_string_literal(ast_string_literal_t *ast_string_literal, semantic_analysis_type_params_t *params) {
    params->return_type = symbol_table_new_base_type(SYMBOL_TABLE_BASE_TYPE_STRING);
}

void semantic_analysis_type_parse_switch(ast_switch_t *ast_switch, semantic_analysis_type_params_t *params) {
    syntax_analyzer_visit(semantic_analysis_type_callback, ast_switch->expression, params);
    if (is_type_compatible_with_base_type(params->return_type, SYMBOL_TABLE_BASE_TYPE_INT) == 0) {
        logger_error("switch expression must be compatible with integer type");
        params->return_code = 1;
    }

    syntax_analyzer_visit(semantic_analysis_type_callback, ast_switch->statement, params);
}

void semantic_analysis_type_parse_while(ast_while_t *ast_while, semantic_analysis_type_params_t *params) {
    syntax_analyzer_visit(semantic_analysis_type_callback, ast_while->expression, params);
    if (is_type_compatible_with_base_type(params->return_type, SYMBOL_TABLE_BASE_TYPE_INT) == 0) {
        logger_error("while-expression must be compatible with integer type");
        params->return_code = 1;
    }

    syntax_analyzer_visit(semantic_analysis_type_callback, ast_while->statement, params);
}

void semantic_analysis_type_parse_postfix(ast_postfix_t *ast_postfix, semantic_analysis_type_params_t *params) {
    syntax_analyzer_visit(semantic_analysis_type_callback, ast_postfix->expression, params);
    if (is_type_compatible_with_base_type(params->return_type, SYMBOL_TABLE_BASE_TYPE_INT) == 0) {
        logger_error("postfix expression must be compatible with integer type");
        params->return_code = 1;
    }
}

void semantic_analysis_type_parse_specifier(ast_specifier_t *ast_specifier, semantic_analysis_type_params_t *params) {
    params->return_type = symbol_table_get_type(
        params->symbol_table,
        ast_specifier->type_specifier,
        NULL,
        NULL
    );
}

void semantic_analysis_type_parse_ternary(ast_ternary_t *ast_ternary, semantic_analysis_type_params_t *params) {
    syntax_analyzer_visit(semantic_analysis_type_callback, ast_ternary->expression, params);
    if (is_type_compatible_with_base_type(params->return_type, SYMBOL_TABLE_BASE_TYPE_INT) == 0) {
        logger_error("ternary condition expression must be compatible with integer type");
        params->return_code = 1;
    }

    syntax_analyzer_visit(semantic_analysis_type_callback, ast_ternary->true_expression, params);
    symbol_table_type_t *true_type = params->return_type;

    syntax_analyzer_visit(semantic_analysis_type_callback, ast_ternary->false_expression, params);
    symbol_table_type_t *false_type = params->return_type;

    if (is_type_compatible(true_type, false_type) == 0) {
        logger_error("ternary true and false expression types must match");
        params->return_code = 1;
    }

    params->return_type = true_type;
}

void semantic_analysis_type_parse_unary(ast_unary_t *ast_unary, semantic_analysis_type_params_t *params) {
    symbol_table_base_type_t required_type;
    switch (ast_unary->operator) {
        case TOKEN_TYPE_OPERATOR_BIT_AND:
        case TOKEN_TYPE_OPERATOR_MUL:
        case TOKEN_TYPE_OPERATOR_SUM:
        case TOKEN_TYPE_OPERATOR_SUB:
        case TOKEN_TYPE_OPERATOR_TILDE:
        case TOKEN_TYPE_OPERATOR_NOT:
        case TOKEN_TYPE_OPERATOR_INC:
        case TOKEN_TYPE_OPERATOR_DEC:
            required_type = SYMBOL_TABLE_BASE_TYPE_INT;
            break;
        default:
            required_type = SYMBOL_TABLE_BASE_TYPE_VOID;
    }

    syntax_analyzer_visit(semantic_analysis_type_callback, ast_unary->expression, params);
    if (is_type_compatible_with_base_type(params->return_type, required_type) == 0) {
        logger_error("unary expression must be compatible with integer type");
        params->return_code = 1;
    }
}

int semantic_analysis_type_callback(ast_type_t ast_type, void *element, void *params) {
    if (element == NULL) {
        return VISITOR_CALLBACK_RETURN_AUTOMATIC;
    }

    switch (ast_type) {
        case AST_TYPE_ARRAY_ACCESS:
            semantic_analysis_type_parse_array_access(element, params);
            return VISITOR_CALLBACK_RETURN_MANUAL;
        case AST_TYPE_BINARY:
            semantic_analysis_type_parse_binary(element, params);
            return VISITOR_CALLBACK_RETURN_MANUAL;
        case AST_TYPE_CASE:
            semantic_analysis_type_parse_case(element, params);
            return VISITOR_CALLBACK_RETURN_MANUAL;
        case AST_TYPE_COMPOUND_STATEMENT:
            semantic_analysis_type_parse_compound_statement(element, params);
            return VISITOR_CALLBACK_RETURN_MANUAL;
        case AST_TYPE_CONSTANT:
            semantic_analysis_type_parse_constant(element, params);
            return VISITOR_CALLBACK_RETURN_MANUAL;
        case AST_TYPE_DECLARATION:
            semantic_analysis_type_parse_declaration(element, params);
            return VISITOR_CALLBACK_RETURN_AUTOMATIC;
        case AST_TYPE_DECLARATOR:
            semantic_analysis_type_parse_declarator(element, params);
            return VISITOR_CALLBACK_RETURN_MANUAL;
        case AST_TYPE_DO_WHILE:
            semantic_analysis_type_parse_do_while(element, params);
            return VISITOR_CALLBACK_RETURN_MANUAL;
        case AST_TYPE_ENUMERATOR:
            semantic_analysis_type_parse_enumerator(element, params);
            return VISITOR_CALLBACK_RETURN_MANUAL;
        case AST_TYPE_FUNCTION:
            semantic_analysis_type_parse_function(element, params);
            return VISITOR_CALLBACK_RETURN_MANUAL;
        case AST_TYPE_FUNCTION_CALL:
            semantic_analysis_type_parse_function_call(element, params);
            return VISITOR_CALLBACK_RETURN_MANUAL;
        case AST_TYPE_IF_ELSE:
            semantic_analysis_type_parse_if_else(element, params);
            return VISITOR_CALLBACK_RETURN_MANUAL;
        case AST_TYPE_POSTFIX:
            semantic_analysis_type_parse_postfix(element, params);
            return VISITOR_CALLBACK_RETURN_MANUAL;
        case AST_TYPE_SPECIFIER:
            semantic_analysis_type_parse_specifier(element, params);
            return VISITOR_CALLBACK_RETURN_MANUAL;
        case AST_TYPE_STRING_LITERAL:
            semantic_analysis_type_parse_string_literal(element, params);
            return VISITOR_CALLBACK_RETURN_MANUAL;
        case AST_TYPE_SWITCH:
            semantic_analysis_type_parse_switch(element, params);
            return VISITOR_CALLBACK_RETURN_MANUAL;
        case AST_TYPE_TERNARY:
            semantic_analysis_type_parse_ternary(element, params);
            return VISITOR_CALLBACK_RETURN_MANUAL;
        case AST_TYPE_UNARY:
            semantic_analysis_type_parse_unary(element, params);
            return VISITOR_CALLBACK_RETURN_MANUAL;
        case AST_TYPE_WHILE:
            semantic_analysis_type_parse_while(element, params);
            return VISITOR_CALLBACK_RETURN_MANUAL;
        case AST_TYPE_ARRAY:
        case AST_TYPE_ARRAY_DECLARATOR:
        case AST_TYPE_ASM:
        case AST_TYPE_BREAK:
        case AST_TYPE_CONTINUE:
        case AST_TYPE_DEFAULT:
        case AST_TYPE_EMPTY:
        case AST_TYPE_ENUM:
        case AST_TYPE_FOR:
        case AST_TYPE_KEYWORD:
        case AST_TYPE_PARAMETER:
        case AST_TYPE_POINTER:
        case AST_TYPE_RETURN:
        case AST_TYPE_STRUCT:
        case AST_TYPE_TYPEDEF:
            return VISITOR_CALLBACK_RETURN_AUTOMATIC;
    }

    return VISITOR_CALLBACK_RETURN_AUTOMATIC;
}

int semantic_analysis_type(ast_node_t *node, symbol_table_symbol_t *symbol_table) {
    semantic_analysis_type_params_t params;
    params.symbol_table = symbol_table;
    params.return_code = 0;

    syntax_analyzer_visit(semantic_analysis_type_callback, node, &params);
    return params.return_code;
}

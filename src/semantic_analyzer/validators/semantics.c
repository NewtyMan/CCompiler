#include <semantic_analyzer/semantic_analyzer.h>
#include <syntax_analyzer/syntax_analyzer_helpers.h>
#include <syntax_analyzer/syntax_analyzer_visitor.h>
#include <utils/logger.h>
#include <stdlib.h>

#include "utils/string.h"

// TODO: Check that default statement is the last statement in switch (if present)
// TODO: Required main function

int semantic_analysis_semantics_callback(ast_type_t type, void *element, void *params_ptr);

typedef struct semantic_analysis_semantics_params {
    symbol_table_symbol_t *symbol_table;

    // Switch statements params
    char is_inside_iteration;
    char is_inside_switch;

    char is_main_function_present;
    char is_default_present;

    int return_code;
} semantic_analysis_semantics_params_t;

void semantic_analysis_semantics_parse_break(semantic_analysis_semantics_params_t *params) {
    if (params->is_inside_iteration == 0 && params->is_inside_switch == 0) {
        logger_error("break statement only allowed inside loops and switch!");
        params->return_code = 1;
    }
}

void semantic_analysis_semantics_parse_continue(semantic_analysis_semantics_params_t *params) {
    if (params->is_inside_iteration == 0 && params->is_inside_switch == 0) {
        logger_error("continue statement only allowed inside loops and switch!");
        params->return_code = 1;
    }
}

void semantic_analysis_semantics_parse_case(ast_case_t *ast_case, semantic_analysis_semantics_params_t *params) {
    if (params->is_inside_switch == 0) {
        logger_error("Case statement only allowed inside a switch statement!");
        params->return_code = 1;
    }

    if (ast_case->expression->type != AST_TYPE_CONSTANT) {
        logger_error("case expression must be a constant");
        params->return_code = 1;
    }

    syntax_analyzer_visit(semantic_analysis_semantics_callback, ast_case->expression, params);
    syntax_analyzer_visit(semantic_analysis_semantics_callback, ast_case->statement, params);
}

void semantic_analysis_semantics_parse_default(ast_default_t *ast_default, semantic_analysis_semantics_params_t *params) {
    if (params->is_inside_switch == 0) {
        logger_error("Default statement only allowed inside a switch statement!");
        params->return_code = 1;
        return;
    }

    syntax_analyzer_visit(semantic_analysis_semantics_callback, ast_default->statement, params);
    params->is_default_present = 1;
}

void semantic_analysis_semantics_parse_do_while(ast_do_while_t *ast_do_while, semantic_analysis_semantics_params_t *params) {
    params->is_inside_iteration = 1;
    syntax_analyzer_visit(semantic_analysis_semantics_callback, ast_do_while->statement, params);
    syntax_analyzer_visit(semantic_analysis_semantics_callback, ast_do_while->expression, params);
    params->is_inside_switch = 0;
}

void semantic_analysis_semantics_parse_for(ast_for_t *ast_for, semantic_analysis_semantics_params_t *params) {
    params->is_inside_iteration = 1;
    syntax_analyzer_visit(semantic_analysis_semantics_callback, ast_for->expression_low, params);
    syntax_analyzer_visit(semantic_analysis_semantics_callback, ast_for->expression_high, params);
    syntax_analyzer_visit(semantic_analysis_semantics_callback, ast_for->expression_step, params);
    syntax_analyzer_visit(semantic_analysis_semantics_callback, ast_for->statement, params);
    params->is_inside_iteration = 0;
}

void semantic_analysis_semantics_parse_function(ast_function_t *ast_function, semantic_analysis_semantics_params_t *params) {
    ast_constant_t *identifier = ast_function->identifier->value;
    symbol_table_function_t *function_symbol = symbol_table_find(params->symbol_table, identifier->constant)->element;

    if (string_cmp(identifier->constant, "main") == 0) {
        params->is_main_function_present = 1;
    }

    // Void functions do not need to have return statements. However, for the correctness of the AST structure, we need
    // to add it as the last statement.
    if (function_symbol->base_type->base_type == SYMBOL_TABLE_BASE_TYPE_VOID) {
        ast_compound_statement_t *function_body = ast_function->compound_statement->value;
        if (function_body->statements == NULL) {
            // If function has an empty expression body, we must manually assign the node, so that we can add the return
            // statement (normal factory function don't create array nodes with empty elements).
            ast_array_t *function_body_array = new_ast_array();
            ast_node_t *function_body_node = malloc(sizeof(ast_node_t));
            function_body_node->type = AST_TYPE_ARRAY;
            function_body_node->value = function_body_array;
            function_body->statements = function_body_node;
        }

        ast_array_t *function_body_array = function_body->statements->value;
        ast_node_t *function_last_node = dynamic_array_get(function_body_array->array, function_body_array->array->size - 1);
        if (function_last_node == NULL || function_last_node->type != AST_TYPE_RETURN) {
            dynamic_array_add(function_body_array->array, new_ast_node(AST_TYPE_RETURN, new_ast_return(NULL)));
        }
    }

    syntax_analyzer_visit(semantic_analysis_semantics_callback, ast_function->parameter_list, params);

    // Function body/compound must be specially parsed, since parameters and root of body are on the same scope level
    if (ast_function->compound_statement != NULL) {
        ast_compound_statement_t *function_body = ast_function->compound_statement->value;
        syntax_analyzer_visit(semantic_analysis_semantics_callback, function_body->declarations, params);
        syntax_analyzer_visit(semantic_analysis_semantics_callback, function_body->statements, params);
    }
}

void semantic_analysis_semantics_parse_switch(ast_switch_t *ast_switch, semantic_analysis_semantics_params_t *params) {
    params->is_default_present = 0;
    params->is_inside_switch = 1;
    syntax_analyzer_visit(semantic_analysis_semantics_callback, ast_switch->expression, params);
    syntax_analyzer_visit(semantic_analysis_semantics_callback, ast_switch->statement, params);
    params->is_inside_switch = 0;

    if (params->is_default_present == 1) {
        ast_compound_statement_t *compound_statement = ast_switch->statement->value;
        ast_array_t *switch_statements = compound_statement->statements->value;
        ast_node_t *last_switch_statement = switch_statements->array->array[switch_statements->array->size - 1];
        if (last_switch_statement->type != AST_TYPE_DEFAULT) {
            logger_error("Default statement must be the last statement in switch!");
            params->return_code = 1;
        }
    }
}

void semantic_analysis_semantics_parse_while(ast_while_t *ast_while, semantic_analysis_semantics_params_t *params) {
    params->is_inside_iteration = 1;
    syntax_analyzer_visit(semantic_analysis_semantics_callback, ast_while->expression, params);
    syntax_analyzer_visit(semantic_analysis_semantics_callback, ast_while->statement, params);
    params->is_inside_switch = 0;
}

int semantic_analysis_semantics_callback(ast_type_t type, void *element, void *params_ptr) {
    semantic_analysis_semantics_params_t *params = params_ptr;
    switch (type) {
        case AST_TYPE_BREAK:
            semantic_analysis_semantics_parse_break(params);
            return VISITOR_CALLBACK_RETURN_MANUAL;
        case AST_TYPE_CASE:
            semantic_analysis_semantics_parse_case(element, params);
            return VISITOR_CALLBACK_RETURN_MANUAL;
        case AST_TYPE_CONTINUE:
            semantic_analysis_semantics_parse_continue(params);
            return VISITOR_CALLBACK_RETURN_MANUAL;
        case AST_TYPE_DEFAULT:
            semantic_analysis_semantics_parse_default(element, params);
            return VISITOR_CALLBACK_RETURN_MANUAL;
        case AST_TYPE_DO_WHILE:
            semantic_analysis_semantics_parse_do_while(element, params);
            return VISITOR_CALLBACK_RETURN_MANUAL;
        case AST_TYPE_FOR:
            semantic_analysis_semantics_parse_for(element, params);
            return VISITOR_CALLBACK_RETURN_MANUAL;
        case AST_TYPE_FUNCTION:
            semantic_analysis_semantics_parse_function(element, params);
            return VISITOR_CALLBACK_RETURN_MANUAL;
        case AST_TYPE_SWITCH:
            semantic_analysis_semantics_parse_switch(element, params);
            return VISITOR_CALLBACK_RETURN_MANUAL;
        case AST_TYPE_WHILE:
            semantic_analysis_semantics_parse_while(element, params);
            return VISITOR_CALLBACK_RETURN_MANUAL;
        default:
            return VISITOR_CALLBACK_RETURN_AUTOMATIC;
    }
}

int semantic_analysis_semantics(ast_node_t *node, symbol_table_symbol_t *symbol_table) {
    semantic_analysis_semantics_params_t params;
    params.symbol_table = symbol_table;
    params.return_code = 0;

    syntax_analyzer_visit(semantic_analysis_semantics_callback, node, &params);

    if (params.is_main_function_present == 0) {
        logger_error("No main function defined!");
        params.return_code = 1;
    }

    return params.return_code;
}
#include <semantic_analyzer/semantic_analyzer.h>
#include <syntax_analyzer/syntax_analyzer.h>
#include <syntax_analyzer/syntax_analyzer_visitor.h>
#include <symbol_table/symbol_table.h>
#include <utils/logger.h>
#include <stdlib.h>

int semantic_analysis_scope_callback(ast_type_t type, void *element, void *params_ptr);

typedef struct semantic_analysis_scope_params {
    symbol_table_symbol_t *symbol_table;
    int return_code;
} semantic_analysis_scope_params_t;

void semantic_analysis_scope_parse_compound_statement(ast_compound_statement_t *compound_statement, semantic_analysis_scope_params_t *params) {
    symbol_table_symbol_t *compound_symbol = symbol_table_find_node(params->symbol_table, compound_statement);

    params->symbol_table = compound_symbol;
    syntax_analyzer_visit(semantic_analysis_scope_callback, compound_statement->declarations, params);
    syntax_analyzer_visit(semantic_analysis_scope_callback, compound_statement->statements, params);
    params->symbol_table = compound_symbol->parent;
}

void semantic_analysis_scope_parse_declarator(ast_declarator_t *ast_declarator, semantic_analysis_scope_params_t *params) {
    symbol_table_symbol_t *declarator_symbol = symbol_table_find_node(params->symbol_table, ast_declarator);
    params->symbol_table = declarator_symbol;
}

void semantic_analysis_scope_parse_constant(ast_constant_t *constant, semantic_analysis_scope_params_t *params) {
    if (constant->type == TOKEN_TYPE_IDENTIFIER) {
        symbol_table_symbol_t *symbol = symbol_table_find(params->symbol_table, constant->constant);
        if (symbol == NULL) {
            logger_error("unknown identifier: '%s'", constant->constant);
            params->return_code = 1;
        }
    }
}

void semantic_analysis_scope_parse_function(ast_function_t *ast_function, semantic_analysis_scope_params_t *params) {
    symbol_table_symbol_t *function_symbol = symbol_table_find_node(params->symbol_table, ast_function);
    if (ast_function->compound_statement == NULL) {
        return;
    }

    params->symbol_table = function_symbol;
    ast_compound_statement_t *function_body = ast_function->compound_statement->value;
    syntax_analyzer_visit(semantic_analysis_scope_callback, function_body->declarations, params);
    syntax_analyzer_visit(semantic_analysis_scope_callback, function_body->statements, params);
    params->symbol_table = params->symbol_table->parent;
}

int semantic_analysis_scope_callback(ast_type_t ast_type, void *element, void *params) {
    switch (ast_type) {
        case AST_TYPE_COMPOUND_STATEMENT:
            semantic_analysis_scope_parse_compound_statement(element, params);
            return VISITOR_CALLBACK_RETURN_MANUAL;
        case AST_TYPE_CONSTANT:
            semantic_analysis_scope_parse_constant(element, params);
            return VISITOR_CALLBACK_RETURN_MANUAL;
        case AST_TYPE_DECLARATOR:
            semantic_analysis_scope_parse_declarator(element, params);
            return VISITOR_CALLBACK_RETURN_AUTOMATIC;
        case AST_TYPE_FUNCTION:
            semantic_analysis_scope_parse_function(element, params);
            return VISITOR_CALLBACK_RETURN_MANUAL;
        default:
            return VISITOR_CALLBACK_RETURN_AUTOMATIC;
    }
}

int semantic_analysis_scope(ast_node_t *node, symbol_table_symbol_t *symbol_table) {
    semantic_analysis_scope_params_t params;
    params.symbol_table = symbol_table;
    params.return_code = 0;

    syntax_analyzer_visit(semantic_analysis_scope_callback, node, &params);
    return params.return_code;
}

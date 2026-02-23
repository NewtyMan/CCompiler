#include <symbol_table/symbol_table.h>
#include <syntax_analyzer/syntax_analyzer.h>
#include <syntax_analyzer/syntax_analyzer_visitor.h>
#include <utils/logger.h>
#include <stdlib.h>

// Offset 0 - old fp, Offset 4 - return address, Offset 8 - return value
const int SYMBOL_TABLE_FUNCTION_PARAM_OFFSET = 12;

int symbol_table_parse_callback(ast_type_t type, void *element, void *params_ptr);

typedef struct symbol_table_parse_params {
    symbol_table_symbol_t *symbol_table;
    ast_node_t *declaration_specifiers;
    int offset, offset_multiplier, offset_precalculate;
    int enumerator_value;
    int return_code;
} symbol_table_parse_params_t;

void symbol_table_add(symbol_table_symbol_t *symbol_table, symbol_table_symbol_t *symbol) {
    if (symbol_table == NULL || symbol == NULL) {
        return;
    }

    switch (symbol_table->symbol_type) {
        case SYMBOL_TABLE_SYMBOL_TYPE_ARRAY:;
            dynamic_array_t *dynamic_array = symbol_table->element;
            dynamic_array_add(dynamic_array, symbol);
            break;
        case SYMBOL_TABLE_SYMBOL_TYPE_ENUM:;
            symbol_table_enum_t *symbol_table_enum = symbol_table->element;
            dynamic_array_add(symbol_table_enum->enumerators, symbol);
            break;
        case SYMBOL_TABLE_SYMBOL_TYPE_FUNCTION:;
            symbol_table_function_t *symbol_table_function = symbol_table->element;
            if (symbol->symbol_type == SYMBOL_TABLE_SYMBOL_TYPE_PARAMETER) {
                dynamic_array_add(symbol_table_function->parameters, symbol);
            } else {
                dynamic_array_add(symbol_table_function->body, symbol);
            }
            break;
        case SYMBOL_TABLE_SYMBOL_TYPE_STRUCT:;
            symbol_table_struct_t *symbol_table_struct = symbol_table->element;
            symbol_table_struct->size += symbol_table_type_size(((symbol_table_declaration_t*)symbol->element)->base_type);
            dynamic_array_add(symbol_table_struct->declarations, symbol);
            break;
        case SYMBOL_TABLE_SYMBOL_TYPE_TYPEDEF:;
            symbol_table_typedef_t *symbol_table_typedef = symbol_table->element;
            symbol_table_typedef->symbol = symbol;
            break;
        default:
            logger_error("unsupported symbol type for adding elements to: %d", symbol_table->symbol_type);
            break;
    }
}

void symbol_table_parse_parse_compound_statement(ast_compound_statement_t *compound_statement, symbol_table_parse_params_t *params) {
    symbol_table_symbol_t *symbol_table = symbol_table_new_symbol(
        SYMBOL_TABLE_SYMBOL_TYPE_ARRAY,
        params->symbol_table,
        dynamic_array_new(),
        compound_statement
    );
    symbol_table_add(params->symbol_table, symbol_table);

    params->symbol_table = symbol_table;
    syntax_analyzer_visit(symbol_table_parse_callback, compound_statement->declarations, params);
    syntax_analyzer_visit(symbol_table_parse_callback, compound_statement->statements, params);
    params->symbol_table = symbol_table->parent;
}

void symbol_table_parse_parse_declarator(ast_declarator_t *ast_declarator, symbol_table_parse_params_t *params) {
    symbol_table_symbol_t *declaration_symbol = symbol_table_new_declaration(
        params->symbol_table,
        params->declaration_specifiers,
        ast_declarator
    );
    symbol_table_add(params->symbol_table, declaration_symbol);

    symbol_table_declaration_t *declaration = declaration_symbol->element;
    if (params->offset_precalculate == 0) {
        declaration->offset = params->offset;
        params->offset += declaration->size * params->offset_multiplier;
    } else {
        params->offset += declaration->size * params->offset_multiplier;
        declaration->offset = params->offset;
    }

    syntax_analyzer_visit(symbol_table_parse_callback, ast_declarator->initializer, params);
}

void symbol_table_parse_parse_declaration(ast_declaration_t *ast_declaration, symbol_table_parse_params_t *params) {
    params->declaration_specifiers = ast_declaration->specifiers;
}

void symbol_table_parse_parse_enum(ast_enum_t *ast_enum, symbol_table_parse_params_t *params) {
    symbol_table_symbol_t *enumerator_symbol = symbol_table_new_enum(params->symbol_table, ast_enum);
    symbol_table_add(params->symbol_table, enumerator_symbol);

    params->symbol_table = enumerator_symbol;
    params->enumerator_value = 0;
    syntax_analyzer_visit(symbol_table_parse_callback, ast_enum->enumerators, params);
    params->symbol_table = params->symbol_table->parent;
}

void symbol_table_parse_parse_enumerator(ast_enumerator_t *ast_enumerator, symbol_table_parse_params_t *params) {
    ast_constant_t *enumerator_name = ast_enumerator->identifier->value;

    int enum_value;
    if (ast_enumerator->value == NULL) {
        enum_value = params->enumerator_value++;
    } else {
        ast_constant_t *enumerator_value = ast_enumerator->value->value;
        enum_value = atoi(enumerator_value->constant);
        params->enumerator_value = enum_value + 1;
    }

    symbol_table_symbol_t *enumerator_symbol = symbol_table_new_enumerator(params->symbol_table, enumerator_name, enum_value);
    symbol_table_add(params->symbol_table, enumerator_symbol);
}

void symbol_table_parse_parse_function(ast_function_t *ast_function, symbol_table_parse_params_t *params) {
    symbol_table_symbol_t *function_symbol = symbol_table_new_function(params->symbol_table, ast_function);
    symbol_table_add(params->symbol_table, function_symbol);

    params->symbol_table = function_symbol;

    params->offset = SYMBOL_TABLE_FUNCTION_PARAM_OFFSET;
    params->offset_multiplier = 1;
    params->offset_precalculate = 0;
    syntax_analyzer_visit(symbol_table_parse_callback, ast_function->parameter_list, params);

    params->offset = 0;
    params->offset_multiplier = -1;
    params->offset_precalculate = 1;

    // Function body/compound must be specially parsed, since parameters and root of body are on the same scope level
    if (ast_function->compound_statement != NULL) {
        ast_compound_statement_t *function_body = ast_function->compound_statement->value;
        syntax_analyzer_visit(symbol_table_parse_callback, function_body->declarations, params);
        syntax_analyzer_visit(symbol_table_parse_callback, function_body->statements, params);
    }

    symbol_table_function_t *function = function_symbol->element;
    function->parameter_size = symbol_table_get_array_size(function->parameters) + SYMBOL_TABLE_FUNCTION_PARAM_OFFSET;
    function->local_size = symbol_table_get_array_size(function->body);
    function->size = function->parameter_size + function->local_size;

    params->symbol_table = params->symbol_table->parent;
}

void symbol_table_parse_parse_parameter(ast_parameter_t *ast_parameter, symbol_table_parse_params_t *params) {
    symbol_table_symbol_t *parameter_symbol = symbol_table_new_parameter(params->symbol_table, ast_parameter);
    symbol_table_add(params->symbol_table, parameter_symbol);

    symbol_table_declaration_t *parameter_declaration = parameter_symbol->element;
    parameter_declaration->offset = params->offset;
    params->offset += parameter_declaration->size * params->offset_multiplier;
}

void symbol_table_parse_parse_struct(ast_struct_t *ast_struct, symbol_table_parse_params_t *params) {
    symbol_table_symbol_t *structure_symbol = symbol_table_new_struct(params->symbol_table, ast_struct);
    symbol_table_add(params->symbol_table, structure_symbol);

    params->symbol_table = structure_symbol;
    params->offset = 0;
    params->offset_precalculate = 0;
    params->offset_multiplier = 1;
    syntax_analyzer_visit(symbol_table_parse_callback, ast_struct->declarations, params);
    params->symbol_table = params->symbol_table->parent;
}

void symbol_table_parse_parse_typedef(ast_typedef_t *ast_typedef, symbol_table_parse_params_t *params) {
    symbol_table_symbol_t *typedef_symbol = symbol_table_new_typedef(params->symbol_table, ast_typedef);
    symbol_table_add(params->symbol_table, typedef_symbol);

    params->symbol_table = typedef_symbol;
    syntax_analyzer_visit(symbol_table_parse_callback, ast_typedef->declaration_specifier, params);
    params->symbol_table = params->symbol_table->parent;
}

int symbol_table_parse_callback(ast_type_t ast_type, void *element, void *params) {
    switch (ast_type) {
        case AST_TYPE_COMPOUND_STATEMENT:
            symbol_table_parse_parse_compound_statement(element, params);
            return VISITOR_CALLBACK_RETURN_MANUAL;
        case AST_TYPE_DECLARATOR:
            symbol_table_parse_parse_declarator(element, params);
            return VISITOR_CALLBACK_RETURN_MANUAL;
        case AST_TYPE_DECLARATION:
            symbol_table_parse_parse_declaration(element, params);
            return VISITOR_CALLBACK_RETURN_AUTOMATIC;
        case AST_TYPE_ENUM:
            symbol_table_parse_parse_enum(element, params);
            return VISITOR_CALLBACK_RETURN_MANUAL;
        case AST_TYPE_ENUMERATOR:
            symbol_table_parse_parse_enumerator(element, params);
            return VISITOR_CALLBACK_RETURN_MANUAL;
        case AST_TYPE_FUNCTION:
            symbol_table_parse_parse_function(element, params);
            return VISITOR_CALLBACK_RETURN_MANUAL;
        case AST_TYPE_PARAMETER:
            symbol_table_parse_parse_parameter(element, params);
            return VISITOR_CALLBACK_RETURN_MANUAL;
        case AST_TYPE_STRUCT:
            symbol_table_parse_parse_struct(element, params);
            return VISITOR_CALLBACK_RETURN_MANUAL;
        case AST_TYPE_TYPEDEF:
            symbol_table_parse_parse_typedef(element, params);
            return VISITOR_CALLBACK_RETURN_MANUAL;
        default:
            return VISITOR_CALLBACK_RETURN_AUTOMATIC;
    }
}

symbol_table_symbol_t* symbol_table_parse(ast_node_t *node) {
    symbol_table_parse_params_t params;
    params.symbol_table = symbol_table_new_symbol(SYMBOL_TABLE_SYMBOL_TYPE_ARRAY, NULL, dynamic_array_new(), node);
    params.return_code = 0;

    symbol_table_add_syscalls_arm32(params.symbol_table);
    syntax_analyzer_visit(symbol_table_parse_callback, node, &params);

    return params.symbol_table;
}

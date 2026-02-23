#include <symbol_table/symbol_table.h>
#include <utils/hash.h>
#include <utils/string.h>
#include <stdlib.h>

symbol_table_symbol_t* symbol_table_new_symbol(
    symbol_table_symbol_type_t symbol_type,
    symbol_table_symbol_t *parent,
    void *element,
    void *ast_node
) {
    symbol_table_symbol_t *symbol = malloc(sizeof(symbol_table_symbol_t));
    symbol->symbol_type = symbol_type;
    symbol->parent = parent;
    symbol->element = element;
    symbol->ast_node = ast_node;
    symbol->is_system = 0;
    return symbol;
}

symbol_table_symbol_t* symbol_table_new_declaration(
    symbol_table_symbol_t *parent,
    ast_node_t *specifiers,
    ast_declarator_t *declarator
) {
    symbol_table_declaration_t *declaration_symbol = malloc(sizeof(symbol_table_declaration_t));
    declaration_symbol->base_type = symbol_table_get_type(parent, specifiers, declarator->pointer, declarator->array);

    ast_constant_t *identifier = declarator->identifier->value;
    declaration_symbol->name = calloc(string_len(identifier->constant) + 1, sizeof(char));
    string_cpy(declaration_symbol->name, identifier->constant);

    declaration_symbol->size = symbol_table_type_size(declaration_symbol->base_type);
    declaration_symbol->offset = 0;
    declaration_symbol->is_parameter = 0;
    declaration_symbol->is_constant = 0;
    declaration_symbol->is_string_literal = declarator->initializer != NULL && declarator->initializer->type == AST_TYPE_STRING_LITERAL;
    declaration_symbol->name_hash = hash_32_fnv1a(declaration_symbol->name);

    return symbol_table_new_symbol(SYMBOL_TABLE_SYMBOL_TYPE_DECLARATION, parent, declaration_symbol, declarator);
}

symbol_table_symbol_t* symbol_table_new_enum(
    symbol_table_symbol_t *parent,
    ast_enum_t *ast_enum
) {
    symbol_table_enum_t *enum_symbol = malloc(sizeof(symbol_table_enum_t));

    ast_constant_t *identifier = ast_enum->identifier->value;
    enum_symbol->name = calloc(string_len(identifier->constant) + 1, sizeof(char));
    string_cpy(enum_symbol->name, identifier->constant);

    enum_symbol->enumerators = dynamic_array_new();
    enum_symbol->name_hash = hash_32_fnv1a(enum_symbol->name);

    return symbol_table_new_symbol(SYMBOL_TABLE_SYMBOL_TYPE_ENUM, parent, enum_symbol, ast_enum);
}

symbol_table_symbol_t* symbol_table_new_enumerator(
    symbol_table_symbol_t *parent,
    ast_constant_t *identifier,
    int value
) {
    symbol_table_enumerator_t *enumerator_symbol = malloc(sizeof(symbol_table_enumerator_t));

    enumerator_symbol->name = calloc(string_len(identifier->constant) + 1, sizeof(char));
    string_cpy(enumerator_symbol->name, identifier->constant);

    enumerator_symbol->value = value;
    enumerator_symbol->name_hash = hash_32_fnv1a(enumerator_symbol->name);

    return symbol_table_new_symbol(SYMBOL_TABLE_SYMBOL_TYPE_ENUMERATOR, parent, enumerator_symbol, identifier);
}

symbol_table_symbol_t* symbol_table_new_function(
    symbol_table_symbol_t *parent,
    ast_function_t *function
) {
    symbol_table_function_t *function_symbol = malloc(sizeof(symbol_table_function_t));
    function_symbol->base_type = symbol_table_get_type(parent, function->specifiers, function->pointer, NULL);

    ast_constant_t *identifier = function->identifier->value;
    function_symbol->name = calloc(string_len(identifier->constant) + 1, sizeof(char));
    string_cpy(function_symbol->name, identifier->constant);
    function_symbol->name_hash = hash_32_fnv1a(function_symbol->name);

    function_symbol->parameters = dynamic_array_new();
    function_symbol->body = dynamic_array_new();

    function_symbol->parameter_size = 0;
    function_symbol->local_size = 0;
    function_symbol->size = 0;

    return symbol_table_new_symbol(SYMBOL_TABLE_SYMBOL_TYPE_FUNCTION, parent, function_symbol, function);
}

symbol_table_symbol_t* symbol_table_new_parameter(
    symbol_table_symbol_t *parent,
    ast_parameter_t *parameter
) {
    symbol_table_declaration_t *parameter_symbol = malloc(sizeof(symbol_table_declaration_t));
    parameter_symbol->base_type = symbol_table_get_type(parent, parameter->specifiers, parameter->pointer, parameter->array);

    ast_constant_t *identifier = parameter->identifier->value;
    parameter_symbol->name = calloc(string_len(identifier->constant) + 1, sizeof(char));
    string_cpy(parameter_symbol->name, identifier->constant);

    parameter_symbol->size = symbol_table_type_size(parameter_symbol->base_type);
    parameter_symbol->offset = 0;
    parameter_symbol->is_parameter = 1;
    parameter_symbol->is_constant = 0;
    parameter_symbol->is_string_literal = 0;
    parameter_symbol->name_hash = hash_32_fnv1a(parameter_symbol->name);

    return symbol_table_new_symbol(SYMBOL_TABLE_SYMBOL_TYPE_PARAMETER, parent, parameter_symbol, parameter);
}

symbol_table_symbol_t* symbol_table_new_struct(
    symbol_table_symbol_t *parent,
    ast_struct_t *ast_struct
) {
    symbol_table_struct_t *structure_symbol = malloc(sizeof(symbol_table_struct_t));

    ast_constant_t *identifier = ast_struct->identifier->value;
    structure_symbol->name = calloc(string_len(identifier->constant) + 1, sizeof(char));
    string_cpy(structure_symbol->name, identifier->constant);

    structure_symbol->declarations = dynamic_array_new();
    structure_symbol->size = 0;
    structure_symbol->name_hash = hash_32_fnv1a(structure_symbol->name);

    return symbol_table_new_symbol(SYMBOL_TABLE_SYMBOL_TYPE_STRUCT, parent, structure_symbol, ast_struct);
}

symbol_table_symbol_t* symbol_table_new_typedef(
    symbol_table_symbol_t *parent,
    ast_typedef_t *ast_typedef
) {
    ast_constant_t *typedef_name = ast_typedef->identifier->value;

    symbol_table_typedef_t *typedef_symbol = malloc(sizeof(symbol_table_typedef_t));
    typedef_symbol->name = calloc(string_len(typedef_name->constant) + 1, sizeof(char));
    string_cpy(typedef_symbol->name, typedef_name->constant);
    typedef_symbol->symbol = NULL;
    typedef_symbol->base_type = symbol_table_get_type(parent, ast_typedef->declaration_specifier, NULL, NULL);
    typedef_symbol->name_hash = hash_32_fnv1a(typedef_symbol->name);

    return symbol_table_new_symbol(SYMBOL_TABLE_SYMBOL_TYPE_TYPEDEF, parent, typedef_symbol, ast_typedef);
}

symbol_table_type_t* symbol_table_new_base_type(symbol_table_base_type_t base_type) {
    symbol_table_type_t *symbol_type = malloc(sizeof(symbol_table_type_t));
    symbol_type->type = SYMBOL_TABLE_DATA_TYPE_BASE_TYPE;
    symbol_type->base_type = base_type;
    symbol_type->next = NULL;
    return symbol_type;
}

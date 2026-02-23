#include <symbol_table/symbol_table.h>
#include <utils/string.h>
#include <stdlib.h>

symbol_table_type_t* symbol_table_get_type(
    symbol_table_symbol_t *symbol_table,
    ast_node_t *specifiers_node,
    ast_node_t *pointer,
    ast_node_t *array
) {
    symbol_table_type_t *data_type = calloc(1, sizeof(symbol_table_type_t));

    if (array != NULL) {
        ast_array_declarator_t *array_declarator = array->value;
        ast_constant_t *array_size = array_declarator->size->value;

        data_type->type = SYMBOL_TABLE_DATA_TYPE_ARRAY;
        data_type->array_size = atoi(array_size->constant);
        data_type->next = symbol_table_get_type(symbol_table, specifiers_node, pointer, array_declarator->array);
        return data_type;
    }

    if (pointer != NULL) {
        ast_pointer_t *ast_pointer = pointer->value;
        data_type->type = SYMBOL_TABLE_DATA_TYPE_POINTER;
        data_type->next = symbol_table_get_type(symbol_table, specifiers_node, ast_pointer->child, array);
        if (data_type->next != NULL) {
            return data_type;
        }
    }

    if (specifiers_node != NULL) {
        data_type->type = SYMBOL_TABLE_DATA_TYPE_BASE_TYPE;

        ast_specifier_t *specifiers = specifiers_node->value;
        if (specifiers->type_specifier->type == AST_TYPE_KEYWORD) {
            ast_keyword_t *ast_keyword = specifiers->type_specifier->value;
            switch (ast_keyword->keyword) {
                case TOKEN_TYPE_KEYWORD_VOID:
                    data_type->base_type = SYMBOL_TABLE_BASE_TYPE_VOID;
                    break;
                case TOKEN_TYPE_KEYWORD_CHAR:
                    data_type->base_type = SYMBOL_TABLE_BASE_TYPE_CHAR;
                    break;
                case TOKEN_TYPE_KEYWORD_INT:
                    data_type->base_type = SYMBOL_TABLE_BASE_TYPE_INT;
                    break;
                case TOKEN_TYPE_KEYWORD_FLOAT:
                    data_type->base_type = SYMBOL_TABLE_BASE_TYPE_FLOAT;
                    break;
                default:
                    break;
            }
        }

        if (specifiers->type_specifier->type == AST_TYPE_ENUM) {
            data_type->base_type = SYMBOL_TABLE_BASE_TYPE_INT;
        }

        if (specifiers->type_specifier->type == AST_TYPE_CONSTANT) {
            ast_constant_t *primitive_type = specifiers->type_specifier->value;
            if (string_cmp(primitive_type->constant, "void") == 0) {
                data_type->base_type = SYMBOL_TABLE_BASE_TYPE_VOID;
            } else if (string_cmp(primitive_type->constant, "char") == 0) {
                data_type->base_type = SYMBOL_TABLE_BASE_TYPE_CHAR;
            } else if (string_cmp(primitive_type->constant, "int") == 0) {
                data_type->base_type = SYMBOL_TABLE_BASE_TYPE_INT;
            } else if (string_cmp(primitive_type->constant, "float") == 0) {
                data_type->base_type = SYMBOL_TABLE_BASE_TYPE_FLOAT;
            } else {
                symbol_table_symbol_t *tmp = symbol_table_find(symbol_table, primitive_type->constant);
                symbol_table_typedef_t *typedef_symbol = tmp->element;
                if (typedef_symbol->symbol->symbol_type == SYMBOL_TABLE_SYMBOL_TYPE_ENUM) {
                    data_type->base_type = SYMBOL_TABLE_BASE_TYPE_INT;
                }
            }
        }
    }

    return data_type;
}

symbol_table_base_type_t symbol_table_get_base_type(symbol_table_type_t *data_type) {
    while (data_type->next != NULL) {
        data_type = data_type->next;
    }
    return data_type->base_type;
}

symbol_table_type_t* symbol_table_resolve_typedef(ast_typedef_t *ast_typedef) {
    return NULL;
}

int symbol_table_get_type_size(symbol_table_type_t *data_type) {
    while (data_type->next != NULL && data_type->type != SYMBOL_TABLE_DATA_TYPE_BASE_TYPE) {
        data_type = data_type->next;
    }
    return symbol_table_type_size(data_type);
}

int symbol_table_type_size(symbol_table_type_t *data_type) {
    if (data_type == NULL) {
        return 0;
    }

    switch (data_type->type) {
        case SYMBOL_TABLE_DATA_TYPE_ARRAY:;
            int array_size = symbol_table_type_size(data_type->next);
            return array_size * data_type->array_size;
        case SYMBOL_TABLE_DATA_TYPE_BASE_TYPE:
            switch (data_type->base_type) {
                case SYMBOL_TABLE_BASE_TYPE_VOID:
                    return 0;
                case SYMBOL_TABLE_BASE_TYPE_CHAR:
                    return 1;
                case SYMBOL_TABLE_BASE_TYPE_ENUM:
                case SYMBOL_TABLE_BASE_TYPE_INT:
                case SYMBOL_TABLE_BASE_TYPE_FLOAT:
                case SYMBOL_TABLE_BASE_TYPE_STRING:
                    return 4;
                case SYMBOL_TABLE_BASE_TYPE_STRUCT:
                    return 0;
            }
        case SYMBOL_TABLE_DATA_TYPE_POINTER:
            return 4;
        default:
            return 0;
    }
}

int symbol_table_get_symbol_size(symbol_table_symbol_t *symbol) {
    switch (symbol->symbol_type) {
        case SYMBOL_TABLE_SYMBOL_TYPE_ARRAY:
            return symbol_table_get_array_size(symbol->element);
        case SYMBOL_TABLE_SYMBOL_TYPE_DECLARATION:
        case SYMBOL_TABLE_SYMBOL_TYPE_PARAMETER:;
            symbol_table_declaration_t *declaration_symbol = symbol->element;
            return symbol_table_type_size(declaration_symbol->base_type);
        case SYMBOL_TABLE_SYMBOL_TYPE_ENUM:;
            symbol_table_enum_t *enum_symbol = symbol->element;
            return symbol_table_get_array_size(enum_symbol->enumerators);
        case SYMBOL_TABLE_SYMBOL_TYPE_ENUMERATOR:
            return 4;
        case SYMBOL_TABLE_SYMBOL_TYPE_FUNCTION:;
            symbol_table_function_t *function_symbol = symbol->element;
            return function_symbol->size;
        case SYMBOL_TABLE_SYMBOL_TYPE_STRUCT:;
            symbol_table_struct_t *struct_symbol = symbol->element;
            return symbol_table_get_array_size(struct_symbol->declarations);
        case SYMBOL_TABLE_SYMBOL_TYPE_TYPEDEF:
            return 0;
    }
    return 0;
}

int symbol_table_get_array_size(dynamic_array_t *dynamic_array) {
    int i, size = 0;
    for (i = 0; i < dynamic_array->size; i++) {
        symbol_table_symbol_t *symbol = dynamic_array_get(dynamic_array, i);
        size += symbol_table_get_symbol_size(symbol);
    }
    return size;
}

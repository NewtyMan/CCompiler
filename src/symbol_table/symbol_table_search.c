#include <symbol_table/symbol_table.h>
#include <utils/hash.h>
#include <utils/string.h>

 /*************************
 *     SEARCH BY NAME     *
 *************************/

symbol_table_symbol_t* symbol_table_find_internal(
    symbol_table_symbol_t *symbol,
    symbol_table_symbol_t *current_symbol,
    void *identifier,
    unsigned int identifier_hash
);

int symbol_table_compare_name(
    symbol_table_symbol_t *symbol,
    void *identifier,
    unsigned int identifier_hash
) {
    if (symbol == NULL) {
        return 0;
    }

    char *name = identifier;
    switch (symbol->symbol_type) {
        case SYMBOL_TABLE_SYMBOL_TYPE_DECLARATION:
        case SYMBOL_TABLE_SYMBOL_TYPE_PARAMETER:;
            symbol_table_declaration_t *declaration_symbol = symbol->element;
            if (declaration_symbol->name_hash == identifier_hash && string_cmp(declaration_symbol->name, name) == 0) {
                return 1;
            }
            break;
        case SYMBOL_TABLE_SYMBOL_TYPE_ENUM:;
            symbol_table_enum_t *enum_symbol = symbol->element;
            if (enum_symbol->name_hash == identifier_hash && string_cmp(enum_symbol->name, name) == 0) {
                return 1;
            }
            break;
        case SYMBOL_TABLE_SYMBOL_TYPE_ENUMERATOR:;
            symbol_table_enumerator_t *enumerator_symbol = symbol->element;
            if (enumerator_symbol->name_hash == identifier_hash && string_cmp(enumerator_symbol->name, name) == 0) {
                return 1;
            }
            break;
        case SYMBOL_TABLE_SYMBOL_TYPE_FUNCTION:;
            symbol_table_function_t *function_symbol = symbol->element;
            if (function_symbol->name_hash == identifier_hash && string_cmp(function_symbol->name, name) == 0) {
                return 1;
            }
            break;
        case SYMBOL_TABLE_SYMBOL_TYPE_STRUCT:;
            symbol_table_struct_t *struct_symbol = symbol->element;
            if (struct_symbol->name_hash == identifier_hash && string_cmp(struct_symbol->name, name) == 0) {
                return 1;
            }
            break;
        case SYMBOL_TABLE_SYMBOL_TYPE_TYPEDEF:;
            symbol_table_typedef_t *typedef_symbol = symbol->element;
            if (typedef_symbol->name_hash == identifier_hash && string_cmp(typedef_symbol->name, name) == 0) {
                return 1;
            }
            break;
        default:
            break;
    }
    return 0;
}

symbol_table_symbol_t* symbol_table_find_internal_array(
    dynamic_array_t *array,
    symbol_table_symbol_t *current_symbol,
    void *identifier,
    unsigned int identifier_hash
) {
    int i;
    for (i = 0; i < array->size; i++) {
        symbol_table_symbol_t *array_symbol = dynamic_array_get(array, i);
        symbol_table_symbol_t *return_symbol = symbol_table_find_internal(array_symbol, current_symbol, identifier, identifier_hash);
        if (return_symbol != NULL) {
            return return_symbol;
        }
        if (array_symbol == current_symbol) {
            return NULL;
        }
    }
    return NULL;
}

symbol_table_symbol_t* symbol_table_find_root(
    symbol_table_symbol_t *symbol,
    symbol_table_symbol_t *current_symbol,
    void *identifier,
    unsigned int identifier_hash
) {
    dynamic_array_t *root_array = symbol->element;

    int i;
    for (i = 0; i < root_array->size; i++) {
        symbol_table_symbol_t *root_symbol = dynamic_array_get(root_array, i);
        if (root_symbol->symbol_type == SYMBOL_TABLE_SYMBOL_TYPE_FUNCTION && symbol_table_compare_name(root_symbol, identifier, identifier_hash) == 1) {
            return root_symbol;
        }

        if (root_symbol->symbol_type == SYMBOL_TABLE_SYMBOL_TYPE_STRUCT) {
            symbol_table_struct_t *struct_symbol = root_symbol->element;
            int j;
            for (j = 0; j < struct_symbol->declarations->size; j++) {
                symbol_table_symbol_t *struct_declaration_symbol = dynamic_array_get(struct_symbol->declarations, j);
                if (symbol_table_compare_name(struct_declaration_symbol, identifier, identifier_hash) == 1) {
                    return struct_declaration_symbol;
                }
            }
            continue;
        }

        if (root_symbol->symbol_type == SYMBOL_TABLE_SYMBOL_TYPE_TYPEDEF) {
            symbol_table_typedef_t *typedef_symbol = root_symbol->element;
            if (symbol_table_compare_name(root_symbol, identifier, identifier_hash) == 1) {
                return typedef_symbol->symbol;
            }
        }

        if (root_symbol == current_symbol) {
            return NULL;
        }

        symbol_table_symbol_t *return_symbol = symbol_table_find_internal(root_symbol, current_symbol, identifier, identifier_hash);
        if (return_symbol != NULL) {
            return return_symbol;
        }
    }

    return NULL;
}

symbol_table_symbol_t *symbol_table_find_function(
    symbol_table_symbol_t *symbol,
    symbol_table_symbol_t *current_symbol,
    void *identifier,
    unsigned int identifier_hash
) {
    symbol_table_function_t *function_symbol = symbol->element;
    int i;

    for (i = 0; i < function_symbol->parameters->size; i++) {
        symbol_table_symbol_t *parameter_symbol = dynamic_array_get(function_symbol->parameters, i);
        if (symbol_table_compare_name(parameter_symbol, identifier, identifier_hash) == 1) {
            return parameter_symbol;
        }
        if (parameter_symbol == current_symbol) {
            break;
        }
    }

    for (i = 0; i < function_symbol->body->size; i++) {
        symbol_table_symbol_t *body_symbol = dynamic_array_get(function_symbol->body, i);
        if (body_symbol->symbol_type != SYMBOL_TABLE_SYMBOL_TYPE_ARRAY && symbol_table_compare_name(body_symbol, identifier, identifier_hash) == 1) {
            return body_symbol;
        }
        if (body_symbol == current_symbol) {
            break;
        }
    }

    if (symbol_table_compare_name(symbol, identifier, identifier_hash) == 1) {
        return symbol;
    }

    if (symbol->parent != NULL) {
        // Since functions can only be defined in the root scope, we know that the parent element will be a root node,
        // and can thus call the corresponding function directly.
        return symbol_table_find_root(symbol->parent, symbol, identifier, identifier_hash);
    }

    return NULL;
}

symbol_table_symbol_t* symbol_table_find_internal(
    symbol_table_symbol_t *symbol,
    symbol_table_symbol_t *current_symbol,
    void *identifier,
    unsigned int identifier_hash
) {
    if (symbol == NULL) {
        return NULL;
    }

    symbol_table_symbol_t *return_symbol = NULL;
    switch (symbol->symbol_type) {
        case SYMBOL_TABLE_SYMBOL_TYPE_ARRAY:;
            dynamic_array_t *array = symbol->element;
            int i;
            for (i = 0; i < array->size; i++) {
                symbol_table_symbol_t *array_symbol = dynamic_array_get(array, i);
                if (symbol_table_compare_name(array_symbol, identifier, identifier_hash) == 1) {
                    return array_symbol;
                }
                if (array_symbol == current_symbol) {
                    break;
                }
            }
            break;
        case SYMBOL_TABLE_SYMBOL_TYPE_ENUM:;
            if (symbol_table_compare_name(symbol, identifier, identifier_hash) == 1) {
                return symbol;
            }
            symbol_table_enum_t *enum_symbol = symbol->element;
            return_symbol = symbol_table_find_internal_array(enum_symbol->enumerators, current_symbol, identifier, identifier_hash);
            if (return_symbol != NULL) {
                return return_symbol;
            }
            break;
        case SYMBOL_TABLE_SYMBOL_TYPE_FUNCTION:
            return symbol_table_find_function(symbol, current_symbol, identifier, identifier_hash);
        default:
            if (symbol_table_compare_name(symbol, identifier, identifier_hash) == 1) {
                return symbol;
            }
            break;
    }

    if (symbol->parent != NULL) {
        return symbol_table_find_internal(symbol->parent, symbol, identifier, identifier_hash);
    }

    return NULL;
}

symbol_table_symbol_t* symbol_table_find(symbol_table_symbol_t *symbol, char *identifier) {
    unsigned int identifier_hash = hash_32_fnv1a(identifier);
    return symbol_table_find_internal(symbol, symbol, identifier, identifier_hash);
}

 /*************************
 *     SEARCH BY NODE     *
 *************************/

symbol_table_symbol_t* symbol_table_find_internal_node(symbol_table_symbol_t *symbol, void *identifier);

symbol_table_symbol_t* symbol_table_find_internal_node_array(dynamic_array_t *array, void *identifier) {
    int i;
    for (i = 0; i < array->size; i++) {
        symbol_table_symbol_t *array_symbol = dynamic_array_get(array, i);
        symbol_table_symbol_t *return_symbol = symbol_table_find_internal_node(array_symbol, identifier);
        if (return_symbol != NULL) {
            return return_symbol;
        }
    }
    return NULL;
}

symbol_table_symbol_t* symbol_table_find_internal_node(symbol_table_symbol_t *symbol, void *identifier) {
    if (symbol == NULL) {
        return NULL;
    }

    if (symbol->ast_node == identifier) {
        return symbol;
    }

    symbol_table_symbol_t *return_symbol = NULL;
    switch (symbol->symbol_type) {
        case SYMBOL_TABLE_SYMBOL_TYPE_ARRAY:;
            dynamic_array_t *array = symbol->element;
            return_symbol = symbol_table_find_internal_node_array(array, identifier);
            if (return_symbol != NULL) {
                return return_symbol;
            }
            break;
        case SYMBOL_TABLE_SYMBOL_TYPE_ENUM:;
            symbol_table_enum_t *enum_symbol = symbol->element;
            return_symbol = symbol_table_find_internal_node_array(enum_symbol->enumerators, identifier);
            if (return_symbol != NULL) {
                return return_symbol;
            }
            break;
        case SYMBOL_TABLE_SYMBOL_TYPE_FUNCTION:;
            symbol_table_function_t *function_symbol = symbol->element;
            return_symbol = symbol_table_find_internal_node_array(function_symbol->parameters, identifier);
            if (return_symbol != NULL) {
                return return_symbol;
            }
            return_symbol = symbol_table_find_internal_node_array(function_symbol->body, identifier);
            if (return_symbol != NULL) {
                return return_symbol;
            }
            break;
        case SYMBOL_TABLE_SYMBOL_TYPE_STRUCT:;
            symbol_table_struct_t *struct_symbol = symbol->element;
            return_symbol = symbol_table_find_internal_node_array(struct_symbol->declarations, identifier);
            if (return_symbol != NULL) {
                return return_symbol;
            }
            break;
        default:
            break;
    }

    return NULL;
}

int symbol_table_compare_node(symbol_table_symbol_t *symbol, void *ast_node) {
    if (symbol == NULL) {
        return 0;
    }

    if (symbol->ast_node == ast_node) {
        return 1;
    }
    return 0;
}

symbol_table_symbol_t* symbol_table_find_node(symbol_table_symbol_t *symbol, void *ast_node) {
    symbol_table_symbol_t *root_node = symbol;
    while (root_node->parent != NULL) {
        root_node = root_node->parent;
    }

    return symbol_table_find_internal_node(root_node, ast_node);
}

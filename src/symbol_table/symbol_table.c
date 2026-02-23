#include <symbol_table/symbol_table.h>
#include <syntax_analyzer/syntax_analyzer_visitor.h>
#include <stdlib.h>

 /***************************
 *          PRINTER         *
 ***************************/

const int SYMBOL_INDENT_OFFSET = 2;

void symbol_table_print_type(symbol_table_type_t *type) {
    switch (type->type) {
        case SYMBOL_TABLE_DATA_TYPE_ARRAY:
            printf("array(%d, ", type->array_size);
            symbol_table_print_type(type->next);
            printf(")");
            break;
        case SYMBOL_TABLE_DATA_TYPE_BASE_TYPE:
            printf("%s", symbol_table_base_type_labels[type->base_type]);
            break;
        case SYMBOL_TABLE_DATA_TYPE_POINTER:
            symbol_table_print_type(type->next);
            printf("*");
            break;
    }
}

void symbol_table_print_array(dynamic_array_t *array, int indent) {
    if (array == NULL || array->size == 0) {
        return;
    }

    int i;
    for (i = 0; i < array->size; i++) {
        symbol_table_symbol_t *symbol = dynamic_array_get(array, i);
        symbol_table_print(symbol, indent);
    }
}

void symbol_table_print(symbol_table_symbol_t *symbol, int indent) {
    if (symbol == NULL || symbol->is_system == 1) {
        return;
    }

    switch (symbol->symbol_type) {
        case SYMBOL_TABLE_SYMBOL_TYPE_ARRAY:;
            dynamic_array_t *dynamic_array = symbol->element;
            printf("%*s[BlockScope]\n", indent, "");
            symbol_table_print_array(dynamic_array, indent + SYMBOL_INDENT_OFFSET);
            break;
        case SYMBOL_TABLE_SYMBOL_TYPE_DECLARATION:;
            symbol_table_declaration_t *declaration_symbol = symbol->element;
            printf("%*s[Declaration, name=%s, type=(", indent, "", declaration_symbol->name);
            symbol_table_print_type(declaration_symbol->base_type);
            printf("), size=%d, offset=%d", declaration_symbol->size, declaration_symbol->offset);
            if (declaration_symbol->is_constant == 1) {
                printf(", const");
            }
            if (declaration_symbol->is_string_literal) {
                printf(", string");
            }
            printf("]\n");
            break;
        case SYMBOL_TABLE_SYMBOL_TYPE_ENUM:;
            symbol_table_enum_t *enum_symbol = symbol->element;
            if (enum_symbol->enumerators != NULL && enum_symbol->enumerators->size > 0) {
                printf("%*s[Enum, name=%s]\n", indent, "", enum_symbol->name);
                symbol_table_print_array(enum_symbol->enumerators, indent + SYMBOL_INDENT_OFFSET);
            }
            break;
        case SYMBOL_TABLE_SYMBOL_TYPE_ENUMERATOR:;
            symbol_table_enumerator_t *enumerator_symbol = symbol->element;
            printf("%*s[Enumerator, name=%s, value=%d]\n", indent, "", enumerator_symbol->name, enumerator_symbol->value);
            break;
        case SYMBOL_TABLE_SYMBOL_TYPE_FUNCTION:;
            symbol_table_function_t *function_symbol = symbol->element;
            printf("%*s[Function, name=%s, size=%d]\n", indent, "", function_symbol->name, function_symbol->size);
            symbol_table_print_array(function_symbol->parameters, indent + SYMBOL_INDENT_OFFSET);
            symbol_table_print_array(function_symbol->body, indent + SYMBOL_INDENT_OFFSET);
            break;
        case SYMBOL_TABLE_SYMBOL_TYPE_PARAMETER:;
            symbol_table_declaration_t *parameter_symbol = symbol->element;
            printf("%*s[Parameter, name=%s, type=(", indent, "",parameter_symbol->name);
            symbol_table_print_type(parameter_symbol->base_type);
            printf("), size=%d, offset=%d", parameter_symbol->size, parameter_symbol->offset);
            if (parameter_symbol->is_constant == 1) {
                printf(", const");
            }
            printf("]\n");
            break;
        case SYMBOL_TABLE_SYMBOL_TYPE_STRUCT:;
            symbol_table_struct_t *structure_symbol = symbol->element;
            printf("%*s[Struct, name=%s, size=%d]\n", indent, "", structure_symbol->name, structure_symbol->size);
            symbol_table_print_array(structure_symbol->declarations, indent + SYMBOL_INDENT_OFFSET);
            break;
        case SYMBOL_TABLE_SYMBOL_TYPE_TYPEDEF:;
            symbol_table_typedef_t *typedef_symbol = symbol->element;
            printf("%*s[Typedef, name=%s]\n", indent, "", typedef_symbol->name);
            symbol_table_print(typedef_symbol->symbol, indent + SYMBOL_INDENT_OFFSET);
            break;
        default:
            printf("[ERROR] [SYMBOL] Unknown symbol table type: %d\n", symbol->symbol_type);
            break;
    }
}

 /***************************
 *          FREEING         *
 ***************************/

void symbol_table_free_data_type(symbol_table_type_t *data_type) {
    if (data_type == NULL) {
        return;
    }
    if (data_type->next != NULL) {
        symbol_table_free_data_type(data_type->next);
    }
    free(data_type);
}

void symbol_table_free(void *symbol_ptr) {
    if (symbol_ptr == NULL) {
        return;
    }

    symbol_table_symbol_t *symbol = symbol_ptr;
    switch (symbol->symbol_type) {
        case SYMBOL_TABLE_SYMBOL_TYPE_ARRAY:;
            dynamic_array_t *dynamic_array = symbol->element;
            dynamic_array_free(dynamic_array, symbol_table_free);
            break;
        case SYMBOL_TABLE_SYMBOL_TYPE_DECLARATION:
        case SYMBOL_TABLE_SYMBOL_TYPE_PARAMETER:;
            symbol_table_declaration_t *declaration_symbol = symbol->element;
            symbol_table_free_data_type(declaration_symbol->base_type);
            free(declaration_symbol->name);
            break;
        case SYMBOL_TABLE_SYMBOL_TYPE_ENUM:;
            symbol_table_enum_t *enum_symbol = symbol->element;
            free(enum_symbol->name);
            dynamic_array_free(enum_symbol->enumerators, symbol_table_free);
            break;
        case SYMBOL_TABLE_SYMBOL_TYPE_ENUMERATOR:;
            symbol_table_enumerator_t *enumerator_symbol = symbol->element;
            free(enumerator_symbol->name);
            break;
        case SYMBOL_TABLE_SYMBOL_TYPE_FUNCTION:;
            symbol_table_function_t *function_symbol = symbol->element;
            symbol_table_free_data_type(function_symbol->base_type);
            free(function_symbol->name);
            dynamic_array_free(function_symbol->parameters, symbol_table_free);
            dynamic_array_free(function_symbol->body, symbol_table_free);
            break;
        case SYMBOL_TABLE_SYMBOL_TYPE_STRUCT:;
            symbol_table_struct_t *struct_symbol = symbol->element;
            free(struct_symbol->name);
            dynamic_array_free(struct_symbol->declarations, symbol_table_free);
            break;
        case SYMBOL_TABLE_SYMBOL_TYPE_TYPEDEF:;
            symbol_table_typedef_t *typedef_symbol = symbol->element;
            symbol_table_free_data_type(typedef_symbol->base_type);
            symbol_table_free(typedef_symbol->symbol);
            free(typedef_symbol->name);
            break;
    }

    free(symbol);
}

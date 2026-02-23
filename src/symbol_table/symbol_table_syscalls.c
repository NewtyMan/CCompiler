#include <symbol_table/symbol_table.h>
#include <utils/dynamic_array.h>
#include <utils/hash.h>
#include <utils/string.h>
#include <stdlib.h>

symbol_table_type_t* new_base_type_pointer(symbol_table_base_type_t base_type) {
    symbol_table_type_t *pointer = malloc(sizeof(symbol_table_type_t));
    pointer->type = SYMBOL_TABLE_DATA_TYPE_POINTER;
    pointer->next = symbol_table_new_base_type(base_type);
    return pointer;
}

void symbol_table_syscall_add_parameter(symbol_table_symbol_t *parent, symbol_table_type_t *type, char *name) {
    symbol_table_function_t *parent_function = parent->element;

    symbol_table_declaration_t* parameter = malloc(sizeof(symbol_table_declaration_t));
    parameter->base_type = type;
    parameter->name = malloc(string_len(name) + 1);
    string_cpy(parameter->name, name);
    parameter->size = symbol_table_type_size(parameter->base_type);
    parameter->offset = parent_function->parameter_size;
    parameter->is_parameter = 1;
    parameter->value = 0;
    parameter->name_hash = hash_32_fnv1a(parameter->name);

    symbol_table_symbol_t *symbol = symbol_table_new_symbol(SYMBOL_TABLE_SYMBOL_TYPE_DECLARATION, parent, parameter, NULL);
    symbol->is_system = 1;

    dynamic_array_add(parent_function->parameters, symbol);
    parent_function->parameter_size += parameter->size;
    parent_function->size += parameter->size;
}

symbol_table_symbol_t* symbol_table_syscall_new_function(symbol_table_symbol_t *root, symbol_table_base_type_t base_type, char *name) {
    symbol_table_function_t *function = malloc(sizeof(symbol_table_function_t));
    function->base_type = symbol_table_new_base_type(base_type);
    function->name = malloc(string_len(name) + 1);
    string_cpy(function->name, name);
    function->name_hash = hash_32_fnv1a(function->name);

    function->parameters = dynamic_array_new();
    function->body = dynamic_array_new();

    function->parameter_size = 12;
    function->local_size = 0;
    function->size = 12;

    symbol_table_symbol_t *symbol = symbol_table_new_symbol(SYMBOL_TABLE_SYMBOL_TYPE_FUNCTION, root, function, NULL);
    symbol->is_system = 1;
    dynamic_array_add(root->element, symbol);
    return symbol;
}

void symbol_table_add_constant(symbol_table_symbol_t *symbol_table, char *name, int value) {
    symbol_table_declaration_t* parameter = malloc(sizeof(symbol_table_declaration_t));
    parameter->base_type = symbol_table_new_base_type(SYMBOL_TABLE_BASE_TYPE_INT);
    parameter->name = malloc(string_len(name) + 1);
    string_cpy(parameter->name, name);
    parameter->size = symbol_table_type_size(parameter->base_type);
    parameter->offset = 0;
    parameter->is_parameter = 0;
    parameter->is_constant = 1;
    parameter->value = value;
    parameter->name_hash = hash_32_fnv1a(parameter->name);

    symbol_table_symbol_t *symbol = symbol_table_new_symbol(SYMBOL_TABLE_SYMBOL_TYPE_DECLARATION, symbol_table, parameter, NULL);
    symbol->is_system = 1;
    dynamic_array_add(symbol_table->element, symbol);
}

void symbol_table_add_syscalls_arm32(symbol_table_symbol_t *symbol_table) {
    /*
     *  This function adds the required syscalls to the symbol table, so that semantic analysis can find the functions
     *  when doing scope and type checking. This is required, since the syscalls are injected into the compiled code,
     *  so we need to provide their definitions here.
     */

    symbol_table_add_constant(symbol_table, "O_RDONLY", 0);
    symbol_table_add_constant(symbol_table, "O_WRONLY", 1);
    symbol_table_add_constant(symbol_table, "O_RDWR", 2);

    symbol_table_add_constant(symbol_table, "STDOUT_FILENO", 0);

    // int close(int fd);
    symbol_table_symbol_t *close = symbol_table_syscall_new_function(symbol_table, SYMBOL_TABLE_BASE_TYPE_INT, "close");
    symbol_table_syscall_add_parameter(close, symbol_table_new_base_type(SYMBOL_TABLE_BASE_TYPE_INT), "fd");

    // int getrandom(int size, char *buf, int size, int flags)
    symbol_table_symbol_t *getrandom = symbol_table_syscall_new_function(symbol_table, SYMBOL_TABLE_BASE_TYPE_INT, "getrandom");
    symbol_table_syscall_add_parameter(getrandom, new_base_type_pointer(SYMBOL_TABLE_BASE_TYPE_CHAR), "buf");
    symbol_table_syscall_add_parameter(getrandom, symbol_table_new_base_type(SYMBOL_TABLE_BASE_TYPE_INT), "size");
    symbol_table_syscall_add_parameter(getrandom, symbol_table_new_base_type(SYMBOL_TABLE_BASE_TYPE_INT), "flags");

    // int open(char *filename, int flags, int mode);
    symbol_table_symbol_t *open = symbol_table_syscall_new_function(symbol_table, SYMBOL_TABLE_BASE_TYPE_INT, "open");
    symbol_table_syscall_add_parameter(open, new_base_type_pointer(SYMBOL_TABLE_BASE_TYPE_CHAR), "filename");
    symbol_table_syscall_add_parameter(open, symbol_table_new_base_type(SYMBOL_TABLE_BASE_TYPE_INT), "flags");
    symbol_table_syscall_add_parameter(open, symbol_table_new_base_type(SYMBOL_TABLE_BASE_TYPE_INT), "mode");

    // int read(int fd, char *buf, int count);
    symbol_table_symbol_t *read = symbol_table_syscall_new_function(symbol_table, SYMBOL_TABLE_BASE_TYPE_INT, "read");
    symbol_table_syscall_add_parameter(read, symbol_table_new_base_type(SYMBOL_TABLE_BASE_TYPE_INT), "fd");
    symbol_table_syscall_add_parameter(read, new_base_type_pointer(SYMBOL_TABLE_BASE_TYPE_CHAR), "buf");
    symbol_table_syscall_add_parameter(read, symbol_table_new_base_type(SYMBOL_TABLE_BASE_TYPE_INT), "count");

    // int write(int fd, char *buf, int count)
    symbol_table_symbol_t *write = symbol_table_syscall_new_function(symbol_table, SYMBOL_TABLE_BASE_TYPE_INT, "write");
    symbol_table_syscall_add_parameter(write, symbol_table_new_base_type(SYMBOL_TABLE_BASE_TYPE_INT), "fd");
    symbol_table_syscall_add_parameter(write, new_base_type_pointer(SYMBOL_TABLE_BASE_TYPE_CHAR), "buf");
    symbol_table_syscall_add_parameter(write, symbol_table_new_base_type(SYMBOL_TABLE_BASE_TYPE_INT), "count");
}

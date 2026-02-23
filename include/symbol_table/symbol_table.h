#ifndef CCOMPILER_SYMBOL_TABLE_H
#define CCOMPILER_SYMBOL_TABLE_H
#include <syntax_analyzer/syntax_analyzer.h>
#include <utils/dynamic_array.h>


/*********************************
 *    Symbol Table Data Types    *
 *********************************/

typedef enum symbol_table_data_type {
    SYMBOL_TABLE_DATA_TYPE_ARRAY,
    SYMBOL_TABLE_DATA_TYPE_BASE_TYPE,
    SYMBOL_TABLE_DATA_TYPE_POINTER,
} symbol_table_data_type_t;

typedef enum symbol_table_base_type {
    SYMBOL_TABLE_BASE_TYPE_VOID,
    SYMBOL_TABLE_BASE_TYPE_CHAR,
    SYMBOL_TABLE_BASE_TYPE_ENUM,
    SYMBOL_TABLE_BASE_TYPE_INT,
    SYMBOL_TABLE_BASE_TYPE_FLOAT,
    SYMBOL_TABLE_BASE_TYPE_STRUCT,
    SYMBOL_TABLE_BASE_TYPE_STRING,
} symbol_table_base_type_t;

static char *symbol_table_base_type_labels[7] = {
    "void",
    "char",
    "enum",
    "int",
    "float",
    "struct",
    "string"
};

typedef struct symbol_table_type {
    symbol_table_data_type_t type;
    symbol_table_base_type_t base_type;
    int array_size;
    struct symbol_table_type *next;
} symbol_table_type_t;


/****************************
 *    Symbol Table Types    *
 ****************************/

typedef enum symbol_table_symbol_type {
    SYMBOL_TABLE_SYMBOL_TYPE_ARRAY,
    SYMBOL_TABLE_SYMBOL_TYPE_DECLARATION,
    SYMBOL_TABLE_SYMBOL_TYPE_ENUM,
    SYMBOL_TABLE_SYMBOL_TYPE_ENUMERATOR,
    SYMBOL_TABLE_SYMBOL_TYPE_FUNCTION,
    SYMBOL_TABLE_SYMBOL_TYPE_PARAMETER,
    SYMBOL_TABLE_SYMBOL_TYPE_STRUCT,
    SYMBOL_TABLE_SYMBOL_TYPE_TYPEDEF,
} symbol_table_symbol_type_t;

typedef struct symbol_table_symbol {
    symbol_table_symbol_type_t symbol_type;
    struct symbol_table_symbol *parent;
    void *ast_node;
    void *element;
    char is_system;
} symbol_table_symbol_t;

typedef struct symbol_table_declaration {
    symbol_table_type_t *base_type;
    char *name, is_parameter, is_constant;
    int offset, size, value, is_string_literal;
    unsigned int name_hash;
} symbol_table_declaration_t;

typedef struct symbol_table_enum {
    char *name;
    unsigned int name_hash;
    dynamic_array_t *enumerators;
} symbol_table_enum_t;

typedef struct symbol_table_enumerator {
    char *name;
    int value;
    unsigned int name_hash;
} symbol_table_enumerator_t;

typedef struct symbol_table_function {
    symbol_table_type_t *base_type;
    char *name;
    unsigned int name_hash;

    dynamic_array_t *parameters;
    dynamic_array_t *body;

    int parameter_size;
    int local_size;
    int size;
} symbol_table_function_t;

typedef struct symbol_table_struct {
    dynamic_array_t *declarations;
    char *name;
    int size;
    unsigned int name_hash;
} symbol_table_struct_t;

typedef struct symbol_table_typedef {
    symbol_table_type_t *base_type;
    symbol_table_symbol_t *symbol;
    char *name;
    unsigned int name_hash;
} symbol_table_typedef_t;


/*************************
 *   FACTORY FUNCTIONS   *
 *************************/

symbol_table_symbol_t* symbol_table_new_symbol(
    symbol_table_symbol_type_t symbol_type,
    symbol_table_symbol_t *parent,
    void *element,
    void *ast_node
);

symbol_table_symbol_t* symbol_table_new_declaration(
    symbol_table_symbol_t *parent,
    ast_node_t *specifiers,
    ast_declarator_t *declarator
);

symbol_table_symbol_t* symbol_table_new_enum(
    symbol_table_symbol_t *parent,
    ast_enum_t *ast_enum
);

symbol_table_symbol_t* symbol_table_new_enumerator(
    symbol_table_symbol_t *parent,
    ast_constant_t *identifier,
    int value
);

symbol_table_symbol_t* symbol_table_new_function(
    symbol_table_symbol_t *parent,
    ast_function_t *function
);

symbol_table_symbol_t* symbol_table_new_parameter(
    symbol_table_symbol_t *parent,
    ast_parameter_t *parameter
);

symbol_table_symbol_t* symbol_table_new_struct(
    symbol_table_symbol_t *parent,
    ast_struct_t *ast_struct
);

symbol_table_symbol_t* symbol_table_new_typedef(
    symbol_table_symbol_t *parent,
    ast_typedef_t *ast_typedef
);

/************************
 *   HELPER FUNCTIONS   *
 ************************/

int symbol_table_get_symbol_size(
    symbol_table_symbol_t *symbol
);

int symbol_table_get_array_size(
    dynamic_array_t *dynamic_array
);

symbol_table_type_t* symbol_table_get_type(
    symbol_table_symbol_t *symbol_table,
    ast_node_t *specifiers_node,
    ast_node_t *pointer,
    ast_node_t *array
);

/************************
 *   SEARCH FUNCTIONS   *
 ************************/

symbol_table_symbol_t* symbol_table_find(
    symbol_table_symbol_t *symbol,
    char *identifier
);

symbol_table_symbol_t* symbol_table_find_node(
    symbol_table_symbol_t *symbol,
    void *ast_node
);

/*************************
 *   SYSCALL FUNCTIONS   *
 *************************/

void symbol_table_add_syscalls_arm32(
    symbol_table_symbol_t *symbol_table
);

/**********************
 *   TYPE FUNCTIONS   *
 **********************/

symbol_table_type_t* symbol_table_new_base_type(
    symbol_table_base_type_t base_type
);

symbol_table_base_type_t symbol_table_get_base_type(
    symbol_table_type_t *data_type
);

int symbol_table_get_type_size(
    symbol_table_type_t *data_type
);

int symbol_table_type_size(
    symbol_table_type_t *data_type
);


/*******************************
 *    Function Declarations    *
 *******************************/

symbol_table_symbol_t* symbol_table_parse(
    ast_node_t *node
);

void symbol_table_print(
    symbol_table_symbol_t* symbol_table,
    int indent
);

void symbol_table_free(
    void *element
);
#endif //CCOMPILER_SYMBOL_TABLE_H

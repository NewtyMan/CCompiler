#include <intermediate_code/intermediate_code.h>
#include <utils/string.h>
#include <stdlib.h>

extern dynamic_array_t *reference_pool;

int tmp_label_counter = 0;
int control_label_counter = 0;

intermediate_code_arg_t* new_int_const(int value) {
    intermediate_code_arg_t *constant = malloc(sizeof(intermediate_code_arg_t));
    constant->type = INTERMEDIATE_CODE_ARG_TYPE_INTEGER_CONSTANT;
    constant->integer_value = value;
    constant->string_value = calloc(32, sizeof(char));
    constant->attrs.is_address = 0;
    sprintf(constant->string_value, "%d", value);
    dynamic_array_add(reference_pool, constant);
    return constant;
}

intermediate_code_arg_t* new_i8_const(int value) {
    intermediate_code_arg_t *integer_constant = new_int_const(value);
    integer_constant->attrs.base_size = 1;
    integer_constant->attrs.type_size = 1;
    return integer_constant;
}

intermediate_code_arg_t* new_i32_const(int value) {
    intermediate_code_arg_t *integer_constant = new_int_const(value);
    integer_constant->attrs.base_size = 4;
    integer_constant->attrs.type_size = 4;
    return integer_constant;
}

intermediate_code_arg_t* new_address(intermediate_code_arg_t *address) {
    address->attrs.is_address = 1;
    return address;
}

intermediate_code_arg_t* new_string_constant(char *value) {
    intermediate_code_arg_t *constant = malloc(sizeof(intermediate_code_arg_t));
    constant->type = INTERMEDIATE_CODE_ARG_TYPE_STRING_CONSTANT;
    constant->integer_value = 0;
    constant->string_value = calloc(string_len(value) + 1, sizeof(char));
    string_cpy(constant->string_value, value);
    constant->attrs.is_address = 0;
    dynamic_array_add(reference_pool, constant);
    return constant;
}

intermediate_code_arg_t* new_tmp_label() {
    intermediate_code_arg_t *tmp_label = malloc(sizeof(intermediate_code_arg_t));
    tmp_label->type = INTERMEDIATE_CODE_ARG_TYPE_TMP_LABEL;
    tmp_label->integer_value = tmp_label_counter;
    tmp_label->string_value = calloc(8, sizeof(char));
    tmp_label->attrs.is_address = 0;
    tmp_label->attrs.no_dereference = 0;
    sprintf(tmp_label->string_value, "t%d", tmp_label_counter);
    tmp_label_counter++;
    dynamic_array_add(reference_pool, tmp_label);
    return tmp_label;
}

intermediate_code_arg_t* new_control_label() {
    intermediate_code_arg_t *label = malloc(sizeof(intermediate_code_arg_t));
    label->type = INTERMEDIATE_CODE_ARG_TYPE_CONTROL_LABEL;
    label->integer_value = control_label_counter;
    label->string_value = calloc(5, sizeof(char));
    label->attrs.base_size = 0;
    label->attrs.type_size = 0;
    label->attrs.is_address = 0;
    sprintf(label->string_value, "L%d", control_label_counter);
    control_label_counter++;
    dynamic_array_add(reference_pool, label);
    return label;
}

intermediate_code_tac_t* new_assign(
    intermediate_code_arg_t *result,
    intermediate_code_arg_t *value
) {
    intermediate_code_tac_t *tac = malloc(sizeof(intermediate_code_tac_t));
    tac->op = INTERMEDIATE_CODE_OP_ASSIGN;
    tac->result = result;
    tac->arg1 = value;
    tac->arg2 = NULL;
    return tac;
}

intermediate_code_tac_t* new_binary(
    intermediate_code_op_t op,
    intermediate_code_arg_t *result,
    intermediate_code_arg_t *arg1,
    intermediate_code_arg_t *arg2
) {
    intermediate_code_tac_t *tac = malloc(sizeof(intermediate_code_tac_t));
    tac->op = op;
    tac->result = result;
    tac->arg1 = arg1;
    tac->arg2 = arg2;
    return tac;
}

intermediate_code_tac_t* new_function(
    intermediate_code_arg_t *identifier,
    int parameter_size,
    int local_size
) {
    intermediate_code_tac_t *tac = malloc(sizeof(intermediate_code_tac_t));
    tac->op = INTERMEDIATE_CODE_OP_FUNCTION;
    tac->result = identifier;
    tac->arg1 = NULL;
    tac->arg2 = NULL;
    tac->attrs.parameter_size = parameter_size;
    tac->attrs.local_size = local_size;
    return tac;
}

intermediate_code_tac_t* new_function_call(
    intermediate_code_arg_t *result,
    intermediate_code_arg_t *function_name
) {
    intermediate_code_tac_t *tac = malloc(sizeof(intermediate_code_tac_t));
    tac->op = INTERMEDIATE_CODE_OP_FUNCTION_CALL;
    tac->result = result;
    tac->arg1 = function_name;
    tac->arg2 = NULL;
    return tac;
}

intermediate_code_tac_t* new_function_parameter(
    intermediate_code_arg_t *function_name,
    intermediate_code_arg_t *parameter_name,
    intermediate_code_arg_t *value
) {
    intermediate_code_tac_t *tac = malloc(sizeof(intermediate_code_tac_t));
    tac->op = INTERMEDIATE_CODE_OP_FUNCTION_PARAMETER;
    tac->result = function_name;
    tac->arg1 = parameter_name;
    tac->arg2 = value;
    return tac;
}

intermediate_code_tac_t *new_goto(
    intermediate_code_arg_t *goto_label
) {
    intermediate_code_tac_t *tac = malloc(sizeof(intermediate_code_tac_t));
    tac->op = INTERMEDIATE_CODE_OP_GOTO;
    tac->result = NULL;
    tac->arg1 = goto_label;
    tac->arg2 = NULL;
    return tac;
}

intermediate_code_tac_t *new_if_goto(
    intermediate_code_arg_t *condition_label,
    intermediate_code_arg_t *compare_arg,
    intermediate_code_arg_t *goto_label
) {
    intermediate_code_tac_t *tac = malloc(sizeof(intermediate_code_tac_t));
    tac->op = INTERMEDIATE_CODE_OP_IF_GOTO;
    tac->result = goto_label;
    tac->arg1 = condition_label;
    tac->arg2 = compare_arg;
    return tac;
}

intermediate_code_tac_t* new_label(
    intermediate_code_arg_t *label
) {
    intermediate_code_tac_t *tac = malloc(sizeof(intermediate_code_tac_t));
    tac->op = INTERMEDIATE_CODE_OP_LABEL;
    tac->result = NULL;
    tac->arg1 = label;
    tac->arg2 = NULL;
    return tac;
}

intermediate_code_tac_t* new_memory_read(
    intermediate_code_arg_t *result,
    intermediate_code_arg_t *argument
) {
    intermediate_code_tac_t *tac = malloc(sizeof(intermediate_code_tac_t));
    tac->op = INTERMEDIATE_CODE_OP_MEMORY_READ;
    tac->result = result;
    tac->arg1 = argument;
    tac->arg2 = NULL;
    return tac;
}

intermediate_code_tac_t* new_memory_write(
    intermediate_code_arg_t *location,
    intermediate_code_arg_t *argument
) {
    intermediate_code_tac_t *tac = malloc(sizeof(intermediate_code_tac_t));
    tac->op = INTERMEDIATE_CODE_OP_MEMORY_WRITE;
    tac->result = location;
    tac->arg1 = argument;
    tac->arg2 = NULL;
    return tac;
}

intermediate_code_tac_t* new_return(
    intermediate_code_arg_t *argument
) {
    intermediate_code_tac_t *tac = malloc(sizeof(intermediate_code_tac_t));
    tac->op = INTERMEDIATE_CODE_OP_FUNCTION_RETURN;
    tac->result = NULL;
    tac->arg1 = argument;
    tac->arg2 = NULL;
    return tac;
}

intermediate_code_tac_t* new_string_read(
    intermediate_code_arg_t *result,
    intermediate_code_arg_t *label,
    intermediate_code_arg_t *value
) {
    intermediate_code_tac_t *tac = malloc(sizeof(intermediate_code_tac_t));
    tac->op = INTERMEDIATE_CODE_OP_STRING_READ;
    tac->result = result;
    tac->arg1 = label;
    tac->arg2 = value;
    return tac;
}

intermediate_code_tac_t* new_unary(
    intermediate_code_op_t operator,
    intermediate_code_arg_t *result,
    intermediate_code_arg_t *argument
) {
    intermediate_code_tac_t *tac = malloc(sizeof(intermediate_code_tac_t));
    tac->op = operator;
    tac->result = result;
    tac->arg1 = argument;
    tac->arg2 = NULL;
    return tac;
}

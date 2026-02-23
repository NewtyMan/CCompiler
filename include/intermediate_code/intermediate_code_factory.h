#ifndef CCOMPILER_INTERMEDIATE_CODE_FACTORY_H
#define CCOMPILER_INTERMEDIATE_CODE_FACTORY_H
#include <intermediate_code/intermediate_code.h>

intermediate_code_arg_t* new_i8_const(int value);
intermediate_code_arg_t* new_i32_const(int value);

intermediate_code_arg_t* new_address(intermediate_code_arg_t *address);

intermediate_code_arg_t* new_string_constant(char *value);
intermediate_code_arg_t* new_tmp_label();
intermediate_code_arg_t* new_control_label();

intermediate_code_tac_t* new_assign(
    intermediate_code_arg_t *result,
    intermediate_code_arg_t *value
);

intermediate_code_tac_t* new_binary(
    intermediate_code_op_t op,
    intermediate_code_arg_t *result,
    intermediate_code_arg_t *arg1,
    intermediate_code_arg_t *arg2
);

intermediate_code_tac_t* new_function(
    intermediate_code_arg_t *identifier,
    int parameter_size,
    int local_size
);

intermediate_code_tac_t* new_function_call(
    intermediate_code_arg_t *result,
    intermediate_code_arg_t *function_name
);

intermediate_code_tac_t* new_function_parameter(
    intermediate_code_arg_t *function_name,
    intermediate_code_arg_t *parameter_name,
    intermediate_code_arg_t *value
);

intermediate_code_tac_t *new_goto(
    intermediate_code_arg_t *goto_label
);

intermediate_code_tac_t *new_if_goto(
    intermediate_code_arg_t *condition_label,
    intermediate_code_arg_t *compare_arg,
    intermediate_code_arg_t *goto_label
);

intermediate_code_tac_t* new_label(
    intermediate_code_arg_t *label
);

intermediate_code_tac_t* new_memory_read(
    intermediate_code_arg_t *result,
    intermediate_code_arg_t *argument
);

intermediate_code_tac_t* new_memory_write(
    intermediate_code_arg_t *location,
    intermediate_code_arg_t *argument
);

intermediate_code_tac_t* new_return(
    intermediate_code_arg_t *argument
);

intermediate_code_tac_t* new_string_read(
    intermediate_code_arg_t *result,
    intermediate_code_arg_t *label,
    intermediate_code_arg_t *value
);

intermediate_code_tac_t* new_unary(
    intermediate_code_op_t operator,
    intermediate_code_arg_t *result,
    intermediate_code_arg_t *argument
);
#endif //CCOMPILER_INTERMEDIATE_CODE_FACTORY_H

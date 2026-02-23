#include <code_generator/code_generator.h>
#include <intermediate_code/intermediate_code.h>
#include <utils/logger.h>
#include <utils/string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>

// OFFSET_PARAMETERS represents the offset required to store the last parameter. This comes from the fact that the
// caller stores registers r0-r8 on stack after settings parameters, which pushes the stack down additional 36 bytes.
const int OFFSET_PARAMETERS = 40;

const int OFFSET_OLD_FRAME_POINTER = 0;
const int OFFSET_RETURN_ADDRESS = 4;
const int OFFSET_RETURN_VALUE = 8;

typedef struct code_generator_params {
    dynamic_array_t *intermediate_code;
    int function_param_size;
    int function_local_size;

    char is_first_param;
    char is_param_present;
} code_generator_params_t;


void code_generator_add(dynamic_array_t *generated_code, char *format, ...) {
    char tmp[64];

    va_list args;
    va_start(args, format);
    vsprintf(tmp, format, args);
    va_end(args);

    char *buffer = calloc(string_len(tmp) + 1, sizeof(char));
    string_cpy(buffer, tmp);
    dynamic_array_add(generated_code, buffer);
}

char* get_mov_logical_instr_positive(intermediate_code_op_t opcode) {
    switch (opcode) {
        case INTERMEDIATE_CODE_OP_LOGICAL_LT:
            return "movlt";
        case INTERMEDIATE_CODE_OP_LOGICAL_LTE:
            return "movle";
        case INTERMEDIATE_CODE_OP_LOGICAL_GT:
            return "movgt";
        case INTERMEDIATE_CODE_OP_LOGICAL_GTE:
            return "movge";
        case INTERMEDIATE_CODE_OP_LOGICAL_EQUAL:
            return "moveq";
        case INTERMEDIATE_CODE_OP_LOGICAL_NOT_EQUAL:
            return "movne";
        default:
            logger_error("unsupported logical instruction for positive mov: %d", opcode);
            return "mov";
    }
}

char* get_mov_logical_instr_negative(intermediate_code_op_t opcode) {
    switch (opcode) {
        case INTERMEDIATE_CODE_OP_LOGICAL_LT:
            return "movge";
        case INTERMEDIATE_CODE_OP_LOGICAL_LTE:
            return "movgt";
        case INTERMEDIATE_CODE_OP_LOGICAL_GT:
            return "movle";
        case INTERMEDIATE_CODE_OP_LOGICAL_GTE:
            return "movlt";
        case INTERMEDIATE_CODE_OP_LOGICAL_EQUAL:
            return "movne";
        case INTERMEDIATE_CODE_OP_LOGICAL_NOT_EQUAL:
            return "moveq";
        default:
            logger_error("unsupported logical instruction for positive mov: %d", opcode);
            return "mov";
    }
}

char* get_load_instr(int size) {
    switch (size) {
        case 1:
            return "ldrb";
        case 4:
            return "ldr";
        case 8:
            return "ldrh";
        default:
            return "ldr";
    }
}

char* get_store_instr(int size) {
    switch (size) {
        case 1:
            return "strb";
        case 4:
            return "str";
        case 8:
            return "strh";
        default:
            return "str";
    }
}

char* get_immediate_value(dynamic_array_t *generated_code, int immediate) {
    char *buffer = calloc(32, sizeof(char));
    if (immediate >= -255 && immediate <= 255) {
        sprintf(buffer, "#%d", immediate);
    } else {
        code_generator_add(generated_code, "ldr r7, =#%d", immediate);
        sprintf(buffer, "r7");
    }
    return buffer;
}

char* get_arg_value(dynamic_array_t *generated_code, intermediate_code_arg_t *argument, char *reg, int allow_immediate) {
    if (argument == NULL) {
        return NULL;
    }

    int dynamic_buffer_size = argument->type == INTERMEDIATE_CODE_ARG_TYPE_STRING_CONSTANT ?
        string_len(argument->string_value) + 1 :
        32;

    char *buffer = calloc(dynamic_buffer_size, sizeof(char));
    switch (argument->type) {
        case INTERMEDIATE_CODE_ARG_TYPE_INTEGER_CONSTANT:
            if (allow_immediate == 1 && argument->integer_value <= 255) {
                sprintf(buffer, "#%d", argument->integer_value);
            } else if (argument->integer_value <= 255) {
                code_generator_add(generated_code, "mov %s, #%d", reg, argument->integer_value);
                string_cpy(buffer, reg);
            } else {
                code_generator_add(generated_code, "ldr %s, =#%d", reg, argument->integer_value);
                string_cpy(buffer, reg);
            }
            break;
        case INTERMEDIATE_CODE_ARG_TYPE_STRING_CONSTANT:
            if (argument->attrs.is_address == 1) {
                char *address_arg_value = get_immediate_value(generated_code, argument->attrs.offset);
                code_generator_add(generated_code, "add %s, fp, %s", reg, address_arg_value);
                string_cpy(buffer, reg);
                free(address_arg_value);
            } else {
                string_cpy(buffer, argument->string_value);
            }
            break;
        case INTERMEDIATE_CODE_ARG_TYPE_CONTROL_LABEL:
            sprintf(buffer, "%s", argument->string_value);
            break;
        case INTERMEDIATE_CODE_ARG_TYPE_TMP_LABEL:
            sprintf(buffer, "r%d", argument->integer_value);
            break;
    }

    return buffer;
}

int get_function_size(int size) {
    return (size + 3) & ~3;
}

void code_generator_generate_internal(dynamic_array_t *generated_code, code_generator_params_t *params) {
    int i;
    for (i = 0; i < params->intermediate_code->size; i++) {
        intermediate_code_tac_t *tac = dynamic_array_get(params->intermediate_code, i);
        char *result = get_arg_value(generated_code, tac->result, "r6", 0);
        char *arg1 = get_arg_value(generated_code, tac->arg1, "r7", 0);
        char *arg2 = get_arg_value(generated_code, tac->arg2, "r8", 0);

        switch (tac->op) {
            case INTERMEDIATE_CODE_OP_NONE:
                break;
            case INTERMEDIATE_CODE_OP_ADD:
                code_generator_add(generated_code, "add %s, %s, %s", result, arg1, arg2);
                break;
            case INTERMEDIATE_CODE_OP_SUB:
                code_generator_add(generated_code, "sub %s, %s, %s", result, arg1, arg2);
                break;
            case INTERMEDIATE_CODE_OP_MUL:
                code_generator_add(generated_code, "mul %s, %s, %s", result, arg1, arg2);
                break;
            case INTERMEDIATE_CODE_OP_DIV:
                code_generator_add(generated_code, "sdiv %s, %s, %s", result, arg1, arg2);
                break;
            case INTERMEDIATE_CODE_OP_MOD:
                code_generator_add(generated_code, "sdiv %s, %s, %s", result, arg1, arg2);
                code_generator_add(generated_code, "mul %s, %s, %s", result, result, arg2);
                code_generator_add(generated_code, "sub %s, %s, %s", result, arg1, result);
                break;
            case INTERMEDIATE_CODE_OP_BITWISE_AND:
                code_generator_add(generated_code, "and %s, %s, %s", result, arg1, arg2);
                break;
            case INTERMEDIATE_CODE_OP_BITWISE_OR:
                code_generator_add(generated_code, "orr %s, %s, %s", result, arg1, arg2);
                break;
            case INTERMEDIATE_CODE_OP_BITWISE_XOR:
                code_generator_add(generated_code, "eor %s, %s, %s", result, arg1, arg2);
                break;
            case INTERMEDIATE_CODE_OP_BITWISE_COMPLEMENT:
                code_generator_add(generated_code, "mvn %s, %s", result, arg1);
                break;
            case INTERMEDIATE_CODE_OP_BITWISE_LSHIFT:
                code_generator_add(generated_code, "mov %s, %s, lsl %s", result, arg1, arg2);
                break;
            case INTERMEDIATE_CODE_OP_BITWISE_RSHIFT:
                code_generator_add(generated_code, "mov %s, %s, lsr %s", result, arg1, arg2);
                break;
            case INTERMEDIATE_CODE_OP_LOGICAL_AND:
                code_generator_add(generated_code, "and %s, %s, %s", result, arg1, arg2);
                code_generator_add(generated_code, "cmp %s, #1", result);
                code_generator_add(generated_code, "moveq %s, #1", result);
                code_generator_add(generated_code, "movne %s, #0", result);
                break;
            case INTERMEDIATE_CODE_OP_LOGICAL_OR:
                code_generator_add(generated_code, "orr %s, %s, %s", result, arg1, arg2);
                code_generator_add(generated_code, "cmp %s, #1", result);
                code_generator_add(generated_code, "moveq %s, #1", result);
                code_generator_add(generated_code, "movne %s, #0", result);
                break;
            case INTERMEDIATE_CODE_OP_LOGICAL_LT:
            case INTERMEDIATE_CODE_OP_LOGICAL_LTE:
            case INTERMEDIATE_CODE_OP_LOGICAL_GT:
            case INTERMEDIATE_CODE_OP_LOGICAL_GTE:
            case INTERMEDIATE_CODE_OP_LOGICAL_EQUAL:
            case INTERMEDIATE_CODE_OP_LOGICAL_NOT_EQUAL:
                code_generator_add(generated_code, "cmp %s, %s", arg1, arg2);
                code_generator_add(generated_code, "%s %s, #0", get_mov_logical_instr_negative(tac->op), result);
                code_generator_add(generated_code, "%s %s, #1", get_mov_logical_instr_positive(tac->op), result);
                break;
            case INTERMEDIATE_CODE_OP_ASSIGN:
                code_generator_add(generated_code, "mov %s, %s", result, arg1);
                break;
            case INTERMEDIATE_CODE_OP_MEMORY_ADDRESS:
                break;
            case INTERMEDIATE_CODE_OP_MEMORY_READ:
                code_generator_add(generated_code, "%s %s, [%s, #0]", get_load_instr(tac->result->attrs.type_size), result, arg1);
                break;
            case INTERMEDIATE_CODE_OP_MEMORY_WRITE:
                code_generator_add(generated_code, "%s %s, [%s, #0]", get_store_instr(tac->result->attrs.type_size), arg1, result);
                break;
            case INTERMEDIATE_CODE_OP_FUNCTION:
                params->function_local_size = get_function_size(tac->attrs.local_size);
                params->function_param_size = get_function_size(tac->attrs.parameter_size);
                code_generator_add(generated_code, "%s:", result);
                code_generator_add(generated_code, "str fp, [sp, #%d]", OFFSET_OLD_FRAME_POINTER);
                code_generator_add(generated_code, "str lr, [sp, #%d]", OFFSET_RETURN_ADDRESS);
                code_generator_add(generated_code, "mov fp, sp");
                char *stack_offset_local_arg = get_immediate_value(generated_code, params->function_local_size);
                code_generator_add(generated_code, "sub sp, sp, %s", stack_offset_local_arg);
                free(stack_offset_local_arg);
                break;
            case INTERMEDIATE_CODE_OP_FUNCTION_CALL:
                if (params->is_param_present == 0) {
                    code_generator_add(generated_code, "push {r0-r8}");
                    char *stack_offset_param_arg = get_immediate_value(generated_code, get_function_size(tac->attrs.parameter_size));
                    code_generator_add(generated_code, "sub sp, sp, %s", stack_offset_param_arg);
                    free(stack_offset_param_arg);
                }
                code_generator_add(generated_code, "bl %s", arg1);
                code_generator_add(generated_code, "str r0, [fp, #%d]", OFFSET_RETURN_VALUE);
                code_generator_add(generated_code, "pop {r0-r8}");
                code_generator_add(generated_code, "ldr %s, [fp, #%d]", result, OFFSET_RETURN_VALUE);
                params->is_first_param = 1;
                params->is_param_present = 0;
                break;
            case INTERMEDIATE_CODE_OP_FUNCTION_PARAMETER:;
                if (params->is_first_param == 1) {
                    code_generator_add(generated_code, "push {r0-r8}");
                    char *stack_offset_param_arg = get_immediate_value(generated_code, get_function_size(tac->attrs.parameter_size));
                    code_generator_add(generated_code, "sub sp, sp, %s", stack_offset_param_arg);
                    free(stack_offset_param_arg);
                }
                char *parameter_offset_value = get_immediate_value(generated_code, tac->arg1->attrs.offset);
                code_generator_add(generated_code, "str %s, [sp, %s]", arg2, parameter_offset_value);
                free(parameter_offset_value);
                params->is_first_param = 0;
                params->is_param_present = 1;
                break;
            case INTERMEDIATE_CODE_OP_FUNCTION_RETURN:
                if (arg1 != NULL) {
                    code_generator_add(generated_code, "mov r0, %s", arg1);
                }
                char *stack_offset_arg = get_immediate_value(generated_code, params->function_local_size + params->function_param_size);
                code_generator_add(generated_code, "ldr lr, [fp, #%d]", OFFSET_RETURN_ADDRESS);
                code_generator_add(generated_code, "ldr fp, [fp, #%d]", OFFSET_OLD_FRAME_POINTER);
                code_generator_add(generated_code, "add sp, sp, %s", stack_offset_arg);
                code_generator_add(generated_code, "bx lr");
                free(stack_offset_arg);
                break;
            case INTERMEDIATE_CODE_OP_IF_GOTO:
                code_generator_add(generated_code, "cmp %s, %s",
                    get_arg_value(generated_code, tac->arg1, "r7", 0),
                    get_arg_value(generated_code, tac->arg2, "r8", 0)
                );
                code_generator_add(generated_code, "beq %s", result);
                break;
            case INTERMEDIATE_CODE_OP_GOTO:
                code_generator_add(generated_code, "b %s", arg1);
                break;
            case INTERMEDIATE_CODE_OP_LABEL:
                code_generator_add(generated_code, "%s:", arg1);
                break;
            case INTERMEDIATE_CODE_OP_STRING_READ:
                code_generator_add(generated_code, "ldr %s, =%s", result, arg1);
                break;
        }

        if (result != NULL) {
            free(result);
        }
        if (arg1 != NULL) {
            free(arg1);
        }
        if (arg2 != NULL) {
            free(arg2);
        }
    }
}

void code_generator_template(dynamic_array_t *generated_code) {
    FILE *template_file = fopen("code_generator_template.s", "r");
    if (template_file == NULL) {
        logger_error("Failed to open template file!");
        return;
    }

    char buffer[1024];
    int buffer_index = 0;
    int buff_chr;

    for (buff_chr = fgetc(template_file); buff_chr != EOF; buff_chr = fgetc(template_file)) {
        if (buff_chr != '\n') {
            buffer[buffer_index++] = (char)buff_chr;
        } else {
            buffer[buffer_index] = '\0';
            buffer_index = 0;

            char *instruction = calloc(string_len(buffer) + 1, sizeof(char));
            string_cpy(instruction, buffer);
            code_generator_add(generated_code, instruction);
        }
    }
}

void code_generator_string_literals(dynamic_array_t *generated_code, dynamic_array_t *string_literals) {
    code_generator_add(generated_code, ".section .data");

    int i;
    for (i = 0; i < string_literals->size; i++) {
        ast_string_literal_t *string_literal = dynamic_array_get(string_literals, i);
        char *buffer = calloc(string_len(string_literal->label) + string_len(string_literal->value) + 22, sizeof(char));
        sprintf(buffer, "\t%s: .asciz %s", string_literal->label, string_literal->value);
        code_generator_add(generated_code, buffer);
    }
    code_generator_add(generated_code, "");
}

dynamic_array_t* code_generator_generate(dynamic_array_t *intermediate_code, dynamic_array_t *string_literals) {
    code_generator_params_t params;
    params.intermediate_code = intermediate_code;
    params.is_first_param = 1;
    params.is_param_present = 0;

    dynamic_array_t *generated_code = dynamic_array_new();

    code_generator_string_literals(generated_code, string_literals);
    code_generator_template(generated_code);
    code_generator_generate_internal(generated_code, &params);

    return generated_code;
}

void code_generator_print(dynamic_array_t *generated_code) {
    int i;
    for (i = 0; i < generated_code->size; i++) {
        char *instruction = dynamic_array_get(generated_code, i);
        printf("%s\n", instruction);
    }
}

void code_generator_free_instruction(void *element) {
    char *instruction = element;
    free(instruction);
}

void code_generator_free(dynamic_array_t *generated_code) {
    dynamic_array_free(generated_code, code_generator_free_instruction);
}

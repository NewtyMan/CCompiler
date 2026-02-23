#include <intermediate_code/intermediate_code.h>
#include <intermediate_code/intermediate_code_helpers.h>
#include <intermediate_code/intermediate_code_factory.h>
#include <utils/logger.h>
#include <utils/string.h>
#include <stdlib.h>

/*****************************
 *     UTILITY FUNCTIONS     *
 *****************************/

int intermediate_code_get_character_constant(char *constant) {
    int constant_length = string_len(constant);
    if (constant_length == 1) {
        return constant[0];
    }

    if (constant[0] == '\\') {
        switch (constant[1]) {
            case '\\':
            case '\'':
            case '"':
                return constant[1];
            case 't':
                return 9;
            case 'n':
                return 10;
            case 'v':
                return 11;
            case 'r':
                return 12;
            default:
                break;
        }

        char *num = string_substr(constant, 1, string_len(constant) - 1);
        int result = 0;

        if (constant[1] == 'x' || constant[1] == 'X') {
            // Prefix x or X indicates that we are dealing with a hexadecimal value
            result = string_atoi(num, 16);
        } else {
            // Otherwise if we have no prefix, it's an octal value
            return string_atoi(num, 8);
        }

        free(num);
        return result;
    }

    return 0;
}

void intermediate_code_add(
    intermediate_code_visitor_params_t *params,
    intermediate_code_tac_t *intermediate_code
) {
    if (intermediate_code == NULL) {
        return;
    }
    dynamic_array_add(params->intermediate_code, intermediate_code);
}

intermediate_code_arg_t* intermediate_code_maybe_load_address(
    intermediate_code_arg_t *address,
    intermediate_code_visitor_params_t *params
) {
    if (address->attrs.is_address == 0 || address->attrs.no_dereference == 1) {
        return address;
    }


    intermediate_code_arg_t *return_value = new_tmp_label();
    return_value->attrs.base_size = address->attrs.base_size;
    return_value->attrs.type_size = address->attrs.type_size;
    return_value->attrs.offset = address->attrs.offset;
    intermediate_code_add(params, new_memory_read(return_value, address));

    return return_value;
}

intermediate_code_op_t get_intermediate_code_operator(token_type_t token_type) {
    switch (token_type) {
        case TOKEN_TYPE_OPERATOR_SUM:
        case TOKEN_TYPE_OPERATOR_SUM_ASSIGN:
            return INTERMEDIATE_CODE_OP_ADD;
        case TOKEN_TYPE_OPERATOR_SUB:
        case TOKEN_TYPE_OPERATOR_SUB_ASSIGN:
            return INTERMEDIATE_CODE_OP_SUB;
        case TOKEN_TYPE_OPERATOR_MUL:
        case TOKEN_TYPE_OPERATOR_MUL_ASSIGN:
            return INTERMEDIATE_CODE_OP_MUL;
        case TOKEN_TYPE_OPERATOR_DIV:
        case TOKEN_TYPE_OPERATOR_DIV_ASSIGN:
            return INTERMEDIATE_CODE_OP_DIV;
        case TOKEN_TYPE_OPERATOR_MOD:
        case TOKEN_TYPE_OPERATOR_MOD_ASSIGN:
            return INTERMEDIATE_CODE_OP_MOD;
        case TOKEN_TYPE_OPERATOR_BIT_AND:
        case TOKEN_TYPE_OPERATOR_BIT_AND_ASSIGN:
            return INTERMEDIATE_CODE_OP_BITWISE_AND;
        case TOKEN_TYPE_OPERATOR_BIT_OR:
        case TOKEN_TYPE_OPERATOR_BIT_OR_ASSIGN:
            return INTERMEDIATE_CODE_OP_BITWISE_OR;
        case TOKEN_TYPE_OPERATOR_XOR:
        case TOKEN_TYPE_OPERATOR_XOR_ASSIGN:
            return INTERMEDIATE_CODE_OP_BITWISE_XOR;
        case TOKEN_TYPE_OPERATOR_BIT_LSHIFT:
        case TOKEN_TYPE_OPERATOR_BIT_LSHIFT_ASSIGN:
            return INTERMEDIATE_CODE_OP_BITWISE_LSHIFT;
        case TOKEN_TYPE_OPERATOR_BIT_RSHIFT:
        case TOKEN_TYPE_OPERATOR_BIT_RSHIFT_ASSIGN:
            return INTERMEDIATE_CODE_OP_BITWISE_RSHIFT;
        case TOKEN_TYPE_OPERATOR_LOG_AND:
            return INTERMEDIATE_CODE_OP_LOGICAL_AND;
        case TOKEN_TYPE_OPERATOR_LOG_OR:
            return INTERMEDIATE_CODE_OP_LOGICAL_OR;
        case TOKEN_TYPE_OPERATOR_LT:
            return INTERMEDIATE_CODE_OP_LOGICAL_LT;
        case TOKEN_TYPE_OPERATOR_LTE:
            return INTERMEDIATE_CODE_OP_LOGICAL_LTE;
        case TOKEN_TYPE_OPERATOR_GT:
            return INTERMEDIATE_CODE_OP_LOGICAL_GT;
        case TOKEN_TYPE_OPERATOR_GTE:
            return INTERMEDIATE_CODE_OP_LOGICAL_GTE;
        case TOKEN_TYPE_OPERATOR_EQUAL:
            return INTERMEDIATE_CODE_OP_LOGICAL_EQUAL;
        case TOKEN_TYPE_OPERATOR_NOT_EQUAL:
            return INTERMEDIATE_CODE_OP_LOGICAL_NOT_EQUAL;
        case TOKEN_TYPE_OPERATOR_ASSIGN:
            return INTERMEDIATE_CODE_OP_ASSIGN;
        default:
            return INTERMEDIATE_CODE_OP_NONE;
    }
}

int is_assignable_arithmetic_operator(token_type_t token_type) {
    switch (token_type) {
        case TOKEN_TYPE_OPERATOR_INC:
        case TOKEN_TYPE_OPERATOR_SUM_ASSIGN:
        case TOKEN_TYPE_OPERATOR_SUB_ASSIGN:
        case TOKEN_TYPE_OPERATOR_DEC:
        case TOKEN_TYPE_OPERATOR_BIT_AND_ASSIGN:
        case TOKEN_TYPE_OPERATOR_BIT_OR_ASSIGN:
        case TOKEN_TYPE_OPERATOR_MUL_ASSIGN:
        case TOKEN_TYPE_OPERATOR_DIV_ASSIGN:
        case TOKEN_TYPE_OPERATOR_MOD_ASSIGN:
        case TOKEN_TYPE_OPERATOR_XOR_ASSIGN:
        case TOKEN_TYPE_OPERATOR_BIT_RSHIFT_ASSIGN:
        case TOKEN_TYPE_OPERATOR_BIT_LSHIFT_ASSIGN:
            return 1;
        default:
            return 0;
    }
}

/****************************
 *         PRINTING         *
 ****************************/

char* intermediate_code_get_op_string(intermediate_code_op_t operator) {
    switch (operator) {
        case INTERMEDIATE_CODE_OP_ADD:
            return "+";
        case INTERMEDIATE_CODE_OP_SUB:
            return "-";
        case INTERMEDIATE_CODE_OP_MUL:
            return "*";
        case INTERMEDIATE_CODE_OP_DIV:
            return "/";
        case INTERMEDIATE_CODE_OP_MOD:
            return "%";
        case INTERMEDIATE_CODE_OP_BITWISE_AND:
            return "&";
        case INTERMEDIATE_CODE_OP_BITWISE_OR:
            return "|";
        case INTERMEDIATE_CODE_OP_BITWISE_XOR:
            return "^";
        case INTERMEDIATE_CODE_OP_BITWISE_COMPLEMENT:
            return "~";
        case INTERMEDIATE_CODE_OP_BITWISE_LSHIFT:
            return "<<";
        case INTERMEDIATE_CODE_OP_BITWISE_RSHIFT:
            return ">>";
        case INTERMEDIATE_CODE_OP_LOGICAL_AND:
            return "&&";
        case INTERMEDIATE_CODE_OP_LOGICAL_OR:
            return "||";
        case INTERMEDIATE_CODE_OP_LOGICAL_LT:
            return "<";
        case INTERMEDIATE_CODE_OP_LOGICAL_LTE:
            return "<=";
        case INTERMEDIATE_CODE_OP_LOGICAL_GT:
            return ">";
        case INTERMEDIATE_CODE_OP_LOGICAL_GTE:
            return ">=";
        case INTERMEDIATE_CODE_OP_LOGICAL_EQUAL:
            return "==";
        case INTERMEDIATE_CODE_OP_LOGICAL_NOT_EQUAL:
            return "!=";
        default:
            logger_error("Unknown operator in string conversion: %d", operator);
            return "";
    }
}

void intermediate_code_print_tac(intermediate_code_tac_t *intermediate_code) {
    switch (intermediate_code->op) {
        case INTERMEDIATE_CODE_OP_NONE:
            break;
        case INTERMEDIATE_CODE_OP_ADD:
        case INTERMEDIATE_CODE_OP_SUB:
        case INTERMEDIATE_CODE_OP_MUL:
        case INTERMEDIATE_CODE_OP_DIV:
        case INTERMEDIATE_CODE_OP_MOD:
        case INTERMEDIATE_CODE_OP_BITWISE_AND:
        case INTERMEDIATE_CODE_OP_BITWISE_OR:
        case INTERMEDIATE_CODE_OP_BITWISE_XOR:
        case INTERMEDIATE_CODE_OP_BITWISE_COMPLEMENT:
        case INTERMEDIATE_CODE_OP_BITWISE_LSHIFT:
        case INTERMEDIATE_CODE_OP_BITWISE_RSHIFT:
        case INTERMEDIATE_CODE_OP_LOGICAL_AND:
        case INTERMEDIATE_CODE_OP_LOGICAL_OR:
        case INTERMEDIATE_CODE_OP_LOGICAL_LT:
        case INTERMEDIATE_CODE_OP_LOGICAL_LTE:
        case INTERMEDIATE_CODE_OP_LOGICAL_GT:
        case INTERMEDIATE_CODE_OP_LOGICAL_GTE:
        case INTERMEDIATE_CODE_OP_LOGICAL_EQUAL:
        case INTERMEDIATE_CODE_OP_LOGICAL_NOT_EQUAL:
            printf("%s = %s %s %s\n",
                intermediate_code->result->string_value,
                intermediate_code->arg1->string_value,
                intermediate_code_get_op_string(intermediate_code->op),
                intermediate_code->arg2->string_value
            );
            break;
        case INTERMEDIATE_CODE_OP_ASSIGN:
            printf("%s = %s\n",
                intermediate_code->result->string_value,
                intermediate_code->arg1->string_value
            );
            break;
        case INTERMEDIATE_CODE_OP_MEMORY_ADDRESS:
            printf("%s = address %s\n",
                intermediate_code->result->string_value,
                intermediate_code->arg1->string_value
            );
            break;
        case INTERMEDIATE_CODE_OP_MEMORY_READ:
            printf("%s = read %s {size=%d}\n",
                intermediate_code->result->string_value,
                intermediate_code->arg1->string_value,
                intermediate_code->arg1->attrs.type_size
            );
            break;
        case INTERMEDIATE_CODE_OP_MEMORY_WRITE:
            printf("write %s, %s {size=%d}\n",
                intermediate_code->result->string_value,
                intermediate_code->arg1->string_value,
                intermediate_code->result->attrs.type_size
            );
            break;
        case INTERMEDIATE_CODE_OP_FUNCTION:
            printf("function %s\n", intermediate_code->result->string_value);
            break;
        case INTERMEDIATE_CODE_OP_FUNCTION_CALL:
            printf("%s = call %s {params=%d}\n",
                intermediate_code->result->string_value, intermediate_code->arg1->string_value,
                intermediate_code->attrs.parameter_size
            );
            break;
        case INTERMEDIATE_CODE_OP_FUNCTION_PARAMETER:
            printf("parameter %s, %s, %s\n",
                intermediate_code->result->string_value,
                intermediate_code->arg1->string_value,
                intermediate_code->arg2->string_value
            );
            break;
        case INTERMEDIATE_CODE_OP_FUNCTION_RETURN:
            printf("return");
            if (intermediate_code->arg1 != NULL) {
                printf(" %s", intermediate_code->arg1->string_value);
            }
            printf("\n");
            break;
        case INTERMEDIATE_CODE_OP_IF_GOTO:
            printf("if %s == %s goto %s\n",
                intermediate_code->arg1->string_value,
                intermediate_code->arg2->string_value,
                intermediate_code->result->string_value
            );
            break;
        case INTERMEDIATE_CODE_OP_GOTO:
            printf("goto %s\n", intermediate_code->arg1->string_value);
            break;
        case INTERMEDIATE_CODE_OP_LABEL:
            printf("%s:\n", intermediate_code->arg1->string_value);
            break;
        case INTERMEDIATE_CODE_OP_STRING_READ:
            printf("%s = read_string %s\n", intermediate_code->result->string_value, intermediate_code->arg2->string_value);
            break;
    }
}

void intermediate_code_print(dynamic_array_t *array) {
    int i;
    for (i = 0; i < array->size; i++) {
        intermediate_code_tac_t *intermediate_code = dynamic_array_get(array, i);
        intermediate_code_print_tac(intermediate_code);
    }
}

/****************************
 *          FREEING         *
 ****************************/

void intermediate_code_arg_free(intermediate_code_arg_t *element) {
    if (element != NULL) {
        if (element->string_value != NULL) {
            free(element->string_value);
        }
        free(element);
    }
}

void intermediate_code_tac_free(void *element) {
    intermediate_code_tac_t *intermediate_code = element;
    intermediate_code_arg_free(intermediate_code->result);
    intermediate_code_arg_free(intermediate_code->arg1);
    intermediate_code_arg_free(intermediate_code->arg2);
    free(intermediate_code);
}

void intermediate_code_free(dynamic_array_t *intermediate_code) {
    dynamic_array_free(intermediate_code, intermediate_code_tac_free);
}

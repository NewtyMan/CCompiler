#include <lexical_analyzer/lexical_analyzer.h>
#include <lexical_analyzer/state_machine.h>

state_machine_params_t state_machine_params_new() {
    state_machine_params_t params;
    params.numerical_constant_index = 1;
    params.comment_index = 1;
    params.string_literal_index = 1;
    params.character_literal_index = 1;
    params.identifier_index = 1;
    params.operator_index = 1;
    params.punctuator_index = 1;
    params.whitespace_index = 1;
    params.preprocessor_directive_index = 1;
    return params;
}

token_type_t state_machine_run_numerical_constants(state_machine_params_t *params, char input) {
    switch (params->numerical_constant_index) {
        case 1:
            if (input == '0') {
                params->numerical_constant_index = 2;
            } else if (input >= '1' && input <= '9') {
                params->numerical_constant_index = 6;
            } else {
                params->numerical_constant_index = 0;
                return TOKEN_TYPE_NONE;
            }
            break;
        case 2:
            if (input == 'x' || input == 'X') {
                params->numerical_constant_index = 3;
            } else if (input >= '0' && input <= '7') {
                params->numerical_constant_index = 5;
            } else if (input == '.') {
                params->numerical_constant_index = 7;
            } else {
                params->numerical_constant_index = 0;
                return TOKEN_TYPE_NUMBER_OCT;
            }
            break;
        case 3:
            if ((input >= '0' && input <= '9') || (input >= 'a' && input <= 'f') || (input >= 'A' && input <= 'F')) {
                params->numerical_constant_index = 4;
            } else {
                params->numerical_constant_index = 0;
                return TOKEN_TYPE_NONE;
            }
            break;
        case 4:
            if ((input >= '0' && input <= '9') || (input >= 'a' && input <= 'f') || (input >= 'A' && input <= 'F')) {
                params->numerical_constant_index = 4;
            } else {
                params->numerical_constant_index = 0;
                return TOKEN_TYPE_NUMBER_HEX;
            }
            break;
        case 5:
            if (input >= '0' && input <= '7') {
                params->numerical_constant_index = 5;
            } else {
                params->numerical_constant_index = 0;
                return TOKEN_TYPE_NUMBER_OCT;
            }
            break;
        case 6:
            if (input >= '0' && input <= '9') {
                params->numerical_constant_index = 6;
            } else if (input == '.') {
                params->numerical_constant_index = 7;
            } else {
                params->numerical_constant_index = 0;
                return TOKEN_TYPE_NUMBER_DEC;
            }
            break;
        case 7:
            if (input >= '0' && input <= '9') {
                params->numerical_constant_index = 8;
            } else {
                params->numerical_constant_index = 0;
                return TOKEN_TYPE_NONE;
            }
            break;
        case 8:
            if (input >= '0' && input <= '9') {
                params->numerical_constant_index = 8;
            } else {
                params->numerical_constant_index = 0;
                return TOKEN_TYPE_NUMBER_FLOAT;
            }
            break;
        default:
            return TOKEN_TYPE_NONE;
    }
    return TOKEN_TYPE_UNKNOWN;
}

token_type_t state_machine_run_comment(state_machine_params_t *params, char input) {
    switch (params->comment_index) {
        case 1:
            if (input == '/') {
                params->comment_index = 2;
            } else {
                params->comment_index = 0;
                return TOKEN_TYPE_NONE;
            }
            break;
        case 2:
            if (input == '*') {
                params->comment_index = 3;
            } else if (input == '/') {
                params->comment_index = 5;
            } else {
                params->comment_index = 0;
                return TOKEN_TYPE_NONE;
            }
            break;
        case 3:
            if (input != '*') {
                params->comment_index = 3;
            } else {
                params->comment_index = 4;
            }
            break;
        case 4:
            if (input == '*') {
                params->comment_index = 4;
            } else if (input != '/') {
                params->comment_index = 3;
            } else {
                params->comment_index = 6;
            }
            break;
        case 5:
            if (input != '\n') {
                params->comment_index = 5;
            } else {
                params->comment_index = 6;
            }
            break;
        case 6:
            params->comment_index = 0;
            return TOKEN_TYPE_COMMENT;
        default:
            return TOKEN_TYPE_NONE;
    }
    return TOKEN_TYPE_UNKNOWN;
}

token_type_t state_machine_run_string_literal(state_machine_params_t *params, char input) {
    switch (params->string_literal_index) {
        case 1:
            if (input == '"') {
                params->string_literal_index = 2;
            } else {
                params->string_literal_index = 0;
                return TOKEN_TYPE_NONE;
            }
            break;
        case 2:
            if (input != '"' && input != '\n' && input != '\\') {
                params->string_literal_index = 3;
            } else if (input == '\\') {
                params->string_literal_index = 4;
            } else if (input == '"') {
                params->string_literal_index = 5;
            } else {
                params->string_literal_index = 0;
                return TOKEN_TYPE_NONE;
            }
            break;
        case 3:
            if (input != '"' && input != '\n' && input != '\\') {
                params->string_literal_index = 3;
            } else if (input == '\\') {
                params->string_literal_index = 4;
            } else if (input == '"') {
                params->string_literal_index = 5;
            } else if (input == '\n') {
                params->string_literal_index = 0;
                return TOKEN_TYPE_NONE;
            }
            break;
        case 4:
            if (input != '\n') {
                params->string_literal_index = 3;
            } else {
                params->string_literal_index = 0;
                return TOKEN_TYPE_NONE;
            }
            break;
        case 5:
            params->string_literal_index = 0;
            return TOKEN_TYPE_STRING_LITERAL;
        default:
            return TOKEN_TYPE_NONE;
    }
    return TOKEN_TYPE_UNKNOWN;
}

token_type_t state_machine_run_character_literal(state_machine_params_t *params, char input) {
    switch (params->character_literal_index) {
        case 1:
            if (input == '\'') {
                params->character_literal_index = 2;
            } else {
                params->character_literal_index = 0;
                return TOKEN_TYPE_NONE;
            }
            break;
        case 2:
            if (input != '\'' && input != '\n' && input != '\\') {
                params->character_literal_index = 3;
            } else if (input == '\\') {
                params->character_literal_index = 4;
            } else {
                params->character_literal_index = 0;
                return TOKEN_TYPE_NONE;
            }
            break;
        case 3:
            if (input != '\'' && input != '\n' && input != '\\') {
                params->character_literal_index = 3;
            } else if (input == '\\') {
                params->character_literal_index = 4;
            } else if (input == '\'') {
                params->character_literal_index = 5;
            } else if (input == '\n') {
                params->character_literal_index = 0;
                return TOKEN_TYPE_NONE;
            }
            break;
        case 4:
            if (input != '\n') {
                params->character_literal_index = 3;
            } else {
                params->character_literal_index = 0;
                return TOKEN_TYPE_NONE;
            }
            break;
        case 5:
            params->character_literal_index = 0;
            return TOKEN_TYPE_CHARACTER_LITERAL;
        default:
            return TOKEN_TYPE_NONE;
    }
    return TOKEN_TYPE_UNKNOWN;
}

token_type_t state_machine_run_identifier(state_machine_params_t *params, char input) {
    switch (params->identifier_index) {
        case 1:
            if ((input >= 'a' && input <= 'z') || (input >= 'A' && input <= 'Z') || input == '_') {
                params->identifier_index = 2;
            } else {
                params->identifier_index = 0;
                return TOKEN_TYPE_NONE;
            }
            break;
        case 2:
            if ((input >= 'a' && input <= 'z') || (input >= 'A' && input <= 'Z') || (input >= '0' && input <= '9') || input == '_') {
                params->identifier_index = 2;
            } else {
                params->identifier_index = 0;
                return TOKEN_TYPE_IDENTIFIER;
            }
            break;
        default:
            return TOKEN_TYPE_NONE;
    }
    return TOKEN_TYPE_UNKNOWN;
}

int state_machine_run_operator_root(char input) {
    switch (input) {
        case '+':
            return 2;
        case '-':
            return 5;
        case '&':
            return 8;
        case '|':
            return 11;
        case '~':
            return 14;
        case '>':
            return 15;
        case '<':
            return 19;
        case '*':
            return 23;
        case '%':
            return 25;
        case '^':
            return 27;
        case '/':
            return 29;
        case '=':
            return 31;
        case '!':
            return 33;
        default:
            return 0;
    }
}

token_type_t state_machine_run_operator(state_machine_params_t *params, char input) {
    switch (params->operator_index) {
        case 1:
            params->operator_index = state_machine_run_operator_root(input);
            if (params->operator_index == 0) {
                return TOKEN_TYPE_NONE;
            }
            break;
        case 2:
            if (input == '+') {
                params->operator_index = 3;
                break;
            }
            if (input == '=') {
                params->operator_index = 4;
                break;
            }
            params->operator_index = 0;
            return TOKEN_TYPE_OPERATOR_SUM;
        case 3:
            params->operator_index = 0;
            return TOKEN_TYPE_OPERATOR_INC;
        case 4:
            params->operator_index = 0;
            return TOKEN_TYPE_OPERATOR_SUM_ASSIGN;
        case 5:
            if (input == '-') {
                params->operator_index = 6;
                break;
            }
            if (input == '=') {
                params->operator_index = 7;
                break;
            }
            params->operator_index = 0;
            return TOKEN_TYPE_OPERATOR_SUB;
        case 6:
            params->operator_index = 0;
            return TOKEN_TYPE_OPERATOR_DEC;
        case 7:
            params->operator_index = 0;
            return TOKEN_TYPE_OPERATOR_SUB_ASSIGN;
        case 8:
            if (input == '&') {
                params->operator_index = 9;
                break;
            }
            if (input == '=') {
                params->operator_index = 10;
                break;
            }
            params->operator_index = 0;
            return TOKEN_TYPE_OPERATOR_BIT_AND;
        case 9:
            params->operator_index = 0;
            return TOKEN_TYPE_OPERATOR_LOG_AND;
        case 10:
            params->operator_index = 0;
            return TOKEN_TYPE_OPERATOR_BIT_AND_ASSIGN;
        case 11:
            if (input == '|') {
                params->operator_index = 12;
                break;
            }
            if (input == '=') {
                params->operator_index = 13;
                break;
            }
            params->operator_index = 0;
            return TOKEN_TYPE_OPERATOR_BIT_OR;
        case 12:
            params->operator_index = 0;
            return TOKEN_TYPE_OPERATOR_LOG_OR;
        case 13:
            params->operator_index = 0;
            return TOKEN_TYPE_OPERATOR_BIT_OR_ASSIGN;
        case 14:
            params->operator_index = 0;
            return TOKEN_TYPE_OPERATOR_TILDE;
        case 15:
            if (input == '>') {
                params->operator_index = 16;
                break;
            }
            if (input == '=') {
                params->operator_index = 17;
                break;
            }
            params->operator_index = 0;
            return TOKEN_TYPE_OPERATOR_GT;
        case 16:
            if (input == '=') {
                params->operator_index = 18;
                break;
            }
            params->operator_index = 0;
            return TOKEN_TYPE_OPERATOR_BIT_RSHIFT;
        case 17:
            params->operator_index = 0;
            return TOKEN_TYPE_OPERATOR_GTE;
        case 18:
            params->operator_index = 0;
            return TOKEN_TYPE_OPERATOR_BIT_RSHIFT_ASSIGN;
        case 19:
            if (input == '<') {
                params->operator_index = 20;
                break;
            }
            if (input == '=') {
                params->operator_index = 21;
                break;
            }
            params->operator_index = 0;
            return TOKEN_TYPE_OPERATOR_LT;
        case 20:
            if (input == '=') {
                params->operator_index = 22;
                break;
            }
            params->operator_index = 0;
            return TOKEN_TYPE_OPERATOR_BIT_LSHIFT;
        case 21:
            params->operator_index = 0;
            return TOKEN_TYPE_OPERATOR_LTE;
        case 22:
            params->operator_index = 0;
            return TOKEN_TYPE_OPERATOR_BIT_LSHIFT_ASSIGN;
        case 23:
            if (input == '=') {
                params->operator_index = 24;
                break;
            }
            params->operator_index = 0;
            return TOKEN_TYPE_OPERATOR_MUL;
        case 24:
            params->operator_index = 0;
            return TOKEN_TYPE_OPERATOR_MUL_ASSIGN;
        case 25:
            if (input == '=') {
                params->operator_index = 26;
                break;
            }
            params->operator_index = 0;
            return TOKEN_TYPE_OPERATOR_MOD;
        case 26:
            params->operator_index = 0;
            return TOKEN_TYPE_OPERATOR_MOD_ASSIGN;
        case 27:
            if (input == '=') {
                params->operator_index = 28;
                break;
            }
            params->operator_index = 0;
            return TOKEN_TYPE_OPERATOR_XOR;
        case 28:
            params->operator_index = 0;
            return TOKEN_TYPE_OPERATOR_XOR_ASSIGN;
        case 29:
            if (input == '=') {
                params->operator_index = 30;
                break;
            }
            params->operator_index = 0;
            return TOKEN_TYPE_OPERATOR_DIV;
        case 30:
            params->operator_index = 0;
            return TOKEN_TYPE_OPERATOR_DIV_ASSIGN;
        case 31:
            if (input == '=') {
                params->operator_index = 32;
                break;
            }
            params->operator_index = 0;
            return TOKEN_TYPE_OPERATOR_ASSIGN;
        case 32:
            params->operator_index = 0;
            return TOKEN_TYPE_OPERATOR_EQUAL;
        case 33:
            if (input == '=') {
                params->operator_index = 34;
                break;
            }
            params->operator_index = 0;
            return TOKEN_TYPE_OPERATOR_NOT;
        case 34:
            params->operator_index = 0;
            return TOKEN_TYPE_OPERATOR_NOT_EQUAL;
        default:
            return TOKEN_TYPE_NONE;

    }
    return TOKEN_TYPE_UNKNOWN;
}

int state_machine_run_punctuator_root(char input) {
    switch (input) {
        case '(':
            return 2;
        case ')':
            return 3;
        case '[':
            return 4;
        case ']':
            return 5;
        case '{':
            return 6;
        case '}':
            return 7;
        case ',':
            return 8;
        case '.':
            return 9;
        case '?':
            return 10;
        case ':':
            return 11;
        case ';':
            return 12;
        default:
            return 0;
    }
}

token_type_t state_machine_run_punctuator(state_machine_params_t *params, char input) {
    switch (params->punctuator_index) {
        case 1:
            params->punctuator_index = state_machine_run_punctuator_root(input);
            if (params->punctuator_index == 0) {
                return TOKEN_TYPE_NONE;
            }
            break;
        case 2:
            params->punctuator_index = 0;
            return TOKEN_TYPE_PUNCTUATOR_LPAREN;
        case 3:
            params->punctuator_index = 0;
            return TOKEN_TYPE_PUNCTUATOR_RPAREN;
        case 4:
            params->punctuator_index = 0;
            return TOKEN_TYPE_PUNCTUATOR_LBRACKET;
        case 5:
            params->punctuator_index = 0;
            return TOKEN_TYPE_PUNCTUATOR_RBRACKET;
        case 6:
            params->punctuator_index = 0;
            return TOKEN_TYPE_PUNCTUATOR_LBRACE;
        case 7:
            params->punctuator_index = 0;
            return TOKEN_TYPE_PUNCTUATOR_RBRACE;
        case 8:
            params->punctuator_index = 0;
            return TOKEN_TYPE_PUNCTUATOR_COMMA;
        case 9:
            params->punctuator_index = 0;
            return TOKEN_TYPE_PUNCTUATOR_DOT;
        case 10:
            params->punctuator_index = 0;
            return TOKEN_TYPE_PUNCTUATOR_QUESTION;
        case 11:
            params->punctuator_index = 0;
            return TOKEN_TYPE_PUNCTUATOR_COLON;
        case 12:
            params->punctuator_index = 0;
            return TOKEN_TYPE_PUNCTUATOR_SEMICOLON;
        default:
            return TOKEN_TYPE_NONE;
    }
    return TOKEN_TYPE_UNKNOWN;
}

token_type_t state_machine_run_whitespace(state_machine_params_t *params, char input) {
    switch (params->whitespace_index) {
        case 1:
            if (input == ' ' || input == '\t' || input == '\n' || input == '\r') {
                params->whitespace_index = 2;
                break;
            }
            params->whitespace_index = 0;
            return TOKEN_TYPE_NONE;
        case 2:
            if (input == ' ' || input == '\t' || input == '\n' || input == '\r') {
                params->whitespace_index = 2;
                break;
            }
            params->whitespace_index = 0;
            return TOKEN_TYPE_WHITESPACE;
        default:
            return TOKEN_TYPE_NONE;
    }
    return TOKEN_TYPE_UNKNOWN;
}

token_type_t state_machine_run_preprocessor_directive(state_machine_params_t *params, char input) {
    switch (params->preprocessor_directive_index) {
        case 1:
            if (input == '#') {
                params->preprocessor_directive_index = 2;
                break;
            }
            params->preprocessor_directive_index = 0;
            return TOKEN_TYPE_NONE;
        case 2:
            if (input != '\n') {
                params->preprocessor_directive_index = 2;
                break;
            }
            params->preprocessor_directive_index = 0;
            return TOKEN_TYPE_PREPROCESSOR_DIRECTIVE;
        default:
            return TOKEN_TYPE_NONE;
    }
    return TOKEN_TYPE_UNKNOWN;
}

void state_machine_setup(dynamic_array_t *state_machines) {
    dynamic_array_add(state_machines, state_machine_run_comment);
    dynamic_array_add(state_machines, state_machine_run_numerical_constants);
    dynamic_array_add(state_machines, state_machine_run_character_literal);
    dynamic_array_add(state_machines, state_machine_run_string_literal);
    dynamic_array_add(state_machines, state_machine_run_identifier);
    dynamic_array_add(state_machines, state_machine_run_operator);
    dynamic_array_add(state_machines, state_machine_run_punctuator);
    dynamic_array_add(state_machines, state_machine_run_whitespace);
    dynamic_array_add(state_machines, state_machine_run_preprocessor_directive);
}

token_type_t state_machine_run(state_machine_params_t *params, char input) {
    token_type_t token_type = TOKEN_TYPE_NONE;
    int is_token_unknown = 0;

    // If we add any new state machine, it should also be added to this dynamic array, so that it gets processed
    dynamic_array_t *state_machines = dynamic_array_new();
    state_machine_setup(state_machines);

    int i;
    for (i = 0; i < state_machines->size; i++) {
        const state_machine_func sm_func = dynamic_array_get(state_machines, i);
        const token_type_t sm_token_type = sm_func(params, input);
        if (sm_token_type == TOKEN_TYPE_UNKNOWN) {
            is_token_unknown = 1;
        } else if (sm_token_type != TOKEN_TYPE_NONE) {
            token_type = sm_token_type;
        }
    }

    // If at least one of state machines is still ongoing, it means we can develop a larger token, thus we want to
    // continue running the state machine for this input.
    if (is_token_unknown == 1) {
        return TOKEN_TYPE_UNKNOWN;
    }

    return token_type;
}

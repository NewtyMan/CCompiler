#include <intermediate_code/intermediate_code.h>
#include <intermediate_code/intermediate_code_factory.h>
#include <intermediate_code/intermediate_code_helpers.h>
#include <syntax_analyzer/syntax_analyzer_visitor.h>
#include <utils/logger.h>
#include <utils/string.h>
#include <stdlib.h>

dynamic_array_t *reference_pool;

int intermediate_code_callback(ast_type_t type, void *element, void *params);

/*****************************
 *     VISITOR FUNCTIONS     *
 *****************************/

void intermediate_code_parse_array_access(ast_array_access_t *ast_array_access, intermediate_code_visitor_params_t *params) {
    /*
     *  t1 = expression_label * 4
     *  t2 = array_label + t1
     *  <return t2>
     */

    syntax_analyzer_visit(intermediate_code_callback, ast_array_access->expression, params);
    intermediate_code_arg_t *expression_label = params->return_value;
    expression_label = intermediate_code_maybe_load_address(expression_label, params);

    syntax_analyzer_visit(intermediate_code_callback, ast_array_access->array, params);
    intermediate_code_arg_t *array_label = params->return_value;
    array_label = intermediate_code_maybe_load_address(array_label, params);
    int array_label_size = array_label->attrs.base_size;

    intermediate_code_arg_t *size_offset = expression_label;
    if (ast_array_access->expression->type != AST_TYPE_CONSTANT && array_label_size != 1) {
        // OPTIMIZATION: Only calculate/multiply the index offset if size of the element is larger than one
        size_offset = new_tmp_label();
        intermediate_code_add(params, new_binary(
            INTERMEDIATE_CODE_OP_MUL,
            size_offset,
            expression_label,
            new_i32_const(array_label_size)
        ));
    } else if (ast_array_access->expression->type == AST_TYPE_CONSTANT) {
        // OPTIMIZATION: If array expression is a constant, calculate index offset in compile time
        ast_constant_t *expression_constant = ast_array_access->expression->value;
        int expression_constant_value = 0;
        switch (expression_constant->type) {
            case TOKEN_TYPE_NUMBER_DEC:
                expression_constant_value = string_atoi(expression_constant->constant, 10);
                break;
            case TOKEN_TYPE_NUMBER_HEX:
                expression_constant_value = string_atoi(expression_constant->constant, 8);
                break;
            case TOKEN_TYPE_NUMBER_OCT:
                expression_constant_value = string_atoi(expression_constant->constant, 16);
                break;
            default:
                break;
        }
        size_offset = new_i32_const(expression_constant_value * array_label_size);
    }

    intermediate_code_arg_t *element_offset = new_tmp_label();
    intermediate_code_add(params, new_binary(
        INTERMEDIATE_CODE_OP_ADD,
        element_offset,
        array_label,
        size_offset
    ));

    params->return_value = new_address(element_offset);
    params->return_value->attrs.base_size = array_label->attrs.base_size;
    params->return_value->attrs.type_size = array_label->attrs.base_size;
}

void intermediate_code_parse_binary(ast_binary_t *ast_binary, intermediate_code_visitor_params_t *params) {
    syntax_analyzer_visit(intermediate_code_callback, ast_binary->right, params);
    intermediate_code_arg_t *binary_right_label = params->return_value;
    binary_right_label = intermediate_code_maybe_load_address(binary_right_label, params);

    params->parse_type = is_assignable_arithmetic_operator(ast_binary->operator) == 1 || ast_binary->operator == TOKEN_TYPE_OPERATOR_ASSIGN ?
        INTERMEDIATE_CODE_PARSE_TYPE_LHS :
        INTERMEDIATE_CODE_PARSE_TYPE_RHS;
    syntax_analyzer_visit(intermediate_code_callback, ast_binary->left, params);
    intermediate_code_arg_t *binary_left_label = params->return_value;
    if (is_assignable_arithmetic_operator(ast_binary->operator) == 0 && ast_binary->operator != TOKEN_TYPE_OPERATOR_ASSIGN) {
        binary_left_label = intermediate_code_maybe_load_address(binary_left_label, params);
    }
    params->parse_type = INTERMEDIATE_CODE_PARSE_TYPE_RHS;

    if (is_assignable_arithmetic_operator(ast_binary->operator) == 1) {
        // In assignable operators, we must first load the value and then perform the appropriate operation with the
        // loaded right hand side value. After that, we store the result.
        intermediate_code_arg_t *binary_left_value = new_tmp_label();
        intermediate_code_add(params, new_memory_read(
            binary_left_value,
            binary_left_label
        ));

        intermediate_code_arg_t *calculated_value = new_tmp_label();
        intermediate_code_add(params, new_binary(
            get_intermediate_code_operator(ast_binary->operator),
            calculated_value,
            binary_left_value,
            binary_right_label
        ));

        intermediate_code_add(params, new_memory_write(
            binary_left_label,
            calculated_value
        ));
    } else if (ast_binary->operator == TOKEN_TYPE_OPERATOR_ASSIGN) {
        // In assign operator, we just assign right hand side of the expression to the left hand side
        intermediate_code_add(params, new_memory_write(
            binary_left_label,
            binary_right_label
        ));
    } else {
        // For all other non-assignable binary operators, we just calculate the value and return it in a new temporary
        // label.
        params->return_value = new_tmp_label();
        intermediate_code_add(params, new_binary(
            get_intermediate_code_operator(ast_binary->operator),
            params->return_value,
            binary_left_label,
            binary_right_label
        ));
    }
}

void intermediate_code_parse_break(intermediate_code_visitor_params_t *params) {
    intermediate_code_add(params, new_goto(params->break_label));
}

void intermediate_code_parse_case(ast_case_t *ast_case, intermediate_code_visitor_params_t *params) {
    /*
     *  if expression_arg_label == switch_expression_label goto L1
     *  goto L2
     *  L1:
     *  <case_statement>
     *  goto switch_end_label
     *  L2:
     */

    intermediate_code_arg_t *case_body_label = new_control_label();
    intermediate_code_arg_t *case_end_label = new_control_label();

    // Since there can be multiple subsequent case statements, we want to handle that use-case separately
    while (1) {
        // Case expression
        syntax_analyzer_visit(intermediate_code_callback, ast_case->expression, params);
        intermediate_code_arg_t *expression_arg_label = params->return_value;

        intermediate_code_add(params, new_if_goto(
            params->switch_expression_label,
            expression_arg_label,
            case_body_label
        ));

        if (ast_case->statement->type == AST_TYPE_CASE) {
            ast_case = ast_case->statement->value;
        } else {
            intermediate_code_add(params, new_goto(case_end_label));
            break;
        }
    }

    intermediate_code_add(params, new_label(case_body_label));
    syntax_analyzer_visit(intermediate_code_callback, ast_case->statement, params);
    intermediate_code_add(params, new_label(case_end_label));
}

void intermediate_code_parse_compound_statement(ast_compound_statement_t *ast_compound_statement, intermediate_code_visitor_params_t *params) {
    symbol_table_symbol_t *compound_symbol = symbol_table_find_node(params->symbol_table, ast_compound_statement);

    params->symbol_table = compound_symbol;
    syntax_analyzer_visit(intermediate_code_callback, ast_compound_statement->declarations, params);
    syntax_analyzer_visit(intermediate_code_callback, ast_compound_statement->statements, params);
    params->symbol_table = compound_symbol->parent;
}

void intermediate_code_parse_constant(ast_constant_t *ast_constant, intermediate_code_visitor_params_t *params) {
    switch (ast_constant->type) {
        case TOKEN_TYPE_CHARACTER_LITERAL:
            params->return_value = new_i8_const(intermediate_code_get_character_constant(ast_constant->constant));
            break;
        case TOKEN_TYPE_NUMBER_DEC:
            params->return_value = new_i32_const(string_atoi(ast_constant->constant, 10));
            break;
        case TOKEN_TYPE_NUMBER_FLOAT:
            // TODO
            break;
        case TOKEN_TYPE_NUMBER_HEX:
            params->return_value = new_i32_const(string_atoi(ast_constant->constant, 16));
            break;
        case TOKEN_TYPE_NUMBER_OCT:
            params->return_value = new_i32_const(string_atoi(ast_constant->constant, 8));
            break;
        case TOKEN_TYPE_IDENTIFIER:;
            symbol_table_declaration_t *identifier_symbol = symbol_table_find(params->symbol_table, ast_constant->constant)->element;
            if (identifier_symbol->is_constant == 1) {
                // OPTIMIZATION: Load constants directly as integers instead of loading them from the identifier
                params->return_value = new_i32_const(identifier_symbol->value);
                params->return_value->attrs.type_size = symbol_table_get_type_size(identifier_symbol->base_type);
                params->return_value->attrs.base_size = params->return_value->attrs.type_size;
                break;
            }

            params->return_value = new_address(new_string_constant(ast_constant->constant));
            params->return_value->attrs.offset = identifier_symbol->offset;
            params->return_value->attrs.is_parameter = identifier_symbol->is_parameter;
            params->return_value->attrs.no_dereference = identifier_symbol->is_parameter == 0 && identifier_symbol->base_type->type == SYMBOL_TABLE_DATA_TYPE_ARRAY;
            params->return_value->attrs.type_size = symbol_table_type_size(identifier_symbol->base_type);
            params->return_value->attrs.base_size = symbol_table_get_type_size(identifier_symbol->base_type);
            break;
        default:
            break;
    }
}

void intermediate_code_parse_continue(intermediate_code_visitor_params_t *params) {
    intermediate_code_add(params, new_goto(params->continue_label));
}

void intermediate_code_parse_declarator(ast_declarator_t *ast_declarator, intermediate_code_visitor_params_t *params) {
    if (ast_declarator->initializer == NULL) {
        return;
    }

    ast_constant_t *declarator_name = ast_declarator->identifier->value;
    symbol_table_declaration_t *declaration_symbol = symbol_table_find(params->symbol_table, declarator_name->constant)->element;

    syntax_analyzer_visit(intermediate_code_callback, ast_declarator->identifier, params);
    intermediate_code_arg_t *identifier_arg = params->return_value;

    if (ast_declarator->initializer->type == AST_TYPE_ARRAY) {
        ast_array_t *initializers = ast_declarator->initializer->value;

        int i;
        for (i = 0; i < initializers->array->size; i++) {
            ast_node_t *initializer_node = dynamic_array_get(initializers->array, i);
            syntax_analyzer_visit(intermediate_code_callback, initializer_node, params);
            intermediate_code_arg_t *initializer_label = params->return_value;
            initializer_label = intermediate_code_maybe_load_address(initializer_label, params);

            int offset_value = i * symbol_table_get_type_size(declaration_symbol->base_type);

            intermediate_code_arg_t *offset_label = new_tmp_label();
            offset_label->attrs.type_size = identifier_arg->attrs.base_size;
            intermediate_code_add(params, new_binary(
                INTERMEDIATE_CODE_OP_ADD,
                offset_label,
                identifier_arg,
                new_i32_const(offset_value)
            ));

            intermediate_code_add(params, new_memory_write(offset_label, initializer_label));
        }
    } else {
        syntax_analyzer_visit(intermediate_code_callback, ast_declarator->initializer, params);
        intermediate_code_arg_t *initializer_arg = params->return_value;
        if (initializer_arg->attrs.no_dereference == 0) {
            initializer_arg = intermediate_code_maybe_load_address(initializer_arg, params);
        }
        intermediate_code_add(params, new_memory_write(identifier_arg, initializer_arg));
    }
}

void intermediate_code_parse_default(ast_default_t *ast_default, intermediate_code_visitor_params_t *params) {
    syntax_analyzer_visit(intermediate_code_callback, ast_default->statement, params);
}

void intermediate_code_parse_do_while(ast_do_while_t *ast_do_while, intermediate_code_visitor_params_t *params) {
    /*
     *  L1:
     *  <statement>
     *  L2: (continue label)
     *  t1 = <expression>
     *  if t1 == 0 goto L3
     *  goto L1
     *  L3: (break label)
     */

    intermediate_code_arg_t *do_while_start_label = new_control_label();
    intermediate_code_arg_t *do_while_condition_label = new_control_label();
    intermediate_code_arg_t *do_while_end_label = new_control_label();

    params->break_label = do_while_end_label;
    params->continue_label = do_while_condition_label;

    intermediate_code_add(params, new_label(do_while_start_label));
    syntax_analyzer_visit(intermediate_code_callback, ast_do_while->statement, params);

    intermediate_code_add(params, new_label(do_while_condition_label));
    syntax_analyzer_visit(intermediate_code_callback, ast_do_while->expression, params);
    params->return_value = intermediate_code_maybe_load_address(params->return_value, params);

    intermediate_code_add(params, new_if_goto(
        params->return_value,
        new_i32_const(0),
        do_while_end_label
    ));
    intermediate_code_add(params, new_goto(do_while_start_label));

    intermediate_code_add(params, new_label(do_while_end_label));
}

void intermediate_code_parse_for(ast_for_t *ast_for, intermediate_code_visitor_params_t *params) {
    /*
     *  <expression_low>
     *  L1:
     *  t1 = expression_high_arg_label
     *  if t1 == 0 goto L3
     *  <statement>
     *  L2:
     *  <expression_step>
     *  goto L1
     *  L3:
     */

    intermediate_code_arg_t *for_condition_label = new_control_label();
    intermediate_code_arg_t *for_post_label = new_control_label();
    intermediate_code_arg_t *for_end_label = new_control_label();

    params->break_label = for_end_label;
    params->continue_label = for_post_label;

    // Initialization
    syntax_analyzer_visit(intermediate_code_callback, ast_for->expression_low, params);

    // Condition evaluation
    intermediate_code_add(params, new_label(for_condition_label));
    syntax_analyzer_visit(intermediate_code_callback, ast_for->expression_high, params);
    intermediate_code_arg_t *expression_high_arg_label = params->return_value;
    expression_high_arg_label = intermediate_code_maybe_load_address(expression_high_arg_label, params);

    intermediate_code_add(params, new_if_goto(
        expression_high_arg_label,
        new_i32_const(0),
        for_end_label
    ));

    // Body
    syntax_analyzer_visit(intermediate_code_callback, ast_for->statement, params);

    // Postfix operation
    intermediate_code_add(params, new_label(for_post_label));
    syntax_analyzer_visit(intermediate_code_callback, ast_for->expression_step, params);
    intermediate_code_add(params, new_goto(for_condition_label));

    // End
    intermediate_code_add(params, new_label(for_end_label));
}

void intermediate_code_parse_function(ast_function_t *ast_function, intermediate_code_visitor_params_t *params) {
    if (ast_function->compound_statement == NULL) {
        return;
    }

    ast_constant_t *function_name = ast_function->identifier->value;
    symbol_table_symbol_t *symbol = symbol_table_find(params->symbol_table, function_name->constant);
    symbol_table_function_t *function_symbol = symbol->element;

    intermediate_code_add(params, new_function(
        new_string_constant(function_name->constant),
        function_symbol->parameter_size,
        function_symbol->local_size
    ));

    params->symbol_table = symbol;
    ast_compound_statement_t *compound_statement = ast_function->compound_statement->value;
    syntax_analyzer_visit(intermediate_code_callback, compound_statement->declarations, params);
    syntax_analyzer_visit(intermediate_code_callback, compound_statement->statements, params);
    params->symbol_table = params->symbol_table->parent;
}

void intermediate_code_parse_function_call(ast_function_call_t *ast_function_call, intermediate_code_visitor_params_t *params) {
    ast_constant_t *function_name = ast_function_call->function->value;
    symbol_table_function_t *function_symbol = symbol_table_find(params->symbol_table, function_name->constant)->element;

    if (ast_function_call->arguments != NULL) {
        ast_array_t *arguments = ast_function_call->arguments->value;
        intermediate_code_arg_t **argument_pool = malloc(arguments->array->size * sizeof(intermediate_code_arg_t*));
        int i;

        for (i = 0; i < function_symbol->parameters->size; i++) {
            ast_node_t *parameter_node = dynamic_array_get(arguments->array, i);

            syntax_analyzer_visit(intermediate_code_callback, parameter_node, params);
            intermediate_code_arg_t *parameter_label = params->return_value;
            parameter_label = intermediate_code_maybe_load_address(parameter_label, params);

            argument_pool[i] = parameter_label;
        }

        for (i = 0; i < function_symbol->parameters->size; i++) {
            symbol_table_symbol_t *symbol = dynamic_array_get(function_symbol->parameters, i);
            symbol_table_declaration_t *parameter_symbol = symbol->element;

            intermediate_code_arg_t *parameter_name_symbol = new_string_constant(parameter_symbol->name);
            parameter_name_symbol->attrs.offset = parameter_symbol->offset;

            intermediate_code_tac_t *parameter_tac = new_function_parameter(
                new_string_constant(function_name->constant),
                parameter_name_symbol,
                argument_pool[i]
            );
            parameter_tac->attrs.parameter_size = function_symbol->parameter_size;
            intermediate_code_add(params, parameter_tac);
        }

        free(argument_pool);
    }

    intermediate_code_arg_t *result_label = new_tmp_label();
    intermediate_code_tac_t *function_call = new_function_call(result_label, new_string_constant(function_name->constant));
    function_call->attrs.parameter_size = function_symbol->parameter_size;
    function_call->attrs.local_size = function_symbol->local_size;
    intermediate_code_add(params, function_call);

    params->return_value = result_label;
    params->return_value->attrs.base_size = symbol_table_type_size(function_symbol->base_type);
    params->return_value->attrs.type_size = params->return_value->attrs.base_size;
}

void intermediate_code_parse_if_else(ast_if_else_t *ast_if_else, intermediate_code_visitor_params_t *params) {
    /*
     *  t1 = <expression>
     *  if t1 == 0 goto L1
     *  <if_statement>
     *  goto L2
     *  L1:
     *  <else_statement>
     *  L2:
     */

    intermediate_code_arg_t *if_else_else_label = NULL;
    if (ast_if_else->else_statement != NULL) {
        if_else_else_label = new_control_label();
    }
    intermediate_code_arg_t *if_else_end_label = new_control_label();

    // If expression evaluation
    syntax_analyzer_visit(intermediate_code_callback, ast_if_else->expression, params);
    params->return_value = intermediate_code_maybe_load_address(params->return_value, params);

    // The jump from the condition check depends on whether else statement is present. If it is, we jump into it,
    // otherwise we jump to the end of if-statement.
    intermediate_code_arg_t *jump_label = ast_if_else->else_statement == NULL ? if_else_end_label : if_else_else_label;
    intermediate_code_add(params, new_if_goto(
        params->return_value,
        new_i32_const(0),
        jump_label
    ));

    // Visit and parse the if statement
    syntax_analyzer_visit(intermediate_code_callback, ast_if_else->if_statement, params);

    if (ast_if_else->else_statement != NULL) {
        // Jump to the outside of if-statement only needs to happen, if the else statement is present
        intermediate_code_add(params, new_goto(if_else_end_label));

        // Visit and parse the else statement
        intermediate_code_add(params, new_label(if_else_else_label));
        syntax_analyzer_visit(intermediate_code_callback, ast_if_else->else_statement, params);
    }

    // End
    intermediate_code_add(params, new_label(if_else_end_label));
}

void intermediate_code_parse_postfix(ast_postfix_t *ast_postfix, intermediate_code_visitor_params_t *params) {
    /*
     *  t1 = read expression_label
     *  t2 = t1 + 1  ;  if INC
     *  t2 = t1 - 1  ;  if DEC
     *  write expression_arg_label, t2
     *  <return t1>
     */

    syntax_analyzer_visit(intermediate_code_callback, ast_postfix->expression, params);
    intermediate_code_arg_t *expression_label = params->return_value;

    intermediate_code_arg_t *expression_loaded_label = new_tmp_label();
    intermediate_code_add(params, new_memory_read(
        expression_loaded_label,
        expression_label
    ));

    intermediate_code_arg_t *postfix_label = new_tmp_label();
    intermediate_code_add(params, new_binary(
        ast_postfix->operator == TOKEN_TYPE_OPERATOR_INC ?
            INTERMEDIATE_CODE_OP_ADD :
            INTERMEDIATE_CODE_OP_SUB,
        postfix_label,
        expression_loaded_label,
        new_i32_const(1)
    ));

    intermediate_code_add(params, new_memory_write(
        expression_label,
        postfix_label
    ));

    params->return_value = expression_loaded_label;
}

void intermediate_code_parse_return(ast_return_t *ast_return, intermediate_code_visitor_params_t *params) {
    intermediate_code_arg_t *return_expression_arg = NULL;
    if (ast_return->expression != NULL) {
        syntax_analyzer_visit(intermediate_code_callback, ast_return->expression, params);
        return_expression_arg = params->return_value;
        return_expression_arg = intermediate_code_maybe_load_address(return_expression_arg, params);
    }

    intermediate_code_add(params, new_return(return_expression_arg));
}

void intermediate_code_parse_string_literal(ast_string_literal_t *ast_string_literal, intermediate_code_visitor_params_t *params) {
    intermediate_code_arg_t *label_arg_label = new_string_constant(ast_string_literal->label);
    intermediate_code_arg_t *value_arg_label = new_string_constant(ast_string_literal->value);
    params->return_value = new_tmp_label();

    intermediate_code_add(params, new_string_read(
        params->return_value,
        label_arg_label,
        value_arg_label
    ));
}

void intermediate_code_parse_switch(ast_switch_t *ast_switch, intermediate_code_visitor_params_t *params) {
    intermediate_code_arg_t *switch_end_label = new_control_label();

    syntax_analyzer_visit(intermediate_code_callback, ast_switch->expression, params);
    intermediate_code_arg_t *expression_label = intermediate_code_maybe_load_address(params->return_value, params);

    params->switch_expression_label = expression_label;
    params->break_label = switch_end_label;

    syntax_analyzer_visit(intermediate_code_callback, ast_switch->statement, params);
    intermediate_code_add(params, new_label(switch_end_label));
}

void intermediate_code_parse_ternary(ast_ternary_t *ast_ternary, intermediate_code_visitor_params_t *params) {
    /*
     *  if expression == 0 goto L1
     *  return <true_expression>
     *  goto L2
     *  L1
     *  return <false_expression>
     *  L2
     */

    intermediate_code_arg_t *ternary_false_label = new_control_label();
    intermediate_code_arg_t *ternary_end_label = new_control_label();
    intermediate_code_arg_t *return_value = new_tmp_label();

    syntax_analyzer_visit(intermediate_code_callback, ast_ternary->expression, params);
    intermediate_code_arg_t *expression_label = params->return_value;
    expression_label = intermediate_code_maybe_load_address(expression_label, params);

    intermediate_code_add(params, new_if_goto(
        expression_label,
        new_i32_const(0),
        ternary_false_label
    ));

    syntax_analyzer_visit(intermediate_code_callback, ast_ternary->true_expression, params);
    params->return_value = intermediate_code_maybe_load_address(params->return_value, params);
    intermediate_code_add(params, new_assign(return_value, params->return_value));

    intermediate_code_add(params, new_goto(ternary_end_label));
    intermediate_code_add(params, new_label(ternary_false_label));

    syntax_analyzer_visit(intermediate_code_callback, ast_ternary->false_expression, params);
    params->return_value = intermediate_code_maybe_load_address(params->return_value, params);
    intermediate_code_add(params, new_assign(return_value, params->return_value));

    intermediate_code_add(params, new_label(ternary_end_label));
    params->return_value = return_value;
}

void intermediate_code_parse_unary(ast_unary_t *ast_unary, intermediate_code_visitor_params_t *params) {
    intermediate_code_parse_type_t original_parse_type = params->parse_type;
    params->parse_type = INTERMEDIATE_CODE_PARSE_TYPE_RHS;

    syntax_analyzer_visit(intermediate_code_callback, ast_unary->expression, params);
    intermediate_code_arg_t *expression_label = params->return_value;

    token_type_t op = ast_unary->operator;
    if (op != TOKEN_TYPE_OPERATOR_BIT_AND && op != TOKEN_TYPE_OPERATOR_INC && op != TOKEN_TYPE_OPERATOR_DEC) {
        expression_label = intermediate_code_maybe_load_address(expression_label, params);
        params->return_value = expression_label;
    }

    switch (ast_unary->operator) {
        case TOKEN_TYPE_OPERATOR_BIT_AND:
            /*
             *  <return expression_label>
             */

            params->return_value = expression_label;
            params->return_value->attrs.no_dereference = 1;
            break;
        case TOKEN_TYPE_OPERATOR_MUL:
            /*
             *  t0 = read expression_label
             *  <return t0>
             */

            if (original_parse_type == INTERMEDIATE_CODE_PARSE_TYPE_RHS) {
                params->return_value = new_tmp_label();
                intermediate_code_add(params, new_memory_read(
                    params->return_value,
                    expression_label
                ));
            }
            break;
        case TOKEN_TYPE_OPERATOR_SUB:
            params->return_value = new_tmp_label();
            intermediate_code_add(params, new_binary(
                INTERMEDIATE_CODE_OP_SUB,
                params->return_value,
                new_i32_const(0),
                expression_label
            ));
            break;
        case TOKEN_TYPE_OPERATOR_TILDE:
            params->return_value = new_tmp_label();
            intermediate_code_add(params, new_unary(
                INTERMEDIATE_CODE_OP_BITWISE_COMPLEMENT,
                params->return_value,
                expression_label
            ));
            break;
        case TOKEN_TYPE_OPERATOR_NOT:
            params->return_value = new_tmp_label();
            intermediate_code_add(params, new_binary(
                INTERMEDIATE_CODE_OP_LOGICAL_EQUAL,
                params->return_value,
                expression_label,
                new_i32_const(0)
            ));
            break;
        case TOKEN_TYPE_OPERATOR_DEC:
        case TOKEN_TYPE_OPERATOR_INC:
            params->return_value = new_tmp_label();
            intermediate_code_add(params, new_memory_read(
                params->return_value,
                expression_label
            ));
            intermediate_code_add(params, new_binary(
                ast_unary->operator == TOKEN_TYPE_OPERATOR_INC ?
                    INTERMEDIATE_CODE_OP_ADD :
                    INTERMEDIATE_CODE_OP_SUB,
                params->return_value,
                params->return_value,
                new_i32_const(1)
            ));
            intermediate_code_add(params, new_memory_write(
                expression_label,
                params->return_value
            ));
            break;
        default:
            break;
    }
}

void intermediate_code_parse_while(ast_while_t *ast_while, intermediate_code_visitor_params_t *params) {
    /*
     *  L1:
     *  t1 = <expression>
     *  if t1 == 0 goto L2
     *  <statement>
     *  goto L1
     *  L2:
     */

    intermediate_code_arg_t *while_condition_label = new_control_label();
    intermediate_code_arg_t *while_end_label = new_control_label();

    params->break_label = while_end_label;
    params->continue_label = while_condition_label;

    intermediate_code_add(params, new_label(while_condition_label));
    syntax_analyzer_visit(intermediate_code_callback, ast_while->expression, params);
    params->return_value = intermediate_code_maybe_load_address(params->return_value, params);

    intermediate_code_add(params, new_if_goto(
    params->return_value,
    new_i32_const(0),
    while_end_label
    ));

    syntax_analyzer_visit(intermediate_code_callback, ast_while->statement, params);
    intermediate_code_add(params, new_goto(while_condition_label));

    intermediate_code_add(params, new_label(while_end_label));
}

int intermediate_code_callback(ast_type_t type, void *element, void *params) {
    switch (type) {
        case AST_TYPE_ARRAY_ACCESS:
            intermediate_code_parse_array_access(element, params);
            return VISITOR_CALLBACK_RETURN_MANUAL;
        case AST_TYPE_BINARY:
            intermediate_code_parse_binary(element, params);
            return VISITOR_CALLBACK_RETURN_MANUAL;
        case AST_TYPE_BREAK:
            intermediate_code_parse_break(params);
            return VISITOR_CALLBACK_RETURN_MANUAL;
        case AST_TYPE_CASE:
            intermediate_code_parse_case(element, params);
            return VISITOR_CALLBACK_RETURN_MANUAL;
        case AST_TYPE_COMPOUND_STATEMENT:
            intermediate_code_parse_compound_statement(element, params);
            return VISITOR_CALLBACK_RETURN_MANUAL;
        case AST_TYPE_CONSTANT:
            intermediate_code_parse_constant(element, params);
            return VISITOR_CALLBACK_RETURN_MANUAL;
        case AST_TYPE_CONTINUE:
            intermediate_code_parse_continue(params);
            return VISITOR_CALLBACK_RETURN_MANUAL;
        case AST_TYPE_DECLARATOR:
            intermediate_code_parse_declarator(element, params);
            return VISITOR_CALLBACK_RETURN_MANUAL;
        case AST_TYPE_DEFAULT:
            intermediate_code_parse_default(element, params);
            return VISITOR_CALLBACK_RETURN_MANUAL;
        case AST_TYPE_DO_WHILE:
            intermediate_code_parse_do_while(element, params);
            return VISITOR_CALLBACK_RETURN_MANUAL;
        case AST_TYPE_FOR:
            intermediate_code_parse_for(element, params);
            return VISITOR_CALLBACK_RETURN_MANUAL;
        case AST_TYPE_FUNCTION:
            intermediate_code_parse_function(element, params);
            return VISITOR_CALLBACK_RETURN_MANUAL;
        case AST_TYPE_FUNCTION_CALL:
            intermediate_code_parse_function_call(element, params);
            return VISITOR_CALLBACK_RETURN_MANUAL;
        case AST_TYPE_IF_ELSE:
            intermediate_code_parse_if_else(element, params);
            return VISITOR_CALLBACK_RETURN_MANUAL;
        case AST_TYPE_POSTFIX:
            intermediate_code_parse_postfix(element, params);
            return VISITOR_CALLBACK_RETURN_MANUAL;
        case AST_TYPE_RETURN:
            intermediate_code_parse_return(element, params);
            return VISITOR_CALLBACK_RETURN_MANUAL;
        case AST_TYPE_STRING_LITERAL:
            intermediate_code_parse_string_literal(element, params);
            return VISITOR_CALLBACK_RETURN_MANUAL;
        case AST_TYPE_SWITCH:
            intermediate_code_parse_switch(element, params);
            return VISITOR_CALLBACK_RETURN_MANUAL;
        case AST_TYPE_TERNARY:
            intermediate_code_parse_ternary(element, params);
            return VISITOR_CALLBACK_RETURN_MANUAL;
        case AST_TYPE_UNARY:
            intermediate_code_parse_unary(element, params);
            return VISITOR_CALLBACK_RETURN_MANUAL;
        case AST_TYPE_WHILE:
            intermediate_code_parse_while(element, params);
            return VISITOR_CALLBACK_RETURN_MANUAL;
        default:
            return VISITOR_CALLBACK_RETURN_AUTOMATIC;
    }
}

dynamic_array_t* intermediate_code_generate(ast_node_t *node, symbol_table_symbol_t *symbol_table) {
    intermediate_code_visitor_params_t params;
    params.intermediate_code = dynamic_array_new();
    params.symbol_table = symbol_table;
    params.return_value = NULL;
    params.parse_type = INTERMEDIATE_CODE_PARSE_TYPE_RHS;
    reference_pool = dynamic_array_new();

    syntax_analyzer_visit(intermediate_code_callback, node, &params);

    // Optimizations
    intermediate_code_optimization_deduplication(params.intermediate_code);
    intermediate_code_optimization_liveliness_check(params.intermediate_code);

    return params.intermediate_code;
}

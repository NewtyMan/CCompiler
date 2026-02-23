#include <syntax_analyzer/syntax_analyzer_visitor.h>

void syntax_analyzer_visit_array(syntax_analyzer_visitor_callback callback, ast_node_t *node, void *params) {
    ast_array_t *ast_array = node->value;
    if (callback(AST_TYPE_ARRAY, ast_array, params) == VISITOR_CALLBACK_RETURN_AUTOMATIC) {
        int i;
        for (i = 0; i < ast_array->array->size; i++) {
            ast_node_t *array_node = dynamic_array_get(ast_array->array, i);
            syntax_analyzer_visit(callback, array_node, params);
        }
    }
}

void syntax_analyzer_visit_array_access(syntax_analyzer_visitor_callback callback, ast_node_t *node, void *params) {
    ast_array_access_t *ast_array_access = node->value;
    if (callback(AST_TYPE_ARRAY_ACCESS, ast_array_access, params) == VISITOR_CALLBACK_RETURN_AUTOMATIC) {
        syntax_analyzer_visit(callback, ast_array_access->array, params);
        syntax_analyzer_visit(callback, ast_array_access->expression, params);
    }
}

void syntax_analyzer_visit_array_declarator(syntax_analyzer_visitor_callback callback, ast_node_t *node, void *params) {
    ast_array_declarator_t *ast_array_declarator = node->value;
    if (callback(AST_TYPE_ARRAY_DECLARATOR, ast_array_declarator, params) == VISITOR_CALLBACK_RETURN_AUTOMATIC) {
        syntax_analyzer_visit(callback, ast_array_declarator->array, params);
        syntax_analyzer_visit(callback, ast_array_declarator->size, params);
        syntax_analyzer_visit(callback, ast_array_declarator->initializer, params);
    }
}

void syntax_analyzer_visit_binary(syntax_analyzer_visitor_callback callback, ast_node_t *node, void *params) {
    ast_binary_t *ast_binary = node->value;
    if (callback(AST_TYPE_BINARY, ast_binary, params) == VISITOR_CALLBACK_RETURN_AUTOMATIC) {
        syntax_analyzer_visit(callback, ast_binary->left, params);
        syntax_analyzer_visit(callback, ast_binary->right, params);
    }
}

void syntax_analyzer_visit_break(syntax_analyzer_visitor_callback callback, ast_node_t *node, void *params) {
    callback(AST_TYPE_BREAK, node, params);
}

void syntax_analyzer_visit_case(syntax_analyzer_visitor_callback callback, ast_node_t *node, void *params) {
    ast_case_t *ast_case = node->value;
    if (callback(AST_TYPE_CASE, ast_case, params) == VISITOR_CALLBACK_RETURN_AUTOMATIC) {
        syntax_analyzer_visit(callback, ast_case->expression, params);
        syntax_analyzer_visit(callback, ast_case->statement, params);
    }
}

void syntax_analyzer_visit_compound(syntax_analyzer_visitor_callback callback, ast_node_t *node, void *params) {
    struct ast_compound_statement *ast_compound_statement = node->value;
    if (callback(AST_TYPE_COMPOUND_STATEMENT, ast_compound_statement, params) == VISITOR_CALLBACK_RETURN_AUTOMATIC) {
        syntax_analyzer_visit(callback, ast_compound_statement->declarations, params);
        syntax_analyzer_visit(callback, ast_compound_statement->statements, params);
    }
}

void syntax_analyzer_visit_constant(syntax_analyzer_visitor_callback callback, ast_node_t *node, void *params) {
    ast_constant_t *ast_constant = node->value;
    callback(AST_TYPE_CONSTANT, ast_constant, params);
}

void syntax_analyzer_visit_continue(syntax_analyzer_visitor_callback callback, ast_node_t *node, void *params) {
    callback(AST_TYPE_CONTINUE, node, params);
}

void syntax_analyzer_visit_declaration(syntax_analyzer_visitor_callback callback, ast_node_t *node, void *params) {
    ast_declaration_t *ast_declaration = node->value;
    if (callback(AST_TYPE_DECLARATION, ast_declaration, params) == VISITOR_CALLBACK_RETURN_AUTOMATIC) {
        syntax_analyzer_visit(callback, ast_declaration->specifiers, params);
        syntax_analyzer_visit(callback, ast_declaration->declarators, params);
    }
}

void syntax_analyzer_visit_declarator(syntax_analyzer_visitor_callback callback, ast_node_t *node, void *params) {
    ast_declarator_t *ast_declarator = node->value;
    if (callback(AST_TYPE_DECLARATOR, ast_declarator, params) == VISITOR_CALLBACK_RETURN_AUTOMATIC) {
        syntax_analyzer_visit(callback, ast_declarator->pointer, params);
        syntax_analyzer_visit(callback, ast_declarator->identifier, params);
        syntax_analyzer_visit(callback, ast_declarator->array, params);
        syntax_analyzer_visit(callback, ast_declarator->initializer, params);
    }
}

void syntax_analyzer_visit_default(syntax_analyzer_visitor_callback callback, ast_node_t *node, void *params) {
    ast_default_t *ast_default = node->value;
    if (callback(AST_TYPE_DEFAULT, ast_default, params) == VISITOR_CALLBACK_RETURN_AUTOMATIC) {
        syntax_analyzer_visit(callback, ast_default->statement, params);
    }
}

void syntax_analyzer_visit_do_while(syntax_analyzer_visitor_callback callback, ast_node_t *node, void *params) {
    ast_do_while_t *ast_do_while = node->value;
    if (callback(AST_TYPE_DO_WHILE, ast_do_while, params) == VISITOR_CALLBACK_RETURN_AUTOMATIC) {
        syntax_analyzer_visit(callback, ast_do_while->expression, params);
        syntax_analyzer_visit(callback, ast_do_while->statement, params);
    }
}

void syntax_analyzer_visit_empty(syntax_analyzer_visitor_callback callback, ast_node_t *node, void *params) {
    callback(AST_TYPE_EMPTY, node, params);
}

void syntax_analyzer_visit_enum(syntax_analyzer_visitor_callback callback, ast_node_t *node, void *params) {
    ast_enum_t *ast_enum = node->value;
    if (callback(AST_TYPE_ENUM, ast_enum, params) == VISITOR_CALLBACK_RETURN_AUTOMATIC) {
        syntax_analyzer_visit(callback, ast_enum->identifier, params);
        syntax_analyzer_visit(callback, ast_enum->enumerators, params);
    }
}

void syntax_analyzer_visit_enumerator(syntax_analyzer_visitor_callback callback, ast_node_t *node, void *params) {
    ast_enumerator_t *ast_enumerator = node->value;
    if (callback(AST_TYPE_ENUMERATOR, ast_enumerator, params) == VISITOR_CALLBACK_RETURN_AUTOMATIC) {
        syntax_analyzer_visit(callback, ast_enumerator->identifier, params);
        syntax_analyzer_visit(callback, ast_enumerator->value, params);
    }
}

void syntax_analyzer_visit_for(syntax_analyzer_visitor_callback callback, ast_node_t *node, void *params) {
    ast_for_t *ast_for = node->value;
    if (callback(AST_TYPE_FOR, ast_for, params) == VISITOR_CALLBACK_RETURN_AUTOMATIC) {
        syntax_analyzer_visit(callback, ast_for->expression_low, params);
        syntax_analyzer_visit(callback, ast_for->expression_high, params);
        syntax_analyzer_visit(callback, ast_for->expression_step, params);
        syntax_analyzer_visit(callback, ast_for->statement, params);
    }
}

void syntax_analyzer_visit_function(syntax_analyzer_visitor_callback callback, ast_node_t *node, void *params) {
    ast_function_t *ast_function = node->value;
    if (callback(AST_TYPE_FUNCTION, ast_function, params) == VISITOR_CALLBACK_RETURN_AUTOMATIC) {
        syntax_analyzer_visit(callback, ast_function->specifiers, params);
        syntax_analyzer_visit(callback, ast_function->pointer, params);
        syntax_analyzer_visit(callback, ast_function->identifier, params);
        syntax_analyzer_visit(callback, ast_function->parameter_list, params);
        syntax_analyzer_visit(callback, ast_function->compound_statement, params);
    }
}

void syntax_analyzer_visit_function_call(syntax_analyzer_visitor_callback callback, ast_node_t *node, void *params) {
    ast_function_call_t *ast_function_call = node->value;
    if (callback(AST_TYPE_FUNCTION_CALL, ast_function_call, params) == VISITOR_CALLBACK_RETURN_AUTOMATIC) {
        syntax_analyzer_visit(callback, ast_function_call->function, params);
        syntax_analyzer_visit(callback, ast_function_call->arguments, params);
    }
}

void syntax_analyzer_visit_if_else(syntax_analyzer_visitor_callback callback, ast_node_t *node, void *params) {
    ast_if_else_t *ast_if_else = node->value;
    if (callback(AST_TYPE_IF_ELSE, ast_if_else, params) == VISITOR_CALLBACK_RETURN_AUTOMATIC) {
        syntax_analyzer_visit(callback, ast_if_else->expression, params);
        syntax_analyzer_visit(callback, ast_if_else->if_statement, params);
        syntax_analyzer_visit(callback, ast_if_else->else_statement, params);
    }
}

void syntax_analyzer_visit_keyword(syntax_analyzer_visitor_callback callback, ast_node_t *node, void *params) {
    ast_keyword_t *ast_keyword = node->value;
    callback(AST_TYPE_KEYWORD, ast_keyword, params);
}

void syntax_analyzer_visit_parameter(syntax_analyzer_visitor_callback callback, ast_node_t *node, void *params) {
    ast_parameter_t *ast_parameter = node->value;
    if (callback(AST_TYPE_PARAMETER, ast_parameter, params) == VISITOR_CALLBACK_RETURN_AUTOMATIC) {
        syntax_analyzer_visit(callback, ast_parameter->specifiers, params);
        syntax_analyzer_visit(callback, ast_parameter->pointer, params);
        syntax_analyzer_visit(callback, ast_parameter->identifier, params);
        syntax_analyzer_visit(callback, ast_parameter->array, params);
    }
}

void syntax_analyzer_visit_pointer(syntax_analyzer_visitor_callback callback, ast_node_t *node, void *params) {
    ast_pointer_t *ast_pointer = node->value;
    if (callback(AST_TYPE_POINTER, ast_pointer, params) == VISITOR_CALLBACK_RETURN_AUTOMATIC) {
        syntax_analyzer_visit(callback, ast_pointer->child, params);
    }
}

void syntax_analyzer_visit_postfix(syntax_analyzer_visitor_callback callback, ast_node_t *node, void *params) {
    ast_postfix_t *ast_postfix = node->value;
    if (callback(AST_TYPE_POSTFIX, ast_postfix, params) == VISITOR_CALLBACK_RETURN_AUTOMATIC) {
        syntax_analyzer_visit(callback, ast_postfix->expression, params);
    }
}

void syntax_analyzer_visit_return(syntax_analyzer_visitor_callback callback, ast_node_t *node, void *params) {
    ast_return_t *ast_return = node->value;
    if (callback(AST_TYPE_RETURN, ast_return, params) == VISITOR_CALLBACK_RETURN_AUTOMATIC) {
        syntax_analyzer_visit(callback, ast_return->expression, params);
    }
}

void syntax_analyzer_visit_specifier(syntax_analyzer_visitor_callback callback, ast_node_t *node, void *params) {
    ast_specifier_t *ast_specifier = node->value;
    if (callback(AST_TYPE_SPECIFIER, ast_specifier, params) == VISITOR_CALLBACK_RETURN_AUTOMATIC) {
        syntax_analyzer_visit(callback, ast_specifier->type_specifier, params);
        syntax_analyzer_visit(callback, ast_specifier->typedef_name, params);
    }
}

void syntax_analyzer_visit_string_literal(syntax_analyzer_visitor_callback callback, ast_node_t *node, void *params) {
    ast_string_literal_t *ast_string_literal = node->value;
    callback(AST_TYPE_STRING_LITERAL, ast_string_literal, params);
}

void syntax_analyzer_visit_struct(syntax_analyzer_visitor_callback callback, ast_node_t *node, void *params) {
    ast_struct_t *ast_struct = node->value;
    if (callback(AST_TYPE_STRUCT, ast_struct, params) == VISITOR_CALLBACK_RETURN_AUTOMATIC) {
        syntax_analyzer_visit(callback, ast_struct->identifier, params);
        syntax_analyzer_visit(callback, ast_struct->declarations, params);
    }
}

void syntax_analyzer_visit_switch(syntax_analyzer_visitor_callback callback, ast_node_t *node, void *params) {
    ast_switch_t *ast_switch = node->value;
    if (callback(AST_TYPE_SWITCH, ast_switch, params) == VISITOR_CALLBACK_RETURN_AUTOMATIC) {
        syntax_analyzer_visit(callback, ast_switch->expression, params);
        syntax_analyzer_visit(callback, ast_switch->statement, params);
    }
}

void syntax_analyzer_visit_typedef(syntax_analyzer_visitor_callback callback, ast_node_t *node, void *params) {
    ast_typedef_t *ast_typedef = node->value;
    if (callback(AST_TYPE_TYPEDEF, ast_typedef, params) == VISITOR_CALLBACK_RETURN_AUTOMATIC) {
        syntax_analyzer_visit(callback, ast_typedef->identifier, params);
        syntax_analyzer_visit(callback, ast_typedef->declaration_specifier, params);
    }
}

void syntax_analyzer_visit_ternary(syntax_analyzer_visitor_callback callback, ast_node_t *node, void *params) {
    ast_ternary_t *ast_ternary = node->value;
    if (callback(AST_TYPE_TERNARY, ast_ternary, params) == VISITOR_CALLBACK_RETURN_AUTOMATIC) {
        syntax_analyzer_visit(callback, ast_ternary->expression, params);
        syntax_analyzer_visit(callback, ast_ternary->true_expression, params);
        syntax_analyzer_visit(callback, ast_ternary->false_expression, params);
    }
}

void syntax_analyzer_visit_unary(syntax_analyzer_visitor_callback callback, ast_node_t *node, void *params) {
    ast_unary_t *ast_unary = node->value;
    if (callback(AST_TYPE_UNARY, ast_unary, params) == VISITOR_CALLBACK_RETURN_AUTOMATIC) {
        syntax_analyzer_visit(callback, ast_unary->expression, params);
    }
}

void syntax_analyzer_visit_while(syntax_analyzer_visitor_callback callback, ast_node_t *node, void *params) {
    ast_while_t *ast_while = node->value;
    if (callback(AST_TYPE_WHILE, ast_while, params) == VISITOR_CALLBACK_RETURN_AUTOMATIC) {
        syntax_analyzer_visit(callback, ast_while->expression, params);
        syntax_analyzer_visit(callback, ast_while->statement, params);
    }
}

void syntax_analyzer_visit(syntax_analyzer_visitor_callback callback, ast_node_t *ast_node, void *params) {
    if (ast_node == NULL) {
        return;
    }

    switch (ast_node->type) {
        case AST_TYPE_ARRAY:
            syntax_analyzer_visit_array(callback, ast_node, params);
            break;
        case AST_TYPE_ARRAY_ACCESS:
            syntax_analyzer_visit_array_access(callback, ast_node, params);
            break;
        case AST_TYPE_ARRAY_DECLARATOR:
            syntax_analyzer_visit_array_declarator(callback, ast_node, params);
            break;
        case AST_TYPE_BINARY:
            syntax_analyzer_visit_binary(callback, ast_node, params);
            break;
        case AST_TYPE_BREAK:
            syntax_analyzer_visit_break(callback, ast_node, params);
            break;
        case AST_TYPE_CASE:
            syntax_analyzer_visit_case(callback, ast_node, params);
            break;
        case AST_TYPE_COMPOUND_STATEMENT:
            syntax_analyzer_visit_compound(callback, ast_node, params);
            break;
        case AST_TYPE_CONSTANT:
            syntax_analyzer_visit_constant(callback, ast_node, params);
            break;
        case AST_TYPE_CONTINUE:
            syntax_analyzer_visit_continue(callback, ast_node, params);
            break;
        case AST_TYPE_DECLARATION:
            syntax_analyzer_visit_declaration(callback, ast_node, params);
            break;
        case AST_TYPE_DECLARATOR:
            syntax_analyzer_visit_declarator(callback, ast_node, params);
            break;
        case AST_TYPE_DEFAULT:
            syntax_analyzer_visit_default(callback, ast_node, params);
            break;
        case AST_TYPE_DO_WHILE:
            syntax_analyzer_visit_do_while(callback, ast_node, params);
            break;
        case AST_TYPE_EMPTY:
            syntax_analyzer_visit_empty(callback, ast_node, params);
            break;
        case AST_TYPE_ENUM:
            syntax_analyzer_visit_enum(callback, ast_node, params);
            break;
        case AST_TYPE_ENUMERATOR:
            syntax_analyzer_visit_enumerator(callback, ast_node, params);
            break;
        case AST_TYPE_FOR:
            syntax_analyzer_visit_for(callback, ast_node, params);
            break;
        case AST_TYPE_FUNCTION:
            syntax_analyzer_visit_function(callback, ast_node, params);
            break;
        case AST_TYPE_FUNCTION_CALL:
            syntax_analyzer_visit_function_call(callback, ast_node, params);
            break;
        case AST_TYPE_IF_ELSE:
            syntax_analyzer_visit_if_else(callback, ast_node, params);
            break;
        case AST_TYPE_KEYWORD:
            syntax_analyzer_visit_keyword(callback, ast_node, params);
            break;
        case AST_TYPE_PARAMETER:
            syntax_analyzer_visit_parameter(callback, ast_node, params);
            break;
        case AST_TYPE_POINTER:
            syntax_analyzer_visit_pointer(callback, ast_node, params);
            break;
        case AST_TYPE_POSTFIX:
            syntax_analyzer_visit_postfix(callback, ast_node, params);
            break;
        case AST_TYPE_RETURN:
            syntax_analyzer_visit_return(callback, ast_node, params);
            break;
        case AST_TYPE_SPECIFIER:
            syntax_analyzer_visit_specifier(callback, ast_node, params);
            break;
        case AST_TYPE_STRING_LITERAL:
            syntax_analyzer_visit_string_literal(callback, ast_node, params);
            break;
        case AST_TYPE_STRUCT:
            syntax_analyzer_visit_struct(callback, ast_node, params);
            break;
        case AST_TYPE_SWITCH:
            syntax_analyzer_visit_switch(callback, ast_node, params);
            break;
        case AST_TYPE_TYPEDEF:
            syntax_analyzer_visit_typedef(callback, ast_node, params);
            break;
        case AST_TYPE_TERNARY:
            syntax_analyzer_visit_ternary(callback, ast_node, params);
            break;
        case AST_TYPE_UNARY:
            syntax_analyzer_visit_unary(callback, ast_node, params);
            break;
        case AST_TYPE_WHILE:
            syntax_analyzer_visit_while(callback, ast_node, params);
            break;
        default:
            printf("[ERROR] [AST] Unhandled AST: %d\n", ast_node->type);
            break;
    }
}

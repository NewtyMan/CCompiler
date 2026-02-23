#include <syntax_analyzer/syntax_analyzer.h>
#include <syntax_analyzer/syntax_analyzer_helpers.h>
#include <utils/string.h>
#include <stdlib.h>

const int INDENT_OFFSET = 2;

lexeme_t* previous_lexeme(syntax_analyzer_params_t *params) {
    return dynamic_array_get(params->lexemes, params->lexemes_index - 1);
}

lexeme_t* current_lexeme(syntax_analyzer_params_t *params) {
    return dynamic_array_get(params->lexemes, params->lexemes_index);
}

int validate_lexeme(syntax_analyzer_params_t *params, token_type_t token_type) {
    const lexeme_t *lexeme = dynamic_array_get(params->lexemes, params->lexemes_index);
    if (lexeme == NULL) {
        return 0;
    }

    if (lexeme->token_type != token_type) {
        printf("[ERROR] [AST] Failed to validate lexeme: Expected: %s, received: %s\n",
            token_type_labels[token_type],
            token_type_labels[lexeme->token_type]
        );
        return 0;
    }

    params->lexemes_index++;
    return 1;
}

int check_lexeme(syntax_analyzer_params_t *params, token_type_t token_type) {
    const lexeme_t *lexeme = dynamic_array_get(params->lexemes, params->lexemes_index);
    if (lexeme == NULL || lexeme->token_type != token_type) {
        return 0;
    }

    params->lexemes_index++;
    return 1;
}

void syntax_analyzer_scope_free(syntax_analyzer_scope_t *scope) {
    dynamic_array_free(scope->enumerators, dynamic_array_free_empty);
    dynamic_array_free(scope->type_definitions, dynamic_array_free_empty);
    free(scope);
}

int is_typedef_name(syntax_analyzer_scope_t *scope, char *typedef_name) {
    int i;
    for (i = 0; i < scope->type_definitions->size; i++) {
        char *typedef_name_arr = dynamic_array_get(scope->type_definitions, i);
        if (string_cmp(typedef_name, typedef_name_arr) == 0) {
            return 1;
        }
    }

    if (scope->parent != NULL) {
        return is_typedef_name(scope->parent, typedef_name);
    }

    return 0;
}

int is_enumerator_constant(syntax_analyzer_scope_t *scope, char *enumerator_name) {
    int i;
    for (i = 0; i < scope->enumerators->size; i++) {
        char *enumerator_name_arr = dynamic_array_get(scope->enumerators, i);
        if (string_cmp(enumerator_name, enumerator_name_arr) == 0) {
            return 1;
        }
    }

    if (scope->parent != NULL) {
        return is_enumerator_constant(scope->parent, enumerator_name);
    }

    return 0;
}

int get_pointer_depth(ast_node_t *node) {
    int depth = 0;
    while (node != NULL && node->type == AST_TYPE_POINTER) {
        ast_pointer_t *ast_pointer = node->value;
        node = ast_pointer->child;
        depth++;
    }
    return depth;
}

void print_pointer(ast_node_t *node) {
    if (node == NULL || node->type != AST_TYPE_POINTER) {
        return;
    }

    ast_pointer_t *pointer = node->value;
    print_pointer(pointer->child);
    printf("*");
}


ast_node_t *declaration_specifiers = NULL;
ast_node_t *declarator_pointer = NULL;

void syntax_analyzer_print(ast_node_t *ast_node, int indent) {
    if (ast_node == NULL) {
        return;
    }

    switch (ast_node->type) {
        case AST_TYPE_ARRAY:;
            const ast_array_t *ast_array = ast_node->value;
            int i;
            for (i = 0; i < ast_array->array->size; i++) {
                ast_node_t *node = dynamic_array_get(ast_array->array, i);
                syntax_analyzer_print(node, indent);
            }
            break;
        case AST_TYPE_ARRAY_ACCESS:;
            const ast_array_access_t *ast_array_access = ast_node->value;
            printf("%*s[ArrayAccess]\n", indent, "");
            printf("%*s[Array]\n", indent + INDENT_OFFSET, "");
            syntax_analyzer_print(ast_array_access->array, indent + INDENT_OFFSET * 2);
            printf("%*s[Expression]\n", indent + INDENT_OFFSET, "");
            syntax_analyzer_print(ast_array_access->expression, indent + INDENT_OFFSET * 2);
            break;
        case AST_TYPE_ARRAY_DECLARATOR:;
            const ast_array_declarator_t *ast_array_declarator = ast_node->value;
            const ast_constant_t *ast_array_declarator_size = ast_array_declarator->size->value;
            printf("%*s[Array, size=%s]\n", indent, "", ast_array_declarator_size->constant);
            syntax_analyzer_print(ast_array_declarator->array, indent + INDENT_OFFSET);
            syntax_analyzer_print(ast_array_declarator->initializer, indent + INDENT_OFFSET);
            break;
        case AST_TYPE_BINARY:;
            const ast_binary_t *ast_binary = ast_node->value;
            printf("%*s[Binary, operator='%s']\n", indent, "", token_type_labels[ast_binary->operator]);
            syntax_analyzer_print(ast_binary->left, indent + INDENT_OFFSET);
            syntax_analyzer_print(ast_binary->right, indent + INDENT_OFFSET);
            break;
        case AST_TYPE_BREAK:;
            printf("%*s[Break]\n", indent, "");
            break;
        case AST_TYPE_CASE:;
            const ast_case_t *ast_case = ast_node->value;
            printf("%*s[Case]\n", indent, "");
            printf("%*s[Expression]\n", indent + INDENT_OFFSET, "");
            syntax_analyzer_print(ast_case->expression, indent + INDENT_OFFSET * 2);
            printf("%*s[Statement]\n", indent + INDENT_OFFSET, "");
            syntax_analyzer_print(ast_case->statement, indent + INDENT_OFFSET * 2);
            break;
        case AST_TYPE_COMPOUND_STATEMENT:;
            const ast_compound_statement_t *ast_compound_statement = ast_node->value;
            printf("%*s[CompoundStmt]\n", indent, "");
            syntax_analyzer_print(ast_compound_statement->declarations, indent + INDENT_OFFSET);
            syntax_analyzer_print(ast_compound_statement->statements, indent + INDENT_OFFSET);
            break;
        case AST_TYPE_CONSTANT:;
            const ast_constant_t *ast_constant = ast_node->value;
            printf("%*s[Constant, type=%s, value='%s']\n", indent, "",
                token_type_labels[ast_constant->type],
                ast_constant->constant
            );
            break;
        case AST_TYPE_CONTINUE:;
            printf("%*s[Continue]\n", indent, "");
            break;
        case AST_TYPE_DECLARATION:;
            const ast_declaration_t *ast_declaration = ast_node->value;
            declaration_specifiers = ast_declaration->specifiers;
            printf("%*s[Declarations]\n", indent, "");
            syntax_analyzer_print(ast_declaration->declarators, indent + INDENT_OFFSET);
            break;
        case AST_TYPE_DECLARATOR:;
            const ast_declarator_t *ast_declarator = ast_node->value;
            const ast_constant_t *ast_declarator_identifier = ast_declarator->identifier->value;
            printf("%*s[Declarator, name=%s, type=", indent, "", ast_declarator_identifier->constant);
            declarator_pointer = ast_declarator->pointer;
            syntax_analyzer_print(declaration_specifiers, indent + INDENT_OFFSET);
            printf("]\n");
            syntax_analyzer_print(ast_declarator->array, indent + INDENT_OFFSET);
            syntax_analyzer_print(ast_declarator->initializer, indent + INDENT_OFFSET);
            break;
        case AST_TYPE_DEFAULT:;
            const ast_default_t *ast_default = ast_node->value;
            printf("%*s[Default]\n", indent, "");
            syntax_analyzer_print(ast_default->statement, indent + INDENT_OFFSET);
            break;
        case AST_TYPE_DO_WHILE:;
            const ast_do_while_t *ast_do_while = ast_node->value;
            printf("%*s[DoWhile]\n", indent, "");
            printf("%*s[Condition]\n", indent + INDENT_OFFSET, "");
            syntax_analyzer_print(ast_do_while->expression, indent + INDENT_OFFSET * 2);
            printf("%*s[Statement]\n", indent + INDENT_OFFSET, "");
            syntax_analyzer_print(ast_do_while->statement, indent + INDENT_OFFSET * 2);
            break;
        case AST_TYPE_EMPTY:;
            break;
        case AST_TYPE_ENUM:;
            const ast_enum_t *ast_enum = ast_node->value;
            const ast_constant_t *ast_enum_identifier = ast_enum->identifier->value;
            printf("%*s[Enum, name=%s]\n", indent, "", ast_enum_identifier->constant);
            syntax_analyzer_print(ast_enum->enumerators, indent + INDENT_OFFSET);
            break;
        case AST_TYPE_ENUMERATOR:;
            const ast_enumerator_t *ast_enumerator = ast_node->value;
            printf("%*s[Enumerator]\n", indent, "");
            syntax_analyzer_print(ast_enumerator->identifier, indent + INDENT_OFFSET);
            syntax_analyzer_print(ast_enumerator->value, indent + INDENT_OFFSET);
            break;
        case AST_TYPE_FOR:;
            const ast_for_t *ast_for = ast_node->value;
            printf("%*s[ForStmt]\n", indent, "");
            printf("%*s[LowExpr]\n", indent + INDENT_OFFSET, "");
            syntax_analyzer_print(ast_for->expression_low, indent + INDENT_OFFSET * 2);
            printf("%*s[HighExpr]\n", indent + INDENT_OFFSET, "");
            syntax_analyzer_print(ast_for->expression_high, indent + INDENT_OFFSET * 2);
            printf("%*s[StepExpr]\n", indent + INDENT_OFFSET, "");
            syntax_analyzer_print(ast_for->expression_step, indent + INDENT_OFFSET * 2);
            printf("%*s[Body]\n", indent + INDENT_OFFSET, "");
            syntax_analyzer_print(ast_for->statement, indent + INDENT_OFFSET * 2);
            break;
        case AST_TYPE_FUNCTION:;
            const ast_function_t *ast_function = ast_node->value;
            const ast_constant_t *ast_function_identifier = ast_function->identifier->value;
            printf("%*s[Function, name=%s, type=", indent, "", ast_function_identifier->constant);
            syntax_analyzer_print(ast_function->specifiers, indent + INDENT_OFFSET);
            printf("]\n");
            if (ast_function->parameter_list != NULL) {
                printf("%*s[Parameters]\n", indent + INDENT_OFFSET, "");
                syntax_analyzer_print(ast_function->parameter_list, indent + INDENT_OFFSET * 2);
            }
            if (ast_function->compound_statement != NULL) {
                printf("%*s[FuncBody]\n", indent + INDENT_OFFSET, "");
                syntax_analyzer_print(ast_function->compound_statement, indent + INDENT_OFFSET * 2);
            }
            break;
        case AST_TYPE_FUNCTION_CALL:;
            ast_function_call_t *ast_function_call = ast_node->value;
            printf("%*s[FunctionCall]\n", indent, "");
            printf("%*s[Function]\n", indent + INDENT_OFFSET, "");
            syntax_analyzer_print(ast_function_call->function, indent + INDENT_OFFSET * 2);
            printf("%*s[Arguments]\n", indent + INDENT_OFFSET, "");
            syntax_analyzer_print(ast_function_call->arguments, indent + INDENT_OFFSET * 2);
            break;
        case AST_TYPE_IF_ELSE:;
            const ast_if_else_t *ast_if_else = ast_node->value;
            printf("%*s[IfElse]\n", indent, "");
            printf("%*s[Condition]\n", indent + INDENT_OFFSET, "");
            syntax_analyzer_print(ast_if_else->expression, indent + INDENT_OFFSET * 2);
            printf("%*s[IfStmt]\n", indent + INDENT_OFFSET, "");
            syntax_analyzer_print(ast_if_else->if_statement, indent + INDENT_OFFSET * 2);
            if (ast_if_else->else_statement != NULL) {
                printf("%*s[ElseStmt]\n", indent + INDENT_OFFSET, "");
                syntax_analyzer_print(ast_if_else->else_statement, indent + INDENT_OFFSET * 2);
            }
            break;
        case AST_TYPE_KEYWORD:;
            const ast_keyword_t *ast_keyword = ast_node->value;
            printf("%*s[Keyword, value=%s]\n", indent, "", token_type_labels[ast_keyword->keyword]);
            break;
        case AST_TYPE_PARAMETER:;
            const ast_parameter_t *ast_parameter = ast_node->value;
            const ast_constant_t *ast_parameter_name = ast_parameter->identifier->value;
            declarator_pointer = ast_parameter->pointer;
            printf("%*s[Parameter, name=%s, type=", indent, "", ast_parameter_name->constant);
            syntax_analyzer_print(ast_parameter->specifiers, indent + INDENT_OFFSET);
            printf("]\n");
            break;
        case AST_TYPE_POINTER:;
            const ast_pointer_t *pointer = ast_node->value;
            printf("%*s[Pointer]\n", indent, "");
            syntax_analyzer_print(pointer->child, indent + INDENT_OFFSET);
            break;
        case AST_TYPE_POSTFIX:;
            const ast_postfix_t *ast_postfix = ast_node->value;
            printf("%*s[Postfix, operator=%s]\n", indent, "", token_type_labels[ast_postfix->operator]);
            syntax_analyzer_print(ast_postfix->expression, indent + INDENT_OFFSET);
            syntax_analyzer_print(ast_postfix->expression_right, indent + INDENT_OFFSET);
            break;
        case AST_TYPE_RETURN:;
            const ast_return_t *ast_return = ast_node->value;
            printf("%*s[Return]\n", indent, "");
            syntax_analyzer_print(ast_return->expression, indent + INDENT_OFFSET);
            break;
        case AST_TYPE_SPECIFIER:;
            const ast_specifier_t *ast_specifier = ast_node->value;
            printf("(");
            if (ast_specifier->type_qualifier == TOKEN_TYPE_KEYWORD_CONST) {
                printf(" const");
            }
            if (ast_specifier->type_specifier->type == AST_TYPE_KEYWORD) {
                ast_keyword_t *ast_specifier_keyword = ast_specifier->type_specifier->value;
                printf(" %s", token_type_labels[ast_specifier_keyword->keyword]);
            } else if (ast_specifier->type_specifier->type == AST_TYPE_STRUCT) {
                ast_struct_t *ast_specifier_struct = ast_specifier->type_specifier->value;
                ast_constant_t *ast_specifier_struct_name = ast_specifier_struct->identifier->value;
                printf(" struct %s", ast_specifier_struct_name->constant);
            } else if (ast_specifier->type_specifier->type == AST_TYPE_ENUM) {
                ast_enum_t *ast_specifier_enum = ast_specifier->type_specifier->value;
                ast_constant_t *ast_specifier_enum_name = ast_specifier_enum->identifier->value;
                printf(" enum %s", ast_specifier_enum_name->constant);
            } else if (ast_specifier->type_specifier->type == AST_TYPE_CONSTANT) {
                ast_constant_t *ast_specifier_constant = ast_specifier->type_specifier->value;
                printf(" %s", ast_specifier_constant->constant);
            }
            print_pointer(declarator_pointer);
            printf(" )");
            break;
        case AST_TYPE_STRING_LITERAL:;
            const ast_string_literal_t *ast_string_literal = ast_node->value;
            printf("%*s[StringLiteral, label='%s', value=%s]\n", indent, "", ast_string_literal->label, ast_string_literal->value);
            break;
        case AST_TYPE_STRUCT:;
            const ast_struct_t *ast_struct = ast_node->value;
            const ast_constant_t *ast_struct_name = ast_struct->identifier->value;
            printf("%*s[Struct, name=%s]\n", indent, "", ast_struct_name->constant);
            syntax_analyzer_print(ast_struct->declarations, indent + INDENT_OFFSET);
            break;
        case AST_TYPE_SWITCH:;
            const ast_switch_t *ast_switch = ast_node->value;
            printf("%*s[Switch]\n", indent, "");
            printf("%*s[Expr]\n", indent + INDENT_OFFSET, "");
            syntax_analyzer_print(ast_switch->expression, indent + INDENT_OFFSET * 2);
            printf("%*s[Cases]\n", indent + INDENT_OFFSET, "");
            syntax_analyzer_print(ast_switch->statement, indent + INDENT_OFFSET * 2);
            break;
        case AST_TYPE_TYPEDEF:;
            const ast_typedef_t *ast_typedef = ast_node->value;
            const ast_constant_t *ast_typedef_name = ast_typedef->identifier->value;
            printf("%*s[TypeDef, name=%s]\n", indent, "", ast_typedef_name->constant);
            syntax_analyzer_print(ast_typedef->declaration_specifier, indent + INDENT_OFFSET);
            break;
        case AST_TYPE_TERNARY:;
            const ast_ternary_t *ast_ternary = ast_node->value;
            printf("%*s[Ternary]\n", indent, "");
            printf("%*s[Expr]\n", indent + INDENT_OFFSET, "");
            syntax_analyzer_print(ast_ternary->expression, indent + INDENT_OFFSET * 2);
            printf("%*s[TrueExpr]\n", indent + INDENT_OFFSET, "");
            syntax_analyzer_print(ast_ternary->true_expression, indent + INDENT_OFFSET * 2);
            printf("%*s[FalseExpr]\n", indent + INDENT_OFFSET, "");
            syntax_analyzer_print(ast_ternary->false_expression, indent + INDENT_OFFSET * 2);
            break;
        case AST_TYPE_UNARY:;
            const ast_unary_t *ast_unary = ast_node->value;
            printf("%*s[Unary, operator=%s]\n", indent, "", token_type_labels[ast_unary->operator]);
            syntax_analyzer_print(ast_unary->expression, indent + INDENT_OFFSET);
            break;
        case AST_TYPE_WHILE:;
            const ast_while_t *ast_while = ast_node->value;
            printf("%*s[While]\n", indent, "");
            printf("%*s[Condition]\n", indent + INDENT_OFFSET, "");
            syntax_analyzer_print(ast_while->expression, indent + INDENT_OFFSET * 2);
            printf("%*s[Statement]\n", indent + INDENT_OFFSET, "");
            syntax_analyzer_print(ast_while->statement, indent + INDENT_OFFSET * 2);
            break;
        default:
            printf("[ERROR] [AST] Unhandled AST during print: %d", ast_node->type);
            break;
    }
}

void syntax_analyzer_free(void *node) {
    ast_node_t *ast_node = node;
    if (ast_node == NULL) {
        return;
    }
    if (ast_node->value == NULL) {
        free(ast_node);
        return;
    }

    switch (ast_node->type) {
        case AST_TYPE_ARRAY:;
            ast_array_t *ast_array = ast_node->value;
            dynamic_array_free(ast_array->array, syntax_analyzer_free);
            break;
        case AST_TYPE_ARRAY_ACCESS:;
            const ast_array_access_t *ast_array_access = ast_node->value;
            syntax_analyzer_free(ast_array_access->array);
            syntax_analyzer_free(ast_array_access->expression);
            break;
        case AST_TYPE_ARRAY_DECLARATOR:;
            const struct ast_array_declarator *ast_array_declarator = ast_node->value;
            syntax_analyzer_free(ast_array_declarator->array);
            syntax_analyzer_free(ast_array_declarator->size);
            syntax_analyzer_free(ast_array_declarator->initializer);
            break;
        case AST_TYPE_BINARY:;
            const ast_binary_t *ast_binary = ast_node->value;
            syntax_analyzer_free(ast_binary->left);
            syntax_analyzer_free(ast_binary->right);
            break;
        case AST_TYPE_BREAK:;
            break;
        case AST_TYPE_CASE:;
            const ast_case_t *ast_case = ast_node->value;
            syntax_analyzer_free(ast_case->expression);
            syntax_analyzer_free(ast_case->statement);
            break;
        case AST_TYPE_COMPOUND_STATEMENT:;
            const ast_compound_statement_t *ast_compound_statement = ast_node->value;
            syntax_analyzer_free(ast_compound_statement->declarations);
            syntax_analyzer_free(ast_compound_statement->statements);
            break;
        case AST_TYPE_CONSTANT:;
            const ast_constant_t *ast_constant = ast_node->value;
            free(ast_constant->constant);
            break;
        case AST_TYPE_CONTINUE:;
            break;
        case AST_TYPE_DECLARATION:;
            const ast_declaration_t *ast_declaration = ast_node->value;
            syntax_analyzer_free(ast_declaration->specifiers);
            syntax_analyzer_free(ast_declaration->declarators);
            break;
        case AST_TYPE_DECLARATOR:;
            const ast_declarator_t *ast_declarator = ast_node->value;
            syntax_analyzer_free(ast_declarator->pointer);
            syntax_analyzer_free(ast_declarator->identifier);
            syntax_analyzer_free(ast_declarator->array);
            syntax_analyzer_free(ast_declarator->initializer);
            break;
        case AST_TYPE_DEFAULT:;
            const ast_default_t *ast_default = ast_node->value;
            syntax_analyzer_free(ast_default->statement);
            break;
        case AST_TYPE_DO_WHILE:;
            const ast_do_while_t *ast_do_while = ast_node->value;
            syntax_analyzer_free(ast_do_while->expression);
            syntax_analyzer_free(ast_do_while->statement);
            break;
        case AST_TYPE_EMPTY:;
            break;
        case AST_TYPE_ENUM:;
            const ast_enum_t *ast_enum = ast_node->value;
            syntax_analyzer_free(ast_enum->identifier);
            syntax_analyzer_free(ast_enum->enumerators);
            break;
        case AST_TYPE_ENUMERATOR:;
            const ast_enumerator_t *ast_enumerator = ast_node->value;
            syntax_analyzer_free(ast_enumerator->identifier);
            syntax_analyzer_free(ast_enumerator->value);
            break;
        case AST_TYPE_FOR:;
            const ast_for_t *ast_for = ast_node->value;
            syntax_analyzer_free(ast_for->expression_low);
            syntax_analyzer_free(ast_for->expression_high);
            syntax_analyzer_free(ast_for->expression_step);
            syntax_analyzer_free(ast_for->statement);
            break;
        case AST_TYPE_FUNCTION:;
            const ast_function_t *ast_function = ast_node->value;
            syntax_analyzer_free(ast_function->specifiers);
            syntax_analyzer_free(ast_function->pointer);
            syntax_analyzer_free(ast_function->identifier);
            syntax_analyzer_free(ast_function->parameter_list);
            syntax_analyzer_free(ast_function->compound_statement);
            break;
        case AST_TYPE_FUNCTION_CALL:;
            const ast_function_call_t *ast_function_call = ast_node->value;
            syntax_analyzer_free(ast_function_call->function);
            syntax_analyzer_free(ast_function_call->arguments);
            break;
        case AST_TYPE_IF_ELSE:;
            const ast_if_else_t *ast_if_else = ast_node->value;
            syntax_analyzer_free(ast_if_else->expression);
            syntax_analyzer_free(ast_if_else->if_statement);
            syntax_analyzer_free(ast_if_else->else_statement);
            break;
        case AST_TYPE_KEYWORD:;
            break;
        case AST_TYPE_PARAMETER:;
            const ast_parameter_t *ast_parameter = ast_node->value;
            syntax_analyzer_free(ast_parameter->specifiers);
            syntax_analyzer_free(ast_parameter->pointer);
            syntax_analyzer_free(ast_parameter->identifier);
            syntax_analyzer_free(ast_parameter->array);
            break;
        case AST_TYPE_POINTER:;
            const ast_pointer_t *ast_pointer = ast_node->value;
            syntax_analyzer_free(ast_pointer->child);
            break;
        case AST_TYPE_POSTFIX:;
            const ast_postfix_t *ast_postfix = ast_node->value;
            syntax_analyzer_free(ast_postfix->expression);
            syntax_analyzer_free(ast_postfix->expression_right);
            break;
        case AST_TYPE_RETURN:;
            const ast_return_t *ast_return = ast_node->value;
            syntax_analyzer_free(ast_return->expression);
            break;
        case AST_TYPE_SPECIFIER:;
            const ast_specifier_t *ast_specifier = ast_node->value;
            syntax_analyzer_free(ast_specifier->type_specifier);
            syntax_analyzer_free(ast_specifier->typedef_name);
            break;
        case AST_TYPE_STRING_LITERAL:;
            const ast_string_literal_t *ast_string_literal = ast_node->value;
            free(ast_string_literal->value);
            free(ast_string_literal->label);
            break;
        case AST_TYPE_STRUCT:;
            const ast_struct_t *ast_struct = ast_node->value;
            syntax_analyzer_free(ast_struct->identifier);
            syntax_analyzer_free(ast_struct->declarations);
            break;
        case AST_TYPE_SWITCH:;
            const ast_switch_t *ast_switch = ast_node->value;
            syntax_analyzer_free(ast_switch->expression);
            syntax_analyzer_free(ast_switch->statement);
            break;
        case AST_TYPE_TYPEDEF:;
            const ast_typedef_t *ast_typedef = ast_node->value;
            syntax_analyzer_free(ast_typedef->identifier);
            syntax_analyzer_free(ast_typedef->declaration_specifier);
            break;
        case AST_TYPE_TERNARY:;
            const ast_ternary_t *ast_ternary = ast_node->value;
            syntax_analyzer_free(ast_ternary->expression);
            syntax_analyzer_free(ast_ternary->true_expression);
            syntax_analyzer_free(ast_ternary->false_expression);
            break;
        case AST_TYPE_UNARY:;
            const ast_unary_t *ast_unary = ast_node->value;
            syntax_analyzer_free(ast_unary->expression);
            break;
        case AST_TYPE_WHILE:;
            const ast_while_t *ast_while = ast_node->value;
            syntax_analyzer_free(ast_while->expression);
            syntax_analyzer_free(ast_while->statement);
            break;
        default:
            printf("[ERROR] [AST] Unhandled AST during free: %d", ast_node->type);
            break;
    }

    free(ast_node->value);
    free(ast_node);
}

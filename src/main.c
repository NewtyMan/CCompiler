#include <lexical_analyzer/lexical_analyzer.h>
#include <syntax_analyzer/syntax_analyzer.h>
#include <symbol_table/symbol_table.h>
#include <semantic_analyzer/semantic_analyzer.h>
#include <intermediate_code/intermediate_code.h>
#include <code_generator/code_generator.h>
#include <utils/logger.h>
#include <utils/string.h>

const int FILE_BUFFER_SIZE = 128;

const int ERROR_CODE_LEXICAL_ANALYSIS = 1;
const int ERROR_CODE_SYNTAX_ANALYSIS = 2;
const int ERROR_CODE_SYMBOL_TABLE = 3;
const int ERROR_CODE_SEMANTIC_ANALYSIS = 4;
const int ERROR_CODE_INTERMEDIATE_CODE = 5;
const int ERROR_CODE_CODE_GENERATION = 6;

int is_argv_flag_present(int argc, char **argv, char *flag) {
    int i;
    for (i = 0; i < argc; i++) {
        if (string_cmp(flag, argv[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

int main(int argc, char** argv) {
    if (argc < 3) {
        printf("Usage: ccompiler.exe <input_file_path> <output_file_path> ...flags");
        return 1;
    }

    dynamic_array_t *lexemes = NULL;
    ast_node_t *root_node = NULL;
    dynamic_array_t *string_literals = NULL;
    symbol_table_symbol_t *symbol_table = NULL;
    dynamic_array_t *intermediate_code = NULL;
    dynamic_array_t *generated_code = NULL;
    int return_code = 0;

    /***************************
    *     LEXICAL ANALYSIS     *
    ***************************/

    file_buffer_t *file_buffer = file_buffer_new_from_path(argv[1], FILE_BUFFER_SIZE);
    if (file_buffer == NULL) {
        return 1;
    }
    file_buffer_load_next(file_buffer);

    lexemes = lexical_analysis(file_buffer);
    if (lexemes == NULL) {
        logger_error("Failed to perform lexical analysis!");
        return_code = ERROR_CODE_LEXICAL_ANALYSIS;
        goto cleanup;
    }

    if (is_argv_flag_present(argc, argv, "--lexical-print")) {
        lexical_analyzer_print(lexemes);
    }

    if (is_argv_flag_present(argc, argv, "--lexical")) {
        goto cleanup;
    }

    /**************************
    *     SYNTAX ANALYSIS     *
    **************************/

    string_literals = dynamic_array_new();
    root_node = syntax_analysis(lexemes, string_literals);
    if (root_node == NULL) {
        logger_error("Failed to perform syntax analysis!");
        return_code = ERROR_CODE_SYNTAX_ANALYSIS;
        goto cleanup;
    }

    if (is_argv_flag_present(argc, argv, "--syntax-print")) {
        syntax_analyzer_print(root_node, 0);
    }

    if (is_argv_flag_present(argc, argv, "--syntax")) {
        goto cleanup;
    }

    /***********************
    *     SYMBOL TABLE     *
    ***********************/

    symbol_table = symbol_table_parse(root_node);
    if (symbol_table == NULL) {
        logger_error("Failed to build symbol table!\n");
        return_code = ERROR_CODE_SYMBOL_TABLE;
        goto cleanup;
    }

    if (is_argv_flag_present(argc, argv, "--symbol-print")) {
        symbol_table_print(symbol_table, 0);
    }

    if (is_argv_flag_present(argc, argv, "--symbol")) {
        goto cleanup;
    }

    /****************************
    *     SEMANTIC ANALYSIS     *
    ****************************/

    return_code = semantic_analysis(root_node, symbol_table);
    if (return_code != 0) {
        logger_error("Failed to perform semantic analysis!\n");
        return_code = ERROR_CODE_SEMANTIC_ANALYSIS;
        goto cleanup;
    }

    if (is_argv_flag_present(argc, argv, "--semantic")) {
        goto cleanup;
    }

    /****************************
    *     INTERMEDIATE CODE     *
    ****************************/

    intermediate_code = intermediate_code_generate(root_node, symbol_table);
    if (intermediate_code == NULL) {
        logger_error("Failed to perform intermediate code generation!\n");
        return_code = ERROR_CODE_INTERMEDIATE_CODE;
        goto cleanup;
    }

    if (is_argv_flag_present(argc, argv, "--intermediate-print")) {
        intermediate_code_print(intermediate_code);
    }

    if (is_argv_flag_present(argc, argv, "--intermediate")) {
        goto cleanup;
    }

    /**************************
    *     CODE GENERATION     *
    **************************/

    generated_code = code_generator_generate(intermediate_code, string_literals);
    if (generated_code == NULL) {
        logger_error("Failed to perform code generation!\n");
        return 1;
    }
    code_generator_output(generated_code, argv[2]);

    if (is_argv_flag_present(argc, argv, "--code-print")) {
        code_generator_print(generated_code);
    }

    /******************
    *     CLEANUP     *
    ******************/

cleanup:
    file_buffer_free(file_buffer);
    if (lexemes != NULL) {
        lexical_analyzer_free(lexemes);
    }
    if (root_node != NULL) {
        syntax_analyzer_free(root_node);
    }
    if (symbol_table != NULL) {
        symbol_table_free(symbol_table);
    }
    if (intermediate_code != NULL) {
        intermediate_code_free(intermediate_code);
    }
    if (generated_code != NULL) {
        code_generator_free(generated_code);
    }

    return return_code;
}

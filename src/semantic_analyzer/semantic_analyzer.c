#include <semantic_analyzer/semantic_analyzer.h>
#include <syntax_analyzer/syntax_analyzer.h>

int semantic_analysis(ast_node_t *node, symbol_table_symbol_t *symbol_table) {
    int return_code = 0;

    return_code = semantic_analysis_scope(node, symbol_table);
    if (return_code != 0) {
        return return_code;
    }

    return_code = semantic_analysis_scope(node, symbol_table);
    if (return_code != 0) {
        return return_code;
    }

    return_code = semantic_analysis_semantics(node, symbol_table);
    if (return_code != 0) {
        return return_code;
    }

    return 0;
}

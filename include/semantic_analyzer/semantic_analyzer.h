#ifndef CCOMPILER_SEMANTIC_ANALYZER_H
#define CCOMPILER_SEMANTIC_ANALYZER_H
#include <syntax_analyzer/syntax_analyzer.h>
#include <symbol_table/symbol_table.h>

int semantic_analysis(
    ast_node_t *node,
    symbol_table_symbol_t *symbol_table
);

int semantic_analysis_scope(
    ast_node_t *node,
    symbol_table_symbol_t *symbol_table
);

int semantic_analysis_type(
    ast_node_t *node,
    symbol_table_symbol_t *symbol_table
);

int semantic_analysis_semantics(
    ast_node_t *node,
    symbol_table_symbol_t *symbol_table
);
#endif //CCOMPILER_SEMANTIC_ANALYZER_H

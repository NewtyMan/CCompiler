#ifndef CCOMPILER_SYNTAX_ANALYZER_VISITOR_H
#define CCOMPILER_SYNTAX_ANALYZER_VISITOR_H
#include <syntax_analyzer/syntax_analyzer.h>

typedef enum visitor_callback_return {
    VISITOR_CALLBACK_RETURN_AUTOMATIC,
    VISITOR_CALLBACK_RETURN_MANUAL,
} visitor_callback_return_t;

typedef int (*syntax_analyzer_visitor_callback)(ast_type_t, void*, void*);

void syntax_analyzer_visit(
    syntax_analyzer_visitor_callback callback,
    ast_node_t *node,
    void *params
);
#endif //CCOMPILER_SYNTAX_ANALYZER_VISITOR_H

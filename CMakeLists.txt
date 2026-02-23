cmake_minimum_required(VERSION 4.0)
project(CCompiler C)

set(CMAKE_C_STANDARD 90)

set(HEADERS
        include/lexical_analyzer/lexical_analyzer.h
        include/lexical_analyzer/state_machine.h

        include/syntax_analyzer/syntax_analyzer.h
        include/syntax_analyzer/syntax_analyzer_helpers.h
        include/syntax_analyzer/syntax_analyzer_visitor.h

        include/symbol_table/symbol_table.h

        include/semantic_analyzer/semantic_analyzer.h

        include/intermediate_code/intermediate_code.h
        include/intermediate_code/intermediate_code_factory.h
        include/intermediate_code/intermediate_code_helpers.h

        include/code_generator/code_generator.h
        include/code_generator/code_generator_helpers.h

        include/utils/dynamic_array.h
        include/utils/file_buffer.h
        include/utils/hash.h
        include/utils/logger.h
        include/utils/string.h
        include/utils/string_buffer.h
)

set(SRC_FILES
        src/lexical_analyzer/lexical_analyzer.c
        src/lexical_analyzer/state_machine.c

        src/syntax_analyzer/syntax_analyzer.c
        src/syntax_analyzer/syntax_analyzer_helpers.c
        src/syntax_analyzer/analyzers/statement.c
        src/syntax_analyzer/analyzers/expression.c
        src/syntax_analyzer/analyzers/function.c
        src/syntax_analyzer/analyzers/enumerator.c
        src/syntax_analyzer/analyzers/structure.c
        src/syntax_analyzer/analyzers/variable.c
        src/syntax_analyzer/analyzers/declaration_specifiers.c
        src/syntax_analyzer/syntax_analyzer_factory.c
        src/syntax_analyzer/syntax_analyzer_visitor.c

        src/symbol_table/symbol_table.c
        src/symbol_table/symbol_table_type.c
        src/symbol_table/symbol_table_search.c
        src/symbol_table/symbol_table_factory.c
        src/symbol_table/symbol_table_parse.c
        src/symbol_table/symbol_table_syscalls.c


        src/semantic_analyzer/semantic_analyzer.c
        src/semantic_analyzer/validators/scope.c
        src/semantic_analyzer/validators/semantics.c
        src/semantic_analyzer/validators/type.c

        src/intermediate_code/intermediate_code.c
        src/intermediate_code/intermediate_code_factory.c
        src/intermediate_code/intermediate_code_helpers.c
        src/intermediate_code/optimizations/liveness_check.c
        src/intermediate_code/optimizations/deduplication.c

        src/code_generator/code_generator.c
        src/code_generator/code_generator_helpers.c

        src/utils/dynamic_array.c
        src/utils/file_buffer.c
        src/utils/hash.c
        src/utils/string_buffer.c
        src/utils/logger.c
        src/utils/string.c
)

add_executable(CCompiler ${HEADERS} ${SRC_FILES} src/main.c)

configure_file(
        ${CMAKE_SOURCE_DIR}/src/code_generator/code_generator_template.s
        ${CMAKE_BINARY_DIR}/code_generator_template.s
        COPYONLY
)

configure_file(
        ${CMAKE_SOURCE_DIR}/tests/assets/file_interaction.txt
        ${CMAKE_BINARY_DIR}/file_interaction.txt
        COPYONLY
)

target_include_directories(CCompiler PRIVATE include)

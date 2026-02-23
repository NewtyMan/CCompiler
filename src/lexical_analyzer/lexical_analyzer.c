#include <lexical_analyzer/lexical_analyzer.h>
#include <lexical_analyzer/state_machine.h>
#include <utils/logger.h>
#include <utils/string_buffer.h>
#include <utils/string.h>
#include <stdio.h>
#include <stdlib.h>

// Usually all lexemes except for strings will be quite short, so having the default string buffer size set to 32 should
// cover more than 90% of all lexemes without having to resize.
const int STRING_BUFFER_DEFAULT_CAPACITY = 32;

typedef struct keyword_entry {
    token_type_t token_type;
    char *keyword;
} keyword_entry_t;

int error_unknown_token(char *token_buffer, position_t position) {
    printf("[!] [LEX] Unknown token: \"%s\" (%d:%d - %d:%d)\n",
        token_buffer,
        position.line_start,
        position.char_start,
        position.line_end,
        position.char_end
    );
    return 1;
}

void lexical_analyzer_print(dynamic_array_t *lexemes) {
    int i;
    for (i = 0; i < lexemes->size; i++) {
        const lexeme_t *lexeme = lexemes->array[i];
        printf("[%s, %d, %d:%d-%d:%d]\n",
            lexeme->token,
            lexeme->token_type,
            lexeme->position.line_start,
            lexeme->position.char_start,
            lexeme->position.line_end,
            lexeme->position.char_end
        );
    }
}

lexeme_t* new_lexeme(const token_type_t token_type, const position_t position, const string_buffer_t *string_buffer) {
    lexeme_t *lexeme = malloc(sizeof(lexeme_t));
    lexeme->token_type = token_type;
    lexeme->position = position;
    lexeme->token = calloc(string_len(string_buffer->buffer) + 1, sizeof(char));
    string_cpy(lexeme->token, string_buffer->buffer);
    return lexeme;
}

void lexical_analyzer_free(dynamic_array_t *array) {
    int i;
    for (i = 0; i < array->size; i++) {
        lexeme_t *lexeme = array->array[i];
        free(lexeme->token);
        free(lexeme);
    }
    free(array->array);
    free(array);
}

keyword_entry_t* new_keyword_entry(token_type_t token_type, char *keyword) {
    keyword_entry_t *new_keyword_entry = malloc(sizeof(keyword_entry_t));
    new_keyword_entry->token_type = token_type;
    new_keyword_entry->keyword = calloc(string_len(keyword) + 1, sizeof(char));
    string_cpy(new_keyword_entry->keyword, keyword);
    return new_keyword_entry;
}

void free_keyword_entries(dynamic_array_t *keywords) {
    int i;
    for (i = 0; i < keywords->size; i++) {
        keyword_entry_t *keyword_entry = keywords->array[i];
        free(keyword_entry->keyword);
        free(keyword_entry);
    }
    free(keywords->array);
    free(keywords);
}

void lexical_analyzer_setup_keywords(dynamic_array_t *keywords) {
    dynamic_array_add(keywords, new_keyword_entry(TOKEN_TYPE_KEYWORD_BREAK, "break"));
    dynamic_array_add(keywords, new_keyword_entry(TOKEN_TYPE_KEYWORD_CASE, "case"));
    dynamic_array_add(keywords, new_keyword_entry(TOKEN_TYPE_KEYWORD_CHAR, "char"));
    dynamic_array_add(keywords, new_keyword_entry(TOKEN_TYPE_KEYWORD_CONST, "const"));
    dynamic_array_add(keywords, new_keyword_entry(TOKEN_TYPE_KEYWORD_CONTINUE, "continue"));
    dynamic_array_add(keywords, new_keyword_entry(TOKEN_TYPE_KEYWORD_DEFAULT, "default"));
    dynamic_array_add(keywords, new_keyword_entry(TOKEN_TYPE_KEYWORD_DO, "do"));
    dynamic_array_add(keywords, new_keyword_entry(TOKEN_TYPE_KEYWORD_ELSE, "else"));
    dynamic_array_add(keywords, new_keyword_entry(TOKEN_TYPE_KEYWORD_ENUM, "enum"));
    dynamic_array_add(keywords, new_keyword_entry(TOKEN_TYPE_KEYWORD_FLOAT, "float"));
    dynamic_array_add(keywords, new_keyword_entry(TOKEN_TYPE_KEYWORD_FOR, "for"));
    dynamic_array_add(keywords, new_keyword_entry(TOKEN_TYPE_KEYWORD_IF, "if"));
    dynamic_array_add(keywords, new_keyword_entry(TOKEN_TYPE_KEYWORD_INT, "int"));
    dynamic_array_add(keywords, new_keyword_entry(TOKEN_TYPE_KEYWORD_RETURN, "return"));
    dynamic_array_add(keywords, new_keyword_entry(TOKEN_TYPE_KEYWORD_STRUCT, "struct"));
    dynamic_array_add(keywords, new_keyword_entry(TOKEN_TYPE_KEYWORD_SWITCH, "switch"));
    dynamic_array_add(keywords, new_keyword_entry(TOKEN_TYPE_KEYWORD_TYPEDEF, "typedef"));
    dynamic_array_add(keywords, new_keyword_entry(TOKEN_TYPE_KEYWORD_VOID, "void"));
    dynamic_array_add(keywords, new_keyword_entry(TOKEN_TYPE_KEYWORD_WHILE, "while"));
}

token_type_t lexical_analyzer_get_keyword(const dynamic_array_t *keywords, string_buffer_t *string_buffer) {
    int i;
    for (i = 0; i < keywords->size; i++) {
        const keyword_entry_t *keyword_entry = keywords->array[i];
        if (string_cmp(string_buffer->buffer, keyword_entry->keyword) == 0) {
            return keyword_entry->token_type;
        }
    }
    return TOKEN_TYPE_IDENTIFIER;
}

dynamic_array_t* lexical_analysis(file_buffer_t *file_buffer) {
    // Initial setup of all control variables
    string_buffer_t *string_buffer = string_buffer_new(STRING_BUFFER_DEFAULT_CAPACITY);
    dynamic_array_t *output = dynamic_array_new();
    int skip_next_char = 0;
    char next_char = '\0';

    state_machine_params_t state_machine_params = state_machine_params_new();
    position_t position = {1, 1, 1, 1};

    dynamic_array_t *keywords = dynamic_array_new();
    lexical_analyzer_setup_keywords(keywords);

    while (file_buffer_is_eof(file_buffer) == 0) {
        if (skip_next_char == 0) {
            next_char = file_buffer_read_char(file_buffer);
        }

        token_type_t token_type = state_machine_run(&state_machine_params, next_char);
        skip_next_char = 0;

        // If token_type is TOKEN_TYPE_NONE, it means that the current character buffer did not match any of the state
        // machines/valid lexemes. For this reason, we exit the lexical analysis and return an error.
        if (token_type == TOKEN_TYPE_NONE) {
            string_buffer_add(string_buffer, next_char);
            position.char_end++;
            if (next_char == '\n') {
                position.char_end = 1;
                position.line_end++;
            }
            error_unknown_token(string_buffer->buffer, position);
            string_buffer_free(string_buffer);
            free_keyword_entries(keywords);
            lexical_analyzer_free(output);
            return NULL;
        }

        // If token_type is TOKEN_TYPE_UNKNOWN, it means that there is still at least one ongoing state machine. In
        // these cases, we know it's still a valid buffer for now, so we append the character to it and update position.
        if (token_type == TOKEN_TYPE_UNKNOWN) {
            string_buffer_add(string_buffer, next_char);
            position.char_end++;
            if (next_char == '\n') {
                position.char_end = 1;
                position.line_end++;
            }
            continue;
        }

        // As soon as we have a valid lexeme, we reset the control variables and set the flag to skip next character.
        state_machine_params = state_machine_params_new();
        skip_next_char = 1;

        // Identifiers could also be keywords, so we need to check for that and adjust the token type accordingly.
        if (token_type == TOKEN_TYPE_IDENTIFIER) {
            token_type = lexical_analyzer_get_keyword(keywords, string_buffer);
        }

        // Comments and whitespaces should be ignored as they do not represent valid lexemes.
        if (token_type != TOKEN_TYPE_COMMENT && token_type != TOKEN_TYPE_WHITESPACE && token_type != TOKEN_TYPE_PREPROCESSOR_DIRECTIVE) {
            lexeme_t *lexeme = new_lexeme(token_type, position, string_buffer);
            dynamic_array_add(output, lexeme);
        }

        // Update position variable, so that the new position starts, where the last/current lexeme ended.
        position.char_start = position.char_end;
        position.line_start = position.line_end;
        string_buffer_reset(string_buffer);
    }

    if (state_machine_params.comment_index > 1) {
        logger_error("Unclosed multiline comment in source code: %d:%d-%d:%d", position.line_start, position.char_start, position.line_end, position.char_end);
        string_buffer_free(string_buffer);
        free_keyword_entries(keywords);
        lexical_analyzer_free(output);
        return NULL;
    }

    if (state_machine_params.character_literal_index > 1) {
        logger_error("Unclosed character constant in source code: %d:%d-%d:%d", position.line_start, position.char_start, position.line_end, position.char_end);
        string_buffer_free(string_buffer);
        free_keyword_entries(keywords);
        lexical_analyzer_free(output);
        return NULL;    }

    string_buffer_free(string_buffer);
    free_keyword_entries(keywords);
    return output;
}

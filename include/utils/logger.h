#ifndef CCOMPILER_LOGGER_H
#define CCOMPILER_LOGGER_H
#include <lexical_analyzer/lexical_analyzer.h>

typedef enum logger_phase {
    LOGGER_PHASE_NONE,
    LOGGER_PHASE_LEXICAL,
    LOGGER_PHASE_SYNTAX,
    LOGGER_PHASE_SYMBOL,
    LOGGER_PHASE_SEMANTIC,
    LOGGER_PHASE_INTERMEDIATE,
    LOGGER_PHASE_CODE_GEN,
} logger_phase_t;

static logger_phase_t logger_phase = LOGGER_PHASE_NONE;

void logger_set_phase(logger_phase_t phase);
void logger_debug(char *message, ...);
void logger_error(char *message, ...);
void logger_error_pos(char *message, position_t pos, ...);
void logger_info(char *message, ...);
void logger_warning(char *message, ...);
#endif //CCOMPILER_LOGGER_H
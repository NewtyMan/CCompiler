#include <utils/logger.h>
#include <stdarg.h>
#include <stdio.h>

void logger_debug(char *message, ...) {
    va_list args;
    printf("[DEBUG] ");
    va_start(args, message);
    vprintf(message, args);
    va_end(args);
    printf("\n");
}

void logger_error(char *message, ...) {
    va_list args;
    printf("[ERROR] ");
    va_start(args, message);
    vprintf(message, args);
    va_end(args);
    printf("\n");
}

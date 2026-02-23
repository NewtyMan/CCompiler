#ifndef CCOMPILER_MINIC_H
#define CCOMPILER_MINIC_H
const int STDOUT_FILENO = 0;

void write(int fd, char *buf, int count);
int read(int fd, char *buf, int count);

const int O_RDONLY = 0;
const int O_WRONLY = 1;
const int O_RDWR = 2;

int open(char *filename, int flags, int mode);
int close(int fd);

void itoa(int num, char *dst);
int strlen(char *str);

void getrandom(void *buf, int size, int flags);
#endif //CCOMPILER_MINIC_H
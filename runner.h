#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <semaphore.h>
#include <string.h>
char *mmap_file(char *jobfile, size_t *sizep);
#define MAXLINE 1024

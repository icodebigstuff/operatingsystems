#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <string.h>

// memory map a whole file and report its size
char *mmap_file(char *jobfile, size_t *sizep){
  int jobfd = open(jobfile, O_RDWR);
  struct stat stat_buf;
  fstat(jobfd, &stat_buf);                   // get stats on the open file such as size
  int size = stat_buf.st_size;               // size for mmap()'ed memory is size of file

  char *file_chars =                         // pointer to file contents
    mmap(NULL, size, PROT_READ | PROT_WRITE, // call mmap with given size and file descriptor
         MAP_SHARED, jobfd, 0);              // read/write, potentially share, offset 0
  *sizep = size;
  return file_chars;
}

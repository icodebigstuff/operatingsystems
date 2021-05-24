// filestats.c: demonstrate the stat() / lstat() system call

#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/sysmacros.h>
#include <dirent.h>
#include <string.h>
#include <unistd.h>

#define NAMELEN 2048
#define BUFSIZE 1024

#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>

char *strmode(mode_t mode);                                    // from strmode.c

int main(int argc, char *argv[]) {
  struct stat sb;

  if (argc != 2) {
    fprintf(stderr, "Usage: %s <pathname>\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  if (lstat(argv[1], &sb) == -1) {                             // call to lstat(), fills sb, distinguishes symbolic links 
    perror("stat");
    exit(EXIT_FAILURE);
  }

  printf("Filename: %s\n",argv[1]);
  printf("File type:                ");
  if(0){}                                                      // checks to determine file type
  else if S_ISBLK (sb.st_mode) {   printf("block device\n");     }
  else if S_ISCHR (sb.st_mode) {   printf("character device\n"); }
  else if S_ISDIR (sb.st_mode) {   printf("directory\n");        }
  else if S_ISFIFO(sb.st_mode) {   printf("FIFO/pipe\n");        }
  else if S_ISLNK (sb.st_mode) {   printf("symlink\n");          }
  else if S_ISREG (sb.st_mode) {   printf("regular file\n");     }
  else if S_ISSOCK(sb.st_mode) {   printf("socket\n");           }
  else{                            printf("unknown?\n");         }
  
  // Use various fields to display information
  printf("I-node number:            %ld\n"               , (long) sb.st_ino);
  printf("Permissions:              %lo (octal)\n"       , (unsigned long) sb.st_mode);
  printf("Link count:               %ld\n"               , (long) sb.st_nlink);
  printf("Ownership:                UID=%ld   GID=%ld\n" , (long) sb.st_uid, (long) sb.st_gid);
  printf("Preferred I/O block size: %ld bytes\n"         , (long) sb.st_blksize);
  printf("File size:                %lld bytes\n"        , (long long) sb.st_size);
  printf("Blocks allocated:         %lld\n"              , (long long) sb.st_blocks);

  // Print times of last access
  printf("Last status change:       %s", ctime(&sb.st_ctime)); // formats time as a string
  printf("Last file access:         %s", ctime(&sb.st_atime)); // fields like st_atime are of 
  printf("Last file modification:   %s", ctime(&sb.st_mtime)); // type time_t taken by ctime()

  char *str_mode = strmode(sb.st_mode);                        // use strmode() form strmode.c to construct permissions string
  printf("Permissions:              %s\n", str_mode);

  // Optionally report device number
  printf("ID of containing device:  [%lx,%lx]\n", (long) major(sb.st_dev), (long) minor(sb.st_dev));

  exit(EXIT_SUCCESS);
}

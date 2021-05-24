// nftw_listsize.c: List all files in a directory structure. Uses
// nftw() to visit all files in a file tree recursively adding them to
// an array. Sorts the array by file size then prints all files out
// ordered by size.

#include "headers.h"

#define MAX_FDS  20          // maximum file descriptors to be used by nftw()

typedef struct {             // TYPE for tracking file/sizes
  char *name;                // malloc()'d via strdup()
  size_t size;               // size in bytes
} file_t;

file_t *files_arr = NULL;    // global array of files, allocate in main(), expand while adding files
int files_count = -1;        // count of files in the files array
int files_capacity = -1;     // capacity of the array

// Used with nftw(). If a file is NOT a regular file as per S_ISREG(),
// it is ignored. Regular files are appended to the the global 'files'
// array. Checks if the 'files' array is full and if so uses realloc()
// array to expand the array by doubling in size. As files are added,
// uses strdup() to duplicate their names into heap memory as the
// 'filename' argument below is likely unstable; these strings must be
// free()'d later.
int add_file(const char *filename, const struct stat *sb,
             const int unused,     struct FTW *ignore2)
{
  // check if file is a Regular file using the 'sb' parameter
  if (!S_ISREG(sb->st_mode)) {
    return 0;                   // skip anything but regular files
  }

  //check if 'files_arr' is at capacity and double it size if so
  if(files_count == files_capacity){
    files_capacity*=2;
    files_arr =  realloc(files_arr,sizeof(file_t)*files_capacity); // Double size of files_arr[] via realloc
  }

  // copy name/size of the current file into files_arr[]
  char *fname = strdup(filename);
  size_t fsize = sb->st_size;
  file_t f;
  f.name = fname;
  f.size = fsize;
  files_arr[files_count] = f;// strdup() filename
 // copy file size
  files_count++; // update count


  return 0;
}


// Compare two file_t's based on their size field to indicate sorting
// order. Used with qsort() to order files smallest to largest.
int cmp_filesize(file_t *x, file_t *y){
  return x->size - y->size;
}

// main() function which calls nftw() with add_file() then sorts the
int main(int argc, char *argv[]) {
  char *filename = ".";                           // default to traversing current directory
  if(argc > 1){                                   // or take command line argument as dir to list
    filename = argv[1];
  }
  int len = strlen(filename);
  if(filename[len-1] == '/'){                     // chop trailing slash if present
    filename[len-1] = '\0';
  }

  files_arr = malloc(sizeof(file_t)*8);           // malloc() initial space to the array
  files_count = 0;                                // no files yet
  files_capacity = 8;                             // initial capacity of 8

  // call nftw() with add_file() to fill in files_arr[]
  nftw(filename,add_file,MAX_FDS,FTW_PHYS);


  // call quicksort() with ((cmp_t) cmp_filesize) to sort by file size
  qsort((void*)files_arr,files_count,sizeof(file_t), (cmp_t) cmp_filesize);


  // Print files_arr[] which is now ordered by file size
  for(int i=0; i<files_count; i++){
    printf("%8lu %s\n", files_arr[i].size, files_arr[i].name);
  }

  // Free strdup()'d names of each file and the array
  for(int i=0; i<files_count; i++){
    free(files_arr[i].name);
  }
  free(files_arr);

  return 0;
}

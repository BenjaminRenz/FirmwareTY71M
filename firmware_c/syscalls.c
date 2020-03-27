//#include <sys/stat.h>
extern char _HeapPlaceholderStart; /* Defined by the linker */
extern char _HeapPlaceholderEnd; /* Defined by the linker */

char *current_heap_end = 0;
char* _sbrk(int incr) {

      char* prev_heap_end;

      if (current_heap_end == 0) {      //Initialize with value from linker
        current_heap_end = &_HeapPlaceholderStart
        ;
      }
      prev_heap_end = current_heap_end;

      if (current_heap_end + incr > &_HeapPlaceholderEnd) {
          /* Heap and stack collision */
          return (char*) -1;
      }

      current_heap_end += incr;
      return (char*) prev_heap_end;
}

void* memset(void* dest,int ch,unsigned int n){
    void* destreturnp=dest;
    while(n--){
        *((unsigned char*)dest++)=(unsigned char)ch;
    }
    return destreturnp;
}

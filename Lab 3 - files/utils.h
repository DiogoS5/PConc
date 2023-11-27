#define MAX_LENGTH 100
#include <time.h>

void strlower(char *p){
    while (*p){
        *p = tolower(*p);
        p++; 
    }
}
struct timespec diff_timespec(const struct timespec *time1,
    const struct timespec *time0) {
  assert(time1);
  assert(time0);
  struct timespec diff = {.tv_sec = time1->tv_sec - time0->tv_sec, //
      .tv_nsec = time1->tv_nsec - time0->tv_nsec};
  if (diff.tv_nsec < 0) {
    diff.tv_nsec += 1000000000; // nsec/sec
    diff.tv_sec--;
  }
  return diff;
}

typedef struct word_info{
      char * word;
      int count;
      struct word_info* next;
} word_info;

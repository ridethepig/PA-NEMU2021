#include <sys/time.h>
#include <stdio.h>
// #include <NDL.h>
int main() {
    int count = 0;
    struct timeval old_tv, cur_tv;
    struct timezone tz;
    unsigned long old_usec, cur_usec;
    gettimeofday(&old_tv, &tz);
    old_usec = old_tv.tv_sec * 1000000 + old_tv.tv_usec;
    while (count < 10) {
      gettimeofday(&cur_tv, &tz);
      unsigned long cur_usec = cur_tv.tv_sec * 1000000 + cur_tv.tv_usec;
      if (cur_usec - old_usec > 500000) {
        old_usec = cur_usec;
        printf("tick %d\n", count);
        count ++;
      }
    }
    // NDL_Init(0);
    // unsigned int old_ms = NDL_GetTicks();
    // unsigned int cur_ms = 0;
    // count = 0;
    // while (count < 40) {
    //     cur_ms = NDL_GetTicks();
    //     if (cur_ms - old_ms > 500) {
    //         old_ms = cur_ms;
    //         printf("ndl tick %d\n", count);
    //         count ++;
    //     }
    // }
    return 0;
}
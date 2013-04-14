#ifdef __APPLE__
#include <mach/clock.h>
#include <mach/mach.h>
#include <mach/mach_time.h>
#elif
#include <time.h>
#endif

unsigned long long int get_time_milliseconds(){
#ifdef __APPLE__
  mach_timebase_info_data_t info;
  unsigned long long int time;
  
  time = mach_absolute_time();

  mach_timebase_info(&info);
  time = time * info.numer / info.denom / 1000000;
  return time;
#elif __linux__
  struct timespec tp;
  if(clock_gettime(CLOCK_MONOTONIC_RAW, &tp) != 0){
    perror("clock_gettime");
    exit(1);
  }
  return (((unsigned long long)tp.tv_sec) * 1000) + (((unsigned long long)tp.tv_nsec) / 1000000);
#endif
}

#include "time_utils.h"
#include <sys/time.h>
#include <time.h>


//static __inline__ uint64_t rdtsc(void)
//{
//    unsigned hi, lo;
//    __asm__ __volatile__ ("rdtsc" : "=a"(lo), "=d"(hi));
//    return ( lo)|(((uint64_t)hi)<<32);
//}

const char *get_time_str(time_t t, bool include_date) {
    static char time_str_cache[]="XX-XX-XX ZZ:ZZ:ZZ  ";
    static time_t t_cache = -1;
    if (t != t_cache) {
        t_cache = t;
        if (include_date) {
            strftime(time_str_cache, sizeof(time_str_cache)-1, "%y-%m-%d %H:%M:%S", localtime(&t));
        } else {
            strftime(time_str_cache, sizeof(time_str_cache)-1, "%H:%M:%S", localtime(&t));
        }
    }
    return time_str_cache;
}


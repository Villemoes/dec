#ifndef TIMING_H
#define TIMING_H

#if USE_RDTSC && (defined(__i386__) || defined(__x86_64__))

typedef unsigned long long abs_time_t;
#define TIME_UNIT "cycles"
static inline signed long long
time_diff(const abs_time_t *a, const abs_time_t *b)
{
	return *a - *b;
}
static inline unsigned long long rdtsc(void)
{
#if defined(__i386__)
	unsigned long long int x;
	__asm__ volatile (".byte 0x0f, 0x31" : "=A" (x));
	return x;
#else
	unsigned hi, lo;
	__asm__ __volatile__ ("rdtsc" : "=a"(lo), "=d"(hi));
	return ( (unsigned long long)lo)|( ((unsigned long long)hi)<<32 );
#endif
}
#define get_time(val) (val = rdtsc())

#elif 1

#include <time.h>
typedef struct timespec abs_time_t;
#define TIME_UNIT "nsecs"
static inline signed long long
time_diff(const abs_time_t *a, const abs_time_t *b)
{
	return 1000000000LL*(a->tv_sec - b->tv_sec) + (a->tv_nsec - b->tv_nsec);
}
#define get_time(val) (clock_gettime(CLOCK_MONOTONIC, &val))

#else

#error "please provide some suitable high-resolution timing interface"

#endif

#endif /* TIMING_H */

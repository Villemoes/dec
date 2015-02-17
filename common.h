#ifndef COMMON_H
#define COMMON_H

#include <stdint.h>

#define ARRAY_SIZE(a) (sizeof(a)/sizeof(a[0]))

#define u16 uint16_t
#define u32 uint32_t
#define u64 uint64_t

#define noinline_for_stack __attribute__((__noinline__))
#define __noinline __attribute__((__noinline__))

char *linux_put_dec(char *buf, unsigned long long n);
char *rv_put_dec(char *buf, unsigned long long n);



#endif /* COMMON_H */

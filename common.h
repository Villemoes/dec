#ifndef COMMON_H
#define COMMON_H

#include <stdint.h>
#include <endian.h>

/* glibc's htole16 doesn't work in initializers, so we roll our own. */
#if __BYTE_ORDER == __LITTLE_ENDIAN
#define cpu_to_le16(x) (x)
#else
#define cpu_to_le16(x) ((((x) & 0xff) << 8) | (((x) >> 8) & 0xff))
#endif

#define ARRAY_SIZE(a) (sizeof(a)/sizeof(a[0]))

#define u16 uint16_t
#define u32 uint32_t
#define u64 uint64_t

#define noinline_for_stack __attribute__((__noinline__))
#define __noinline __attribute__((__noinline__))

char *linux_put_dec(char *buf, unsigned long long n);
char *rv_put_dec(char *buf, unsigned long long n);



#endif /* COMMON_H */

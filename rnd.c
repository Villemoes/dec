#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <assert.h>
#include "rnd.h"

static unsigned short state[3];

void rnd_init(u64 seed)
{
	if (!seed) {
		int fd = open("/dev/urandom", O_RDONLY);
		if (fd < 0)
			exit(1);
		if (read(fd, &seed, sizeof(seed)) < (int)sizeof(seed))
			exit(1);
		close(fd);
	}

	state[0] = seed;
	state[1] = seed >> 16;
	state[2] = seed >> 32;
}

u32 rnd_u32(void)
{
	/*
	 * jrand48 returns longs with values between -2^31 and
	 * 2^31-1. Implicitly casting such a value to u32 will DTRT.
	 */
	return jrand48(state);
}
u64 rnd_u64(void)
{
	return (((u64)rnd_u32()) << 32) | rnd_u32();
}
double rnd_double(void)
{
	return erand48(state);
}
/*
 * This is kind of "truncated negative binomial distribution with
 * offset", more suitable for our purposes.
 */
int rnd_neg_binom(double param, int lo, int hi)
{
	int x;

	assert(0.0 < param && param < 1.0);
	assert(lo < hi);
	while (1) {
		for (x = lo; x < hi; ++x) {
			if (rnd_double() < param)
				return x;
		}
	}
}

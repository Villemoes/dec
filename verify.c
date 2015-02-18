#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <pthread.h>
#include <string.h>

#include "common.h"

#ifndef NTHR
#define NTHR 8
#endif

#define LO_START 10ULL
#define LO_STOP  10000000000ULL
#define HI_START ULLONG_MAX
#define HI_STOP  ULLONG_MAX - LO_STOP


static void *check(void *arg)
{
	char buf1[24];
	char buf2[24];
	int len1, len2;

	unsigned long idx = (unsigned long)arg;
	unsigned long long n;

	for (n = LO_START; n % NTHR != idx; ++n)
		;
	for (; n <= LO_STOP; n += NTHR) {
		len1 = linux_put_dec(buf1, n) - buf1;
		len2 = rv_put_dec(buf2, n) - buf2;
		if (len1 != len2 || memcmp(buf1, buf2, len1)) {
			fprintf(stderr, "n = %llu; linux: %.*s; rv: %.*s\n",
				n, len1, buf1, len2, buf2);
			return (void*) -1;
		}
	}

	for (n = HI_START; n % NTHR != idx; --n)
		;
	for (; n >= HI_STOP; n -= NTHR) {
		len1 = linux_put_dec(buf1, n) - buf1;
		len2 = rv_put_dec(buf2, n) - buf2;
		if (len1 != len2 || memcmp(buf1, buf2, len1)) {
			fprintf(stderr, "n = %llu; linux: %.*s; rv: %.*s\n",
				n, len1, buf1, len2, buf2);
			return (void*) -1;
		}
	}

	return NULL;
}


int main(void)
{
	pthread_t thr[NTHR];
	int i, e, ret;
	void *res;

	for (i = 0; i < NTHR; ++i) {
		e = pthread_create(&thr[i], NULL, check, (void*)(long)i);
		if (e)
			exit(1);
	}

	ret = 0;
	for (i = 0; i < NTHR; ++i) {
		e = pthread_join(thr[i], &res);
		if (e)
			exit(1);
		if (res != NULL)
			ret = 1;
	}
	
	return ret;
}

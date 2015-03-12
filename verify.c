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

static unsigned long long lenfreq[NTHR][32];

static int do_check(unsigned long long n, unsigned idx)
{
	char buf1[24];
	char buf2[24];
	int len1, len2;

	len1 = linux_put_dec(buf1, n) - buf1;
	len2 = rv_put_dec(buf2, n) - buf2;
	if (len1 != len2 || memcmp(buf1, buf2, len1)) {
		fprintf(stderr, "n = %llu; linux: %.*s; rv: %.*s\n",
			n, len1, buf1, len2, buf2);
		return -1;
	}
	lenfreq[idx][len1]++;
	return 0;
}

static void *check(void *arg)
{
	unsigned long idx = (unsigned long)arg;
	unsigned long long n;

	for (n = LO_START; n % NTHR != idx; ++n)
		;
	for (; n <= LO_STOP; n += NTHR) {
		if (do_check(n, idx))
			return (void*) -1;
	}
	printf("Thread %lu: low range ok\n", idx);

	for (n = HI_START; n % NTHR != idx; --n)
		;
	for (; n >= HI_STOP; n -= NTHR) {
		if (do_check(n, idx))
			return (void*) -1;
	}
	printf("Thread %lu: high range ok\n", idx);

	/*
	 * This will also visit a few one-digit numbers, but both the
	 * old and new code actually handle that just fine for
	 * non-zero n (it's just irrelevant because all callers of
	 * put_dec take a shortcut for n < 10).
	 */
	n = 2*idx + 1;
	do {
		if (do_check(n, idx))
			return (void*) -1;
		n *= 17179869185ull;
	} while (n != 2*idx + 1);
	printf("Thread %lu: mid range ok\n", idx);

	return NULL;
}


int main(void)
{
	pthread_t thr[NTHR];
	int i, e, ret;
	void *res;

	printf("Using %d threads\n", NTHR);
	printf("Checking [%llu, %llu] and [%llu, %llu]\n",
		LO_START, LO_STOP, HI_STOP, HI_START);
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

	printf("Distribution of lengths checked:\n");
	for (e = 1; e <= 20; ++e) {
		unsigned long long s = 0;
		for (i = 0; i < NTHR; ++i)
			s += lenfreq[i][e];
		printf("%d\t%llu\n", e, s);
	}
	
	return ret;
}

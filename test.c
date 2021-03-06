#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <assert.h>

#include "common.h"
#include "timing.h"
#include "rnd.h"

#define REPS 1000

static volatile sig_atomic_t done;
static u64 number[2048];
static char dist_name[64];

#define TOTAL_CONV (REPS * ARRAY_SIZE(number))

/*
 * To prevent gcc from optimizing the function calls away, we
 * accumulate the combined lengths into this global dummy.
 */
static unsigned long dummy; 

static void set_done(int s)
{
	(void) s;
	done = 1;
}

struct result {
	const char         *name;
	signed long long   time;
	unsigned long long conversions;
};

static struct result linux_result, rv_result;

static void fill_uniform(void)
{
	unsigned i;

	snprintf(dist_name, sizeof(dist_name), "uniform([10, 2^64-1])");
	for (i = 0; i < ARRAY_SIZE(number); ++i) {
		do {
			number[i] = rnd_u64();
		} while (number[i] < 10);
	}
}

static void fill_neg_binom(double param)
{
	unsigned i;

	snprintf(dist_name, sizeof(dist_name), "3 + neg_binom(%.2f)", param);
	for (i = 0; i < ARRAY_SIZE(number); ++i) {
		unsigned idx;
		u64 mask;
		u64 msb;

		/*
		 * We want the MSB to be somewhere between 3 and 63,
		 * with smaller numbers more likely than larger. The
		 * negative binomial distribution provides such
		 * behaviour.
		 */
		idx = rnd_neg_binom(param, 3, 64);
		msb = 1ULL << idx;
		mask = (msb << 1) - 1; /* this will DTRT even if idx==63 */
		do {
			number[i] = rnd_u64();
			number[i] &= mask;
			number[i] |= msb;
		} while (number[i] < 10);
	}
}

static void _do_test(const char *name, char* (*func)(char *, unsigned long long), struct result *res)
{
	char buf[24];
	abs_time_t start, stop;
	unsigned long long iter;
	unsigned total, r, i;

	total = 0;
	get_time(start);
	for (r = 0; r < REPS; ++r) {
		for (i = 0; i < ARRAY_SIZE(number); ++i) {
			total += func(buf, number[i]) - buf;
		}
	}
	get_time(stop);

	done = 0;
	signal(SIGALRM, set_done);
	alarm(1);
	for (iter = 0; !done; ++iter) {
		total += func(buf, number[iter % ARRAY_SIZE(number)]) - buf;
	}

	res->name = name;
	res->time = time_diff(&stop, &start);
	res->conversions = iter;
	dummy += total;
}

#define do_test(prefix) _do_test(#prefix "_put_dec", prefix ## _put_dec, & prefix ## _result)

static void report(const struct result *res)
{
	printf("%-25s %-16s %12.2f %16llu\n",
		dist_name, res->name, (double)res->time/TOTAL_CONV, res->conversions);
}
static void delta(const struct result *r0, const struct result *r1)
{
	double time_delta, conv_delta;

	time_delta = (double)r1->time - (double)r0->time;
	time_delta /= r0->time;
	time_delta *= 100;
	conv_delta = (double)r1->conversions - (double)r0->conversions;
	conv_delta /= r0->conversions;
	conv_delta *= 100;

	printf("%-25s %-16s %+11.2f%% %+15.2f%%\n", "", "+/-", time_delta, conv_delta);
}

static void compare(void)
{
	do_test(linux);
	do_test(rv);
	report(&linux_result);
	report(&rv_result);
	delta(&linux_result, &rv_result);
}

int main(int argc, char *argv[])
{
	if (LONG_BIT != 8*sizeof(long)) {
		fprintf(stderr, "error: compiled with LONG_BIT=%d but sizeof(long) = %zu\n",
			LONG_BIT, sizeof(long));
		exit(1);
	}

	rnd_init(argc > 1 ? atoi(argv[1]) : 0);

	printf("%-25s %-16s %-12s %-16s\n", "Distribution", "Function", TIME_UNIT "/conv", "Conv/1 sec");

	fill_uniform();
	compare();

	fill_neg_binom(0.05);
	compare();

	fill_neg_binom(0.10);
	compare();

	fill_neg_binom(0.15);
	compare();

	fill_neg_binom(0.20);
	compare();

	fill_neg_binom(0.50);
	compare();

	return dummy == 12345678ul;
}

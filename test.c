#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <assert.h>
#include <gsl/gsl_rng.h>
#include <gsl/gsl_randist.h>
#include <rdtsc.h>

#include "common.h"

#define REPS 1000

static sig_atomic_t done;
static u64 number[2048];
static gsl_rng *rng;
static char dist_name[64];
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
	unsigned long long cycles;
	unsigned long long conversions;
};

static struct result linux_result, rv_result;

/*
 * Not a fair comparison since this also needs to decode the format
 * etc. Note that this also does output in the "right" (big-endian)
 * order, contrary to the other put_dec.
 */
#ifdef DO_GLIBC
static struct result glibc_result;
static char*
glibc_put_dec(char *buf, unsigned long long n)
{
	return buf + sprintf(buf, "%llu", n);
}
#endif

static u64 rand_u64(void)
{
	u64 lo = gsl_rng_get(rng);
	u64 hi = gsl_rng_get(rng);
	return lo ^ (hi << 32);
}


static void fill_uniform(void)
{
	unsigned i;

	snprintf(dist_name, sizeof(dist_name), "uniform([10, 2^64-1])");
	for (i = 0; i < ARRAY_SIZE(number); ++i) {
		do {
			number[i] = rand_u64();
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
		do {
			idx = 3 + gsl_ran_negative_binomial(rng, param, 1);
		} while (idx >= 64);
		msb = 1ULL << idx;
		mask = (msb << 1) - 1; /* this will DTRT even if idx==63 */
		do {
			number[i] = rand_u64();
			number[i] &= mask;
			number[i] |= msb;
		} while (number[i] < 10);
	}
}

static void _do_test(const char *name, char* (*func)(char *, unsigned long long), struct result *res)
{
	char buf[24];
	unsigned long long start, stop;
	unsigned long long iter;
	unsigned total, r, i;

	total = 0;
	start = rdtsc();
	for (r = 0; r < REPS; ++r) {
		for (i = 0; i < ARRAY_SIZE(number); ++i) {
			total += func(buf, number[i]) - buf;
		}
	}
	stop = rdtsc();

	done = 0;
	signal(SIGALRM, set_done);
	alarm(1);
	for (iter = 0; !done; ++iter) {
		total += func(buf, number[iter % ARRAY_SIZE(number)]) - buf;
	}

	res->name = name;
	res->cycles = stop-start;
	res->conversions = iter;
	dummy += total;
}

#define do_test(prefix) _do_test(#prefix "_put_dec", prefix ## _put_dec, & prefix ## _result)

static void report(const struct result *res)
{
	printf("%-25s %-20s %12llu %20llu\n", dist_name, res->name, res->cycles, res->conversions);
}
static void delta(const struct result *r0, const struct result *r1)
{
	double cyc_delta, conv_delta;

	cyc_delta = (double)r1->cycles - (double)r0->cycles;
	cyc_delta /= r0->cycles;
	cyc_delta *= 100;
	conv_delta = (double)r1->conversions - (double)r0->conversions;
	conv_delta /= r0->conversions;
	conv_delta *= 100;

	printf("%-25s %-20s %+11.2f%% %+19.2f%%\n", "", "+/-", cyc_delta, conv_delta);
}

static void compare(void)
{
	do_test(linux);
	do_test(rv);
	report(&linux_result);
	report(&rv_result);
	delta(&linux_result, &rv_result);
#ifdef DO_GLIBC
	do_test(glibc);
	report(&glibc_result);
	delta(&linux_result, &glibc_result);
#endif
}

int main(int argc, char *argv[])
{
	rng = gsl_rng_alloc(gsl_rng_mt19937);
	assert(gsl_rng_min(rng) == 0);
	assert(gsl_rng_max(rng) == UINT32_MAX);

	printf("%-25s %-20s %-12s %-20s\n", "Distribution", "Function", "Cycles", "Conversions/1 sec");

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

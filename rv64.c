#include "common.h"

static const u16 two_char[100] = {
#define _(x) (((x % 10) | ((x / 10) << 8)) + 0x3030)
	_( 0), _( 1), _( 2), _( 3), _( 4), _( 5), _( 6), _( 7), _( 8), _( 9),
	_(10), _(11), _(12), _(13), _(14), _(15), _(16), _(17), _(18), _(19),
	_(20), _(21), _(22), _(23), _(24), _(25), _(26), _(27), _(28), _(29),
	_(30), _(31), _(32), _(33), _(34), _(35), _(36), _(37), _(38), _(39),
	_(40), _(41), _(42), _(43), _(44), _(45), _(46), _(47), _(48), _(49),
	_(50), _(51), _(52), _(53), _(54), _(55), _(56), _(57), _(58), _(59),
	_(60), _(61), _(62), _(63), _(64), _(65), _(66), _(67), _(68), _(69),
	_(70), _(71), _(72), _(73), _(74), _(75), _(76), _(77), _(78), _(79),
	_(80), _(81), _(82), _(83), _(84), _(85), _(86), _(87), _(88), _(89),
	_(90), _(91), _(92), _(93), _(94), _(95), _(96), _(97), _(98), _(99),
#undef _
};

static __noinline
char *rv_put_dec_full8(char *buf, unsigned r)
{
	*((uint16_t*)buf) = two_char[r % 100];
	r /= 100;
	buf += 2;
	*((uint16_t*)buf) = two_char[r % 100];
	r /= 100;
	buf += 2;
	*((uint16_t*)buf) = two_char[r % 100];
	r /= 100;
	buf += 2;
	*((uint16_t*)buf) = two_char[r];
	buf += 2;
	return buf;
}

static __noinline
char *rv_put_dec_trunc8(char *buf, unsigned r)
{
	if (r < 100)
		goto out;
	*((uint16_t*)buf) = two_char[r % 100];
	r /= 100;
	buf += 2;
	if (r < 100)
		goto out;
	*((uint16_t*)buf) = two_char[r % 100];
	r /= 100;
	buf += 2;
	if (r < 100)
		goto out;
	*((uint16_t*)buf) = two_char[r % 100];
	r /= 100;
	buf += 2;

out:
	*((uint16_t*)buf) = two_char[r];
	buf += 2;
	if (buf[-1] == '0')
		buf--;
	return buf;
}

# define do_div(n,base) ({                                      \
        uint32_t __base = (base);                               \
        uint32_t __rem;                                         \
        __rem = ((uint64_t)(n)) % __base;                       \
        (n) = ((uint64_t)(n)) / __base;                         \
        __rem;                                                  \
 })

char *
rv_put_dec(char *buf, unsigned long long n)
{
	if (n >= 100*1000*1000)
		buf = rv_put_dec_full8(buf, do_div(n, 100*1000*1000));
	/* 1 <= n <= 1.6e11 */
	if (n >= 100*1000*1000)
		buf = rv_put_dec_full8(buf, do_div(n, 100*1000*1000));
	/* 1 <= n < 1e8 */
	buf = rv_put_dec_trunc8(buf, n);
	return buf;
}

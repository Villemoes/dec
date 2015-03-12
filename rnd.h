#ifndef RND_H
#define RND_H

#include "common.h"

void rnd_init(u64 seed);

u32 rnd_u32(void);
u64 rnd_u64(void);
double rnd_double(void);
int rnd_neg_binom(double param, int lo, int hi);

#endif /* RND_H */

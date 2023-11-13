/*  Adapted from the code included on Sebastiano Vigna's website */

#include "os.h"

#define FACT 2.32830643653869628906e-10

uint64_t rol64(uint64_t x, int k)
{
	return (x << k) | (x >> (64 - k));
}

struct xoshiro256ss_state {
	uint64_t s[4];
} state = {{1, 2, 3, 4}};

uint64_t xoshiro256ss(struct xoshiro256ss_state *state)
{
	uint64_t *s = state->s;
	uint64_t const result = rol64(s[1] * 5, 7) * 9;
	uint64_t const t = s[1] << 17;

	s[2] ^= s[0];
	s[3] ^= s[1];
	s[1] ^= s[2];
	s[0] ^= s[3];

	s[2] ^= t;
	s[3] = rol64(s[3], 45);

	return result;
}

uint32_t randx(void)
{
	return xoshiro256ss(&state);
}

void srandx(uint32_t seed)
{
	uint32_t mix_seed = seed + 0x9e3779b9;
	state.s[0] = seed;
	state.s[1] = mix_seed;
	state.s[2] = seed;
	state.s[3] = mix_seed;
	xoshiro256ss(&state);
}

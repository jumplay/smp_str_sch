#include "stdafx.h"
#include <stdint.h>

#define CHECK_SA 0

template <typename uint_t>
__forceinline bool cmp_ueq_len_uint_str(const uint_t* a, const uint_t* b)
{
	while (*a++ == *b++) {}
	return a[-1] < b[-1];
}

template <typename uint_t>
bool check_suffix_array(uint_t* p, uint32_t n, uint32_t* s_a)
{
	for (uint32_t i = 1; i < n; i++) {
		if (!cmp_ueq_len_uint_str(p + s_a[i - 1], p + s_a[i])) {
			return false;
		}
	}
	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////
//require:
//    0 come up to the end of string
////////////////////////////////////////////////////////////////////////////////////////////
template <typename uint_t>
void build_sa(uint_t* p, uint32_t n, uint32_t* s_a, uint32_t* tmp_buf)
{
	for (uint32_t i = 0; i < n; i++) {
		s_a[i] = i;
	}
	
	for (uint32_t i = 1; i <= n; i++) {
		for (uint32_t m = 1; !(i & m); m <<= 1) {
			uint32_t* t = s_a + i - (m << 1);
			memcpy(tmp_buf, t, sizeof(uint32_t) * (m << 1));
			uint32_t* t1 = tmp_buf;
			uint32_t* t2 = t1 + m;
			uint32_t b1 = 0;
			uint32_t b2 = 0;
			while (b1 < m && b2 < m) {
				if (cmp_ueq_len_uint_str(p + t1[b1], p + t2[b2])) {
					*t++ = t1[b1++];
				}
				else {
					*t++ = t2[b2++];
				}
			}
			while (b1 < m) { *t++ = t1[b1++]; }
			while (b2 < m) { *t++ = t2[b2++]; }
		}
	}

	uint32_t m1 = 1;
	while (!(n & m1) && m1 < n) { m1 <<= 1; }
	uint32_t m2 = m1;
	
	while (1) {
		m1 <<= 1;
		while (!(n & m1) && m1 < n) { m1 <<= 1; }
		if (m1 > n) { break; }

		uint32_t* t = s_a + n - m1 - m2;
		memcpy(tmp_buf, t, (m1 + m2) * sizeof(uint32_t));
		uint32_t* t1 = tmp_buf;
		uint32_t* t2 = t1 + m1;

		uint32_t b1 = 0;
		uint32_t b2 = 0;
		while (b1 < m1 && b2 < m2) {
			if (cmp_ueq_len_uint_str(p + t1[b1], p + t2[b2])) {
				*t++ = t1[b1++];
			}
			else {
				*t++ = t2[b2++];
			}
		}
		while (b1 < m1) { *t++ = t1[b1++]; }
		while (b2 < m2) { *t++ = t2[b2++]; }

		m2 += m1;
	}

#	if CHECK_SA
	if (!check_suffix_array(p, n, s_a)) {
		print_error("failed to check suffix array");
	}
#	endif
}
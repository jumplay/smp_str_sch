#include "stdafx.h"
#include <stdint.h>
#include <string.h>
#include "text.h"
#include "suffix_array.h"

bool text_t::load(const char* file_path)
{
	FILE* fp = fopen(file_path, "rb");
	if (!fp) {
		print_error("failed to open \"%s\"", file_path);
		return false;
	}

	uint8_t* read_buf;
	try {
		read_buf = new uint8_t[read_buf_size + 1];
	}
	catch (...) {
		print_error("failed to new read_buf");
		return false;
	}

	chara_cnt = 0;
	for (uint32_t read_len = 0; read_len = (uint32_t)fread(read_buf, 1, read_buf_size, fp); ) {
		const uint8_t* b = read_buf;
		const uint8_t* e = b + read_len;
		while (b < e) {
			if (*b != '\r') { chara_cnt++; }
			b += *b > 128 ? 2 : 1;
		}
		if (b != e) {
			chara_cnt--;
			if (feof(fp)) { break; }
			fseek(fp, -1, SEEK_CUR);
		}
	}

	try {
		p_chara = new uint16_t[chara_cnt + 1];
	}
	catch (...) {
		print_error("failed to new p_chara");
		p_chara = NULL;
		return false;
	}

	rewind(fp);
	chara_cnt = 0;
	for (uint32_t read_len = 0; read_len = (uint32_t)fread(read_buf, 1, read_buf_size, fp); ) {
		const uint8_t* b = read_buf;
		const uint8_t* e = b + read_len;
		while (b < e) {
			if (*b > 128) {
				p_chara[chara_cnt++] = *(uint16_t*)b;
				b += 2;
			}
			else {
				p_chara[chara_cnt++] = *b++;
				if (b[-1] == '\r') { chara_cnt--; }
			}
		}
		if (b != e) {
			chara_cnt--;
			if (feof(fp)) { break; }
			fseek(fp, -1, SEEK_CUR);
		}
	}
	p_chara[chara_cnt++] = 0;

	delete[] read_buf;

#	define DBG_TEXT_LOAD 0
#	if DBG_TEXT_LOAD
	char* b = (char*)p_chara;
	char* e = (char*)(p_chara + chara_cnt);
	char* w = b;
	while (b < e) {
		*w = *b++;
		w += *w ? 1 : 0;
	}
	*w = 0;
	print_tmp<1>("%s", (char*)p_chara);
#	endif
#	undef DBG_TEXT_LOAD

	return true;
}

void text_t::gen_bwt(uint16_t* bwt)
{
	for (uint32_t i = 0; i < chara_cnt; i++) {
		bwt[i] = p_chara[uint32_t(s_a[i] + chara_cnt - 1) % chara_cnt];
	}
}

bool text_t::gen_ba(uint16_t* bwt, uint16_t* tmp_buf)
{
	try {
		b_a = new uint8_t[((chara_cnt + 7) >> 3) << 4];
	}
	catch (...) {
		print_error("failed to new b_a");
		b_a = NULL;
		return false;
	}

	const uint32_t m = (chara_cnt + 7) >> 3;
	const uint32_t w = 16;
	uint8_t* p = b_a;

	uint16_t* b1 = bwt;
	uint16_t* b2 = (uint16_t*)tmp_buf;

	for (uint32_t i = 0; i < w; i++, p += m) {
		memset(p, 0, m);
		uint32_t c1 = 0;
		for (uint32_t j = 0; j < chara_cnt; j++) {
			p[j >> 3] |= (((b1[j] >> i) & 1) << (j & 7));
			if ((b1[j] >> i) & 1) { c1++; }
		}
		uint32_t c0 = 0;
		c1 = chara_cnt - c1;
		bit_0_cnt[i] = c1;
		if (i == w - 1) { break; }
		for (uint32_t j = 0; j < chara_cnt; j++) {
			if ((b1[j] >> i) & 1) {
				b2[c1++] = b1[j];
			}
			else {
				b2[c0++] = b1[j];
			}
		}
		uint16_t* t = b1;
		b1 = b2;
		b2 = t;
	}

	return true;
}

bool text_t::set_chara_pos()
{
	try {
		chara_pos = new uint32_t[(1 << 16) + 1];
	}
	catch (...) {
		print_error("failed to new chara_pos");
		chara_pos = NULL;
		return false;
	}

	memset(chara_pos, 0, sizeof(uint32_t) << 16);
	for (uint32_t i = 0; i < chara_cnt; i++) {
		chara_pos[p_chara[s_a[i]]]++;
	}

	uint32_t tmp = 0;
	for (uint32_t i = 0; i < (1 << 16); i++) {
		tmp += chara_pos[i];
		chara_pos[i] = tmp - chara_pos[i];
	}
	chara_pos[1 << 16] = chara_cnt;

	return true;
}

bool text_t::build_index()
{
	uint8_t* tmp_buf = NULL;
	uint32_t tmp_buf_size = chara_cnt * sizeof(uint32_t);
	try {
		tmp_buf = new uint8_t[tmp_buf_size];
	}
	catch (...) {
		print_error("failed to new tmp_buf");
		return false;
	}

	try {
		s_a = new uint32_t[chara_cnt];
	}
	catch (...) {
		print_error("failed to new s_a");
		s_a = NULL;
		return false;
	}
	build_sa(p_chara, chara_cnt, s_a, (uint32_t*)tmp_buf);

	if (!set_chara_pos()) {
		print_error("failed to set_chara_pos()");
		return false;
	}

	uint16_t* bwt = (uint16_t*)(tmp_buf);
	gen_bwt(bwt);

	if (!gen_ba(bwt, (uint16_t*)(tmp_buf + chara_cnt * sizeof(uint16_t)))) {
		print_error("failed to gen_ba()");
		return false;
	}
	
	delete[] tmp_buf;
	
	return true;
}

uint32_t text_t::cnt_le(uint16_t chara, uint32_t pos)
{
	const uint32_t w = 16;
	uint8_t* p = b_a;

	for (uint32_t i = 0; i < 16; i++) {
		uint32_t m = pos >> 3;
		uint32_t tmp = 0;

		for (uint32_t j = 0; j < m; j++) {
			tmp += bit_0_cnt_map[p[j]];
		}
		tmp += bit_0_cnt_map[uint8_t(p[m] | (0 - (1 << (pos & 7))))];

		if ((chara >> i) & 1) {
			pos = pos - tmp + bit_0_cnt[i];
		}
		else {
			pos = tmp;
		}
		p += (chara_cnt + 7) >> 3;
	}
	return pos;
}

void text_t::find(const uint16_t* p, uint32_t n)
{
	rlt_cnt = 0;
	if (!n) { return; }
	uint32_t l_b = chara_pos[p[n - 1]];
	uint32_t r_b = chara_pos[p[n - 1] + 1];

	for (uint32_t i = n - 1; l_b != r_b && i-- != 0; ) {
		l_b = cnt_le(p[i], l_b);
		r_b = cnt_le(p[i], r_b);
	}

	rlt_cnt = r_b - l_b;
	rlt_cnt = rlt_cnt < max_rlt_cnt ? rlt_cnt : max_rlt_cnt;
	for (uint32_t i = 0; i < rlt_cnt; i++) {
		rlt_pos[i] = s_a[l_b + i];
	}
}

void text_t::find(const char* s)
{
	const uint32_t buf_16_size = 100;
	uint16_t buf_16[buf_16_size];

	const uint8_t* b = (uint8_t*)s;
	const uint8_t* e = b + strlen(s);
	
	uint32_t n = 0;
	while (b < e) {
		if (*b > 128) {
			buf_16[n++] = *(uint16_t*)b;
			b += 2;
		}
		else {
			buf_16[n++] = *b++;
		}
		if (n == buf_16_size) { break; }
	}
	this->find(buf_16, n);
}

uint32_t text_t::bit_0_cnt_map[1 << 8];
void text_t::init_bit_0_cnt_map()
{
	for (uint32_t i = 0; i < 0x100; i++) {
		bit_0_cnt_map[i] = 8
			- ((i >> 0) & 1)
			- ((i >> 1) & 1)
			- ((i >> 2) & 1)
			- ((i >> 3) & 1)
			- ((i >> 4) & 1)
			- ((i >> 5) & 1)
			- ((i >> 6) & 1)
			- ((i >> 7) & 1);
	}
}
static uint32_t _init_bit_0_cnt_map = (text_t::init_bit_0_cnt_map(), 0);


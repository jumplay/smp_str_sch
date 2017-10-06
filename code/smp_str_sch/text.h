#pragma once
#include <stdio.h>
#include <stdint.h>
#include "print_log.h"

class text_t
{
	enum cfg
	{
		read_buf_size = 100,
		max_rlt_cnt = 100,
	};

	uint16_t* p_chara;
	uint32_t chara_cnt;
	uint32_t* s_a;
	uint8_t* b_a;
	uint32_t* chara_pos;
	uint32_t bit_0_cnt[16];
	static uint32_t bit_0_cnt_map[1 << 8];

	bool set_chara_pos();
	void gen_bwt(uint16_t* bwt);
	bool gen_ba(uint16_t* bwt, uint16_t* tmp_buf);

public:
	struct
	{
		uint32_t rlt_pos[max_rlt_cnt];
		uint32_t rlt_cnt;
	};

	text_t()
	{
		p_chara = NULL;
		chara_cnt = 0;
	}
	
	~text_t()
	{
		delete[] p_chara;
	}

	bool load(const char* file_path);
	bool build_index();
	void find(const uint16_t* p, uint32_t n);
	void find(const char* s);
	static void init_bit_0_cnt_map();
	uint32_t cnt_le(uint16_t chara, uint32_t pos);
};
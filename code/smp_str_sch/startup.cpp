#include "stdafx.h"
#include "print_log.h"
#include <stdint.h>
#include <new>
#include "text.h"

const char* text_path = "E:\\text.txt";

void startup()
{
	text_t text;
	if (!text.load(text_path)) {
		print_error("failed to text.load()");
		return;
	}

	if (!text.build_index()) {
		print_error("failed to build index");
		return;
	}

	text.find("");
	
	printf("find cnt: %u\n", text.rlt_cnt);
	for (uint32_t i = 0; i < text.rlt_cnt; i++) {
		printf("    %u\n", text.rlt_pos[i]);
	}
}
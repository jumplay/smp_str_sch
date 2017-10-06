#pragma once
#include <stdio.h>
#include <stdarg.h>

#define print_log(fmt,...) printf("(L) " fmt "\n", ##__VA_ARGS__);
#define print_error(fmt,...) printf("(E) %s> " fmt "\n", __FUNCTION__, ##__VA_ARGS__);

template <int idx>
void print_tmp(const char* fmt, ...)
{
	static char path[260];
	static FILE* fp = NULL;
	if (!fp) {
		sprintf(path, "E:\\tmp_%d.txt", idx);
		fp = fopen(path, "w");
		if (!fp) {
			print_error("failed to open \"%s\"", path);
			return;
		}
	}

	va_list v_l;
	va_start(v_l, fmt);
	vfprintf(fp, fmt, v_l);
	va_end(v_l);
	fflush(fp);
}
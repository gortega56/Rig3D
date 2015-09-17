#pragma once
#define RIG_SUCCESS		0
#define RIG_ERROR		1

#define CSTR2WSTR(in, out)					\
{											\
wchar_t wtext[256];							\
size_t num = 0;								\
mbstowcs_s(&num, wtext, in, strlen(in) + 1);\
out = wtext;								\
}
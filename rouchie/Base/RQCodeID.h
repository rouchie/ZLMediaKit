#pragma once

#include <iostream>
#include <unordered_map>

using code_t = int64_t;

#define CODE_LIST                  \
    X(CODE_SUCCESS,   0)           \
    X(CODE_DESTROYED, -100)        \
    X(CODE_EMPTY,     -101)

typedef enum code_id : code_t {
#define X(name, value) name = value,
    CODE_LIST
#undef X
    CODE_MAX
} CODE_ID;

inline const std::string& GetCodeID(code_t id)
{
	static std::unordered_map<code_t, std::string> g_code_id_strings = {
	#define X(name, value) {name, #name},
		CODE_LIST
	#undef X
	};

    return g_code_id_strings[id];
}

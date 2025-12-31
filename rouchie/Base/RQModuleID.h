#pragma once

#include <iostream>
#include <unordered_map>

using mod_t  = int64_t;

#define MODULE_LIST                \
    X(MOD_INVALID,    -1)           \
    X(MOD_NONAME,     0)            \
    X(MOD_RTSPPLAYER, 100)         \
    X(MOD_MP4READER,  101)          \
    X(MOD_HELLO,      1000)

typedef enum module_id : mod_t{
#define X(name, value) name = value,
    MODULE_LIST
#undef X
} MODULE_ID;

inline const std::string& GetModuleID(mod_t id)
{
	static std::unordered_map<mod_t, std::string> g_module_id_strings = {
	#define X(name, value) {name, #name},
		MODULE_LIST
	#undef X
	};

    return g_module_id_strings[id];
}

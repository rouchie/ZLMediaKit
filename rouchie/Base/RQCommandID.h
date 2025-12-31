#pragma once

#include <iostream>
#include <unordered_map>

using cmd_t = int64_t;

#define CMD_LIST               \
    X(CMD_START,     1)        \
    X(CMD_STOP,      2)        \
    X(CMD_HEARTBEAT, 3)        \
    X(CMD_HELLO,     3000)

typedef enum cmd_id : cmd_t {
#define X(name, value) name = value,
    CMD_LIST
#undef X
    CMD_MAX
} COMMAND_ID;

inline const std::string& GetCommandID(cmd_t id)
{
	static std::unordered_map<cmd_t, std::string> g_cmd_id_strings = {
	#define X(name, value) {name, #name},
		CMD_LIST
	#undef X
	};

    return g_cmd_id_strings[id];
}

#pragma once

#include <functional>
#include <string>
#include <vector>

int AddPlayer(const std::string &url, std::function<void(void *, const std::vector<std::string> &)> func, void *arg);
void DelPlayer(int id);
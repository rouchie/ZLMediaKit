#pragma once

#include "Util/logger.h"
#include "nlohmann/json.hpp"

inline nlohmann::json ParseJson(const std::string& s) {
    nlohmann::json js;

    if (s.empty()) {
        return js;
    }

    try {
        js = nlohmann::json::parse(s);
    } catch (...) {
        WarnL << "解析json失败:" << s;
    }

    return js;
}

inline bool IsJson(const std::string& s) {
    try {
        (void) nlohmann::json::parse(s);
    } catch (...) {
        return false;
    }
    return true;
}

template <typename T>
struct JsonReturnType {
    using type = T;
};

template <>
struct JsonReturnType<const char*> {
    using type = std::string;
};

template <typename T>
typename JsonReturnType<T>::type Get(const nlohmann::json& js, const std::string& key, T tDefVal) {
    typename JsonReturnType<T>::type t;

    if (js[key].is_null()) {
        return tDefVal;
    }

    try {
        t = js[key].get<typename JsonReturnType<T>::type>();
    } catch (const std::exception& e) {
        t = tDefVal;
        WarnL << "json解析失败:" << key << " " << e.what();
    }

    return t;
}

template <typename T>
typename JsonReturnType<T>::type Get(const nlohmann::json& js, const std::string& key, const std::string& key2, T tDefVal) {
    if (js[key].is_null() || !js[key].is_object()) {
        return tDefVal;
    }
    return Get(js[key], key2, tDefVal);
}


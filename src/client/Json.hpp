#pragma once

#include <nlohmann/json.hpp>

using Json = nlohmann::json;

template <typename T> inline auto value_or_default(const Json& j, std::string_view name, const T& default_value) -> T {
    if (j.find(name) == std::end(j)) {
        return default_value;
    }

    return j[std::data(name)].get<T>();
}

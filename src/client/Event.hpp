#pragma once

#include <queue>
#include <string>
#include <variant>

namespace Application {
namespace Detail {

    struct QuitEvent { };
    struct StartUpEvent { };

} // namespace Detail

using Event = std::variant<Detail::StartUpEvent, Detail::QuitEvent>;
using EventQueue = std::queue<Event>;

} // namespace Application
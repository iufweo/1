#pragma once
#include <source_location>
#include <string>

#include "token.hpp"

[[noreturn]] void myAssert(
    const Token& token,
    std::string msg,
    std::source_location loc = std::source_location::current());

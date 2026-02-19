#pragma once
#include <string>
#include <vector>
#include <optional>
#include <string_view>
#include "types.h"


std::optional<ParsedCommand> parse_line(std::string_view line);

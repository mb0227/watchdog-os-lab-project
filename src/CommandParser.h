#pragma once
#include <string>
#include <vector>

enum class CommandType {
    NONE,
    KILL,
    PAUSE,
    RESUME,
    PRIORITY,
    RUN,
    AI,
    QUIT,
    UNKNOWN
};

struct Command {
    CommandType type;
    std::vector<std::string> args;
};

class CommandParser {
public:
    Command parse(const std::string& input);
};

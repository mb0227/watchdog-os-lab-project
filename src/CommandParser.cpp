#include "CommandParser.h"
#include <sstream>
#include <algorithm>

Command CommandParser::parse(const std::string& input) {
    Command cmd;
    cmd.type = CommandType::NONE;

    if (input.empty()) return cmd;

    std::istringstream iss(input);
    std::string token;
    std::vector<std::string> parts;

    while (iss >> token) {
        parts.push_back(token);
    }

    if (parts.empty()) return cmd;

    std::string action = parts[0];
    std::transform(action.begin(), action.end(), action.begin(), ::tolower);

    if (action == "kill") cmd.type = CommandType::KILL;
    else if (action == "pause") cmd.type = CommandType::PAUSE;
    else if (action == "resume") cmd.type = CommandType::RESUME;
    else if (action == "priority") cmd.type = CommandType::PRIORITY;
    else if (action == "run") cmd.type = CommandType::RUN;
    else if (action == "ai") cmd.type = CommandType::AI;
    else if (action == "sort") cmd.type = CommandType::SORT;
    else if (action == "filter" || action == "search") cmd.type = CommandType::FILTER;
    else if (action == "info" || action == "details") cmd.type = CommandType::INFO;
    else if (action == "quit" || action == "q") cmd.type = CommandType::QUIT;
    else cmd.type = CommandType::UNKNOWN;

    if (parts.size() > 1) {
        cmd.args.assign(parts.begin() + 1, parts.end());
    }

    return cmd;
}

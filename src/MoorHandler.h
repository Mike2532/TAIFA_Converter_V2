#ifndef TAIFA_CONVERTER_V2_MOORHANDLER_H
#define TAIFA_CONVERTER_V2_MOORHANDLER_H

#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>
#include <map>
#include <algorithm>
#include <stdexcept>

class MoorHandler
{
public:
    void HandleMoor(std::ifstream& file)
    {
        std::vector<std::string> lines;
        std::string line;
        while (std::getline(file, line)) {
            lines.push_back(line);
        }

        ParseFile(lines);
        PrintResult();
    }
private:
    struct FileLine
    {
        std::string From;
        std::string To;
        std::string X;
    };

    std::string m_startState;
    std::map<std::string, std::string> m_stateOutputs;
    std::vector<FileLine> m_transitions;
    std::vector<std::string> m_froms;

    void ParseFile(const std::vector<std::string>& lines)
    {
        enum Section { None, States, Transitions };
        Section section = None;

        for (const auto& line : lines) {
            std::string trimmed = Trim(line);
            if (trimmed.empty()) continue;

            if (trimmed.find("start state:") == 0) {
                size_t pos = trimmed.find(':');
                m_startState = Trim(trimmed.substr(pos + 1));
                continue;
            }
            if (trimmed == "states:") {
                section = States;
                continue;
            }
            if (trimmed == "transitions:") {
                section = Transitions;
                continue;
            }
            if (trimmed.find("type:") == 0) {
                continue;
            }

            if (section == States) {
                std::stringstream ss(trimmed);
                std::string fullName, pipe, shortName, slash, output;
                if (ss >> fullName >> pipe >> shortName >> slash >> output) {
                    m_stateOutputs[fullName] = output;
                } else {
                    throw std::runtime_error("Invalid state line: " + trimmed);
                }
            }
            else if (section == Transitions) {
                std::stringstream ss(trimmed);
                std::string from, to, x;
                if (ss >> from >> to >> x) {
                    m_transitions.push_back({from, to, x});
                    if (std::find(m_froms.begin(), m_froms.end(), from) == m_froms.end()) {
                        m_froms.push_back(from);
                    }
                } else {
                    throw std::runtime_error("Invalid transition line: " + trimmed);
                }
            }
        }
    }

    std::string Trim(const std::string& str) {
        size_t first = str.find_first_not_of(" \t\r\n");
        if (std::string::npos == first) return "";
        size_t last = str.find_last_not_of(" \t\r\n");
        return str.substr(first, (last - first + 1));
    }

    std::string TrimToData(const std::string& word)
    {
        size_t pos = word.find('_');
        if (pos == std::string::npos) {
            return word;
        }
        return word.substr(0, pos);
    }

    void PrintResult()
    {
        std::cout << "type: mealy" << std::endl;
        std::string startShort = TrimToData(m_startState);
        std::cout << "start: " << startShort << std::endl;
        std::cout << std::endl;
        std::cout << "transitions:" << std::endl;

        for (const auto& line : m_transitions) {
            auto it = m_stateOutputs.find(line.To);
            if (it == m_stateOutputs.end()) {
                throw std::runtime_error("No output for state: " + line.To);
            }
            std::string y = it->second;

            std::string fromShort = TrimToData(line.From);
            std::string toShort = TrimToData(line.To);

            std::cout << fromShort << " " << toShort << " " << line.X << " / " << y << std::endl;
        }
    }
};

#endif //TAIFA_CONVERTER_V2_MOORHANDLER_H
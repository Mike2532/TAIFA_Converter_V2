#ifndef TAIFA_CONVERTER_V2_MOORHANDLER_H
#define TAIFA_CONVERTER_V2_MOORHANDLER_H

#include <fstream>
#include <iostream>
#include <sstream>

class MoorHandler
{
public:
    void HandleMoor(std::ifstream& file)
    {
        SkipUntilTransitions(file);
        auto fileLines = ParseTransitions(file);
        FillActions(fileLines);
        PrintResult(fileLines);
    }
private:
    struct FileLine
{
    std::string From;
    std::string To;
    std::string X;
};

std::vector<std::string> m_froms;

std::optional<int> GetFromInd(const std::string& from)
{
    auto fromSize = m_froms.size();
    for (auto i = 0; i < fromSize; i++) {
        if (m_froms[i] == from) {
            return i;
        }
    }
    return std::nullopt;
}

void SkipUntilTransitions(std::ifstream& file)
{
    std::string line;
    while (std::getline(file, line)) {
        if (line == "transitions:") {
            return;
        }
    }
}

std::string TrimToData(const std::string& word)
{
    size_t pos = word.find('_');
    if (pos == std::string::npos) {
        throw std::invalid_argument("Trimmed word is not a valid character");
    }

    return word.substr(0, pos);
}

std::vector<FileLine> ParseTransitions(std::ifstream& file)
{
    std::vector<FileLine> fileLines;

    std::string line;
    while (std::getline(file, line)) {
        std::stringstream stream(line);
        std::string from, to, x;
        if (!(stream >> from >> to >> x)) {
            throw std::invalid_argument("Invalid transition");
        }
        fileLines.emplace_back(from, to, x);
    }

    return fileLines;
}

void FillActions(const std::vector<FileLine>& fileLines)
{
    for (const auto& line : fileLines) {
        auto it = std::find(m_froms.begin(), m_froms.end(), line.From);
        if (it == m_froms.end()) {
            m_froms.emplace_back(line.From);
        }
    }
}

void PrintResult(const std::vector<FileLine>& fileLines)
{
    for (const auto& line : fileLines) {
        auto y = GetFromInd(line.From);
        if (!y.has_value()) {
            throw std::invalid_argument("Invalid can not get from ind for: " + line.From);
        }
        std::string resultLine;
        resultLine += TrimToData(line.From) + ' ';
        resultLine += TrimToData(line.To) + ' ';
        resultLine += line.X + " / ";
        resultLine += 'Y' + std::to_string(y.value());
        std::cout << resultLine << std::endl;
    }
}
};

#endif //TAIFA_CONVERTER_V2_MOORHANDLER_H
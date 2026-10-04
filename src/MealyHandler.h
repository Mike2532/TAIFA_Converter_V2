#ifndef TAIFA_CONVERTER_V2_MEALYHANDLER_H
#define TAIFA_CONVERTER_V2_MEALYHANDLER_H

#include <fstream>
#include <iostream>
#include <sstream>
#include <unordered_map>
#include <algorithm>
#include <ranges>
#include <set>

class MealyHandler {
public:
    void HandleMealy(std::ifstream& file)
    {
        std::string line;
        if (!std::getline(file, line)) {
            throw std::runtime_error("Empty file");
        }

        size_t colonPos = line.find(':');
        if (colonPos == std::string::npos) {
            throw std::runtime_error("Expected 'start:' line");
        }
        m_originalStartState = Trim(line.substr(colonPos + 1));

        SkipUntilTransitions(file);

        auto [fileLies, allX] = ParseMealyFileData(file);
        InitTransactions(allX);

        auto moors = ConstructEmptyMoors(fileLies);
        FillMoors(moors, fileLies);
        PrintMoors(moors);
    }
private:
    struct FileLine
    {
        std::string From;
        std::string To;
        std::string X;
        std::string Y;
    };

    struct MoorElem
    {
        std::string Name;
        std::string To;
        std::string Y;
        std::unordered_map<std::string, std::optional<std::string>> transactions;
    };

    std::string m_originalStartState;
    std::unordered_map<std::string, std::optional<std::string>> m_clearTransactions;

    std::string Trim(const std::string& str) {
        size_t first = str.find_first_not_of(" \t\r\n");
        if (std::string::npos == first) return "";
        size_t last = str.find_last_not_of(" \t\r\n");
        return str.substr(first, (last - first + 1));
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

    void InitTransactions(const std::vector<std::string>& allX)
    {
        for (const auto& x : allX)
        {
            m_clearTransactions[x] = std::nullopt;
        }
    }

    std::tuple<std::vector<FileLine>, std::vector<std::string>> ParseMealyFileData(std::ifstream& file)
    {
        std::vector<FileLine> fileLies;
        std::vector<std::string> allX;

        std::string line;
        while (std::getline(file, line)) {
            if (line.empty()) {
                continue;
            }
            std::stringstream stream(line);
            std::string from, to, x, dash, y;
            if (!(stream >> from >> to >> x >> dash >> y)) {
                throw std::runtime_error("Could not parse line");
            }
            if (auto it = std::find(allX.begin(), allX.end(), x); it == allX.end()) {
                allX.push_back(x);
            }
            fileLies.push_back(FileLine(from, to, x, y));
        }

        return std::make_tuple(fileLies, allX);
    }

        std::vector<MoorElem> ConstructEmptyMoors(const std::vector<FileLine>& fileLines)
    {
        std::vector<MoorElem> moors;
        moors.reserve(fileLines.size());

        for (const auto& line : fileLines) {
            std::string moorName = line.To + '_' + line.Y;
            auto it = std::find_if(moors.begin(), moors.end(), [moorName](const MoorElem& moor) {
                return moor.Name == moorName;
            });
            if (it != moors.end()) {
                continue;
            }
            moors.push_back(MoorElem(
                moorName,
                line.To,
                line.Y,
                m_clearTransactions
            ));
        }

        std::set<std::string> sourceStates;
        for (const auto& line : fileLines) {
            sourceStates.insert(line.From);
        }

        for (const auto& src : sourceStates) {
            bool alreadyPresent = false;
            for (const auto& moor : moors) {
                if (moor.To == src) {
                    alreadyPresent = true;
                    break;
                }
            }
            if (alreadyPresent) {
                continue;
            }

            std::string output;
            for (const auto& line : fileLines) {
                if (line.From == src) {
                    output = line.Y;
                    break;
                }
            }
            if (output.empty()) {
                continue;
            }

            std::string moorName = src + '_' + output;
            moors.push_back(MoorElem(
                moorName,
                src,
                output,
                m_clearTransactions
            ));
        }

        return moors;
    }

    void FillMoors(std::vector<MoorElem>& moors, const std::vector<FileLine>& fileLines)
    {
        for (auto& moor : moors) {
            for (auto& [X, value] : moor.transactions) {
                auto callback = [moor, X](const FileLine& fileLine) {
                    return fileLine.From == moor.To && fileLine.X == X;
                };

                auto it = std::find_if(fileLines.begin(), fileLines.end(), callback);
                if (it == fileLines.end()) {
                    continue;
                }
                value = it->To + '_' + it->Y;
            }
        }
    }

    void PrintMoors(std::vector<MoorElem> moors)
    {
        std::ranges::sort(moors, [](const MoorElem& a, const MoorElem& b) {
            return a.Name < b.Name;
        });

        std::string startStateName;
        for (const auto& moor : moors) {
            if (moor.To == m_originalStartState) {
                startStateName = moor.Name;
                break;
            }
        }
        if (startStateName.empty()) {
            throw std::runtime_error("Could not find Moore state for original start state: " + m_originalStartState);
        }

        std::cout << "type: moore" << std::endl;
        std::cout << "start state: " << startStateName << std::endl;
        std::cout << std::endl;
        std::cout << "states:" << std::endl;
        for (const auto& moor : moors) {
            std::cout << moor.Name << " | " << moor.To << " / " << moor.Y << std::endl;
        }
        std::cout << std::endl;
        std::cout << "transitions:" << std::endl;
        for (const auto& moor : moors) {
            std::vector<std::string> tails;
            for (auto& [X, value] : moor.transactions) {
                if (!value.has_value()) {
                    continue;
                }
                tails.push_back(value.value() + ' ' + X);
            }
            std::ranges::sort(tails, [](const std::string& a, const std::string& b) {
                return a < b;
            });
            for (const auto& tail : tails) {
                std::cout << moor.Name << ' ' << tail << std::endl;
            }
        }
    }
};

#endif //TAIFA_CONVERTER_V2_MEALYHANDLER_H
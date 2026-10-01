#include <iostream>

#include "src/MealyHandler.h"
#include "src/MoorHandler.h"

void RunApp() {
    std::ifstream file("../input.txt");
    if (!file.is_open()) {
        throw std::runtime_error("Could not open file");
    }

    std::string line;
    std::getline(file, line);
    if (line == "type: mealy") {
        auto handler = MealyHandler();
        handler.HandleMealy(file);
    } else if (line == "type: moore") {
        auto handler = MoorHandler();
        handler.HandleMoor(file);
    } else {
        file.close();
        throw std::runtime_error("Could not parse file");
    }
    file.close();
}

int main() {
    try {
        RunApp();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }
}

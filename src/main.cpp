#include <fstream>
#include <iostream>
#include <string>

#include "compiler.h"


int main() {
    nlohmann::json json;
    std::cin >> json;

    auto res = compiler::compiler(json);
    std::cout << to_string(res) << '\n';
}

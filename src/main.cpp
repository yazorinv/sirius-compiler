#include <fstream>
#include <iostream>
#include <string>

#include "compiler.h"


int main() {
    nlohmann::json json;
    std::cin >> json;


    compiler::Compiler compiler;
    std::cout << to_string(compiler(json)) << '\n';
}

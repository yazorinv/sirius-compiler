#include <stdexcept>

#include "compiler.h"


namespace compiler {


Compiler::Compiler() {
    result_program = Json::array();
    fn["Const"] = [](Compiler& compiler, const Json& json) { return compiler.add_constant(json); };
    fn["Var"] = [](Compiler& compiler, const Json& json) { return compiler.add_variable(json); };
    fn["op"] = [](Compiler& compiler, const Json& json) { return compiler.add_binary_operator(json); };
}


void Compiler::dfs(const compiler::Json& json) {
    if (!json.contains("kind"))
        throw std::logic_error("not found \"kind\"");
    std::string type = json["kind"];


    auto fnd = fn.find(type);
    if (fnd == fn.end())
        throw std::logic_error("unknown type");
    fnd->second(*this, json);
}


void Compiler::clear(const bool clear_all) {
    if (clear_all)
        fn.clear();
    result_program = Json::array();
}


Json Compiler::operator()(const compiler::Json& json) {
    dfs(json);
    return result_program;
}


void Compiler::add_constant(const compiler::Json& json) {
    if (!json.contains("value"))
        throw std::logic_error("not found \"value\" for constant");
    result_program.emplace_back(json["value"]);
}


void Compiler::add_variable(const compiler::Json& json) {
    if (!json.contains("name"))
        throw std::logic_error("not found \"name\" for variable");
    result_program.emplace_back(json["name"]);
}


void Compiler::add_binary_operator(const compiler::Json& json) {
    if (!json.contains("name"))
        throw std::logic_error("not found \"name\" for binary operator");
    if (!json.contains("left"))
        throw std::logic_error("not found \"left\" for binary operator");
    if (!json.contains("right"))
        throw std::logic_error("not found \"right\" for binary operator");
    dfs(json["left"]);
    dfs(json["right"]);

    Json buf;
    buf["kind"] = "Binop";
    buf["value"] = json["name"];
    result_program.emplace_back(std::move(buf));
}


}  // namespace compiler

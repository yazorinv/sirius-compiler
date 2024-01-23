#include <stdexcept>

#include "compiler.h"


namespace compiler {


Compiler::Compiler() {
    result_program = Json::array();
    fn["Const"] = [](Compiler& compiler, const Json& json) { return compiler.add_constant(json); };
    fn["Var"] = [](Compiler& compiler, const Json& json) { return compiler.add_variable(json); };
    fn["op"] = [](Compiler& compiler, const Json& json) { return compiler.add_binary_operator(json); };


    fn["Assn"] = [](Compiler& compiler, const Json& json) { return compiler.add_assignment(json); };

    fn["Read"] = [](Compiler& compiler, const Json& json) { return compiler.add_read_console(json); };
    fn["Write"] = [](Compiler& compiler, const Json& json) { return compiler.add_write_console(json); };

    fn["if"] = [](Compiler& compiler, const Json& json) { return compiler.add_if(json); };
    fn["While"] = [](Compiler& compiler, const Json& json) { return compiler.add_while(json); };

    fn["Seq"] = [](Compiler& compiler, const Json& json) { return compiler.add_sequence(json); };
}


void Compiler::dfs(const compiler::Json& json) {
    if (json == "Skip")
        return;
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


void Compiler::add_assignment(const compiler::Json& json) {
    if (!json.contains("lvalue"))
        throw std::logic_error("not found \"lvalue\" for assignment");
    if (!json.contains("rvalue"))
        throw std::logic_error("not found \"rvalue\" for assignment");

    dfs(json["rvalue"]);

    Json buf;
    buf["kind"] = "ST";
    buf["value"] = json["lvalue"];
    result_program.emplace_back(buf);
}


void Compiler::add_read_console(const compiler::Json& json) {
    if (!json.contains("name"))
        throw std::logic_error("not found \"name\" for read in console");

    result_program.emplace_back("READ");

    Json buf;
    buf["kind"] = "ST";
    buf["value"] = json["name"];
    result_program.emplace_back(buf);
}


void Compiler::add_write_console(const compiler::Json& json) {
    if (!json.contains("value"))
        throw std::logic_error("not found \"value\" for write in console");

    dfs(json["value"]);
    result_program.emplace_back("WRITE");
}


void Compiler::add_sequence(const compiler::Json& json) {
    if (!json.contains("left"))
        throw std::logic_error("not found \"left\" for sequence");
    if (!json.contains("right"))
        throw std::logic_error("not found \"right\" for sequence");

    dfs(json["left"]);
    dfs(json["right"]);
}


void Compiler::add_while(const compiler::Json& json) {
    if (!json.contains("cond"))
        throw std::logic_error("not found \"cond\" for while");
    if (!json.contains("body"))
        throw std::logic_error("not found \"body\" for while");

    Json label_cond, label_body;
    Json jmp_cond, jmp_body;

    label_cond["kind"] = label_body["kind"] = "LABEL";
    jmp_cond["kind"] = "JMP";
    jmp_body["kind"] = "JNZ";
    label_cond["value"] = jmp_cond["value"] = "WHILE_COND_" + std::to_string(while_count);
    label_body["value"] = jmp_body["value"] = "WHILE_BODY_" + std::to_string(while_count);
    ++while_count;

    result_program.emplace_back(std::move(jmp_cond));
    result_program.emplace_back(std::move(label_body));
    dfs(json["body"]);
    result_program.emplace_back(std::move(label_cond));
    dfs(json["cond"]);
    result_program.emplace_back(std::move(jmp_body));
}


void Compiler::add_if(const compiler::Json& json) {
    if (!json.contains("cond"))
        throw std::logic_error("not found \"cond\" for if");
    if (!json.contains("then"))
        throw std::logic_error("not found \"then\" for if");
    if (!json.contains("else"))
        throw std::logic_error("not found \"else\" for if");

    Json label_else, label_end;
    Json jmp_else, jmp_end;

    label_else["kind"] = label_end["kind"] = "LABEL";
    jmp_else["kind"] = "JZ";
    jmp_end["kind"] = "JMP";
    label_else["value"] = jmp_else["value"] = "IF_ELSE_" + std::to_string(if_else_count);
    label_end["value"] = jmp_end["value"] = "IF_END_" + std::to_string(if_else_count);
    ++if_else_count;


    dfs(json["cond"]);
    result_program.emplace_back(jmp_else);
    dfs(json["then"]);
    result_program.emplace_back(jmp_end);
    result_program.emplace_back(label_else);
    dfs(json["else"]);
    result_program.emplace_back(label_end);
}


}  // namespace compiler

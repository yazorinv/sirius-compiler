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


    fn["Fun"] = [](Compiler& compiler, const Json& json) { return compiler.add_procedure(json); };
    fn["Call"] = [](Compiler& compiler, const Json& json) { return compiler.add_call(json); };
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
    if (!json.contains("prog"))
        dfs(json);
    dfs(json["prog"]);
    result_program.emplace_back("END");
    if (json.contains("funs")) {
        for (auto& t : json["funs"])
            dfs(t);
    }
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

    label_cond = create_label("WHILE_CONDITION_" + std::to_string(while_count));
    label_body = create_label("WHILE_BODY_" + std::to_string(while_count));

    jmp_cond = create_jmp("WHILE_CONDITION_" + std::to_string(while_count));
    jmp_body = create_jnz("WHILE_BODY_" + std::to_string(while_count));

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

    label_else = create_label("IF_ELSE_" + std::to_string(if_else_count));
    label_end = create_label("IF_END" + std::to_string(if_else_count));

    jmp_else = create_jz("IF_ELSE_" + std::to_string(if_else_count));
    jmp_end = create_jmp("IF_END" + std::to_string(if_else_count));

    ++if_else_count;


    dfs(json["cond"]);
    result_program.emplace_back(jmp_else);
    dfs(json["then"]);
    result_program.emplace_back(jmp_end);
    result_program.emplace_back(label_else);
    dfs(json["else"]);
    result_program.emplace_back(label_end);
}


void Compiler::add_procedure(const nlohmann::json& json) {
    if (!json.contains("name"))
        throw std::logic_error("not found \"name\" for procedure");
    if (!json.contains("params"))
        throw std::logic_error("not found \"params\" for procedure");
    if (!json.contains("body"))
        throw std::logic_error("not found \"body\" for procedure");

    Json proc_label = create_label("FUNCTION_" + std::string(json["name"]));
    Json proc_begin;
    proc_begin["kind"] = "BEGIN";
    proc_begin["value"] = json["params"];
    result_program.emplace_back(std::move(proc_label));
    result_program.emplace_back(std::move(proc_begin));
    dfs(json["body"]);
    result_program.emplace_back("END");
}


void Compiler::add_call(const compiler::Json& json) {
    if (!json.contains("func"))
        throw std::logic_error("not found \"func\" for call procedure");
    if (!json.contains("args"))
        throw std::logic_error("not found \"args\" for call procedure");

    auto& args = json["args"];
    for (auto it = args.rbegin(); it != args.rend(); ++it)
        dfs(*it);
    Json call;
    call["kind"] = "CALL";
    call["value"] = "FUNCTION_" + std::string(json["func"]);
    result_program.emplace_back(std::move(call));
}


Json Compiler::create_label(const std::string& label) const {
    Json buf;
    buf["kind"] = "LABEL";
    buf["value"] = label;
    return buf;
}


Json Compiler::create_jmp(const std::string& label) const {
    Json buf;
    buf["kind"] = "JMP";
    buf["value"] = label;
    return buf;
}


Json Compiler::create_jz(const std::string& label) const {
    Json buf;
    buf["kind"] = "JZ";
    buf["value"] = label;
    return buf;
}


Json Compiler::create_jnz(const std::string& label) const {
    Json buf;
    buf["kind"] = "JNZ";
    buf["value"] = label;
    return buf;
}


}  // namespace compiler

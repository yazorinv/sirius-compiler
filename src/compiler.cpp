#include <stdexcept>

#include "compiler.h"


namespace compiler {


nlohmann::json compiler(const nlohmann::json& json) {
    nlohmann::json res = nlohmann::json::array();
    dfs(json, res);
    return res;
}


void dfs(const nlohmann::json& json, nlohmann::json& res) {
    auto type = std::string(json["kind"]);

    if (type == "Const") {
        res.emplace_back(json["value"]);
    } else if (type == "Var") {
        res.emplace_back(json["name"]);
    } else if (type == "op") {
        auto type_op = std::string(json["name"]);
        dfs(json["left"], res);
        dfs(json["right"], res);

        nlohmann::json buf;
        buf["kind"] = "Binop";
        buf["value"] = type_op;
        res.emplace_back(std::move(buf));
    }
}


}  // namespace compiler

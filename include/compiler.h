#pragma once
#include <nlohmann/json.hpp>
#include <string>


namespace compiler {


nlohmann::json compiler(const nlohmann::json& json);


void dfs(const nlohmann::json& json, nlohmann::json& res);

}  // namespace compiler

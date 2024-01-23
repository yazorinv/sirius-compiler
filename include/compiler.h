#pragma once
#include <functional>
#include <map>
#include <nlohmann/json.hpp>
#include <string>


namespace compiler {


using Json = nlohmann::json;


class Compiler {
   private:
    std::map<std::string, std::function<void(Compiler&, const Json&)>> fn;
    Json result_program;
    // More state for compiler

    void dfs(const Json& json);

   public:
    Compiler();
    Compiler(const Compiler&) = default;
    Compiler(Compiler&&) noexcept = default;
    ~Compiler() noexcept = default;

    Compiler& operator=(const Compiler&) = default;
    Compiler& operator=(Compiler&&) noexcept = default;


    void clear(const bool clear_all = false);


    Json operator()(const Json& json);


    void add_constant(const Json& json);
    void add_variable(const Json& json);
    void add_binary_operator(const Json& json);


    void add_assignment(const Json& json);

    void add_read_console(const Json& json);
    void add_write_console(const Json& json);

    void add_if(const Json& json);
    void add_while(const Json& json);
};


}  // namespace compiler

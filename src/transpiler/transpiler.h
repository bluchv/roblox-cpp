#pragma once

#include <clang-c/Index.h>
#include <memory>
#include <string>
#include "../ast/luau_node.h"
#include "../codegen/luau_codegen.h"

namespace roblox_transpiler {
  class Transpiler {
  public:
    Transpiler();
    ~Transpiler();

    bool transpileFile(const std::string &inputFile, const std::string &outputFile = "");
    std::string transpileToString(const std::string &inputFile);
    bool generateBootstrapper(const std::string &inputFile, const std::string &bootstrapFile);

    void setVerbose(bool verbose) { verbose_ = verbose; }
    bool hasErrors() const { return hasErrors_; }
    const std::string &getLastError() const { return lastError_; }

  private:
    CXIndex index_;
    bool verbose_;
    bool hasErrors_;
    std::string lastError_;

    bool parseFile(const std::string &inputFile, CXTranslationUnit &unit);
    std::shared_ptr<LuauNode> buildAST(CXTranslationUnit unit, const std::string &inputFile);
    std::string generateLuau(std::shared_ptr<LuauNode> rootNode);

    void logError(const std::string &error);
    void logInfo(const std::string &info);
  };
} // namespace roblox_transpiler

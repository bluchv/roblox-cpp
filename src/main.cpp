#include <algorithm>
#include <chrono>
#include <clang-c/Index.h>
#include <cstdio>
#include <functional>
#include <iostream>
#include <string>
#include <unordered_map>
#include <utility>
#include "luau-ast/LuauAstGenerator.h"
#include "luau-code-gen/LuauCodeGen.h"
#include "utils.h"

const std::string OUTPUT_FILENAME = "output.lua";

class AstVisitor {
public:
  explicit AstVisitor(std::string mainSourceFile) :
      mainSourceFile_(std::move(mainSourceFile)), translationUnit_(nullptr) {}

  template<typename T>
  CXChildVisitResult visit(const CXCursor &cursor, T &parent) {
    if (!isFromMainFile(cursor)) {
      return CXChildVisit_Continue;
    }

    const CXCursorKind kind = clang_getCursorKind(cursor);
    const std::string kindSpelling = utils::getCursorKindSpelling(cursor);

    switch (kind) {
      case CXCursor_FunctionDecl:
        return handleFunctionDecl(cursor, parent);
      case CXCursor_VarDecl:
        return handleVarDecl(cursor, parent);
      default:
        const std::string spelling = utils::getCursorSpelling(cursor);
        if (!spelling.empty()) {
          std::cout << "Unknown high-level cursor kind " << kindSpelling << ". Spelling: " << spelling << std::endl;
        } else {
          std::cout << "Unknown high-level cursor kind " << kindSpelling << ". Spelling empty. " << std::endl;
        }
        return CXChildVisit_Recurse;
    }
  }

  void setTranslationUnit(CXTranslationUnit translationUnit) { translationUnit_ = translationUnit; }
  void setRootNode(LuauNode *rootNode) { root = rootNode; };
  [[nodiscard]] CXTranslationUnit getTranslationUnit() const { return translationUnit_; }
  LuauNode *root;

private:
  [[nodiscard]] bool isFromMainFile(const CXCursor &cursor) const {
    CXSourceLocation location = clang_getCursorLocation(cursor);
    CXFile file;
    clang_getSpellingLocation(location, &file, nullptr, nullptr, nullptr);

    if (file == nullptr)
      return false;

    CXString fileName = clang_getFileName(file);
    std::string fileNameStr = clang_getCString(fileName);
    clang_disposeString(fileName);

    return fileNameStr == mainSourceFile_;
  }

  template<typename T>
  CXChildVisitResult handleFunctionDecl(const CXCursor &cursor, T &parentNode) {
    std::string functionName = utils::getCursorSpelling(cursor);

    struct ClientData {
      AstVisitor *visitor;
      FunctionNode *parentNode;
    };

    FunctionNode *newFunction = new FunctionNode(functionName);
    ClientData clientData = {this, newFunction};

    // Write args
    clang_visitChildren(
        cursor,
        [](CXCursor child, CXCursor parent, CXClientData client_data) -> CXChildVisitResult {
          const auto data = static_cast<ClientData *>(client_data);
          if (clang_getCursorKind(child) == CXCursor_ParmDecl) {
            const std::string name = utils::getCursorSpelling(child);
            const std::string kind = utils::getCursorKindSpelling(child);
            const CXString typeSpelling = clang_getTypeSpelling(clang_getCursorType(child));
            const std::string type = clang_getCString(typeSpelling);

            clang_disposeString(typeSpelling);
            // auto type = clang_getCString(clang_getTypeSpelling(clang_getCursorType(child)));
            // std::cout << name << " " << kind << std::endl;
            data->parentNode->addArg(name);
            return CXChildVisit_Continue;
          }
          return CXChildVisit_Continue;
        },
        &clientData);

    // Write fn body
    clang_visitChildren(
        cursor,
        [](CXCursor child, CXCursor parent, CXClientData client_data) -> CXChildVisitResult {
          const auto data = static_cast<ClientData *>(client_data);
          data->visitor->visit(child, data->parentNode);
          return CXChildVisit_Continue;
        },
        &clientData);

    parentNode->addChild(newFunction);
    return CXChildVisit_Continue;
  }

  template<typename T>
  CXChildVisitResult handleVarDecl(const CXCursor &cursor, T &parentNode) {
    const std::string varName = utils::getCursorSpelling(cursor);
    const CXType type = clang_getCursorType(cursor);
    const CXString typeSpelling = clang_getTypeSpelling(type);
    const char *typeSpellingStr = clang_getCString(typeSpelling);

    struct ClientData {
      AstVisitor *visitor;
      T parentNode;
    };
    ClientData clientData = {this, parentNode};

    clang_disposeString(typeSpelling);
    clang_visitChildren(
        cursor,
        [](CXCursor child, CXCursor parent, CXClientData client_data) -> CXChildVisitResult {
          auto clientData = static_cast<ClientData *>(client_data);
          auto cursorKind = clang_getCursorKind(child);
          auto visitor = clientData->visitor;
          auto parentNode = clientData->parentNode;
          const std::string varName = utils::getCursorSpelling(parent);

          if (cursorKind == CXCursor_IntegerLiteral) {
            std::string intValue = utils::getIntegerLiteralValue(child, visitor->getTranslationUnit());
            auto *variable = new VariableNode(varName, std::atoi(intValue.c_str()));
            parentNode->addChild(variable);
            return CXChildVisit_Break;
          } else if (cursorKind == CXCursor_StringLiteral) {
            std::cout << "StringLiteral: " << utils::getCursorSpelling(child) << std::endl;
            std::string stringValue = utils::getCursorSpelling(child);
            auto *variable = new VariableNode(varName, stringValue);
            parentNode->addChild(variable);
          } else if (cursorKind == CXCursor_CallExpr) {
            std::string calledFunctionName = utils::getCursorSpelling(clang_getCursorReferenced(child));
            // output->write(calledFunctionName + "(");
            //
            // clang_visitChildren(
            //     child,
            //     [](CXCursor arg, CXCursor parent, CXClientData client_data) {
            //       auto output = static_cast<LuauCodeGen *>(client_data);
            //       if (clang_getCursorKind(arg) == CXCursor_DeclRefExpr) {
            //         std::string argName = utils::getCursorSpelling(clang_getCursorReferenced(arg));
            //         output->write(argName + ", ");
            //       }
            //       return CXChildVisit_Continue;
            //     },
            //     output);
            //
            // output->removeTrailingComma();
            // output->write(")\n");
            // // visitor->visit(child, parent);
            return CXChildVisit_Break;
          } else {
            std::cout << "Unknown variable-cursor kind: " << utils::getCursorKindSpelling(child) << std::endl;
          }
          return CXChildVisit_Recurse;
        },
        &clientData);

    return CXChildVisit_Continue;
  }

  std::string mainSourceFile_;
  CXTranslationUnit translationUnit_;
};

int main(const int argc, char *argv[]) {
  LuauAstGenerator generator;
  if (argc != 2) {
    std::cerr << "Incorrect Use! Proper Usage: " << argv[0] << " <source_file.cpp>" << std::endl;
    return 1;
  }

  const std::string mainSourceFile = argv[1];

  CXIndex index = clang_createIndex(0, 0);
  CXTranslationUnit unit =
      clang_parseTranslationUnit(index, mainSourceFile.c_str(), nullptr, 0, nullptr, 0, CXTranslationUnit_None);
  if (unit == nullptr) {
    std::cerr << "Error: Unable to parse translation unit." << std::endl;
    clang_disposeIndex(index);
    return 1;
  }

  const auto compileStartClock = std::chrono::high_resolution_clock::now();

  LuauNode root;
  LuauCodeGen luaOutput;
  luaOutput.writeln("-- im mentally unstable now --");

  AstVisitor visitor(mainSourceFile);
  visitor.setTranslationUnit(unit);

  const CXCursor rootCursor = clang_getTranslationUnitCursor(unit);
  visitor.setRootNode(&root);
  clang_visitChildren(
      rootCursor,
      [](CXCursor c, CXCursor p, CXClientData d) {
        auto visitor = static_cast<AstVisitor *>(d);
        return visitor->visit(c, visitor->root);
      },
      &visitor);

  root.render(luaOutput);
  luaOutput.writeToFile(OUTPUT_FILENAME);
  luaOutput.writeToConsole();

  const auto compileEndClock = std::chrono::high_resolution_clock::now();
  const auto compileDurationInMS =
      std::chrono::duration_cast<std::chrono::milliseconds>(compileEndClock - compileStartClock);
  printf("Time taken: %lld ms.", compileDurationInMS.count());

  clang_disposeTranslationUnit(unit);
  clang_disposeIndex(index);
  return 0;
}

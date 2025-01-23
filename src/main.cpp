#include <chrono>
#include <clang-c/Index.h>
#include <functional>
#include <iostream>
#include <string>
#include <unordered_map>
#include <utility>
#include "luau-code-gen/LuauCodeGen.h"
#include "utils.h"

const std::string OUTPUT_FILENAME = "output.lua";

class AstVisitor {
public:
  explicit AstVisitor(std::string mainSourceFile) :
      mainSourceFile_(std::move(mainSourceFile)), output(nullptr), translationUnit_(nullptr) {}

  CXChildVisitResult visit(const CXCursor &cursor, CXCursor parent) {
    if (!isFromMainFile(cursor)) {
      return CXChildVisit_Continue;
    }

    const CXCursorKind kind = clang_getCursorKind(cursor);
    const std::string kindSpelling = utils::getCursorKindSpelling(cursor);
    // switch (kind) {
    //   case CXCursor_FunctionDecl:
    //     return handleFunctionDecl(cursor);
    //   case CXCursor_VarDecl:
    //     return handleVarDecl(cursor);
    //   case CXCursor_ReturnStmt:
    //     return handleReturnStmt(cursor);
    //   case CXCursor_ForStmt:
    //     return handleForStmt(cursor);
    //   case CXCursor_UnexposedExpr:
    //     return handleUnexpectedExpr(cursor);
    //   case CXCursor_NamespaceRef:
    //     return handleNamespaceRef(cursor);
    //   case CXCursor_CallExpr:
    //     return handleCallRef(cursor);
    //   default:
    //     const std::string spelling = utils::getCursorSpelling(cursor);
    //     auto cursorKind = utils::getCursorKindSpelling(cursor);
    //
    //     if (!spelling.empty()) {
    //       std::cout << "Unknown high-level cursor kind " << cursorKind << ". Spelling: " << spelling << std::endl;
    //     } else {
    //       std::cout << "Unknown high-level cursor kind " << cursorKind << ". Spelling empty. " << std::endl;
    //     }
    //     return CXChildVisit_Recurse;
    // }

    /* Experimental feature */
    auto nodeHandleFn = methodMap.find(kind);
    if (nodeHandleFn != methodMap.end()) {
      return (nodeHandleFn->second)(this, cursor);
    } else {
      const std::string spelling = utils::getCursorSpelling(cursor);
      if (!spelling.empty()) {
        std::cout << "Unknown high-level cursor kind " << kindSpelling << ". Spelling: " << spelling << std::endl;
      } else {
        std::cout << "Unknown high-level cursor kind " << kindSpelling << ". Spelling empty. " << std::endl;
      }
      return CXChildVisit_Recurse;
    }
  }

  void setOutput(LuauCodeGen *_output) { output = _output; }

  [[nodiscard]] CXTranslationUnit getTranslationUnit() const { return translationUnit_; }

  void setTranslationUnit(CXTranslationUnit translationUnit) { translationUnit_ = translationUnit; }

private:
  static std::unordered_map<CXCursorKind, std::function<CXChildVisitResult(AstVisitor *, const CXCursor &)>>
  createMethodMap() {
    std::unordered_map<CXCursorKind, std::function<CXChildVisitResult(AstVisitor *, const CXCursor &)>> functionMap = {
        {CXCursor_FunctionDecl, &AstVisitor::handleFunctionDecl},
        {CXCursor_VarDecl, &AstVisitor::handleVarDecl},
        {CXCursor_ReturnStmt, &AstVisitor::handleReturnStmt},
        {CXCursor_ForStmt, &AstVisitor::handleForStmt},
        {CXCursor_UnexposedExpr, &AstVisitor::handleUnexpectedExpr},
        {CXCursor_NamespaceRef, &AstVisitor::handleNamespaceRef},
        {CXCursor_CallExpr, &AstVisitor::handleCallRef}};
    return functionMap;
  }

  friend std::unordered_map<CXCursorKind, std::function<CXChildVisitResult(AstVisitor *, const CXCursor &)>>
  createMethodMap();
  static inline std::unordered_map<CXCursorKind, std::function<CXChildVisitResult(AstVisitor *, const CXCursor &)>>
      methodMap = createMethodMap();

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

  CXChildVisitResult handleFunctionDecl(const CXCursor &cursor) {
    std::string functionName = utils::getCursorSpelling(cursor);
    CXType returnType = clang_getCursorResultType(cursor);
    CXString typeSpelling = clang_getTypeSpelling(returnType);
    auto typeSpellingC = clang_getCString(typeSpelling);
    output->writefn(functionName, typeSpellingC);
    clang_disposeString(typeSpelling);

    clang_visitChildren(
        cursor,
        [](CXCursor child, CXCursor parent, CXClientData client_data) -> CXChildVisitResult {
          if (clang_getCursorKind(child) == CXCursor_CompoundStmt) {
            const auto visitor = static_cast<AstVisitor *>(client_data);
            // Visit the children of the CompoundStmt
            clang_visitChildren(
                child,
                [](CXCursor c, CXCursor p, CXClientData d) -> CXChildVisitResult {
                  return static_cast<AstVisitor *>(d)->visit(c, p);
                },
                visitor);
            // Return CXChildVisit_Continue to continue visiting other children
            // of the function
            return CXChildVisit_Continue;
          }
          return CXChildVisit_Continue;
        },
        this);

    output->write_eof();
    return CXChildVisit_Continue;
  }

  CXChildVisitResult handleVarDecl(const CXCursor &cursor) {
    const std::string varName = utils::getCursorSpelling(cursor);
    const CXType type = clang_getCursorType(cursor);
    const CXString typeSpelling = clang_getTypeSpelling(type);
    const char *typeSpellingStr = clang_getCString(typeSpelling);

    output->indent();
    output->writeVariable(varName, typeSpellingStr);
    clang_disposeString(typeSpelling);

    clang_visitChildren(
        cursor,
        [](CXCursor child, CXCursor parent, CXClientData client_data) -> CXChildVisitResult {
          auto visitor = static_cast<AstVisitor *>(client_data);
          LuauCodeGen *output = visitor->output;

          auto cursorKind = clang_getCursorKind(child);

          if (cursorKind == CXCursor_IntegerLiteral) {
            std::string intValue = utils::getIntegerLiteralValue(child, visitor->getTranslationUnit());
            output->write(intValue + "\n");
            return CXChildVisit_Break;
          } else if (cursorKind == CXCursor_StringLiteral) {
            std::cout << "StringLiteral: " << utils::getCursorSpelling(child) << std::endl;
            std::string stringValue = utils::getCursorSpelling(child);
            output->write(stringValue + "\n");
          } else if (cursorKind == CXCursor_CallExpr) {
            std::string calledFunctionName = utils::getCursorSpelling(clang_getCursorReferenced(child));
            output->write(calledFunctionName + "(");

            clang_visitChildren(
                child,
                [](CXCursor arg, CXCursor parent, CXClientData client_data) {
                  auto output = static_cast<LuauCodeGen *>(client_data);
                  if (clang_getCursorKind(arg) == CXCursor_DeclRefExpr) {
                    std::string argName = utils::getCursorSpelling(clang_getCursorReferenced(arg));
                    output->write(argName + ", ");
                  }
                  return CXChildVisit_Continue;
                },
                output);

            output->removeTrailingComma();
            output->write(")\n");

            return CXChildVisit_Break;
          } else {
            std::cout << "Unknown cursor-child kind: " << cursorKind << std::endl;
          }
          return CXChildVisit_Recurse;
        },
        this);

    return CXChildVisit_Continue;
  }

  CXChildVisitResult handleReturnStmt(const CXCursor &cursor) {
    output->indent();
    output->write("return ");

    clang_visitChildren(
        cursor,
        [](CXCursor child, CXCursor parent, CXClientData client_data) -> CXChildVisitResult {
          auto visitor = static_cast<AstVisitor *>(client_data);
          if (clang_getCursorKind(child) == CXCursor_IntegerLiteral) {
            std::string intValue = utils::getIntegerLiteralValue(child, visitor->getTranslationUnit());
            visitor->output->write(intValue);
            return CXChildVisit_Break;
          } else if (clang_getCursorKind(child) == CXCursor_UnexposedExpr) {
            std::string varName = utils::getCursorSpelling(clang_getCursorReferenced(child));
            visitor->output->write(varName);
            return CXChildVisit_Continue;
          }
          return CXChildVisit_Continue;
        },
        this);

    output->write("\n");
    return CXChildVisit_Continue;
  }

  CXChildVisitResult handleForStmt(const CXCursor &cursor) {
    output->indent();
    output->write("for i = 0, ");

    clang_visitChildren(
        cursor,
        [](CXCursor child, CXCursor parent, CXClientData client_data) -> CXChildVisitResult {
          auto childKind = clang_getCursorKind(child);
          auto visitor = static_cast<AstVisitor *>(client_data);

          if (childKind == CXCursor_BinaryOperator) {
            auto childSpelling = utils::getIntegerLiteralValue(child, visitor->getTranslationUnit());
            std::cout << childSpelling << std::endl;
            clang_visitChildren(
                child,
                [](CXCursor child, CXCursor parent, CXClientData client_data) -> CXChildVisitResult {
                  auto visitor = static_cast<AstVisitor *>(client_data);
                  auto childKind = clang_getCursorKind(child);

                  if (childKind == CXCursor_IntegerLiteral) {
                    int value = std::strtol(utils::getIntegerLiteralValue(child, visitor->getTranslationUnit()).c_str(),
                                            nullptr, 10);
                    visitor->output->write(std::format("{}", value - 1));
                  }
                  return CXChildVisit_Continue;
                },
                visitor);
          }
          return CXChildVisit_Continue;
        },
        this);

    output->write(" do\n");
    output->writeln("end");

    return CXChildVisit_Continue;
  }

  CXChildVisitResult handleUnexpectedExpr(const CXCursor &cursor) {
    std::string cursor_spelling = utils::getCursorSpelling(cursor);
    auto cursor_kind = clang_getCursorKind(cursor);
    std::cout << cursor_kind << " + " << cursor_spelling << std::endl;

    clang_visitChildren(
        cursor,
        [](CXCursor child, CXCursor parent, CXClientData client_data) -> CXChildVisitResult {
          auto visitor = static_cast<AstVisitor *>(client_data);
          CXCursorKind child_cursor_kind = clang_getCursorKind(child);
          std::cout << child_cursor_kind << std::endl;

          auto child_spelling = utils::getCursorSpelling(child);
          auto isInIgnoreTable = std::find(utils::IgnoreMap.begin(), utils::IgnoreMap.end(), child_spelling);
          if (!child_spelling.empty() && isInIgnoreTable == utils::IgnoreMap.end()) {
            CXSourceLocation loc = clang_getCursorLocation(child);
            CXFile file;
            unsigned int line, column;
            clang_getExpansionLocation(loc, &file, &line, &column, nullptr);

            std::string filename = clang_getCString(clang_getFileName(file));
            // clang_getCursorLexicalParent(parent);
            //  Get the parent, of the CXCursor that called
            //  handleUnexpectedExpr, visit all the nodes in there, if they're
            //  strings, add it to the print statement

            visitor->output->writeln(std::format("print(\"[{} - Line {}]:\", {})",
                                                 filename.erase(0, filename.find("c++") + 4), line, child_spelling));
          }
          return CXChildVisit_Continue;
        },
        this);

    return CXChildVisit_Continue;
  }

  CXChildVisitResult handleNamespaceRef(const CXCursor &cursor) {
    const std::string cursor_spelling = utils::getCursorSpelling(cursor);
    const CXCursor declCursor = clang_getCursorReferenced(cursor);

    if (cursor_spelling == "std") {
      std::cout << "Detected std namespace, skipping" << std::endl;
      return CXChildVisit_Continue;
    }

    if (clang_Cursor_isNull(declCursor)) {
      std::cerr << "Error: Could not resolve namespace declaration." << std::endl;
      return CXChildVisit_Continue;
    }

    CXSourceLocation declLocation = clang_getCursorLocation(declCursor);
    CXFile declFile;
    unsigned int declLine, declColumn, declOffset;
    clang_getFileLocation(declLocation, &declFile, &declLine, &declColumn, &declOffset);

    CXString declFilename = clang_getFileName(declFile);
    // std::cout << "Namespace declared at:" << std::endl;
    // std::cout << "File: " << clang_getCString(declFilename) << std::endl;

    clang_disposeString(declFilename);
    return CXChildVisit_Continue;
  }

  CXChildVisitResult handleCallRef(const CXCursor &cursor) {
    const std::string cursor_spelling = utils::getCursorSpelling(cursor);
    const CXString test = clang_getCursorDisplayName(cursor);

    std::cout << clang_getCString(test) << std::endl;
    clang_disposeString(test);

    const int numArgs = clang_Cursor_getNumArguments(cursor);
    std::cout << "Call to function with " << numArgs << " arguments:" << std::endl;

    for (int i = 0; i < numArgs; i++) {
      CXCursor argCursor = clang_Cursor_getArgument(cursor, i);
      CXString argSpelling = clang_getCursorSpelling(argCursor);

      std::cout << "  Argument " << i << ": " << clang_getCString(argSpelling) << std::endl;
      clang_disposeString(argSpelling);
    }

    return CXChildVisit_Continue;
  }

  std::string mainSourceFile_;
  LuauCodeGen *output;
  CXTranslationUnit translationUnit_;
};

int main(const int argc, char *argv[]) {
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
  LuauCodeGen luaOutput;
  luaOutput.writeln("-- im mentally unstable now --");

  AstVisitor visitor(mainSourceFile);
  visitor.setOutput(&luaOutput);
  visitor.setTranslationUnit(unit);

  const CXCursor rootCursor = clang_getTranslationUnitCursor(unit);
  clang_visitChildren(
      rootCursor, [](CXCursor c, CXCursor p, CXClientData d) { return static_cast<AstVisitor *>(d)->visit(c, p); },
      &visitor);

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

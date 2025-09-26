#pragma once

#include <clang-c/Index.h>
#include <string>
#include <vector>

namespace roblox_transpiler {
  class ClangUtils {
  public:
    // Cursor utilities
    static std::string getCursorSpelling(const CXCursor &cursor);
    static std::string getCursorKindSpelling(const CXCursor &cursor);
    static std::string getTypeSpelling(const CXType &type);

    // Source location utilities
    static bool isFromMainFile(const CXCursor &cursor, const std::string &mainFile);
    static std::string getFilename(const CXCursor &cursor);

    // Literal value extraction
    static std::string getIntegerLiteralValue(const CXCursor &cursor, CXTranslationUnit unit);
    static std::string getStringLiteralValue(const CXCursor &cursor, CXTranslationUnit unit);
    static std::string getFloatLiteralValue(const CXCursor &cursor, CXTranslationUnit unit);

    // Type utilities
    static bool isPointerType(const CXType &type);
    static bool isReferenceType(const CXType &type);
    static bool isConstType(const CXType &type);
    static CXType getPointeeType(const CXType &type);

    // Function utilities
    static std::vector<CXType> getFunctionParameterTypes(const CXCursor &cursor);
    static CXType getFunctionReturnType(const CXCursor &cursor);

    // Token extraction utility
    static std::string extractTokenSpelling(const CXCursor &cursor, CXTranslationUnit unit);

  private:
  };
} // namespace roblox_transpiler

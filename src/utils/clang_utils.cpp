#include "clang_utils.h"
#include <iostream>

namespace roblox_transpiler {
  std::string ClangUtils::getCursorSpelling(const CXCursor &cursor) {
    CXString spelling = clang_getCursorSpelling(cursor);
    std::string result = clang_getCString(spelling);
    clang_disposeString(spelling);
    return result;
  }

  std::string ClangUtils::getCursorKindSpelling(const CXCursor &cursor) {
    CXCursorKind kind = clang_getCursorKind(cursor);
    CXString spelling = clang_getCursorKindSpelling(kind);
    std::string result = clang_getCString(spelling);
    clang_disposeString(spelling);
    return result;
  }

  std::string ClangUtils::getTypeSpelling(const CXType &type) {
    CXString spelling = clang_getTypeSpelling(type);
    std::string result = clang_getCString(spelling);
    clang_disposeString(spelling);
    return result;
  }

  bool ClangUtils::isFromMainFile(const CXCursor &cursor, const std::string &mainFile) {
    CXSourceLocation location = clang_getCursorLocation(cursor);
    CXFile file;
    clang_getSpellingLocation(location, &file, nullptr, nullptr, nullptr);

    if (file == nullptr) {
      return false;
    }

    CXString fileName = clang_getFileName(file);
    std::string fileNameStr = clang_getCString(fileName);
    clang_disposeString(fileName);

    return fileNameStr == mainFile;
  }

  std::string ClangUtils::getFilename(const CXCursor &cursor) {
    CXSourceLocation location = clang_getCursorLocation(cursor);
    CXFile file;
    clang_getSpellingLocation(location, &file, nullptr, nullptr, nullptr);

    if (file == nullptr) {
      return "";
    }

    CXString fileName = clang_getFileName(file);
    std::string result = clang_getCString(fileName);
    clang_disposeString(fileName);
    return result;
  }

  std::string ClangUtils::extractTokenSpelling(const CXCursor &cursor, CXTranslationUnit unit) {
    CXToken *tokens;
    unsigned numTokens;
    clang_tokenize(unit, clang_getCursorExtent(cursor), &tokens, &numTokens);

    std::string result;
    if (numTokens > 0) {
      CXString tokenSpelling = clang_getTokenSpelling(unit, tokens[0]);
      result = clang_getCString(tokenSpelling);
      clang_disposeString(tokenSpelling);
    }

    clang_disposeTokens(unit, tokens, numTokens);
    return result;
  }

  std::string ClangUtils::getIntegerLiteralValue(const CXCursor &cursor, CXTranslationUnit unit) {
    return extractTokenSpelling(cursor, unit);
  }

  std::string ClangUtils::getStringLiteralValue(const CXCursor &cursor, CXTranslationUnit unit) {
    std::string raw = extractTokenSpelling(cursor, unit);
    // Remove quotes if present
    if (raw.length() >= 2 && raw.front() == '"' && raw.back() == '"') {
      return raw.substr(1, raw.length() - 2);
    }
    return raw;
  }

  std::string ClangUtils::getFloatLiteralValue(const CXCursor &cursor, CXTranslationUnit unit) {
    return extractTokenSpelling(cursor, unit);
  }

  bool ClangUtils::isPointerType(const CXType &type) { return type.kind == CXType_Pointer; }

  bool ClangUtils::isReferenceType(const CXType &type) {
    return type.kind == CXType_LValueReference || type.kind == CXType_RValueReference;
  }

  bool ClangUtils::isConstType(const CXType &type) { return clang_isConstQualifiedType(type) != 0; }

  CXType ClangUtils::getPointeeType(const CXType &type) { return clang_getPointeeType(type); }

  std::vector<CXType> ClangUtils::getFunctionParameterTypes(const CXCursor &cursor) {
    std::vector<CXType> paramTypes;
    CXType funcType = clang_getCursorType(cursor);
    int numParams = clang_getNumArgTypes(funcType);

    for (int i = 0; i < numParams; ++i) {
      paramTypes.push_back(clang_getArgType(funcType, i));
    }

    return paramTypes;
  }

  CXType ClangUtils::getFunctionReturnType(const CXCursor &cursor) {
    CXType funcType = clang_getCursorType(cursor);
    return clang_getResultType(funcType);
  }
} // namespace roblox_transpiler

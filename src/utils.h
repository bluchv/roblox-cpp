//
// Created by bluec on 1/22/2025.
//

#ifndef UTILS_H
#define UTILS_H

#include <clang-c/Index.h>
#include <string>
#include <vector>

namespace utils {
  inline std::string getCursorSpelling(const CXCursor &cursor) {
    const CXString spelling = clang_getCursorSpelling(cursor);
    std::string result = clang_getCString(spelling);
    clang_disposeString(spelling);
    return result;
  }

  inline std::string getCursorKind(CXCursorKind kind);

  inline std::string getIntegerLiteralValue(const CXCursor &cursor, CXTranslationUnit translationUnit) {
    CXToken *tokens;
    unsigned numTokens;
    clang_tokenize(translationUnit, clang_getCursorExtent(cursor), &tokens, &numTokens);
    std::string result;

    if (numTokens > 0) {
      const CXString tokenSpelling = clang_getTokenSpelling(translationUnit, tokens[0]);
      result = clang_getCString(tokenSpelling);
      clang_disposeString(tokenSpelling);
    }

    clang_disposeTokens(translationUnit, tokens, numTokens);
    return result;
  }
  inline std::vector<std::string> IgnoreMap = {
      "operator<<",
      "endl",
  };

} // namespace utils

#endif // UTILS_H

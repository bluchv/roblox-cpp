#pragma once

#include <clang-c/Index.h>
#include <memory>
#include <stack>
#include <string>
#include "../ast/luau_node.h"

namespace roblox_transpiler {
  class ASTVisitor {
  public:
    explicit ASTVisitor(std::string mainSourceFile);

    void setTranslationUnit(CXTranslationUnit unit);
    void setRootNode(std::shared_ptr<LuauNode> root);

    CXChildVisitResult visit(CXCursor cursor, CXCursor parent);

    std::shared_ptr<LuauNode> getRootNode() const { return rootNode_; }

  private:
    std::string mainSourceFile_;
    CXTranslationUnit translationUnit_;
    std::shared_ptr<LuauNode> rootNode_;
    std::stack<LuauNode *> contextStack_; // For tracking current context (function vs global)
    bool isInClassMethod_; // Track if we're currently processing a class method

    // Visit handlers for different cursor kinds
    CXChildVisitResult handleFunctionDecl(const CXCursor &cursor);
    CXChildVisitResult handleVarDecl(const CXCursor &cursor);
    CXChildVisitResult handleCallExpr(const CXCursor &cursor);
    CXChildVisitResult handleReturnStmt(const CXCursor &cursor);
    CXChildVisitResult handleBinaryOperator(const CXCursor &cursor);
    CXChildVisitResult handleDeclRefExpr(const CXCursor &cursor);
    CXChildVisitResult handleClassDecl(const CXCursor &cursor);
    CXChildVisitResult handleIfStmt(const CXCursor &cursor);
    CXChildVisitResult handleMemberRefExpr(const CXCursor &cursor);

    // Helper methods
    bool isFromMainFile(const CXCursor &cursor) const;
    void processFunctionParameters(const CXCursor &cursor, class FunctionNode *funcNode);
    void processFunctionBody(const CXCursor &cursor, FunctionNode *funcNode);
    void processVariableInitializer(const CXCursor &cursor, const std::string &varName, LuauNode *parent);
    void processFunctionCallArguments(const CXCursor &cursor, class FunctionCallNode *callNode);
    void processClassBody(const CXCursor &cursor, class ClassNode *classNode);
    Value extractValueFromCursor(const CXCursor &cursor);

    // Context management
    LuauNode *getCurrentContext() const;
  };
} // namespace roblox_transpiler

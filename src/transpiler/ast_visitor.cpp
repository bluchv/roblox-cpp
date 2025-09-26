#include "ast_visitor.h"
#include <iostream>
#include "../ast/class_node.h"
#include "../ast/function_node.h"
#include "../ast/statement_node.h"
#include "../ast/variable_node.h"
#include "../utils/clang_utils.h"

namespace roblox_transpiler {
  ASTVisitor::ASTVisitor(std::string mainSourceFile) :
      mainSourceFile_(std::move(mainSourceFile)), translationUnit_(nullptr), isInClassMethod_(false) {}

  void ASTVisitor::setTranslationUnit(CXTranslationUnit unit) { translationUnit_ = unit; }

  void ASTVisitor::setRootNode(std::shared_ptr<LuauNode> root) {
    rootNode_ = root;
    contextStack_.push(rootNode_.get()); // Initialize with root context
  }

  LuauNode *ASTVisitor::getCurrentContext() const {
    return contextStack_.empty() ? rootNode_.get() : contextStack_.top();
  }

  CXChildVisitResult ASTVisitor::visit(CXCursor cursor, CXCursor parent) {
    // Skip system/standard library cursors by checking the file
    if (!isFromMainFile(cursor)) {
      return CXChildVisit_Continue;
    }

    CXCursorKind kind = clang_getCursorKind(cursor);

    switch (kind) {
      case CXCursor_FunctionDecl:
        return handleFunctionDecl(cursor);
      case CXCursor_VarDecl:
        return handleVarDecl(cursor);
      case CXCursor_CallExpr:
        return handleCallExpr(cursor);
      case CXCursor_ReturnStmt:
        return handleReturnStmt(cursor);
      case CXCursor_BinaryOperator:
        return handleBinaryOperator(cursor);
      case CXCursor_DeclRefExpr:
        return handleDeclRefExpr(cursor);
      case CXCursor_ClassDecl:
        return handleClassDecl(cursor);
      case CXCursor_CXXMethod:
        return handleFunctionDecl(cursor); // Handle class methods as functions for now
      case CXCursor_FieldDecl:
        return handleVarDecl(cursor); // Handle class member variables
      case CXCursor_Constructor:
        return handleFunctionDecl(cursor); // Handle constructors as functions
      case CXCursor_IfStmt:
        return handleIfStmt(cursor);
      case CXCursor_MemberRefExpr:
        return handleMemberRefExpr(cursor);
      case CXCursor_CompoundStmt:
        // Recurse into compound statements (function bodies, blocks)
        return CXChildVisit_Recurse;
      case CXCursor_IntegerLiteral:
      case CXCursor_FloatingLiteral:
      case CXCursor_StringLiteral:
      case CXCursor_CXXBoolLiteralExpr:
        // These are handled by their parent nodes
        return CXChildVisit_Continue;
      case CXCursor_ParmDecl:
        // Parameters are handled by function processing
        return CXChildVisit_Continue;
      default: {
        // For debugging, uncomment the next lines:
        // std::string kindSpelling = ClangUtils::getCursorKindSpelling(cursor);
        // std::string spelling = ClangUtils::getCursorSpelling(cursor);
        // if (!spelling.empty()) {
        //   std::cout << "Unknown cursor kind: " << kindSpelling
        //             << " with spelling: " << spelling << std::endl;
        // }
        return CXChildVisit_Recurse;
      }
    }
  }

  bool ASTVisitor::isFromMainFile(const CXCursor &cursor) const {
    return ClangUtils::isFromMainFile(cursor, mainSourceFile_);
  }

  CXChildVisitResult ASTVisitor::handleFunctionDecl(const CXCursor &cursor) {
    std::string functionName = ClangUtils::getCursorSpelling(cursor);
    CXType returnType = ClangUtils::getFunctionReturnType(cursor);
    std::string returnTypeStr = ClangUtils::getTypeSpelling(returnType);

    CXCursorKind kind = clang_getCursorKind(cursor);

    auto functionNode = std::make_unique<FunctionNode>(functionName, returnTypeStr);
    FunctionNode *funcPtr = functionNode.get();

    // Process parameters
    processFunctionParameters(cursor, funcPtr);

    // Check if we're in a class context
    ClassNode *classCtx = dynamic_cast<ClassNode *>(getCurrentContext());
    if (classCtx != nullptr) {
      // We're in a class context - set the flag
      bool wasInClassMethod = isInClassMethod_;
      isInClassMethod_ = true;

      if (kind == CXCursor_Constructor || functionName == classCtx->getName()) {
        // This is a constructor - rename it to init for Luau
        functionNode = std::make_unique<FunctionNode>("init", returnTypeStr);
        funcPtr = functionNode.get();
        funcPtr->setClassName(classCtx->getName());
        processFunctionParameters(cursor, funcPtr);

        // Process function body
        processFunctionBody(cursor, funcPtr);

        // Add as constructor to class
        classCtx->addConstructor(std::move(functionNode));
      } else {
        // This is a regular method
        funcPtr->setClassName(classCtx->getName());

        // Process function body
        processFunctionBody(cursor, funcPtr);

        // Add as method to class
        classCtx->addMethod(std::move(functionNode));
      }

      // Restore the flag
      isInClassMethod_ = wasInClassMethod;
    } else {
      // Regular function - add to current context
      getCurrentContext()->addChild(std::move(functionNode));

      // Process function body with function as new context
      processFunctionBody(cursor, funcPtr);
    }

    return CXChildVisit_Continue;
  }

  CXChildVisitResult ASTVisitor::handleVarDecl(const CXCursor &cursor) {
    std::string varName = ClangUtils::getCursorSpelling(cursor);
    CXType type = clang_getCursorType(cursor);
    std::string typeStr = ClangUtils::getTypeSpelling(type);

    // Check if we're in a class context
    ClassNode *classCtx = dynamic_cast<ClassNode *>(getCurrentContext());
    if (classCtx != nullptr) {
      // This is a class member variable
      classCtx->addMember(varName, typeStr);
    } else {
      // Regular variable declaration - temporarily disable class method flag for local vars
      bool wasInClassMethod = isInClassMethod_;
      isInClassMethod_ = false;
      processVariableInitializer(cursor, varName, getCurrentContext());
      isInClassMethod_ = wasInClassMethod;
    }

    return CXChildVisit_Continue;
  }

  CXChildVisitResult ASTVisitor::handleCallExpr(const CXCursor &cursor) {
    std::string functionName = ClangUtils::getCursorSpelling(clang_getCursorReferenced(cursor));

    // Check if this is a constructor call (function name matches a known class)
    // For now, we'll assume any function starting with uppercase is a constructor
    if (!functionName.empty() && std::isupper(functionName[0])) {
      // This is likely a constructor call - convert to .new()
      functionName = functionName + ".new";
    }

    auto callNode = std::make_unique<FunctionCallNode>(functionName);

    // Process function call arguments
    processFunctionCallArguments(cursor, callNode.get());

    getCurrentContext()->addChild(std::move(callNode));
    return CXChildVisit_Continue;
  }

  CXChildVisitResult ASTVisitor::handleReturnStmt(const CXCursor &cursor) {
    // Check if there's a return value
    bool hasReturnValue = false;
    Value returnValue = std::string("");

    struct ReturnVisitorData {
      bool *hasValue;
      Value *value;
      CXTranslationUnit translationUnit;
    };

    ReturnVisitorData returnData = {&hasReturnValue, &returnValue, translationUnit_};

    clang_visitChildren(
        cursor,
        [](CXCursor child, CXCursor parent, CXClientData clientData) -> CXChildVisitResult {
          auto *data = static_cast<ReturnVisitorData *>(clientData);
          CXCursorKind childKind = clang_getCursorKind(child);

          if (childKind == CXCursor_IntegerLiteral) {
            // Extract integer literal
            CXToken *tokens;
            unsigned numTokens;
            clang_tokenize(data->translationUnit, clang_getCursorExtent(child), &tokens, &numTokens);
            if (numTokens > 0) {
              CXString tokenSpelling = clang_getTokenSpelling(data->translationUnit, tokens[0]);
              std::string value = clang_getCString(tokenSpelling);
              *(data->value) = std::stoi(value);
              *(data->hasValue) = true;
              clang_disposeString(tokenSpelling);
            }
            clang_disposeTokens(data->translationUnit, tokens, numTokens);
            return CXChildVisit_Break;
          } else if (childKind == CXCursor_DeclRefExpr || childKind == 100) {
            // Variable reference (handle both DeclRefExpr and the specific case we're seeing)
            std::string varName = ClangUtils::getCursorSpelling(child);
            *(data->value) = varName;
            *(data->hasValue) = true;
            return CXChildVisit_Break;
          } else if (childKind == CXCursor_FloatingLiteral) {
            // Extract floating literal
            CXToken *tokens;
            unsigned numTokens;
            clang_tokenize(data->translationUnit, clang_getCursorExtent(child), &tokens, &numTokens);
            if (numTokens > 0) {
              CXString tokenSpelling = clang_getTokenSpelling(data->translationUnit, tokens[0]);
              std::string value = clang_getCString(tokenSpelling);
              *(data->value) = std::stod(value);
              *(data->hasValue) = true;
              clang_disposeString(tokenSpelling);
            }
            clang_disposeTokens(data->translationUnit, tokens, numTokens);
            return CXChildVisit_Break;
          } else {
            // Handle any other cursor type that might be a variable reference
            std::string cursorSpelling = ClangUtils::getCursorSpelling(child);
            if (!cursorSpelling.empty()) {
              *(data->value) = cursorSpelling;
              *(data->hasValue) = true;
              return CXChildVisit_Break;
            }
          }

          return CXChildVisit_Continue;
        },
        &returnData);

    if (hasReturnValue) {
      auto returnNode = std::make_unique<ReturnNode>(returnValue);
      getCurrentContext()->addChild(std::move(returnNode));
    } else {
      auto returnNode = std::make_unique<ReturnNode>();
      getCurrentContext()->addChild(std::move(returnNode));
    }

    return CXChildVisit_Continue;
  }

  CXChildVisitResult ASTVisitor::handleBinaryOperator(const CXCursor &cursor) {
    // Handle assignment operations like "x = 5"
    CXCursor lhs, rhs;
    bool foundLhs = false, foundRhs = false;

    struct BinaryOpData {
      CXCursor cursors[2];
      bool found[2];
    };

    BinaryOpData data = {{}, {false, false}};

    clang_visitChildren(
        cursor,
        [](CXCursor child, CXCursor parent, CXClientData clientData) -> CXChildVisitResult {
          auto *opData = static_cast<BinaryOpData *>(clientData);

          if (!opData->found[0]) { // LHS not found yet
            opData->cursors[0] = child;
            opData->found[0] = true;
          } else if (!opData->found[1]) { // RHS not found yet
            opData->cursors[1] = child;
            opData->found[1] = true;
            return CXChildVisit_Break;
          }

          return CXChildVisit_Continue;
        },
        &data);

    // Check if this is an assignment operator
    if (data.found[0] && data.found[1]) {
      // Get LHS variable name (should be a DeclRefExpr)
      std::string variableName = ClangUtils::getCursorSpelling(data.cursors[0]);

      if (!variableName.empty()) {
        // Check if we're in a class method and this is a member variable
        if (isInClassMethod_) {
          if (variableName == "health" || variableName == "damage" || variableName == "weapon" ||
              variableName == "mana" || variableName == "level" || variableName == "experience") {
            variableName = "self." + variableName;
          }
        }

        // Extract RHS value
        Value rhsValue = extractValueFromCursor(data.cursors[1]);

        // Create assignment node
        auto assignmentNode = std::make_unique<AssignmentNode>(variableName, rhsValue);
        getCurrentContext()->addChild(std::move(assignmentNode));

        return CXChildVisit_Continue;
      }
    }

    // For other binary operations, just recurse into children
    return CXChildVisit_Recurse;
  }

  CXChildVisitResult ASTVisitor::handleDeclRefExpr(const CXCursor &cursor) {
    // Handle variable references - these are usually handled by their parent nodes
    return CXChildVisit_Continue;
  }

  void ASTVisitor::processFunctionParameters(const CXCursor &cursor, FunctionNode *funcNode) {
    clang_visitChildren(
        cursor,
        [](CXCursor child, CXCursor parent, CXClientData clientData) -> CXChildVisitResult {
          auto *visitor = static_cast<ASTVisitor *>(clientData);
          auto *funcNode = static_cast<FunctionNode *>(clientData);

          if (clang_getCursorKind(child) == CXCursor_ParmDecl) {
            std::string paramName = ClangUtils::getCursorSpelling(child);
            CXType paramType = clang_getCursorType(child);
            std::string typeStr = ClangUtils::getTypeSpelling(paramType);

            funcNode->addParameter(paramName, typeStr);
          }

          return CXChildVisit_Continue;
        },
        funcNode);
  }

  void ASTVisitor::processFunctionBody(const CXCursor &cursor, FunctionNode *funcNode) {
    struct VisitorData {
      ASTVisitor *visitor;
      FunctionNode *parentNode;
    };

    VisitorData data = {this, funcNode};

    // Push function context onto stack
    contextStack_.push(funcNode);

    clang_visitChildren(
        cursor,
        [](CXCursor child, CXCursor parent, CXClientData clientData) -> CXChildVisitResult {
          auto *data = static_cast<VisitorData *>(clientData);
          return data->visitor->visit(child, parent);
        },
        &data);

    // Pop function context
    contextStack_.pop();
  }

  void ASTVisitor::processVariableInitializer(const CXCursor &cursor, const std::string &varName, LuauNode *parent) {
    struct VisitorData {
      ASTVisitor *visitor;
      std::string varName;
      LuauNode *parent;
      bool foundInitializer;
      bool isInClassMethod;
    };

    VisitorData data = {this, varName, parent, false, isInClassMethod_};

    clang_visitChildren(
        cursor,
        [](CXCursor child, CXCursor parentCursor, CXClientData clientData) -> CXChildVisitResult {
          auto *data = static_cast<VisitorData *>(clientData);
          CXCursorKind childKind = clang_getCursorKind(child);

          // Determine the actual variable name to use
          std::string actualVarName = data->varName;
          if (data->isInClassMethod) {
            // Check if this variable name matches any known class members
            // For now, assume common member variables should use self.
            if (data->varName == "health" || data->varName == "damage" || data->varName == "weapon" ||
                data->varName == "mana" || data->varName == "level" || data->varName == "experience") {
              actualVarName = "self." + data->varName;
            }
          }

          if (childKind == CXCursor_IntegerLiteral) {
            std::string value = ClangUtils::getIntegerLiteralValue(child, data->visitor->translationUnit_);
            int intValue = std::stoi(value);
            auto varNode = std::make_unique<VariableNode>(actualVarName, intValue);
            data->parent->addChild(std::move(varNode));
            data->foundInitializer = true;
            return CXChildVisit_Break;
          } else if (childKind == CXCursor_StringLiteral) {
            std::string value = ClangUtils::getStringLiteralValue(child, data->visitor->translationUnit_);
            // Mark as string literal to ensure proper quoting
            auto varNode = std::make_unique<VariableNode>(actualVarName, "STRING_LITERAL:" + value);
            data->parent->addChild(std::move(varNode));
            data->foundInitializer = true;
            return CXChildVisit_Break;
          } else if (childKind == CXCursor_FloatingLiteral) {
            std::string value = ClangUtils::getFloatLiteralValue(child, data->visitor->translationUnit_);
            double doubleValue = std::stod(value);
            auto varNode = std::make_unique<VariableNode>(actualVarName, doubleValue);
            data->parent->addChild(std::move(varNode));
            data->foundInitializer = true;
            return CXChildVisit_Break;
          } else if (childKind == CXCursor_CXXBoolLiteralExpr) {
            std::string value = ClangUtils::extractTokenSpelling(child, data->visitor->translationUnit_);
            bool boolValue = (value == "true");
            auto varNode = std::make_unique<VariableNode>(actualVarName, boolValue);
            data->parent->addChild(std::move(varNode));
            data->foundInitializer = true;
            return CXChildVisit_Break;
          } else if (childKind == CXCursor_CallExpr) {
            // Handle function call as initializer - we'll render this as var = functionCall()
            std::string functionName = ClangUtils::getCursorSpelling(clang_getCursorReferenced(child));

            // Create a temporary function call node to collect arguments
            auto tempCallNode = std::make_unique<FunctionCallNode>(functionName);
            data->visitor->processFunctionCallArguments(child, tempCallNode.get());

            // Create function call string with arguments
            std::string callStr = "FUNCTION_CALL_WITH_ARGS:" + functionName;
            for (const auto &arg: tempCallNode->getArguments()) {
              callStr += ":" + std::visit(
                                   [](const auto &v) -> std::string {
                                     using T = std::decay_t<decltype(v)>;
                                     if constexpr (std::is_same_v<T, std::string>) {
                                       return v;
                                     } else if constexpr (std::is_same_v<T, bool>) {
                                       return v ? "true" : "false";
                                     } else {
                                       return std::to_string(v);
                                     }
                                   },
                                   arg);
            }

            auto varNode = std::make_unique<VariableNode>(actualVarName, callStr);
            data->parent->addChild(std::move(varNode));
            data->foundInitializer = true;
            return CXChildVisit_Break;
          } else if (childKind == CXCursor_UnexposedExpr) {
            // Recurse into unexposed expressions to find the actual literal
            clang_visitChildren(
                child,
                [](CXCursor grandChild, CXCursor parent, CXClientData clientData) -> CXChildVisitResult {
                  auto *data = static_cast<VisitorData *>(clientData);
                  CXCursorKind grandChildKind = clang_getCursorKind(grandChild);

                  // Determine the actual variable name to use
                  std::string actualVarName = data->varName;
                  if (data->isInClassMethod) {
                    // Check if this variable name matches any known class members
                    if (data->varName == "health" || data->varName == "damage" || data->varName == "weapon" ||
                        data->varName == "mana" || data->varName == "level" || data->varName == "experience") {
                      actualVarName = "self." + data->varName;
                    }
                  }

                  if (grandChildKind == CXCursor_StringLiteral) {
                    std::string value = ClangUtils::getStringLiteralValue(grandChild, data->visitor->translationUnit_);
                    // Mark as string literal to ensure proper quoting
                    auto varNode = std::make_unique<VariableNode>(actualVarName, "STRING_LITERAL:" + value);
                    data->parent->addChild(std::move(varNode));
                    data->foundInitializer = true;
                    return CXChildVisit_Break;
                  } else if (grandChildKind == CXCursor_IntegerLiteral) {
                    std::string value = ClangUtils::getIntegerLiteralValue(grandChild, data->visitor->translationUnit_);
                    int intValue = std::stoi(value);
                    auto varNode = std::make_unique<VariableNode>(actualVarName, intValue);
                    data->parent->addChild(std::move(varNode));
                    data->foundInitializer = true;
                    return CXChildVisit_Break;
                  } else if (grandChildKind == CXCursor_UnexposedExpr) {
                    // Recurse deeper for nested UnexposedExpr
                    return CXChildVisit_Recurse;
                  }
                  return CXChildVisit_Continue;
                },
                data);
            if (data->foundInitializer) {
              return CXChildVisit_Break;
            }
          }

          return CXChildVisit_Continue;
        },
        &data);

    // If no initializer was found, create variable with nil/default value
    if (!data.foundInitializer) {
      auto varNode = std::make_unique<VariableNode>(data.varName, std::string("nil"));
      data.parent->addChild(std::move(varNode));
    }
  }

  void ASTVisitor::processFunctionCallArguments(const CXCursor &cursor, FunctionCallNode *callNode) {
    clang_visitChildren(
        cursor,
        [](CXCursor child, CXCursor parent, CXClientData clientData) -> CXChildVisitResult {
          auto *callNode = static_cast<FunctionCallNode *>(clientData);
          CXCursorKind childKind = clang_getCursorKind(child);

          if (childKind == CXCursor_IntegerLiteral) {
            CXToken *tokens;
            unsigned numTokens;
            CXTranslationUnit tu = clang_Cursor_getTranslationUnit(child);
            clang_tokenize(tu, clang_getCursorExtent(child), &tokens, &numTokens);
            if (numTokens > 0) {
              CXString tokenSpelling = clang_getTokenSpelling(tu, tokens[0]);
              std::string value = clang_getCString(tokenSpelling);
              callNode->addArgument(std::stoi(value));
              clang_disposeString(tokenSpelling);
            }
            clang_disposeTokens(tu, tokens, numTokens);
          } else if (childKind == CXCursor_FloatingLiteral) {
            CXToken *tokens;
            unsigned numTokens;
            CXTranslationUnit tu = clang_Cursor_getTranslationUnit(child);
            clang_tokenize(tu, clang_getCursorExtent(child), &tokens, &numTokens);
            if (numTokens > 0) {
              CXString tokenSpelling = clang_getTokenSpelling(tu, tokens[0]);
              std::string value = clang_getCString(tokenSpelling);
              callNode->addArgument(std::stod(value));
              clang_disposeString(tokenSpelling);
            }
            clang_disposeTokens(tu, tokens, numTokens);
          } else if (childKind == CXCursor_StringLiteral) {
            CXToken *tokens;
            unsigned numTokens;
            CXTranslationUnit tu = clang_Cursor_getTranslationUnit(child);
            clang_tokenize(tu, clang_getCursorExtent(child), &tokens, &numTokens);
            if (numTokens > 0) {
              CXString tokenSpelling = clang_getTokenSpelling(tu, tokens[0]);
              std::string value = clang_getCString(tokenSpelling);
              // Remove quotes from string literal
              if (value.length() >= 2 && value[0] == '"' && value.back() == '"') {
                value = value.substr(1, value.length() - 2);
              }
              callNode->addArgument(value);
              clang_disposeString(tokenSpelling);
            }
            clang_disposeTokens(tu, tokens, numTokens);
          } else if (childKind == CXCursor_DeclRefExpr) {
            // Variable reference as argument
            std::string varName = ClangUtils::getCursorSpelling(child);
            callNode->addArgument(varName);
          }

          return CXChildVisit_Continue;
        },
        callNode);
  }

  Value ASTVisitor::extractValueFromCursor(const CXCursor &cursor) {
    CXCursorKind kind = clang_getCursorKind(cursor);

    switch (kind) {
      case CXCursor_IntegerLiteral: {
        std::string value = ClangUtils::getIntegerLiteralValue(cursor, translationUnit_);
        return std::stoi(value);
      }
      case CXCursor_FloatingLiteral: {
        std::string value = ClangUtils::getFloatLiteralValue(cursor, translationUnit_);
        return std::stod(value);
      }
      case CXCursor_StringLiteral: {
        std::string value = ClangUtils::getStringLiteralValue(cursor, translationUnit_);
        return "STRING_LITERAL:" + value;
      }
      case CXCursor_CXXBoolLiteralExpr: {
        std::string value = ClangUtils::extractTokenSpelling(cursor, translationUnit_);
        return (value == "true");
      }
      case CXCursor_DeclRefExpr:
      case 100: { // Handle the specific cursor type we've seen for variable references
        std::string varName = ClangUtils::getCursorSpelling(cursor);
        return varName;
      }
      case CXCursor_BinaryOperator: {
        // Handle binary expressions like "a + b"
        std::string expression = "";
        struct BinaryExprData {
          std::string *result;
          ASTVisitor *visitor;
          bool first;
        };

        BinaryExprData data = {&expression, this, true};

        clang_visitChildren(
            cursor,
            [](CXCursor child, CXCursor parent, CXClientData clientData) -> CXChildVisitResult {
              auto *exprData = static_cast<BinaryExprData *>(clientData);

              if (!exprData->first) {
                // Add operator - for now just use + (could be enhanced to detect actual operator)
                *(exprData->result) += " + ";
              }
              exprData->first = false;

              // Recursively extract the operand value
              Value operandValue = exprData->visitor->extractValueFromCursor(child);
              std::string operandStr = std::visit(
                  [](const auto &v) -> std::string {
                    using T = std::decay_t<decltype(v)>;
                    if constexpr (std::is_same_v<T, std::string>) {
                      return v;
                    } else if constexpr (std::is_same_v<T, bool>) {
                      return v ? "true" : "false";
                    } else {
                      return std::to_string(v);
                    }
                  },
                  operandValue);

              *(exprData->result) += operandStr;
              return CXChildVisit_Continue;
            },
            &data);

        return "EXPRESSION:" + expression;
      }
      default: {
        // Try to get cursor spelling as fallback for variable references
        std::string spelling = ClangUtils::getCursorSpelling(cursor);
        if (!spelling.empty()) {
          return spelling;
        }
        return std::string("nil");
      }
    }
  }

  CXChildVisitResult ASTVisitor::handleClassDecl(const CXCursor &cursor) {
    std::string className = ClangUtils::getCursorSpelling(cursor);

    auto classNode = std::make_unique<ClassNode>(className);
    ClassNode *classPtr = classNode.get();

    // Add class to current context (should be root for top-level classes)
    getCurrentContext()->addChild(std::move(classNode));

    // Process class body with class as new context
    processClassBody(cursor, classPtr);

    return CXChildVisit_Continue;
  }

  void ASTVisitor::processClassBody(const CXCursor &cursor, ClassNode *classNode) {
    // Push class context
    contextStack_.push(classNode);

    // Visit all children of the class declaration
    clang_visitChildren(
        cursor,
        [](CXCursor child, CXCursor parent, CXClientData clientData) -> CXChildVisitResult {
          auto *visitor = static_cast<ASTVisitor *>(clientData);
          return visitor->visit(child, parent);
        },
        this);

    // Pop class context
    contextStack_.pop();
  }

  CXChildVisitResult ASTVisitor::handleIfStmt(const CXCursor &cursor) {
    // For now, create a simple function call representing the if statement
    // In a more complete implementation, we'd parse the condition and body properly
    auto ifNode = std::make_unique<FunctionCallNode>("if");

    // Visit children to process condition and body
    clang_visitChildren(
        cursor,
        [](CXCursor child, CXCursor parent, CXClientData clientData) -> CXChildVisitResult {
          auto *visitor = static_cast<ASTVisitor *>(clientData);
          return visitor->visit(child, parent);
        },
        this);

    getCurrentContext()->addChild(std::move(ifNode));
    return CXChildVisit_Continue;
  }

  CXChildVisitResult ASTVisitor::handleMemberRefExpr(const CXCursor &cursor) {
    // This handles method calls like obj.method() -> obj:method()
    std::string memberName = ClangUtils::getCursorSpelling(cursor);

    // For now, just create a function call node
    // In a more sophisticated implementation, we'd track the object and convert to obj:method
    auto callNode = std::make_unique<FunctionCallNode>(memberName);

    getCurrentContext()->addChild(std::move(callNode));
    return CXChildVisit_Continue;
  }
} // namespace roblox_transpiler

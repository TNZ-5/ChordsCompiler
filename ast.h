#ifndef AST_H
#define AST_H

#include <llvm/IR/Value.h>
#include <vector>
#include <string>

class CodeGenContext;

// Base class for all AST nodes
class ASTNode {
public:
    virtual ~ASTNode() {}
    virtual llvm::Value* codeGen(CodeGenContext& context) = 0;
};

// AST node for variables
class VariableNode : public ASTNode {
public:
    std::string name;
    VariableNode(const std::string& name) : name(name) {}
    llvm::Value* codeGen(CodeGenContext& context) override;
};

// AST node for constants
class ConstantNode : public ASTNode {
public:
    int value;
    ConstantNode(int value) : value(value) {}
    llvm::Value* codeGen(CodeGenContext& context) override;
};

// AST node for boolean constants
class BooleanConstantNode : public ASTNode {
public:
    bool value;
    BooleanConstantNode(bool value) : value(value) {}
    llvm::Value* codeGen(CodeGenContext& context) override;
};

// AST node for binary operators
class BinaryOpNode : public ASTNode {
public:
    char op;
    ASTNode* lhs;
    ASTNode* rhs;
    BinaryOpNode(char op, ASTNode* lhs, ASTNode* rhs)
        : op(op), lhs(lhs), rhs(rhs) {}
    llvm::Value* codeGen(CodeGenContext& context) override;
};

// AST node for unary operators
class UnaryOpNode : public ASTNode {
public:
    char op;
    ASTNode* operand;
    UnaryOpNode(char op, ASTNode* operand)
        : op(op), operand(operand) {}
    llvm::Value* codeGen(CodeGenContext& context) override;
};

// AST node for if statements
class IfNode : public ASTNode {
public:
    ASTNode* condition;
    ASTNode* thenStmt;
    ASTNode* elseStmt;
    IfNode(ASTNode* condition, ASTNode* thenStmt, ASTNode* elseStmt)
        : condition(condition), thenStmt(thenStmt), elseStmt(elseStmt) {}
    llvm::Value* codeGen(CodeGenContext& context) override;
};

// AST node for while loops
class WhileNode : public ASTNode {
public:
    ASTNode* condition;
    ASTNode* body;
    WhileNode(ASTNode* condition, ASTNode* body)
        : condition(condition), body(body) {}
    llvm::Value* codeGen(CodeGenContext& context) override;
};

// AST node for for loops
class ForNode : public ASTNode {
public:
    ASTNode* init;
    ASTNode* condition;
    ASTNode* update;
    ASTNode* body;
    ForNode(ASTNode* init, ASTNode* condition, ASTNode* update, ASTNode* body)
        : init(init), condition(condition), update(update), body(body) {}
    llvm::Value* codeGen(CodeGenContext& context) override;
};

// AST node for break statements
class BreakNode : public ASTNode {
public:
    BreakNode() {}
    llvm::Value* codeGen(CodeGenContext& context) override;
};

// AST node for continue statements
class ContinueNode : public ASTNode {
public:
    ContinueNode() {}
    llvm::Value* codeGen(CodeGenContext& context) override;
};

// AST node for return statements
class ReturnNode : public ASTNode {
public:
    ASTNode* expr;
    ReturnNode(ASTNode* expr) : expr(expr) {}
    llvm::Value* codeGen(CodeGenContext& context) override;
};

// AST node for function declarations
class FunctionNode : public ASTNode {
public:
    std::string name;
    std::vector<std::string> argNames;
    std::vector<ASTNode*> body;
    FunctionNode(const std::string& name, const std::vector<std::string>& argNames, const std::vector<ASTNode*>& body)
        : name(name), argNames(argNames), body(body) {}
    llvm::Value* codeGen(CodeGenContext& context) override;
};

// AST node for function calls
class FunctionCallNode : public ASTNode {
public:
    std::string name;
    std::vector<ASTNode*> args;
    FunctionCallNode(const std::string& name, const std::vector<ASTNode*>& args)
        : name(name), args(args) {}
    llvm::Value* codeGen(CodeGenContext& context) override;
};

// AST node for declarations
class DeclarationNode : public ASTNode {
public:
    std::string type;
    std::vector<std::string> varNames;
    std::vector<ASTNode*> initializers;
    DeclarationNode(const std::string& type, const std::vector<std::string>& varNames, const std::vector<ASTNode*>& initializers)
        : type(type), varNames(varNames), initializers(initializers) {}
    llvm::Value* codeGen(CodeGenContext& context) override;
};

// AST node for assignments
class AssignmentNode : public ASTNode {
public:
    std::string varName;
    ASTNode* expr;
    AssignmentNode(const std::string& varName, ASTNode* expr)
        : varName(varName), expr(expr) {}
    llvm::Value* codeGen(CodeGenContext& context) override;
};

#endif
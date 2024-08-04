#include "ast.h"
#include "codegen.h"
#include <llvm/IR/Constants.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Type.h>

llvm::Value* VariableNode::codeGen(CodeGenContext& context) {
    // Look up the variable in the symbol table
    llvm::Value* value = context.locals[name];
    if (!value) {
        // If the variable is not found, generate an error
        // ...
    }
    return value;
}

llvm::Value* ConstantNode::codeGen(CodeGenContext& context) {
    // Create a constant LLVM value for the integer
    return llvm::ConstantInt::get(llvm::Type::getInt32Ty(context.context), value, true);
}

llvm::Value* BooleanConstantNode::codeGen(CodeGenContext& context) {
    // Create a constant LLVM value for the boolean
    return llvm::ConstantInt::get(llvm::Type::getInt1Ty(context.context), value, false);
}

llvm::Value* BinaryOpNode::codeGen(CodeGenContext& context) {
    llvm::Value* lhs_value = lhs->codeGen(context);
    llvm::Value* rhs_value = rhs->codeGen(context);

    if (!lhs_value || !rhs_value) {
        // Handle error if either operand is nullptr
        // ...
    }

    switch (op) {
        case '+':
            return context.builder.CreateAdd(lhs_value, rhs_value, "addtmp");
        case '-':
            return context.builder.CreateSub(lhs_value, rhs_value, "subtmp");
        case '*':
            return context.builder.CreateMul(lhs_value, rhs_value, "multmp");
        case '/':
            return context.builder.CreateSDiv(lhs_value, rhs_value, "divtmp");
        // Handle other binary operators as needed
        default:
            // Handle error for unsupported operator
            // ...
            return nullptr;
    }
}

llvm::Value* UnaryOpNode::codeGen(CodeGenContext& context) {
    llvm::Value* operand_value = operand->codeGen(context);

    if (!operand_value) {
        // Handle error if the operand is nullptr
        // ...
    }

    switch (op) {
        case '!':
            return context.builder.CreateNot(operand_value, "nottmp");
        case '-':
            return context.builder.CreateNeg(operand_value, "negtmp");
        // Handle other unary operators as needed
        default:
            // Handle error for unsupported operator
            // ...
            return nullptr;
    }
}

llvm::Value* IfNode::codeGen(CodeGenContext& context) {
    llvm::Function* function = context.builder.GetInsertBlock()->getParent();
    llvm::BasicBlock* then_bb = llvm::BasicBlock::Create(context.context, "then", function);
    llvm::BasicBlock* else_bb = llvm::BasicBlock::Create(context.context, "else");
    llvm::BasicBlock* merge_bb = llvm::BasicBlock::Create(context.context, "merge");

    llvm::Value* cond_value = condition->codeGen(context);
    context.builder.CreateCondBr(cond_value, then_bb, else_bb);

    // Generate code for the 'then' block
    context.builder.SetInsertPoint(then_bb);
    llvm::Value* then_value = thenStmt->codeGen(context);
    context.builder.CreateBr(merge_bb);

    // Generate code for the 'else' block
    function->getBasicBlockList().push_back(else_bb);
    context.builder.SetInsertPoint(else_bb);
    llvm::Value* else_value = nullptr;
    if (elseStmt) {
        else_value = elseStmt->codeGen(context);
    }
    context.builder.CreateBr(merge_bb);

    // Create the merge block
    function->getBasicBlockList().push_back(merge_bb);
    context.builder.SetInsertPoint(merge_bb);

    // Phi node to choose the correct result based on the condition
    llvm::PHINode* phi_node = context.builder.CreatePHI(then_value->getType(), 2, "iftmp");
    phi_node->addIncoming(then_value, then_bb);
    if (else_value) {
        phi_node->addIncoming(else_value, else_bb);
    } else {
        phi_node->addIncoming(llvm::Constant::getNullValue(then_value->getType()), else_bb);
    }

    return phi_node;
}

llvm::Value* WhileNode::codeGen(CodeGenContext& context) {
    llvm::Function* function = context.builder.GetInsertBlock()->getParent();
    llvm::BasicBlock* cond_bb = llvm::BasicBlock::Create(context.context, "cond", function);
    llvm::BasicBlock* body_bb = llvm::BasicBlock::Create(context.context, "body", function);
    llvm::BasicBlock* merge_bb = llvm::BasicBlock::Create(context.context, "merge");

    context.builder.CreateBr(cond_bb);

    // Generate code for the condition block
    function->getBasicBlockList().push_back(cond_bb);
    context.builder.SetInsertPoint(cond_bb);
    llvm::Value* cond_value = condition->codeGen(context);
    context.builder.CreateCondBr(cond_value, body_bb, merge_bb);

    // Generate code for the body block
    function->getBasicBlockList().push_back(body_bb);
    context.builder.SetInsertPoint(body_bb);
    body->codeGen(context);
    context.builder.CreateBr(cond_bb);

    // Create the merge block
    function->getBasicBlockList().push_back(merge_bb);
    context.builder.SetInsertPoint(merge_bb);

    return nullptr; // While loops don't have a return value
}

llvm::Value* ForNode::codeGen(CodeGenContext& context) {
    llvm::Function* function = context.builder.GetInsertBlock()->getParent();
    llvm::BasicBlock* init_bb = llvm::BasicBlock::Create(context.context, "init", function);
    llvm::BasicBlock* cond_bb = llvm::BasicBlock::Create(context.context, "cond", function);
    llvm::BasicBlock* body_bb = llvm::BasicBlock::Create(context.context, "body", function);
    llvm::BasicBlock* update_bb = llvm::BasicBlock::Create(context.context, "update", function);
    llvm::BasicBlock* merge_bb = llvm::BasicBlock::Create(context.context, "merge");

    context.builder.CreateBr(init_bb);

    // Generate code for the initialization block
    function->getBasicBlockList().push_back(init_bb);
    context.builder.SetInsertPoint(init_bb);
    if (init) {
        init->codeGen(context);
    }
    context.builder.CreateBr(cond_bb);

    // Generate code for the condition block
    function->getBasicBlockList().push_back(cond_bb);
    context.builder.SetInsertPoint(cond_bb);
    llvm::Value* cond_value = condition->codeGen(context);
    context.builder.CreateCondBr(cond_value, body_bb, merge_bb);

    // Generate code for the body block
    function->getBasicBlockList().push_back(body_bb);
    context.builder.SetInsertPoint(body_bb);
    llvm::Value* body_value = body->codeGen(context);
context.builder.CreateBr(update_bb);

// Generate code for the update block
function->getBasicBlockList().push_back(update_bb);
context.builder.SetInsertPoint(update_bb);
if (update) {
   update->codeGen(context);
}
context.builder.CreateBr(cond_bb);

// Create the merge block
function->getBasicBlockList().push_back(merge_bb);
context.builder.SetInsertPoint(merge_bb);

return nullptr; // For loops don't have a return value
}

llvm::Value* BreakNode::codeGen(CodeGenContext& context) {
   // Create a branch to the merge block of the enclosing loop
   context.builder.CreateBr(context.breakTarget);
   return nullptr;
}

llvm::Value* ContinueNode::codeGen(CodeGenContext& context) {
   // Create a branch to the update block of the enclosing loop
   context.builder.CreateBr(context.continueTarget);
   return nullptr;
}

llvm::Value* ReturnNode::codeGen(CodeGenContext& context) {
   llvm::Value* return_value = nullptr;
   if (expr) {
       return_value = expr->codeGen(context);
   }
   context.builder.CreateRet(return_value);
   return nullptr; // Return statements don't have a return value
}

llvm::Value* FunctionNode::codeGen(CodeGenContext& context) {
   // Create the function type
   std::vector<llvm::Type*> arg_types;
   for (const auto& arg_name : argNames) {
       arg_types.push_back(llvm::Type::getInt32Ty(context.context));
   }
   llvm::FunctionType* function_type = llvm::FunctionType::get(llvm::Type::getInt32Ty(context.context), arg_types, false);

   // Create the function
   llvm::Function* function = llvm::Function::Create(function_type, llvm::Function::ExternalLinkage, name, context.module);

   // Set the names for function arguments
   unsigned idx = 0;
   for (auto& arg : function->args()) {
       arg.setName(argNames[idx++]);
   }

   // Create a new basic block to start inserting instructions
   llvm::BasicBlock* entry_bb = llvm::BasicBlock::Create(context.context, "entry", function);
   context.builder.SetInsertPoint(entry_bb);

   // Generate code for the function body
   for (const auto& node : body) {
       node->codeGen(context);
   }

   return function;
}

llvm::Value* FunctionCallNode::codeGen(CodeGenContext& context) {
   // Look up the function in the module
   llvm::Function* function = context.module->getFunction(name);
   if (!function) {
       // Handle error if the function is not found
       // ...
   }

   // Generate code for the function arguments
   std::vector<llvm::Value*> arg_values;
   for (const auto& arg_node : args) {
       arg_values.push_back(arg_node->codeGen(context));
   }

   // Call the function
   return context.builder.CreateCall(function, arg_values, "calltmp");
}

llvm::Value* DeclarationNode::codeGen(CodeGenContext& context) {
   for (unsigned i = 0; i < varNames.size(); i++) {
       llvm::AllocaInst* alloc_inst = nullptr;
       if (type == "int") {
           alloc_inst = context.builder.CreateAlloca(llvm::Type::getInt32Ty(context.context), nullptr, varNames[i]);
       } else if (type == "bool") {
           alloc_inst = context.builder.CreateAlloca(llvm::Type::getInt1Ty(context.context), nullptr, varNames[i]);
       }
       // Handle other types as needed

       if (initializers.size() > i) {
           llvm::Value* init_value = initializers[i]->codeGen(context);
           context.builder.CreateStore(init_value, alloc_inst);
       }

       context.locals[varNames[i]] = alloc_inst;
   }

   return nullptr; // Declarations don't have a return value
}

llvm::Value* AssignmentNode::codeGen(CodeGenContext& context) {
   llvm::Value* var_value = context.locals[varName];
   if (!var_value) {
       // Handle error if the variable is not found
       // ...
   }

   llvm::Value* expr_value = expr->codeGen(context);
   context.builder.CreateStore(expr_value, var_value);
   return expr_value;
}
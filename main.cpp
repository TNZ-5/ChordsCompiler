#include <iostream>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Verifier.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Support/TargetRegistry.h>
#include "ast.h"

extern int yyparse();
extern ASTNode* root;

class CodeGenContext {
public:
    llvm::LLVMContext& context;
    llvm::Module* module;
    llvm::IRBuilder<> builder;
    std::map<std::string, llvm::Value*> locals;
    CodeGenContext(llvm::Module* module, llvm::LLVMContext& context)
        : context(context), module(module), builder(context) {}

    void generateCode(ASTNode* node) {
        llvm::Value* value = node->codeGen(*this);
        // Do something with the generated value (e.g., print it)
        // ...
    }
};

llvm::LLVMContext context;
llvm::Module* module = new llvm::Module("chordProg", context);
CodeGenContext codegen_context(module, context);

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <input_file>" << std::endl;
        return 1;
    }

    FILE* input_file = fopen(argv[1], "r");
    if (!input_file) {
        std::cerr << "Error opening file: " << argv[1] << std::endl;
        return 1;
    }

    yyin = input_file;
    yyparse();
    fclose(input_file);

    // Generate LLVM IR from the AST
    codegen_context.generateCode(root);

    // Verify the generated IR
    if (llvm::verifyModule(*module, &llvm::errs())) {
        std::cerr << "Error: LLVM IR verification failed" << std::endl;
        return 1;
    }

    // Initialize the target registry
    llvm::InitializeAllTargetInfos();
    llvm::InitializeAllTargets();
    llvm::InitializeAllTargetMCs();
    llvm::InitializeAllAsmParsers();
    llvm::InitializeAllAsmPrinters();

    // Get the target machine
    auto target_triple = llvm::sys::getDefaultTargetTriple();
    std::string error_message;
    auto target = llvm::TargetRegistry::lookupTarget(target_triple, error_message);
    if (!target) {
        std::cerr << "Error: " << error_message << std::endl;
        return 1;
    }

    auto cpu = "generic";
    auto features = "";
    llvm::TargetOptions options;
    auto reloc_model = llvm::Optional<llvm::Reloc::Model>();
    auto target_machine = target->createTargetMachine(target_triple, cpu, features, options, reloc_model);

    // Optimize the module
    llvm::legacy::PassManager pass_manager;
    llvm::raw_ostream* out = &llvm::errs();
    target_machine->addPassesToEmitFile(pass_manager, *out, nullptr, llvm::CodeGenFileType::CGFT_ObjectFile);
    pass_manager.run(*module);

    // Write the object file
    std::error_code ec;
    llvm::raw_fd_ostream output_file("output.o", ec, llvm::sys::fs::OF_None);
    if (ec) {
        std::cerr << "Error opening output file: " << ec.message() << std::endl;
        return 1;
    }
    output_file << module;
    output_file.flush();

    return 0;
}
#include "IRGenerator.h"

#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/GlobalVariable.h>
#include <llvm/IR/PassManager.h>
#include <llvm/Passes/PassBuilder.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/TargetRegistry.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Target/TargetOptions.h>
#include <llvm/Transforms/Scalar/SimplifyCFG.h>

#include <iostream>

#include "../Utils.h"

IRGenerator::IRGenerator(const Program &program, const bool verbose)
    : program(program), verbose(verbose), builder(context), module(program.fileName, context) {
    pushScope();
}

void IRGenerator::logError(const std::string &msg) { errors.push_back(msg); }

llvm::Type *IRGenerator::getType(ast::DataType type) {
    switch (type) {
    case ast::DataType::VOID:
        return llvm::Type::getVoidTy(context);
    case ast::DataType::INT:
        return llvm::Type::getInt64Ty(context);
    case ast::DataType::FLOAT:
        return llvm::Type::getDoubleTy(context);
    case ast::DataType::BOOL:
        return llvm::Type::getInt1Ty(context);
    default:
        return nullptr;
    }
}

llvm::AllocaInst *IRGenerator::createEntryBlockAlloca(llvm::Type *type, const std::string &name) {
    llvm::BasicBlock *block = builder.GetInsertBlock();
    if (block == nullptr) {
        return nullptr;
    }
    auto function = block->getParent();
    llvm::IRBuilder<> tmpB(&function->getEntryBlock(), function->getEntryBlock().begin());
    return tmpB.CreateAlloca(type, nullptr, name);
}

llvm::Constant *IRGenerator::getInitializer(const ast::DataType &dt) {
    llvm::Type *ty = getType(dt);
    switch (dt) {
    case ast::DataType::FLOAT:
        return llvm::ConstantFP::get(ty, 0);
    case ast::DataType::INT:
    case ast::DataType::BOOL:
        return llvm::ConstantInt::get(ty, 0);
    case ast::DataType::VOID:
    default:
        return nullptr;
    }
}

void IRGenerator::setupGlobalInitialization(llvm::Function *func) {
    std::vector<llvm::Type *> types = {llvm::Type::getInt32Ty(context), func->getType(),
                                       llvm::PointerType::getInt8PtrTy(context)};
    auto structType = llvm::StructType::get(context, types);
    module.getOrInsertGlobal("llvm.global_ctors", llvm::ArrayType::get(structType, 1));
    llvm::GlobalVariable *ctorsVar = module.getGlobalVariable("llvm.global_ctors");
    ctorsVar->setLinkage(llvm::GlobalValue::LinkageTypes::AppendingLinkage);

    const int globalInitializerNumber = 65535;
    llvm::ConstantInt *intValue = llvm::ConstantInt::get(llvm::Type::getInt32Ty(context), globalInitializerNumber);
    llvm::ConstantPointerNull *nullValue = llvm::ConstantPointerNull::get(llvm::PointerType::getInt8PtrTy(context));
    std::vector<llvm::Constant *> structValues = {intValue, func, nullValue};
    std::vector<llvm::Constant *> arrayValues = {llvm::ConstantStruct::get(structType, structValues)};
    llvm::Constant *initializer = llvm::ConstantArray::get(llvm::ArrayType::get(structType, 1), arrayValues);
    ctorsVar->setInitializer(initializer);
}

void IRGenerator::visitSequenceNode(SequenceNode *node) {
    LOG("Enter Sequence")

    llvm::Function *initFunc = nullptr;
    if (currentFunction == nullptr) {
        // TODO(henne): make sure this function name does not collide with any user defined functions
        initFunc = getOrCreateFunctionDefinition("__ctor", ast::DataType::VOID, {});

        llvm::BasicBlock *BB = llvm::BasicBlock::Create(context, "entry-ctor", initFunc);
        builder.SetInsertPoint(BB);

        currentFunction = initFunc;
        isGlobalScope = true;
    }

    for (auto child : node->getChildren()) {
        child->accept(this);
    }

    if (!node->getChildren().empty()) {
        nodesToValues[node] = nodesToValues[node->getChildren().back()];
    }

    if (initFunc != nullptr) {
        finalizeFunction(initFunc, ast::DataType::VOID, false);
        // TODO(henne): henne: don't generate global init function, if there are no globals
        setupGlobalInitialization(initFunc);
        isGlobalScope = false;
    }

    LOG("Exit Sequence")
}

void IRGenerator::print(const bool writeToFile) {
    std::string fileName = program.fileName + ".llvm";
    std::error_code EC;
    llvm::raw_fd_ostream dest(fileName, EC, llvm::sys::fs::OF_None);
    module.print(llvm::outs(), nullptr);
    if (writeToFile) {
        module.print(dest, nullptr);
    }
}

void IRGenerator::generateDummyMain() {
    if (module.getFunction("main") != nullptr) {
        return;
    }

    auto zero = new IntegerNode(0);

    auto returnStatement = new StatementNode();
    returnStatement->setIsReturnStatement(true);
    returnStatement->setChild(zero);

    auto function = new FunctionNode("main", ast::DataType::INT);
    function->setBody(returnStatement);

    visitFunctionNode(function);
}

void IRGenerator::run(AstNode *root) {
    if (root == nullptr) {
        return;
    }

    root->accept(this);

    generateDummyMain();

    this->printMetrics();
    if (!errors.empty()) {
        printErrors();
        exit(1);
    } else {
        this->print();
    }
}

llvm::Value *IRGenerator::findVariable(const std::string &name) {
    metrics["variableLookups"]++;

    int currentScope = definedVariables.size() - 1;
    while (currentScope >= 0) {
        auto &scope = definedVariables[currentScope];
        auto result = scope.find(name);
        if (result != scope.end()) {

            metrics["variableLookupsSuccessful"]++;

            return result->second;
        }
        currentScope--;
    }

    metrics["variableLookupsFailure"]++;

    return nullptr;
}

std::unordered_map<std::string, llvm::Value *> &IRGenerator::currentScope() {
    return definedVariables[definedVariables.size() - 1];
}

void IRGenerator::pushScope() { definedVariables.emplace_back(); }

void IRGenerator::popScope() { definedVariables.pop_back(); }

void IRGenerator::withScope(const std::function<void(void)> &func) {
    pushScope();
    func();
    popScope();
}

void IRGenerator::printMetrics() {
    for (auto metric : metrics) {
        std::cout << metric.first << ": " << metric.second << std::endl;
    }
}

void IRGenerator::printErrors() {
    std::cerr << std::endl;
    std::cerr << "The following errors occured:" << std::endl;
    for (auto msg : errors) {
        std::cerr << "\t" << msg << std::endl;
    }
    std::cerr << std::endl;
}

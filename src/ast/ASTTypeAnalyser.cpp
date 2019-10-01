#include "ASTTypeAnalyser.h"

#include <iostream>

void ASTTypeAnalyser::visitFunctionNode(FunctionNode *node) {
    for (auto &argument : node->getArguments()) {
        argument->accept(this);
    }
    functionMap[node->getName()] = node->getReturnType();
    node->getBody()->accept(this);
}

void ASTTypeAnalyser::visitCallNode(CallNode *node) {
    const auto &itr = functionMap.find(node->getName());
    if (itr == functionMap.end()) {
        std::cerr << "Undefined function " << node->getName() << std::endl;
    }
    typeMap[node] = itr->second;
}

void ASTTypeAnalyser::visitVariableNode(VariableNode *node) {
    const auto &itr = variableMap.find(node->getName());
    if (itr == variableMap.end()) {
        std::cerr << "Undefined variable " << node->getName() << std::endl;
        return;
    }
    typeMap[node] = itr->second;
}

void ASTTypeAnalyser::visitVariableDefinitionNode(VariableDefinitionNode *node) {
    typeMap[node] = node->getType();
    variableMap[node->getName()] = node->getType();
}

void ASTTypeAnalyser::visitBinaryOperationNode(BinaryOperationNode *node) {
    node->getLeft()->accept(this);
    node->getRight()->accept(this);
    auto leftType = typeMap[node->getLeft()];
    auto rightType = typeMap[node->getRight()];
    if ((node->getType() == BinaryOperationNode::BinaryOperationType::ADDITION ||
         node->getType() == BinaryOperationNode::BinaryOperationType::SUBTRACTION ||
         node->getType() == BinaryOperationNode::BinaryOperationType::MULTIPLICATION ||
         node->getType() == BinaryOperationNode::BinaryOperationType::DIVISION) &&
        leftType == rightType) {
        typeMap[node] = leftType;
        return;
    } else {
        std::cerr << "Binary operation type mismatch: " << to_string(node->getAstNodeType()) << std::endl;
        return;
    }
}

void ASTTypeAnalyser::visitUnaryOperationNode(UnaryOperationNode *node) {
    node->getChild()->accept(this);
    if (node->getType() == UnaryOperationNode::UnaryOperationType::NOT &&
        typeMap[node->getChild()] == AstNode::DataType::BOOL) {
        typeMap[node] = AstNode::DataType::BOOL;
        return;
    } else {
        // TODO unary operators can also be of other types than bool, we need to add support for that as well
        std::cerr << "Unary operation type mismatch: " << to_string(node->getAstNodeType()) << std::endl;
        return;
    }
}

void ASTTypeAnalyser::visitAssignmentNode(AssignmentNode *node) {
    node->getRight()->accept(this);
    node->getLeft()->accept(this);
    AstNode::DataType leftType = typeMap[node->getLeft()];
    AstNode::DataType rightType = typeMap[node->getRight()];
    if (leftType != rightType) {
        std::cerr << "Assignment type mismatch: " << to_string(node->getLeft()->getAstNodeType()) << " = "
                  << to_string(node->getRight()->getAstNodeType()) << std::endl;
    } else {
        typeMap[node] = leftType;
    }
}

void ASTTypeAnalyser::visitSequenceNode(SequenceNode *node) {
    for (auto child : node->getChildren()) {
        child->accept(this);
    }
}

void ASTTypeAnalyser::visitStatementNode(StatementNode *node) {
    if (node->getChild() == nullptr) {
        return;
    }
    node->getChild()->accept(this);
    typeMap[node] = typeMap[node->getChild()];
}

void ASTTypeAnalyser::visitFloatNode(FloatNode *node) { typeMap[node] = AstNode::DataType::FLOAT; }

void ASTTypeAnalyser::visitIntegerNode(IntegerNode *node) { typeMap[node] = AstNode::DataType::INT; }

void ASTTypeAnalyser::visitBoolNode(BoolNode *node) { typeMap[node] = AstNode::DataType::BOOL; }

void analyseTypes(AstNode *root) {
    if (root == nullptr) {
        return;
    }
    auto analyser = new ASTTypeAnalyser();
    root->accept(analyser);
}

#include "VariableNode.h"

#include "../AstVisitor.h"

VariableNode::VariableNode(std::string name) : AstNode(ast::NodeType::VARIABLE), name(std::move(name)) {}

void VariableNode::accept(AstVisitor *v) { v->visitVariableNode(this); }

std::string &VariableNode::getName() { return name; }

bool VariableNode::isArrayAccess() { return arrayIndex != nullptr; }

AstNode *VariableNode::getArrayIndex() { return this->arrayIndex; }

void VariableNode::setArrayIndex(AstNode *arrayIndex) { this->arrayIndex = arrayIndex; }

std::string VariableNode::toString() {
    if (isArrayAccess()) {
        return name + "[" + arrayIndex->toString() + "]";
    }
    return name;
}

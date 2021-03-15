#pragma once

#include "../Types.h"
#include <string>

class AstVisitor;

class AstNode {
  public:
    explicit AstNode(ast::NodeType type) : type(type) {}

    virtual void accept(AstVisitor *v) = 0;

    // TODO make this a pure virtual function
    virtual std::string toString() { return "not implemented"; }

    ast::NodeType getAstNodeType() { return type; }

  private:
    ast::NodeType type;
};

std::string to_string(ast::NodeType type);

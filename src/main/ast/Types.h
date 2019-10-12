#pragma once

namespace ast {

enum DataType { VOID, INT, FLOAT, BOOL };

enum NodeType {
    SEQUENCE,
    STATEMENT,
    LITERAL,
    UNARY_OPERATION,
    BINARY_OPERATION,
    FUNCTION,
    CALL,
    VARIABLE_DEFINITION,
    VARIABLE,
    ASSIGNMENT,
    IF_STATEMENT,
};

enum BinaryOperationType {
    ADDITION,
    MULTIPLICATION,
    SUBTRACTION,
    DIVISION,
    EQUALS,
    NOT_EQUALS,
    LESS_EQUALS,
    LESS_THAN,
    GREATER_EQUALS,
    GREATER_THAN,
};

}
#include "ImportFinder.h"

#include "../nodes/AllNodes.h"

void ImportFinder::visitSequenceNode(SequenceNode *node) {
    for (auto child : node->getChildren()) {
        child->accept(this);
    }
}

void ImportFinder::visitStatementNode(StatementNode *node) { node->getChild()->accept(this); }

void ImportFinder::visitImportNode(ImportNode *node) {
    auto path = directoryPath / std::filesystem::path(node->getFileName());
    std::string absolutePath = std::filesystem::absolute(path).string();
    importedModules.push_back(absolutePath);
}

std::vector<std::string> ImportFinder::run(AstNode *rootNode) {
    rootNode->accept(this);
    return importedModules;
}
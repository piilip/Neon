#include "Lexer.h"
#include "Linker.h"
#include "ObjectFileWriter.h"
#include "Parser.h"
#include "Program.h"
#include "ast/ASTGenerator.h"
#include "ast/visitors/ASTPrinter.h"
#include "ast/visitors/ASTTestCasePrinter.h"
#include "ast/visitors/ASTTypeAnalyser.h"
#include "ir/IRGenerator.h"

int main() {
    bool verbose = true;
    Program program = {"main.ne"};
    CodeProvider *codeProvider = new FileCodeProvider(program);
    Lexer lexer(codeProvider, program, verbose);
    Parser parser(lexer, program, verbose);

    auto parseTreeRoot = parser.createParseTree();
    if (verbose) {
        printParseTree(parseTreeRoot);
        printParseTreeTestCase(parseTreeRoot, program);
    }

    auto astRoot = createAstFromParseTree(parseTreeRoot);
    if (verbose) {
        printAst(astRoot);
        printAstTestCase(program, astRoot);
    }

    analyseTypes(astRoot);

    auto generator = new IRGenerator(program, verbose);
    generator->run(astRoot);

    writeModuleToObjectFile(program, generator);

    auto linker = new Linker(program);
    linker->link();

    return 0;
}
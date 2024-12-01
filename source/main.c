#include <stdlib.h>
#include <stdio.h>

#include "logger.h"
#include "exprTree.h"
#include "derivative.h"
#include "nameTable.h"
#include "treeDSL.h"
#include "tex.h"

int main() {
    logOpen("log.html", L_HTML_MODE);
    texInit("textest.md");
    setLogLevel(L_EXTRA);
    logDisableBuffering();

    Node_t *expr = OPR_(ADD, OPR_(MUL, NUM_(5), NUM_(3)),
                             OPR_(SUB, NUM_(12), NUM_(7)));

    DUMP_TREE(expr, 0);

    setVariable("x", 7.0);
    printf("Evaluated: %lg\n", evaluate(expr, NULL));
    deleteTree(expr);

    expr = parseExpressionPrefix("(+ (x) (- (-3) (52) ) )");
    DUMP_TREE(expr, 1);
    deleteTree(expr);

    char *exprStr = NULL;
    scanf("%m[^\n]", &exprStr);
    expr = parseExpression(exprStr);
    exprTexDump(expr);
    DUMP_TREE(expr, 1);
    Node_t *d = derivative(expr, "x");
    DUMP_TREE(d, 1);
    d = simplifyExpression(d);
    DUMP_TREE(d, 1);

    exprTexDump(d);

    deleteTree(expr);
    deleteTree(d);

    texClose();
    logClose();
    free(exprStr);
    return 0;
}

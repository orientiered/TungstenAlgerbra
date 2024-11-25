#include <stdlib.h>
#include <stdio.h>

#include "logger.h"
#include "exprTree.h"
#include "derivative.h"
#include "treeDSL.h"

int main() {
    logOpen("log.html", L_HTML_MODE);
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
    expr = parseExpressionPrefix(exprStr);
    DUMP_TREE(expr, 1);
    Node_t *d = derivative(expr);
    DUMP_TREE(d, 1);
    deleteTree(expr);
    deleteTree(d);

    logClose();
    return 0;
}

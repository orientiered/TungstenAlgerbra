#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include "hashTable.h"
#include "logger.h"
#include "tex.h"
#include "exprTree.h"
#include "derivative.h"
#include "treeDSL.h"

int main() {
    logOpen("log.html", L_HTML_MODE);
    setLogLevel(L_EXTRA);
    logDisableBuffering();
    TexContext_t tex = texInit("textest.tex");
    TungstenContext_t context = TungstenCtor();

    char *exprStr = NULL;
    scanf("%m[^\n]", &exprStr);

    Node_t *expr = parseExpression(&context, exprStr);
    DUMP_TREE(&context, expr, true);


    exprTexDump(&tex, &context, expr);
    texPrintf(&tex, "\n\n");

    exprTexDump(&tex, &context, expr);
    Node_t *diff = derivative(&context, expr, "x");
    exprTexDump(&tex, &context, diff);
    diff = simplifyExpression(&tex, &context, diff);
    exprTexDump(&tex, &context, diff);

    Node_t *taylor = TaylorExpansion(&tex, &context, expr, "x", 0, 8);
    exprTexDump(&tex, &context, taylor);

    texBeginGraph(&tex);
    for (double xCoord = -10; xCoord < 10; xCoord += 0.05) {
        setVariable(&context, "x", xCoord);
        texAddCoordinates(&tex, xCoord, evaluate(&context, expr));
    }
    texEndGraph(&tex);

    deleteTree(taylor);
    deleteTree(expr);
    deleteTree(diff);
    texClose(&tex);
    TungstenDtor(&context);
    logClose();
    free(exprStr);
    return 0;
}

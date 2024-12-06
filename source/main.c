#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <math.h>

#include "hashTable.h"
#include "logger.h"
#include "argvProcessor.h"
#include "tex.h"
#include "exprTree.h"
#include "derivative.h"
#include "treeDSL.h"

int main(int argc, const char *argv[]) {
    logOpen("log.html", L_HTML_MODE);
    setLogLevel(L_EXTRA);

    registerFlag(TYPE_INT, "-t", "--taylor", "Compute taylor expansion");
    processArgs(argc, argv);
    // logDisableBuffering();
    TexContext_t tex = texInit("textest.tex");
    TungstenContext_t context = TungstenCtor();

    char *exprStr = NULL;
    scanf("%m[^\n]", &exprStr);

    Node_t *expr = parseExpression(&context, exprStr);
    if (!expr) {
        logPrint(L_ZERO, 0, "Try again\n");
    }

    DUMP_TREE(&context, expr, false);
    expr = simplifyExpression(&tex, &context, expr);
    DUMP_TREE(&context, expr, false);

    texPrintf(&tex, "Дано:\n\n");
    exprTexDump(&tex, &context, expr);
    texPrintf(&tex, "\n\n");

    // exprTexDump(&tex, &context, expr);
    Node_t *diff = derivative(&tex, &context, expr, "x");
    DUMP_TREE(&context, diff, 0);
    exprTexDump(&tex, &context, diff);
    texPrintf(&tex, "\n\n");
    diff = simplifyExpression(&tex, &context, diff);
    DUMP_TREE(&context, diff, 0);
    exprTexDump(&tex, &context, diff);
    texPrintf(&tex, "\n\n");

    if (isFlagSet("-t")) {
        Node_t *taylor = TaylorExpansion(&tex, &context, expr, "x", 0, getFlagValue("-t").int_);
        // exprTexDump(&tex, &context, taylor);

        texBeginGraph(&tex);
        for (double xCoord = -10; xCoord < 10; xCoord += 0.05) {
            setVariable(&context, "x", xCoord);
            double yCoord = evaluate(&context, expr);
            if (fabs(yCoord) < 10)
                texAddCoordinates(&tex, xCoord, evaluate(&context, expr));
        }
        texEndGraph(&tex);
        deleteTree(taylor);
    }

    deleteTree(expr);
    deleteTree(diff);
    texClose(&tex);
    TungstenDtor(&context);
    logClose();
    free(exprStr);
    return 0;
}


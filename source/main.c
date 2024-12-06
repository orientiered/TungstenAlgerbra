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


const size_t GRAPH_POINTS_COUNT = 100;
const double GRAPH_X_DELTA = 10;
const double GRAPH_Y_MAX = 10; //module of y

int main(int argc, const char *argv[]) {
    logOpen("log.html", L_HTML_MODE);
    setLogLevel(L_EXTRA);

    registerFlag(TYPE_INT, "-t", "--taylor", "Compute taylor expansion");
    registerFlag(TYPE_FLOAT, "-p", "--point", "Point where taylor expansion is computed");
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

    texPrintf(&tex, "Дано:");
    exprTexDump(&tex, &context, expr);
    texPrintf(&tex, "\n\n");

    // exprTexDump(&tex, &context, expr);
    Node_t *diff = derivative(&tex, &context, expr, "x");
    DUMP_TREE(&context, diff, 0);
    exprTexDump(&tex, &context, diff);
    texPrintf(&tex, "\n\n");
    diff = simplifyExpression(&tex, &context, diff);
    DUMP_TREE(&context, diff, 0);

    texPrintf(&tex, "$$(");
    exprTexDumpRecursive(&tex, &context, expr);
    texPrintf(&tex, ")' = ");
    exprTexDumpRecursive(&tex, &context, diff);
    texPrintf(&tex, "$$\n\n");

    if (isFlagSet("-t")) {
        double expansionPoint = (isFlagSet("-p")) ? getFlagValue("-p").float_ : 0;
        Node_t *taylor = TaylorExpansion(&tex, &context, expr, "x", expansionPoint, getFlagValue("-t").int_);
        // exprTexDump(&tex, &context, taylor);

        texBeginGraph(&tex, "x", "y", "График функций");
        texPrintf(&tex,
                "\\legend{\n"
                "$f(x)$,\n"
                "Taylor\n"
                "};\n"
        );

        plotExprGraph(&tex, &context, expr,   "x", expansionPoint - GRAPH_X_DELTA, expansionPoint + GRAPH_X_DELTA, GRAPH_Y_MAX, GRAPH_POINTS_COUNT);
        plotExprGraph(&tex, &context, taylor, "x", expansionPoint - GRAPH_X_DELTA, expansionPoint + GRAPH_X_DELTA, GRAPH_Y_MAX, GRAPH_POINTS_COUNT);

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


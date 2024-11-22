#include <stdlib.h>
#include <stdio.h>

#include "logger.h"
#include "exprTree.h"

#define OPR_(op, left, right) createNode(OPERATOR, op, 0, left, right)
#define NUM_(num) createNode(NUMBER, 0, num, NULL, NULL)
#define VAR_(variable) createNode(VARIABLE, variable, 0, NULL, NULL)

int main() {
    logOpen("log.html", L_HTML_MODE);

    Node_t *expr = OPR_('+', OPR_('*', NUM_(5), VAR_('x')),
                             OPR_('-', NUM_(12), NUM_(7)));

    DUMP_TREE(expr);

    setVariable('x', 7.0);
    printf("Evaluated: %lg\n", evaluate(expr, NULL));
    deleteTree(expr);

    logClose();
    return 0;
}

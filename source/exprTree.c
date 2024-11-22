#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <stdbool.h>

#include "logger.h"
#include "exprTree.h"

#define CALLOC(size, type) (type *) calloc((size), sizeof(type))

/*========Nametables==============*/
const double defaultVariableValue = 0.0;
// static double variables[]
static double globalX = 0.0;

void setVariable(char variable, double value) {
    if (variable == 'x')
        globalX = value;

}

double getVariable(char variable) {
    if (variable == 'x')
        return globalX;

    return defaultVariableValue;
}
/*================================*/

Node_t *createNode(enum ElemType type, int iVal, double dVal, Node_t *left, Node_t *right) {
    Node_t *newNode = CALLOC(1, Node_t);
    newNode->type = type;
    newNode->left  = left;
    newNode->right = right;

    if (left)
        left->parent = newNode;
    if (right)
        right->parent = newNode;

    switch(type) {
        case OPERATOR:
            newNode->value.op = (enum OperatorType) iVal;
            break;
        case VARIABLE:
            newNode->value.var = iVal;
            break;
        case NUMBER:
            newNode->value.number = dVal;
            break;
        default:
            assert(0);
    }

    return newNode;
}

Node_t *copyTree(Node_t *node) {
    assert(node);

    Node_t *left  = (node->left)  ? copyTree(node->left)  : NULL,
           *right = (node->right) ? copyTree(node->right) : NULL;

    return createNode(node->type, node->value.var, node->value.number, left, right);
}

double evaluate(Node_t *node, bool *usedVariable) {
    assert(node);

    switch(node->type) {
        case VARIABLE:
            if (usedVariable)
                *usedVariable = true;
            return getVariable(node->value.var);
        case NUMBER:
            return node->value.number;
        case OPERATOR:
            switch(node->value.op) {
                case ADD:
                    return evaluate(node->left, usedVariable) + evaluate(node->right, usedVariable);
                case SUB:
                    return evaluate(node->left, usedVariable) - evaluate(node->right, usedVariable);
                case MUL:
                    return evaluate(node->left, usedVariable) * evaluate(node->right, usedVariable);
                case DIV:
                    return evaluate(node->left, usedVariable) / evaluate(node->right, usedVariable);
                default:
                    assert(0);
                    break;
            }
            break;
        default:
            assert(0);
            break;
    }

    assert(0);
    return 0.0;
}


TungstenStatus_t verifyTree(Node_t *node) {
    return TA_SUCCESS;
}

static TungstenStatus_t recursiveDumpTree(Node_t *node, FILE *dotFile);

TungstenStatus_t dumpTree(Node_t *node) {
    static size_t dumpCounter = 0;
    dumpCounter++;
    system("mkdir -p " LOGS_DIR "/" DOTS_DIR " " LOGS_DIR "/" IMGS_DIR);

    char buffer[DUMP_BUFFER_SIZE] = "";
    sprintf(buffer, LOGS_DIR "/" DOTS_DIR "/" EXPR_TREE_DUMP_DOT_FORMAT, dumpCounter);
    FILE *dotFile = fopen(buffer, "w");

    fprintf(dotFile, "digraph {\n"
                      "graph [splines=line]\n");

    recursiveDumpTree(node, dotFile);

    fprintf(dotFile, "}\n");
    fclose(dotFile);

    const char *extension = "svg";
    sprintf(buffer, "dot " LOGS_DIR "/" DOTS_DIR "/" EXPR_TREE_DUMP_DOT_FORMAT
                    " -T%s -o " LOGS_DIR "/" IMGS_DIR "/" EXPR_TREE_DUMP_IMG_FORMAT "%s",
                    dumpCounter, extension, dumpCounter, extension);
    if (system(buffer) != 0)
        return TA_DUMP_ERROR;

    logPrint(L_ZERO, 0, "<img src=\"" IMGS_DIR "/" EXPR_TREE_DUMP_IMG_FORMAT "%s\" width=76%%>\n<hr>\n", dumpCounter, extension);
    return TA_SUCCESS;
}

static TungstenStatus_t recursiveDumpTree(Node_t *node, FILE *dotFile) {
    if (!node) return TA_DUMP_ERROR;
    if (!dotFile) return TA_DUMP_ERROR;

    fprintf(dotFile, "\tnode%p [shape = Mrecord, "
                     "label = \"{node[%p] | parent[%p] |", node, node, node->parent);

    switch(node->type) {
        case OPERATOR:
            fprintf(dotFile, "TYPE = OPR(%d) | OP = %c | ", node->type, node->value.op);
            break;
        case VARIABLE:
            fprintf(dotFile, "TYPE = VAR(%d) | VAR = %c | ", node->type, node->value.var);
            break;
        case NUMBER:
            fprintf(dotFile, "TYPE = OPR(%d) | OP = %lg | ", node->type, node->value.number);
            break;
        default:
            fprintf(dotFile, "TYPE = ???(%d) | OP = %d | ", node->type, node->value.var);
            break;
    }

    fprintf(dotFile, " { <left>left[%p] | <right>right[%p] }}\"];\n", node->left, node->right);

    if (node->left) {
        fprintf(dotFile, "\tnode%p:<left> -> node%p;\n", node, node->left);
        recursiveDumpTree(node->left, dotFile);
    }

    if (node->right) {
        fprintf(dotFile, "\tnode%p:<right> -> node%p;\n", node, node->right);
        recursiveDumpTree(node->right, dotFile);
    }

    return TA_SUCCESS;
}

TungstenStatus_t deleteTree(Node_t *node) {
    assert(node);

    logPrint(L_EXTRA, 0, "Deleting tree[%p]\n", node);

    if (node->left)
        deleteTree(node->left);

    if (node->right)
        deleteTree(node->right);

    free(node);

    return TA_SUCCESS;
}

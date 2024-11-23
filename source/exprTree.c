#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <stdbool.h>
#include <ctype.h>

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
    logPrint(L_EXTRA, 0, "ExprTree:Creating node\n");

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

    logPrint(L_EXTRA, 0, "ExprTree:Created node[%p]\n", newNode);
    return newNode;
}

Node_t *copyTree(Node_t *node) {
    assert(node);

    logPrint(L_EXTRA, 0, "ExprTree:Copying node[%p]\n", node);

    Node_t *left  = (node->left)  ? copyTree(node->left)  : NULL,
           *right = (node->right) ? copyTree(node->right) : NULL;

    return createNode(node->type, node->value.var, node->value.number, left, right);
}

double evaluate(Node_t *node, bool *usedVariable) {
    assert(node);

    logPrint(L_EXTRA, 0, "ExprTree:Evaluating node[%p]\n", node);

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

static Node_t *recursiveParseExpressionPrefix(const char **expression);

Node_t *parseExpressionPrefix(const char *expression) {
    assert(expression);
    const char *exprCopy = expression;
    Node_t *exprTree = recursiveParseExpressionPrefix(&exprCopy);
    return exprTree;
}

static enum ElemType getElemType(const char **expression, int *varOrOperator) {
    if (**expression == '+' || **expression == '-') {
        //if next symbol is digit, + and - is unary
        if (isdigit((*expression)[1])) {
            return NUMBER;
        }


        *varOrOperator = **expression;
        (*expression)++;
        return OPERATOR;
    }

    if (**expression == '/' || **expression == '*') {
        *varOrOperator = **expression;
        (*expression)++;
        return OPERATOR;
    }

    if (**expression == 'x') {
        *varOrOperator = 'x';
        (*expression)++;
        return VARIABLE;
    }

    return NUMBER;
}

static Node_t *recursiveParseExpressionPrefix(const char **expression) {
    assert(expression);
    assert(*expression);

    int shift = 0;
    sscanf(*expression, " ( %n", &shift);
    if (shift == 0) {
        logPrint(L_ZERO, 1, "Wrong syntax: no (\n");
        return NULL;
    }

    (*expression) += shift;
    shift = 0;


    int varOrOperator = 0;
    double number = 0;
    enum ElemType type = getElemType(expression, &varOrOperator);

    Node_t *left = NULL, *right = NULL;
    switch(type) {
        case NUMBER:
            sscanf(*expression, " %lg %n", &number, &shift);
            if (shift == 0) {
                logPrint(L_ZERO, 1, "Can't read number\n");
                return NULL;
            }
            (*expression) += shift;
            shift = 0;
            break;
        case VARIABLE:
            break;
        case OPERATOR:
            left = recursiveParseExpressionPrefix(expression);
            if (!left) return NULL;
            right = recursiveParseExpressionPrefix(expression);
            if (!right) return NULL;

            break;
        default:
            logPrint(L_ZERO, 1, "Unknown element type\n");
            return NULL;
    }

    sscanf(*expression, " ) %n", &shift);
    if (shift == 0) {
        logPrint(L_ZERO, 1, "Wrong syntax: no )\n");
        return NULL;
    }

    (*expression) += shift;
    shift = 0;

    return createNode(type, varOrOperator, number, left, right);
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
            fprintf(dotFile, "TYPE = OPR(%d) | %c | ", node->type, node->value.op);
            break;
        case VARIABLE:
            fprintf(dotFile, "TYPE = VAR(%d) | %c | ", node->type, node->value.var);
            break;
        case NUMBER:
            fprintf(dotFile, "TYPE = NUM(%d) | %lg | ", node->type, node->value.number);
            break;
        default:
            fprintf(dotFile, "TYPE = ???(%d) | %d | ", node->type, node->value.var);
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

    logPrint(L_EXTRA, 0, "ExprTree:Deleting tree[%p]\n", node);

    if (node->left)
        deleteTree(node->left);

    if (node->right)
        deleteTree(node->right);

    free(node);

    return TA_SUCCESS;
}

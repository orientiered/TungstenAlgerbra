#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>
#include <stdint.h>
#include <math.h>

#include "utils.h"
#include "logger.h"
#include "exprTree.h"

#define CALLOC(size, type) (type *) calloc((size), sizeof(type))

/*========Nametables==============*/
const double defaultVariableValue = 0.0;

typedef struct {
    double number;
    char str[MAX_VARIABLE_LEN];
} variable_t;

static variable_t variables[VARIABLES_TABLE_LEN] = {0};
static size_t variablesCount = 0;

static size_t insertVariable(const char *buffer) {
    assert(strlen(buffer) < MAX_VARIABLE_LEN);

    strcpy(variables[variablesCount++].str, buffer);
    return variablesCount - 1;
}

void setVariable(const char *variableName, double value) {
    for (size_t idx = 0; idx < variablesCount; idx++) {
        if (strcmp(variableName, variables[idx].str) == 0)
            variables[idx].number = value;
        return;
    }

    logPrint(L_ZERO, 1, "Attempt to set unknown variable '%s'\n", variableName);
}

static double getVariable(int varIdx) {
    return variables[varIdx].number;
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
        {
            double leftValue = 0, rightValue = 0;
            leftValue = evaluate(node->left, usedVariable);

            if (operators[node->value.op].binary)
                rightValue = evaluate(node->right, usedVariable);

            switch(node->value.op) {
                case ADD:
                    return leftValue + rightValue;
                case SUB:
                    return leftValue - rightValue;
                case MUL:
                    return leftValue * rightValue;
                case DIV:
                    return leftValue /rightValue;
                case POW:
                    return pow(leftValue, rightValue);
                case SIN:
                    return sin(leftValue);
                case COS:
                    return cos(leftValue);
                case TAN:
                    return tan(leftValue);
                case CTG:
                    return 1/tan(leftValue);
                case LOG:
                    return log(rightValue) / log(leftValue);
                case LOGN:
                    return log(leftValue);
                default:
                    assert(0);
                    break;
            }
            break;
        }
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
    if (!exprTree) {
        logPrint(L_ZERO, 1, "Error ocurred here: '%s'\n", exprCopy);
    }
    return exprTree;
}

static const Operator_t *operatorFind(const char *str) {
    assert(str);
    for (size_t idx = 0; idx < ARRAY_SIZE(operators); idx++) {
        if (strcmp(str, operators[idx].str) == 0)
            return operators + idx;
    }
    return NULL;
}

static enum ElemType scanElement(char *buffer, union NodeValue *value) {
    assert(buffer);
    assert(value);

    logPrint(L_EXTRA, 0, "Scanning elem '%s'\n", buffer);
    const Operator_t *op = operatorFind(buffer);
    if (op != NULL) {
        value->op = op->opCode;
        return OPERATOR;
    }

    size_t elemLen = strlen(buffer);

    size_t scanned = 0;
    if (sscanf(buffer, "%lg%n", &value->number, &scanned) == 1 && scanned == elemLen) {
        return NUMBER;
    }

    if (elemLen >= MAX_VARIABLE_LEN) {
        buffer[MAX_VARIABLE_LEN] = '\0';
        logPrint(L_ZERO, 1, "Variable name is too long\nIt will be truncated to '%s'\n", buffer);
    }
    value->var = insertVariable(buffer);
    return VARIABLE;
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


    union NodeValue value = {0};

    char buffer[PARSER_BUFFER_SIZE] = "";
    sscanf(*expression, " %[^() ] %n", buffer, &shift);
    (*expression) += shift;
    shift = 0;
    enum ElemType type = scanElement(buffer, &value);

    Node_t *left = NULL, *right = NULL;
    switch(type) {
        case NUMBER:
        case VARIABLE:
            break;
        case OPERATOR:
            left = recursiveParseExpressionPrefix(expression);
            if (!left) return NULL;
            if (operators[value.op].binary) {
                right = recursiveParseExpressionPrefix(expression);
                if (!right) return NULL;
            }

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

    return createNode(type, value.var, value.number, left, right);
}


TungstenStatus_t verifyTree(Node_t *node) {
    return TA_SUCCESS;
}

static TungstenStatus_t recursiveDumpTree(Node_t *node, bool minified, FILE *dotFile);

TungstenStatus_t dumpTree(Node_t *node, bool minified) {
    static size_t dumpCounter = 0;
    dumpCounter++;
    system("mkdir -p " LOGS_DIR "/" DOTS_DIR " " LOGS_DIR "/" IMGS_DIR);

    char buffer[DUMP_BUFFER_SIZE] = "";
    sprintf(buffer, LOGS_DIR "/" DOTS_DIR "/" EXPR_TREE_DUMP_DOT_FORMAT, dumpCounter);
    FILE *dotFile = fopen(buffer, "w");

    fprintf(dotFile, "digraph {\n"
                      "graph [splines=line]\n");

    recursiveDumpTree(node, minified, dotFile);

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

static TungstenStatus_t recursiveDumpTree(Node_t *node, bool minified, FILE *dotFile) {
    if (!node) return TA_DUMP_ERROR;
    if (!dotFile) return TA_DUMP_ERROR;

    fprintf(dotFile, "\tnode%p [shape = Mrecord, label = \"{", node);

    if (!minified) {
        fprintf(dotFile, "node[%p] | parent[%p] |", node, node->parent);
    }

    const char *fillColor = DEFAULT_NODE_COLOR;

    switch(node->type) {
        case OPERATOR:
            fillColor = OPERATOR_COLOR;
            fprintf(dotFile, "TYPE = OPR(%d) | %s | ", node->type, operators[node->value.op].str);
            break;
        case VARIABLE:
            fillColor = VARIABLE_COLOR;
            fprintf(dotFile, "TYPE = VAR(%d) | %s | ", node->type, variables[node->value.var].str);
            break;
        case NUMBER:
            fillColor = NUMBER_COLOR;
            fprintf(dotFile, "TYPE = NUM(%d) | %lg | ", node->type, node->value.number);
            break;
        default:
            fprintf(dotFile, "TYPE = ???(%d) | %d | ", node->type, node->value.var);
            break;
    }

    if (!minified)
        fprintf(dotFile, " { <left>left[%p] | <right>right[%p] }}\"", node->left, node->right);
    else {
        fprintf(dotFile, " {<left> L | <right> R}}\"");
    }

    fprintf(dotFile, ", style = filled, fillcolor = \"%s\"];\n", fillColor);

    if (node->left) {
        fprintf(dotFile, "\tnode%p:<left> -> node%p;\n", node, node->left);
        recursiveDumpTree(node->left, minified, dotFile);
    }

    if (node->right) {
        fprintf(dotFile, "\tnode%p:<right> -> node%p;\n", node, node->right);
        recursiveDumpTree(node->right, minified, dotFile);
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

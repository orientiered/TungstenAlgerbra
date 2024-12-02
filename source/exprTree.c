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
#include "hashTable.h"
#include "tex.h"
#include "exprTree.h"

#define CALLOC(size, type) (type *) calloc((size), sizeof(type))

static double calculateOperation(enum OperatorType op, double left, double right) {
    switch(op) {
        case ADD:
            return left + right;
        case SUB:
            return left - right;
        case MUL:
            return left * right;
        case DIV:
            return left /right;
        case POW:
            return pow(left, right);
        case SIN:
            return sin(left);
        case COS:
            return cos(left);
        case TAN:
            return tan(left);
        case CTG:
            return 1/tan(left);
        case LOG:
            return log(right) / log(left);
        case LOGN:
            return log(left);
        default:
            LOG_PRINT(L_ZERO, 1, "Operation %d is not implemented\n", op);
            return 0;
            break;
    }
}

TungstenContext_t TungstenCtor() {
    TungstenContext_t context = {hashTableCtor(VARIABLE_TABLE_SIZE), {0}, 0};
    return context;
}

TungstenStatus_t TungstenDtor(TungstenContext_t *context) {
    hashTableDtor(&context->variablesTable);
    return TA_SUCCESS;
}

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

double evaluate(TungstenContext_t *context, Node_t *node) {
    assert(node);

    logPrint(L_EXTRA, 0, "ExprTree:Evaluating node[%p]\n", node);

    switch(node->type) {
        case VARIABLE:
            return getVariable(context, node->value.var);
        case NUMBER:
            return node->value.number;
        case OPERATOR:
        {
            double leftValue = 0, rightValue = 0;
            leftValue = evaluate(context, node->left);

            if (operators[node->value.op].binary)
                rightValue = evaluate(context, node->right);

            return calculateOperation(node->value.op, leftValue, rightValue);
            break;
        }
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

static TungstenStatus_t recursiveDumpTree(TungstenContext_t *context, Node_t *node, bool minified, FILE *dotFile);

TungstenStatus_t dumpTree(TungstenContext_t *context, Node_t *node, bool minified) {
    static size_t dumpCounter = 0;
    dumpCounter++;
    system("mkdir -p " LOGS_DIR "/" DOTS_DIR " " LOGS_DIR "/" IMGS_DIR);

    char buffer[DUMP_BUFFER_SIZE] = "";
    sprintf(buffer, LOGS_DIR "/" DOTS_DIR "/" EXPR_TREE_DUMP_DOT_FORMAT, dumpCounter);
    FILE *dotFile = fopen(buffer, "w");

    fprintf(dotFile, "digraph {\n"
                      "graph [splines=line]\n");

    recursiveDumpTree(context, node, minified, dotFile);

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

static TungstenStatus_t recursiveDumpTree(TungstenContext_t *context, Node_t *node, bool minified, FILE *dotFile) {
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
            fprintf(dotFile, "TYPE = VAR(%d) | %s(%d) | ", node->type, getVariableName(context, node->value.var), node->value.var);
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
        recursiveDumpTree(context, node->left, minified, dotFile);
    }

    if (node->right) {
        fprintf(dotFile, "\tnode%p:<right> -> node%p;\n", node, node->right);
        recursiveDumpTree(context, node->right, minified, dotFile);
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


/*===========Tree simplification================================*/
Node_t *foldConstants(Node_t *node, bool *changedTree) {
    if (node->type == NUMBER || node->type == VARIABLE) {
        return node;
    }

    assert(node->type == OPERATOR);

    bool binary = operators[node->value.op].binary;
    Node_t *left = NULL, *right = NULL;
    left = foldConstants(node->left, changedTree);
    if (binary)
        right = foldConstants(node->right, changedTree);

    if (left->type == NUMBER && (!binary ||  right->type == NUMBER) ) {
        if (changedTree)
            *changedTree = true;
        node->value.number = calculateOperation(node->value.op, left->value.number,
                                                                (right) ? (right->value.number) : 0);
        deleteTree(node->left); node->left = NULL;
        if (right) {
            deleteTree(node->right); node->right = NULL;
        }
        node->type = NUMBER;
    }

    return node;
}

static bool isEqualDouble(double a, double b) {
    return fabs(b-a) < DOUBLE_EPSILON;
}

// static linkNode(Node_t *destination, Node_t *source)
Node_t *removeNeutralOperations(Node_t *node, bool *changedTree) {
    assert(node);

    if (node->type == NUMBER || node->type == VARIABLE)
        return node;

    if (node->left)
        node->left = removeNeutralOperations(node->left, changedTree);

    if (node->right)
        node->right = removeNeutralOperations(node->right, changedTree);

    Node_t *result = node;
    bool leftIsNumber  = (node->left->type == NUMBER);
    bool rightIsNumber = (node->right && node->right->type == NUMBER);

    bool leftIsZero    = leftIsNumber && isEqualDouble(node->left->value.number, 0);
    bool leftIsOne     = leftIsNumber && isEqualDouble(node->left->value.number, 1);

    bool rightIsZero   = rightIsNumber && isEqualDouble(node->right->value.number, 0);
    bool rightIsOne    = rightIsNumber && isEqualDouble(node->right->value.number, 1);

    if (node->type == OPERATOR) {
        if (node->value.op == ADD) {
        /*ADDITION*/
            if (leftIsZero) {
            // 0 + x = x
                node->right->parent = node->parent;
                result = node->right;
            } else if (rightIsZero) {
            // x + 0 = 0
                node->left->parent = node->parent;
                result = node->left;
            }

        } else if (node->value.op == SUB) {
        /*SUBSTRACTION*/
            if (rightIsZero) {
            // x - 0 = x
                node->left->parent = node->parent;
                result = node->left;
            }
        } else if (node->value.op == MUL) {
        /*MULTIPLICATION*/
            if (leftIsOne) {
                node->right->parent = node->parent;
                result = node->right;
            } else if (leftIsZero) {
                node->left->parent = node->parent;
                result = node->left;
            } else if (rightIsOne) {
                node->left->parent = node->parent;
                result = node->left;
            } else if (rightIsZero) {
                node->right->parent = node->parent;
                result = node->right;
            }

        } else if (node->value.op == POW) {
        /*POWER*/
            if (rightIsOne) {
            // x ^ 1 = x
                node->left->parent = node->parent;
                result = node->left;
            } else if (rightIsZero) {
            // x ^ 0 = 1
                node->right->parent = node->parent;
                node->right->value.number = 1;
                result = node->right;
            } else if (leftIsZero) {
            //0 ^ x = 0
                node->left->parent = node->parent;
                result = node->left;
            }
        }
    }

    if (result != node) {
        if (changedTree)
            *changedTree = true;
        //unlinking result subtree from node to use deleteTree() function
        if (result == node->left)
            node->left = NULL;
        else
            node->right = NULL;

        deleteTree(node);
    }
    return result;
}

Node_t *simplifyExpression(TexContext_t *tex, TungstenContext_t *context, Node_t *node) {
    bool changedTree = false;
    do {
        exprTexDump(tex, context, node);
        texPrintf(tex, " = ");
        changedTree = false;
        node = foldConstants(node, &changedTree);
        node = removeNeutralOperations(node, &changedTree);
        exprTexDump(tex, context, node);
        texPrintf(tex, "\n\n");
    } while (changedTree);

    return node;
}

static int exprTexDumpRecursive(TexContext_t *tex, TungstenContext_t *context, Node_t *node);

int exprTexDump(TexContext_t *tex, TungstenContext_t *context, Node_t *node) {
    assert(node);

    int result = 0;

    result += texPrintf(tex, "$$");
    result += exprTexDumpRecursive(tex, context, node);
    result += texPrintf(tex, "$$");

    return result;
}

static bool needBrackets(Node_t *node) {
    assert(node);
    if (node->type == NUMBER || node->type == VARIABLE)
        return false;

    if (node->type == OPERATOR) {
        if (!operators[node->value.op].binary) return false;
        if (!node->parent) return false;
        return (operators[node->parent->value.op].opCode == POW || operators[node->parent->value.op].priority > operators[node->value.op].priority);
    }
    return false;
}

static int exprTexDumpRecursive(TexContext_t *tex, TungstenContext_t *context, Node_t *node) {
    assert(node);

    if (node->type == NUMBER)
        return texPrintf(tex, "%lg", node->value.number);
    if (node->type == VARIABLE)
        return texPrintf(tex, "%s", getVariableName(context, node->value.var));

    int result = 0;
    if (node->type == OPERATOR) {
        if (operators[node->value.op].binary) {
            if (node->value.op == DIV) {
                result += texPrintf(tex, "\\frac{");
                result += exprTexDumpRecursive(tex, context, node->left);
                result += texPrintf(tex, "}{");
                result += exprTexDumpRecursive(tex, context, node->right);
                result += texPrintf(tex, "}");
            } else if (node->value.op == POW) {
                bool brackets = needBrackets(node->left);
                if (brackets)
                    result += texPrintf(tex, "(");
                result += exprTexDumpRecursive(tex, context, node->left);
                if (brackets)
                    result += texPrintf(tex, ")");
                result += texPrintf(tex, "^{");
                result += exprTexDumpRecursive(tex, context, node->right);
                result += texPrintf(tex, "}");
            } else {
                bool brackets = needBrackets(node->left);
                if (brackets)
                    result += texPrintf(tex, "(");
                result += exprTexDumpRecursive(tex, context, node->left);
                if (brackets)
                    result += texPrintf(tex, ")");

                result += texPrintf(tex, "%s", operators[node->value.op].str);

                brackets = needBrackets(node->right);
                if (brackets)
                    result += texPrintf(tex, "(");
                result += exprTexDumpRecursive(tex, context, node->right);
                if (brackets)
                    result += texPrintf(tex, ")");
            }
        } else {
            result += texPrintf(tex, "\\%s(", operators[node->value.op].str);
            result += exprTexDumpRecursive(tex, context, node->left);
            result += texPrintf(tex, ")");
        }
    }

    return result;
}

/*================PREFIX EQUATION PARSING==============================*/
//!DEPRECATED
static Node_t *recursiveParseExpressionPrefix(TungstenContext_t *context, const char **expression);

Node_t *parseExpressionPrefix(TungstenContext_t *context, const char *expression) {
    assert(expression);
    const char *exprCopy = expression;
    Node_t *exprTree = recursiveParseExpressionPrefix(context, &exprCopy);
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

static enum ElemType scanElement(TungstenContext_t *context, char *buffer, union NodeValue *value) {
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

    value->var = insertVariable(context, buffer);
    return VARIABLE;
}

static Node_t *recursiveParseExpressionPrefix(TungstenContext_t *context, const char **expression) {
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
    enum ElemType type = scanElement(context, buffer, &value);

    Node_t *left = NULL, *right = NULL;
    switch(type) {
        case NUMBER:
        case VARIABLE:
            break;
        case OPERATOR:
            left = recursiveParseExpressionPrefix(context, expression);
            if (!left) return NULL;
            if (operators[value.op].binary) {
                right = recursiveParseExpressionPrefix(context, expression);
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

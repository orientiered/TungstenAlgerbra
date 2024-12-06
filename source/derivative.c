#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <stdbool.h>

#include "hashTable.h"
#include "tex.h"
#include "logger.h"
#include "exprTree.h"
#include "derivative.h"

#include "treeDSL.h"

static bool hasVariable(Node_t *node, int variable) {
    assert(node);

    if (node->type == NUMBER) return false;
    if (node->type == VARIABLE) return (node->value.var == variable);

    if (operators[node->value.op].binary)
        return hasVariable(node->left, variable) || hasVariable(node->right, variable);
    else
        return hasVariable(node->left, variable);
}


/*=======DSL FOR DERIVATIVES==================*/
#define dL_ derivativeBase(tex, context, expr->left, variable)
#define dR_ derivativeBase(tex, context, expr->right, variable)
#define cL_ copyTree(expr->left)
#define cR_ copyTree(expr->right)

static Node_t *derivativeOperator(TexContext_t *tex, TungstenContext_t *context, Node_t *expr, int variable) {
    assert(tex);
    assert(context);
    assert(expr);

    Node_t *result = NULL;

    switch(expr->value.op) {
        case ADD:
            result = OPR_(ADD, dL_, dR_);
            break;
        case SUB:
            result = OPR_(SUB, dL_, dR_);
            break;
        case MUL:
            result = OPR_(ADD,  OPR_(MUL, dL_, cR_),
                                OPR_(MUL, cL_, dR_));
            break;
        case DIV:
        {
            Node_t *nominator = OPR_(SUB,   OPR_(MUL, dL_, cR_),
                                            OPR_(MUL, cL_, dR_));
            Node_t *denominator = OPR_(POW, cR_, NUM_(2));
            result = OPR_(DIV, nominator, denominator);
            break;
        }
        case POW:
        {
            //base ^ power
            logPrint(L_EXTRA, 0, "Derivative: pow operator\n");
            bool noVarBase  = !hasVariable(expr->left, variable);
            bool noVarPower = !hasVariable(expr->right, variable);
            logPrint(L_EXTRA, 0, "Derivative: noVarBase = %d, noVarPower = %d\n", noVarBase, noVarPower);

            if (noVarPower) {
                if (noVarBase)
                    result = NUM_(0);
                else {
                    // d(f^n) = d(f)*n*f^(n-1)
                    Node_t *tempTree = OPR_(POW, cL_, OPR_(SUB, cR_, NUM_(1) ) );

                    result = OPR_(MUL, OPR_(MUL, dL_, cR_), tempTree);
                }
            } else {
                if (noVarBase) {
                    //d(a^f) = a^f * ln(a) * d(f)
                    result = OPR_(MUL, OPR_(MUL, copyTree(expr), dR_),
                                        OPR_(LOGN, cL_, NULL) );
                } else {
                    //d(g^f) = d( e^(f*ln(g)) ) = g^f * d( f*ln(g) ) = g^f * (df*ln(g) + f*dg/g)
                    Node_t *tempTree = OPR_(ADD, OPR_(MUL, dR_,
                                                           OPR_(LOGN, cL_, NULL) ),
                                                 OPR_(DIV, OPR_(MUL, cR_, dL_),
                                                           cL_)
                                            );
                    result = OPR_(MUL, copyTree(expr), tempTree);
                }
            }
            break;
        }
        case SIN:
            result = OPR_(MUL, OPR_(COS, cL_, NULL), dL_);
            break;
        case COS:
            result = OPR_(MUL, OPR_(SIN, cL_, NULL),
                                OPR_(MUL, NUM_(-1), dL_));
            break;
        case TAN:
            result = OPR_(MUL, dL_,
                               OPR_(POW, OPR_(COS, cL_, NULL), NUM_(-2) ) );
            break;
        case CTG:
            result = OPR_(MUL, OPR_(MUL, NUM_(-1), dL_),
                                OPR_(POW, OPR_(SIN, cL_, NULL), NUM_(-2) ) );
            break;
        case LOG:
            result = OPR_(DIV, dR_,
                                OPR_(MUL, cR_, OPR_(LOGN, cL_, NULL) ) );
            break;
        case LOGN:
            result = OPR_(DIV, dL_, cL_);
            break;
        default:
            logPrint(L_ZERO, 1, "Unknown operator type %d\n", expr->value.op);
            break;
    }

    return result;
}

/// @brief Create derivative of expr with respect to variable
/// @param expr expression tree
/// @param variable derivative variable
/// @return Derivative tree
Node_t *derivativeBase(TexContext_t *tex, TungstenContext_t *context, Node_t *expr, int variable) {
    texPrintf(tex, "Нужно найти производную выражения: ");
    exprTexDump(tex, context, expr);
    texPrintf(tex, "\n\n");

    Node_t *result = NULL;
    switch (expr->type) {
        case VARIABLE:
            if (expr->value.var == variable)
                result = NUM_(1);
            else
                result = NUM_(0);

            break;
        case NUMBER:
            result = NUM_(0);
            break;
        case OPERATOR:
            result = derivativeOperator(tex, context, expr, variable);
            break;
        default:
            logPrint(L_ZERO, 1, "Unknown expression type %d\n", expr->type);
            break;
    }

    texPrintf(tex, "(");
    exprTexDump(tex, context, expr);
    texPrintf(tex, ")' = ");
    exprTexDump(tex, context, result);
    texPrintf(tex, "\n\n");

    return result;
}

Node_t *derivative(TexContext_t *tex, TungstenContext_t *context, Node_t *expr, const char *variable) {
    assert(expr);
    assert(variable);

    int varIdx = findVariable(context, variable);
    if (varIdx == NULL_VARIABLE) {
        logPrint(L_ZERO, 1, "Unknown variable '%s'\n", variable);
        return NULL;
    } else {
        logPrint(L_EXTRA, 0, "Derivative: var = %d, varName = %s\n", varIdx, getVariableName(context, varIdx));
    }

    return derivativeBase(tex, context, expr, varIdx);
}

Node_t *TaylorExpansion(TexContext_t *tex, TungstenContext_t *context,
                        Node_t *expr, const char *variable,
                        double point, size_t nmemb) {

    Node_t *current = copyTree(expr);
    current = simplifyExpression(tex, context, current);
    texPrintf(tex, "Оттейлорим функцию ");
    exprTexDump(tex, context, current);
    texPrintf(tex, "\n\n");

    size_t varIdx = findVariable(context, variable);
    setVariable(context, variable, point);

    double curVal = evaluate(context, current);
    Node_t *taylor = NUM_(curVal);

    //TODO: 64 -> constant
    char firstCol[64] = "", secondCol[64] = "";

    // texBeginTable(tex, 2);
    // sprintf(firstCol, "$f(%lg)$", point);
    // sprintf(secondCol, "$%lg$", curVal);
    // texAddTableLine(tex, true, 2, firstCol, secondCol);

    double factorial = 1;

    for (unsigned membPower = 1; membPower < nmemb; membPower++) {
        tex->active = false;
        Node_t *toDelete = current;

        current = derivative(tex, context, current, variable);
        deleteTree(toDelete);
        tex->active = true;


        curVal = evaluate(context, current);

        // sprintf(firstCol, "$f^{(%d)}(%lg)$", membPower, point);
        // sprintf(secondCol, "$%lg$", curVal);
        // texAddTableLine(tex, membPower != (nmemb - 1), 2, firstCol, secondCol);

        factorial *= membPower;
        double coefficient = curVal / factorial;

        taylor = OPR_(ADD,
                        taylor,
                        OPR_(MUL,
                                NUM_(coefficient),
                                OPR_(POW,
                                        VAR_(varIdx),
                                        NUM_(membPower)
                                    )
                            )
                     );

        tex->active = false;
        current = simplifyExpression(tex, context, current);
        tex->active = true;
        DUMP_TREE(context, current, 0);
        texPrintf(tex, "$f^{(%d)}(x) = $", membPower);
        exprTexDump(tex, context, current);
        texPrintf(tex, "\n\n");
        texPrintf(tex, "$f^{(%d)}(%lg) = %lg$\n\n", membPower, point, curVal);
    }

    // texEndTable(tex);
    deleteTree(current);



    tex->active = false;
    DUMP_TREE(context, taylor, false);
    taylor = simplifyExpression(tex, context, taylor);
    DUMP_TREE(context, taylor, false);

    tex->active = true;


    texPrintf(tex, " Имеем ");
    exprTexDump(tex, context, expr);
    texPrintf(tex, " = ");
    exprTexDump(tex, context, taylor);
    texPrintf(tex, "$ + o(x^%d)$\n\n", nmemb - 1);

    return taylor;
}

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <stdbool.h>

#include "logger.h"
#include "exprTree.h"
#include "derivative.h"
#include "nameTable.h"

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
#define dL_ derivativeBase(expr->left, variable)
#define dR_ derivativeBase(expr->right, variable)
#define cL_ copyTree(expr->left)
#define cR_ copyTree(expr->right)

/// @brief Create derivative of expr with respect to variable
/// @param expr expression tree
/// @param variable derivative variable
/// @return Derivative tree
Node_t *derivativeBase(Node_t *expr, int variable) {
    switch (expr->type) {
        case VARIABLE:
            if (expr->value.var == variable)
                return NUM_(1);
            else
                return NUM_(0);
        case NUMBER:
            return NUM_(0);
        case OPERATOR:
        {
            switch(expr->value.op) {
                case ADD:
                    return OPR_(ADD, dL_, dR_);
                case SUB:
                    return OPR_(SUB, dL_, dR_);
                case MUL:
                    return OPR_(ADD, OPR_(MUL, dL_, cR_),
                                     OPR_(MUL, cL_, dR_));
                case DIV:
                {
                    Node_t *nominator = OPR_(SUB, OPR_(MUL, dL_, cR_),
                                                  OPR_(MUL, cL_, dR_));
                    Node_t *denominator = OPR_(POW, cR_, NUM_(2));
                    return OPR_(DIV, nominator, denominator);
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
                            return NUM_(0);
                        else
                            // d(f^n) = d(f)*n*f^(n-1)
                            return OPR_(MUL, OPR_(MUL, dL_, cR_),
                                             OPR_(POW, cL_, OPR_(SUB, cR_, NUM_(1)) ) );
                    } else {
                        if (noVarBase) {
                            //d(a^f) = a^f * ln(a) * d(f)
                            return OPR_(MUL, OPR_(MUL, copyTree(expr), dR_),
                                             OPR_(LOGN, cL_, NULL) );
                        } else {
                            //d(g^f) = d( e^(f*ln(g)) ) = g^f * d( f*ln(g) ) = g^f * (df*ln(g) + f*dg/g)
                            Node_t *tempTree = OPR_(MUL, cR_, OPR_(LOGN, cL_, NULL) );

                            Node_t *answer = OPR_(MUL, copyTree(expr), derivativeBase(tempTree, variable));

                            deleteTree(tempTree);
                            return answer;
                        }
                    }

                }
                case SIN:
                    return OPR_(MUL, OPR_(COS, cL_, NULL), dL_);
                case COS:
                    return OPR_(MUL, OPR_(SIN, cL_, NULL),
                                     OPR_(MUL, NUM_(-1), dL_));
                case TAN:
                    return OPR_(DIV, dL_,
                                     OPR_(POW, OPR_(COS, cL_, NULL), NUM_(2) ) );
                case CTG:
                    return OPR_(DIV, OPR_(MUL, NUM_(-1), dL_),
                                     OPR_(POW, OPR_(SIN, cL_, NULL), NUM_(2) ) );
                case LOG:
                    return OPR_(DIV, dR_,
                                     OPR_(MUL, cR_, OPR_(LOGN, cL_, NULL) ) );
                case LOGN:
                    return OPR_(DIV, dL_, cL_);
                default:
                    logPrint(L_ZERO, 1, "Unknown operator type %d\n", expr->value.op);
                    break;
            }
        }
        break;
        default:
            logPrint(L_ZERO, 1, "Unknown expression type %d\n", expr->type);
            break;
    }
    return NULL;
}

Node_t *derivative(Node_t *expr, const char *variable) {
    assert(expr);
    assert(variable);

    int varIdx = findVariable(variable);
    if (varIdx == NULL_VARIABLE) {
        logPrint(L_ZERO, 1, "Unknown variable '%s'\n", variable);
        return NULL;
    } else {
        logPrint(L_EXTRA, 0, "Derivative: var = %d, varName = %s\n", varIdx, getVariableName(varIdx));
    }

    return derivativeBase(expr, varIdx);


}


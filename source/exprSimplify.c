#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <assert.h>
#include <math.h>

#include "hashTable.h"
#include "logger.h"
#include "tex.h"
#include "exprTree.h"
#include "treeDSL.h"
/*===========Tree simplification================================*/

Node_t *foldConstants(Node_t *node, bool *changedTree) {
    if (node->type == NUMBER || node->type == VARIABLE) {
        return node;
    }

    assert(node->type == OPERATOR);

    if (!operators[node->value.op].commutative) {
        Node_t *left = NULL, *right = NULL;
        bool binary = operators[node->value.op].binary;

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
    } else {
        enum OperatorType op = node->value.op;

        //array with leafs of current commutative operation (e.g. + or *)
        Node_t *operLeafs[EXPR_TREE_MAX_LIST_COUNT] = {0};
        size_t leafsCount = 0;

        //array to find lists using depth-first search
        Node_t *listsStack[EXPR_TREE_MAX_LIST_COUNT] = {0};
        size_t stackSize = 0;

        //array with nodes of type operator
        Node_t *operNodes[EXPR_TREE_MAX_LIST_COUNT] = {0};
        operNodes[0] = node;
        size_t operNodesCount = 1;

        // Depth-first search
        listsStack[0] = node->right;
        listsStack[1] = node->left;
        stackSize = 2;

        while (stackSize > 0) {
            stackSize--;
            Node_t *current = listsStack[stackSize];
            logPrint(L_EXTRA, 0, "StackSize = %d, current = %p\n", stackSize, current);

            if (current->type != OPERATOR || current->value.op != op) {
                logPrint(L_EXTRA, 0, "Calling foldConstants for %p: type = %d\n", current->type);
                current = foldConstants(current, changedTree);
            }

            if (current->type == OPERATOR && current->value.op == op) {
                logPrint(L_EXTRA, 0, "Added = %p\n", current->right);
                logPrint(L_EXTRA, 0, "Added = %p\n", current->left);

                listsStack[stackSize] = current->right;
                stackSize++;
                listsStack[stackSize] = current->left;
                stackSize++;

                operNodes[operNodesCount++] = current;
            } else {
                logPrint(L_EXTRA, 0, "Pushed %p to leafs array\n", current);
                //! FIRST ELEMENT IS RESERVED FOR NUMBER NODE
                operLeafs[1 + leafsCount++] = current;
            }

            logPrint(L_EXTRA, 0, "~StackSize = %d, current = %p\n", stackSize, current);

        }

        // simplifying constants and rearranging tree
        //? NOTE: starting from 1 in operLeafs[...] because first element is reserved for numberNode
        Node_t *numberNode = NULL;

        size_t writeIdx = 1;
        for (size_t readIdx = 1; readIdx < leafsCount + 1; readIdx++) {
            if (operLeafs[readIdx]->type == NUMBER) {
                if (!numberNode) {
                    numberNode = operLeafs[readIdx];
                } else {
                    *changedTree = true;
                    numberNode->value.number = calculateOperation(op,
                                                                  numberNode->value.number,
                                                                  operLeafs[readIdx]->value.number);
                    deleteTree(operLeafs[readIdx]);
                }

            } else {
                operLeafs[writeIdx++] = operLeafs[readIdx];
            }
        }


        size_t operCounter = 0;
        Node_t *current = operNodes[0];
        size_t leafStartIdx = 1;
        if (numberNode) {
            operLeafs[0] = numberNode;
            leafStartIdx = 0;
        }
        leafsCount = writeIdx - leafStartIdx;

        // if all nodes were numbers
        if (leafsCount == 1) {
            node->type = NUMBER;
            node->value.number = numberNode->value.number;
            node->left = NULL;
            node->right = NULL;
            deleteTree(numberNode);
        } else {
            logPrint(L_EXTRA, 0, "Operator leafs: total = %d\n", leafsCount);
            for (int i = leafStartIdx; (i - leafStartIdx) < leafsCount; i++) {
                logPrint(L_EXTRA, 0, "i=%d : %p : type = %d\n", i, operLeafs[i], operLeafs[i]->type);
            }

            logPrint(L_EXTRA, 0, "Rearranging tree\n");

            for (size_t idx = 0; idx < leafsCount - 1; idx++) {
                current->left = operLeafs[leafStartIdx + idx];
                operLeafs[leafStartIdx + idx]->parent = current;
                if (idx == (leafsCount - 2) )
                    current->right = operLeafs[leafStartIdx + idx + 1];
                else if (operCounter < operNodesCount)
                    current->right = operNodes[++operCounter];
                else
                    current->right = OPR_(op, NULL, NULL);

                current->right->parent = current;
                current = current->right;
            }
        }

        for (size_t operIdx = operCounter + 1; operIdx < operNodesCount; operIdx++) {
            free(operNodes[operIdx]);
        }
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
        Node_t *copy = copyTree(node);
        changedTree = false;
        node = foldConstants(node, &changedTree);
        node = removeNeutralOperations(node, &changedTree);
        if (changedTree) {
            texPrintf(tex, "Упростим выражение:\n\n");
            exprTexDump(tex, context, copy);
            texPrintf(tex, " = ");
            exprTexDump(tex, context, node);
            texPrintf(tex, "\n\n");
        }
        deleteTree(copy);
    } while (changedTree);

    return node;
}

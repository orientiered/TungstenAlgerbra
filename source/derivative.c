#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "logger.h"
#include "exprTree.h"
#include "derivative.h"

#include "treeDSL.h"

Node_t *derivative(Node_t *expression) {
    switch (expression->type) {
        case VARIABLE:
            return NUM_(1);
        case NUMBER:
            return NUM_(0);
        case OPERATOR:
        {
            switch(expression->value.op) {
                case ADD:
                    return OPR_(ADD, dL_(expression), dR_(expression));
                case SUB:
                    return OPR_(SUB, dL_(expression), dR_(expression));
                case MUL:
                    return OPR_(ADD, OPR_(MUL, dL_(expression), cR_(expression)),
                                     OPR_(MUL, cL_(expression), dR_(expression)));
                case DIV:
                {
                    Node_t *nominator = OPR_(SUB, OPR_(MUL, dL_(expression), cR_(expression)),
                                                  OPR_(MUL, cL_(expression), dR_(expression)));
                    Node_t *denominator = OPR_(POW, cR_(expression), NUM_(2));
                    return OPR_(DIV, nominator, denominator);
                }
                case POW:
                    //TODO:
                    return NULL;
                case SIN:
                    return OPR_(MUL, OPR_(COS, cL_(expression), NULL), dL_(expression));
                case COS:
                    return OPR_(MUL, OPR_(SIN, cL_(expression), NULL),
                                     OPR_(MUL, NUM_(-1), dL_(expression)));
                case TAN:
                    return OPR_(DIV, dL_(expression),
                                     OPR_(POW, OPR_(COS, cL_(expression), NULL), NUM_(2) ) );
                case CTG:
                    return OPR_(DIV, OPR_(MUL, NUM_(-1), dL_(expression)),
                                     OPR_(POW, OPR_(SIN, cL_(expression), NULL), NUM_(2) ) );
                case LOG:
                    return OPR_(DIV, dR_(expression),
                                     OPR_(MUL, cR_(expression), OPR_(LOGN, cL_(expression), NULL) ) );
                case LOGN:
                    return OPR_(DIV, dL_(expression), cL_(expression));

            }
        }
    }
    return NULL;
}


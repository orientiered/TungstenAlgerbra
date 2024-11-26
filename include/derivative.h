#ifndef DERIVATIVE_H
#define DERIVATIVE_H

Node_t *derivativeBase(Node_t *expr, int variable);

Node_t *derivative(Node_t *expr, const char *variable);
#endif

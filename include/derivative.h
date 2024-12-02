#ifndef DERIVATIVE_H
#define DERIVATIVE_H

Node_t *derivativeBase(Node_t *expr, int variable);

Node_t *derivative(TungstenContext_t *context, Node_t *expr, const char *variable);

Node_t *TaylorExpansion(TexContext_t *tex, TungstenContext_t *context,
                        Node_t *expr, const char *variable,
                        double point, size_t nmemb);


#endif

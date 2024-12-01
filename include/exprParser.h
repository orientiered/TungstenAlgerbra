#ifndef EXPR_PARSER_H
#define EXPR_PARSER_H

typedef struct {
    const char *str;
    const char *pointer;
    bool success;
} ParseContext_t;

/*
Positive examples = {x * 3 + 2 - x^(3+2), 4, x, (2+x)^2}
Negative examples = {y, sin(), 5.23}

Grammar::=Expr'\0'
Expr   ::=MulPr{ ['+''-']  MulPr}*
MulPr  ::=PowPr{ ['*''/']  PowPr}*
PowPr  ::=Primary{ '^'  PowPr}?

Primary::= '(' Expr ')' | Var | Num

Var    ::=['a'-'z''_']+ ['a'-'z''_''0'-'9']*
Num    ::=['0'-'9']+
*/

#define SyntaxError(context, ret, ...)                                      \
    do {                                                                    \
        context->success = false;                                           \
        logPrint(L_ZERO, 1, "Error while parsing context[%p]\n", context);  \
        logPrint(L_ZERO, 1, "Current position: '%s'\n", context->pointer);  \
        logPrint(L_ZERO, 1, __VA_ARGS__);                                   \
        return ret;                                                         \
    } while(0)

Node_t *GetGrammar(ParseContext_t *context);
Node_t *GetExpr(ParseContext_t *context);
Node_t *GetMulPr(ParseContext_t *context);
Node_t *GetPowPr(ParseContext_t *context);

Node_t *GetPrimary(ParseContext_t *context);
Node_t *GetVar(ParseContext_t *context);
Node_t *GetNum(ParseContext_t *context);
#endif
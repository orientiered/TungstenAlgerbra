#ifndef EXPR_PARSER_H
#define EXPR_PARSER_H

typedef struct {
    const char *str;
    const char *pointer;      // current position in str
    bool success;
} ParseContext_t;

/*
Positive examples = {x * 3 + 2 - x^(3+2), 4, x, (2+x)^2}
Negative examples = {y, sin(), 5.23}

Grammar::=Expr'\0'
Expr   ::=MulPr{ ['+''-']  MulPr}*
MulPr  ::=PowPr{ ['*''/']  PowPr}*
PowPr  ::=Primary{ '^'  PowPr}?

Primary::= '(' Expr ')' | Func | Var | Num

Func   ::=["sin" "cos" "tg" "ctg" "ln"]'(' Expr ')'
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

Node_t *GetGrammar(ParseContext_t *context, TungstenContext_t *tungstenContext);
Node_t *GetExpr(ParseContext_t *context, TungstenContext_t *tungstenContext);
Node_t *GetMulPr(ParseContext_t *context, TungstenContext_t *tungstenContext);
Node_t *GetPowPr(ParseContext_t *context, TungstenContext_t *tungstenContext);

Node_t *GetPrimary(ParseContext_t *context, TungstenContext_t *tungstenContext);

Node_t *GetFunc(ParseContext_t *context, TungstenContext_t *tungstenContext);
Node_t *GetVar(ParseContext_t *context, TungstenContext_t *tungstenContext);
Node_t *GetNum(ParseContext_t *context, TungstenContext_t *tungstenContext);
#endif

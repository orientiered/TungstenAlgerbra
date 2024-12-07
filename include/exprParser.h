#ifndef EXPR_PARSER_H
#define EXPR_PARSER_H

enum ParseStatus {
    SOFT_ERROR,
    HARD_ERROR,
    PARSE_SUCCESS
};

typedef struct {
    const char *str;
    const char *pointer;      // current position in str
    enum ParseStatus status;
} ParseContext_t;

const size_t PARSER_BUFFER_SIZE = 64;
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

Num    ::=strtod() (any valid floating point number)
*/

#define SyntaxError(context, ret, ...)                                      \
    do {                                                                    \
        context->status = HARD_ERROR;                                       \
        logPrint(L_ZERO, 1, "Error while parsing context[%p]\n", context);  \
        logPrint(L_ZERO, 1, "Current position: '%s'\n", context->pointer);  \
        logPrint(L_ZERO, 1, __VA_ARGS__);                                   \
        return ret;                                                         \
    } while(0)

#endif

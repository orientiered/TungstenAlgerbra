#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <assert.h>
#include <string.h>

#include "logger.h"
#include "exprTree.h"
#include "exprParser.h"
#include "nameTable.h"

Node_t *parseExpression(const char *expression) {
    assert(expression);
    ParseContext_t context = {expression, expression, true};
    return GetGrammar(&context);
}

static size_t skipSpaces(ParseContext_t *context) {
    assert(context);
    size_t shift = 0;
    while (context->pointer[shift] == ' ' || context->pointer[shift] == '\t')
        shift++;
    context->pointer += shift;
    return shift;
}

static void movePointer(ParseContext_t *context) {
    context->pointer++;
    skipSpaces(context);
}

Node_t *GetGrammar(ParseContext_t *context) {
    skipSpaces(context);
    Node_t *val = GetExpr(context);
    if (!context->success)
        return val;

    if (*context->pointer == '\0') {
        context->pointer++;
        return val;
    } else SyntaxError(context, val, "GetGrammar: Expected '\\0', got '%c'", *context->pointer);
}

Node_t *GetExpr(ParseContext_t *context) {
    Node_t *left = GetMulPr(context);
    if (!context->success)
        return left;

    while (*context->pointer == '+' || *context->pointer == '-') {
        char op = *context->pointer;
        movePointer(context);
        Node_t *right = GetMulPr(context);

        if (!context->success)
            return left;

        left = createNode(OPERATOR, (op == '+') ? ADD : SUB, 0, left, right);
    }

    return left;
}

Node_t *GetMulPr(ParseContext_t *context) {
    Node_t *left = GetPowPr(context);
    if (!context->success)
        return left;

    while (*context->pointer == '*' || *context->pointer == '/') {
        char op = *context->pointer;
        movePointer(context);
        Node_t *right = GetPowPr(context);

        if (!context->success)
            return left;

        left = createNode(OPERATOR, (op == '*') ? MUL : DIV, 0, left, right);
    }

    return left;
}

Node_t *GetPowPr(ParseContext_t *context) {
    Node_t *left = GetPrimary(context);
    if (!context->success)
        return left;

    if (*context->pointer == '^') {
        movePointer(context);
        Node_t *right = GetPowPr(context);
        if (!context->success)
            return left;

        left = createNode(OPERATOR, POW, 0, left, right);
    }

    return left;
}

Node_t *GetPrimary(ParseContext_t *context) {
    if (*context->pointer == '(') {
        movePointer(context);
        Node_t *val = GetExpr(context);
        if (!context->success)
            return val;


        if (*context->pointer != ')')
            SyntaxError(context, val, "GetPrimary: expected ')', got %c\n", *context->pointer);
        else {
            movePointer(context);
            return val;
        }
    }

    Node_t *val = GetVar(context);
    if (context->success)
        return val;
    else
        context->success = true;

    val = GetNum(context);
    if (!context->success)
        SyntaxError(context, val, "GetPrimary: expected (expr), 'x', or Number, got neither\n");

    return val;

}

Node_t *GetVar(ParseContext_t *context) {
    char buffer[MAX_VARIABLE_LEN] = "";
    const char *current = context->pointer;

    if ('a' <= *current && *current <= 'z' || *current == '_') {
        current++;
    } else {
        context->success = false;
        return NULL;
    }

    while ('a' <= *current && *current <= 'z' ||
           '0' <= *current && *current <= '9' ||
           '_' == *current)
        current++;

    strncpy(buffer, context->pointer, (size_t)(current - context->pointer));
    size_t varIdx = insertVariable(buffer);
    context->pointer = current;
    skipSpaces(context);
    return createNode(VARIABLE, varIdx, 0, NULL, NULL);
}

Node_t *GetNum(ParseContext_t *context) {
    int num = 0;

    if (*context->pointer < '0' || *context->pointer > '9') {
        context->success = false;
        return NULL;
    }
    while (*context->pointer >= '0' && *context->pointer <= '9') {
        num = num * 10 + *context->pointer - '0';
        context->pointer++;
    }

    skipSpaces(context);
    Node_t *val = createNode(NUMBER, 0, num, NULL, NULL);
    return val;
}

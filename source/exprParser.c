#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <assert.h>
#include <string.h>

#include "tex.h"
#include "hashTable.h"
#include "logger.h"
#include "exprTree.h"
#include "exprParser.h"

Node_t *parseExpression(TungstenContext_t *tungstenContext, const char *expression) {
    assert(expression);
    ParseContext_t context = {expression, expression, true};
    return GetGrammar(&context, tungstenContext);
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

Node_t *GetGrammar(ParseContext_t *context, TungstenContext_t *tungstenContext) {
    skipSpaces(context);
    Node_t *val = GetExpr(context, tungstenContext);
    if (!context->success) {
        deleteTree(val);
        return NULL;
    }

    if (*context->pointer == '\0') {
        context->pointer++;
        return val;
    } else {
        deleteTree(val);
        SyntaxError(context, NULL, "GetGrammar: Expected '\\0', got '%c'", *context->pointer);
    }
}

Node_t *GetExpr(ParseContext_t *context, TungstenContext_t *tungstenContext) {
    Node_t *left = GetMulPr(context, tungstenContext);
    if (!context->success)
        return left;

    while (*context->pointer == '+' || *context->pointer == '-') {
        char op = *context->pointer;
        movePointer(context);
        Node_t *right = GetMulPr(context, tungstenContext);

        if (!context->success)
            return left;

        left = createNode(OPERATOR, (op == '+') ? ADD : SUB, 0, left, right);
    }

    return left;
}

Node_t *GetMulPr(ParseContext_t *context, TungstenContext_t *tungstenContext) {
    Node_t *left = GetPowPr(context, tungstenContext);
    if (!context->success)
        return left;

    while (*context->pointer == '*' || *context->pointer == '/') {
        char op = *context->pointer;
        movePointer(context);
        Node_t *right = GetPowPr(context, tungstenContext);

        if (!context->success)
            return left;

        left = createNode(OPERATOR, (op == '*') ? MUL : DIV, 0, left, right);
    }

    return left;
}

Node_t *GetPowPr(ParseContext_t *context, TungstenContext_t *tungstenContext) {
    Node_t *left = GetPrimary(context, tungstenContext);
    if (!context->success)
        return left;

    if (*context->pointer == '^') {
        movePointer(context);
        Node_t *right = GetPowPr(context, tungstenContext);
        if (!context->success)
            return left;

        left = createNode(OPERATOR, POW, 0, left, right);
    }

    return left;
}

Node_t *GetPrimary(ParseContext_t *context, TungstenContext_t *tungstenContext) {
    if (*context->pointer == '(') {
        movePointer(context);
        Node_t *val = GetExpr(context, tungstenContext);
        if (!context->success)
            return val;


        if (*context->pointer != ')')
            SyntaxError(context, val, "GetPrimary: expected ')', got %c\n", *context->pointer);
        else {
            movePointer(context);
            return val;
        }
    }

    Node_t *val = GetFunc(context, tungstenContext);

    if (context->success) return val;
    else context->success = true;

    val = GetVar(context, tungstenContext);

    if (context->success) return val;
    else context->success = true;

    val = GetFloat(context, tungstenContext);
    if (!context->success)
        SyntaxError(context, val, "GetPrimary: expected (expr), 'x', or Number, got neither\n");

    return val;

}

Node_t *GetFunc(ParseContext_t *context, TungstenContext_t *tungstenContext) {
    const char *current = context->pointer;
    Operator_t op = operators[ADD];
    if (strncmp(current, "sin", 3) == 0) {
        op = operators[SIN];
    } else if (strncmp(current, "cos", 3) == 0) {
        op = operators[COS];
    } else if (strncmp(current, "tg", 2) == 0) {
        op = operators[TAN];
    } else if (strncmp(current, "ctg", 3) == 0) {
        op = operators[CTG];
    } else if (strncmp(current, "ln", 2) == 0) {
        op = operators[LOGN];
    } else {
        context->success = false;
        return NULL;
    }

    context->pointer += strlen(op.str);
    if (*context->pointer != '(') {
        SyntaxError(context, NULL, "GetFunc: expected '(' but found '%c'\n", *current);
    }
    movePointer(context);
    Node_t *val = GetExpr(context, tungstenContext);

    if (!context->success)
        return val;

    if (*context->pointer != ')') {
        SyntaxError(context, NULL, "GetFunc: expected ')' but found '%c'\n", *current);
    }
    movePointer(context);

    return createNode(OPERATOR, op.opCode, 0, val, NULL);
}

Node_t *GetVar(ParseContext_t *context, TungstenContext_t *tungstenContext) {
    const char *current = context->pointer;

    if (('a' <= *current && *current <= 'z') || *current == '_') {
        current++;
    } else {
        context->success = false;
        return NULL;
    }

    while (('a' <= *current && *current <= 'z') ||
           ('0' <= *current && *current <= '9') ||
           '_' == *current)
        current++;

    size_t varNameLen = (size_t)(current - context->pointer);
    char buffer[MAX_VARIABLE_LEN] = "";
    strncpy(buffer, context->pointer, (varNameLen <  MAX_VARIABLE_LEN) ? varNameLen : MAX_VARIABLE_LEN - 1);
    size_t varIdx = insertVariable(tungstenContext, buffer);

    context->pointer = current;
    skipSpaces(context);
    return createNode(VARIABLE, varIdx, 0, NULL, NULL);
}

Node_t *GetFloat(ParseContext_t *context, TungstenContext_t *tungstenContext) {
    Node_t *leading = GetNum(context, tungstenContext);
    if (!context->success)
        return leading;

    if (*context->pointer == '.') {
        context->pointer++;

        Node_t *trailing = GetNum(context, tungstenContext);
        if (!context->success) {
            deleteTree(trailing);
            SyntaxError(context, leading, "GetFloat: expected number after '.', got %c\n", *context->pointer);
        }

        double trailingPart = trailing->value.number;
        deleteTree(trailing);

        while (trailingPart > 1)
            trailingPart /= 10;

        leading->value.number += trailingPart;

    }

    return leading;
}

Node_t *GetNum(ParseContext_t *context, TungstenContext_t *tungstenContext) {
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

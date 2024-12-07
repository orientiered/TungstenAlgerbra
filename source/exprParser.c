#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>

#include "tex.h"
#include "hashTable.h"
#include "logger.h"
#include "exprTree.h"
#include "exprParser.h"

static Node_t *GetGrammar(ParseContext_t *context, TungstenContext_t *tungstenContext);

Node_t *parseExpression(TungstenContext_t *tungstenContext, const char *expression) {
    assert(expression);
    ParseContext_t context = {expression, expression, PARSE_SUCCESS};
    return GetGrammar(&context, tungstenContext);
}

static Node_t *GetExpr(ParseContext_t *context, TungstenContext_t *tungstenContext);
static Node_t *GetMulPr(ParseContext_t *context, TungstenContext_t *tungstenContext);
static Node_t *GetPowPr(ParseContext_t *context, TungstenContext_t *tungstenContext);

static Node_t *GetPrimary(ParseContext_t *context, TungstenContext_t *tungstenContext);

static Node_t *GetFunc(ParseContext_t *context, TungstenContext_t *tungstenContext, const char *buffer);
static Node_t *GetVar(ParseContext_t *context, TungstenContext_t *tungstenContext, const char *buffer);

static Node_t *GetNum(ParseContext_t *context, TungstenContext_t *tungstenContext);

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
    if (context->status != PARSE_SUCCESS) {
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
    logPrint(L_EXTRA, 0, "%s: %s\n\n", __PRETTY_FUNCTION__, context->pointer);

    Node_t *left = GetMulPr(context, tungstenContext);
    if (context->status != PARSE_SUCCESS)
        return left;

    while (*context->pointer == '+' || *context->pointer == '-') {
        char op = *context->pointer;
        movePointer(context);
        Node_t *right = GetMulPr(context, tungstenContext);

        if (context->status != PARSE_SUCCESS)
            return left;

        left = createNode(OPERATOR, (op == '+') ? ADD : SUB, 0, left, right);
    }

    return left;
}

Node_t *GetMulPr(ParseContext_t *context, TungstenContext_t *tungstenContext) {
    logPrint(L_EXTRA, 0, "%s: %s\n\n", __PRETTY_FUNCTION__, context->pointer);

    Node_t *left = GetPowPr(context, tungstenContext);
    if (context->status != PARSE_SUCCESS)
        return left;

    while (*context->pointer == '*' || *context->pointer == '/') {
        char op = *context->pointer;
        movePointer(context);
        Node_t *right = GetPowPr(context, tungstenContext);

        if (context->status != PARSE_SUCCESS)
            return left;

        left = createNode(OPERATOR, (op == '*') ? MUL : DIV, 0, left, right);
    }

    return left;
}

Node_t *GetPowPr(ParseContext_t *context, TungstenContext_t *tungstenContext) {
    logPrint(L_EXTRA, 0, "%s: %s\n\n", __PRETTY_FUNCTION__, context->pointer);

    Node_t *left = GetPrimary(context, tungstenContext);
    if (context->status != PARSE_SUCCESS)
        return left;

    if (*context->pointer == '^') {
        movePointer(context);
        Node_t *right = GetPowPr(context, tungstenContext);
        if (context->status != PARSE_SUCCESS)
            return left;

        left = createNode(OPERATOR, POW, 0, left, right);
    }

    return left;
}

static size_t GetId(char *buffer, const char *str) {
    assert(buffer);
    assert(str);
    assert(*str);

    const char *startPos = buffer;

    if (isalpha(*str) || *str == '_') {
        *buffer++ = *str++;
    }

    while (isalnum(*str) || *str == '_') {
        *buffer++ = *str++;
    }

    *buffer = '\0';
    return buffer - startPos;
}

Node_t *GetPrimary(ParseContext_t *context, TungstenContext_t *tungstenContext) {
    logPrint(L_EXTRA, 0, "%s: %s\n\n", __PRETTY_FUNCTION__, context->pointer);

    if (*context->pointer == '(') {
        movePointer(context);
        Node_t *val = GetExpr(context, tungstenContext);
        if (context->status != PARSE_SUCCESS)
            return val;


        if (*context->pointer != ')')
            SyntaxError(context, val, "GetPrimary: expected ')', got %c\n", *context->pointer);
        else {
            movePointer(context);
            return val;
        }
    }

    Node_t *val = GetNum(context, tungstenContext);

    if (context->status == PARSE_SUCCESS) return val;
    else context->status = PARSE_SUCCESS;

    char buffer[PARSER_BUFFER_SIZE] = "";
    size_t idLength = GetId(buffer, context->pointer);

    context->pointer += idLength;

    val = GetFunc(context, tungstenContext, buffer);

    if (context->status != SOFT_ERROR) return val;
    else context->status = PARSE_SUCCESS;

    val = GetVar(context, tungstenContext, buffer);

    if (context->status != PARSE_SUCCESS)
        SyntaxError(context, val, "GetPrimary: expected (expr), function(), Variable or Number, got neither\n");

    return val;

}


Node_t *GetFunc(ParseContext_t *context, TungstenContext_t *tungstenContext, const char *buffer) {
    logPrint(L_EXTRA, 0, "%s: %s\nBuffer: %s\n", __PRETTY_FUNCTION__, context->pointer, buffer);


    Operator_t op = {};

    bool foundOperator = false;
    for (unsigned idx = 0; idx < sizeof(operators) / sizeof(operators[0]); idx++) {
        if (strcmp(buffer, operators[idx].str) == 0) {
            op = operators[idx];
            foundOperator = true;
            break;
        }
    }


    if (!foundOperator) {
        logPrint(L_EXTRA, 0, "%s: no operators \n\n", __PRETTY_FUNCTION__);
        context->status = SOFT_ERROR;
        return NULL;
    }

    logPrint(L_EXTRA, 0, "%s: found operator %s\n\n", __PRETTY_FUNCTION__, op.str);

    if (*context->pointer != '(') {
        SyntaxError(context, NULL, "GetFunc: expected '(' but found '%c'\n", *context->pointer);
    }
    movePointer(context);
    Node_t *val = GetExpr(context, tungstenContext);

    if (context->status != PARSE_SUCCESS)
        return val;

    if (*context->pointer != ')') {
        SyntaxError(context, NULL, "GetFunc: expected ')' but found '%c'\n", *context->pointer);
    }
    movePointer(context);

    return createNode(OPERATOR, op.opCode, 0, val, NULL);
}

Node_t *GetVar(ParseContext_t *context, TungstenContext_t *tungstenContext, const char *buffer) {
    logPrint(L_EXTRA, 0, "%s: %s\nBuffer: '%s'\n", __PRETTY_FUNCTION__, context->pointer, buffer);

    if (strlen(buffer) == 0) {
        context->status = SOFT_ERROR;
        return NULL;
    }

    size_t varIdx = insertVariable(tungstenContext, buffer);

    skipSpaces(context);
    return createNode(VARIABLE, varIdx, 0, NULL, NULL);
}

Node_t *GetNum(ParseContext_t *context, TungstenContext_t *tungstenContext) {
    logPrint(L_EXTRA, 0, "%s: %s\n\n", __PRETTY_FUNCTION__, context->pointer);

    char *endPtr = NULL;
    double num = strtod(context->pointer, &endPtr);
    if (endPtr == context->pointer) {
        context->status = SOFT_ERROR;
        return NULL;
    }
    context->pointer = (const char *) endPtr;

    skipSpaces(context);
    Node_t *val = createNode(NUMBER, 0, num, NULL, NULL);
    return val;
}

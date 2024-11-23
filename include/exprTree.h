#ifndef EXPRESSION_TREE_H
#define EXPRESSION_TREE_H

#define DOTS_DIR "dot"
#define IMGS_DIR "img"

#define EXPR_TREE_DUMP_DOT_FORMAT "expr_%04zu.dot"
#define EXPR_TREE_DUMP_IMG_FORMAT "expr_dump_%04zu."

const size_t DUMP_BUFFER_SIZE = 128;

enum ElemType {
    OPERATOR,
    VARIABLE,
    NUMBER
};

typedef enum OperatorType {
    NULL_OPERATOR = '\0',
    ADD = '+',
    SUB = '-',
    MUL = '*',
    DIV = '/'
} OperatorType_t;

typedef enum TungstenStatus_t {
    TA_SUCCESS,
    TA_SYNTAX_ERROR,
    TA_MEMORY_ERROR,
    TA_NULL_PTR,
    TA_DUMP_ERROR
} TungstenStatus_t;

typedef struct Node_t {
    Node_t *parent;

    enum ElemType type;

    union {
        enum OperatorType op; ///< +, -, /, etc.
        int var;              ///< x, y, z, etc.
        double number;        ///< Floating point number
    } value;

    Node_t *left;
    Node_t *right;
} Node_t;


void setVariable(char variable, double value);
double getVariable(char variable);


Node_t *createNode(enum ElemType type, int iVal, double dVal, Node_t *left, Node_t *right);
TungstenStatus_t deleteTree(Node_t *node);

Node_t *copyTree(Node_t *node);


TungstenStatus_t verifyTree(Node_t *node);
TungstenStatus_t dumpTree(Node_t *node);

Node_t *parseExpressionPrefix(const char *expression);

double evaluate(Node_t *node, bool *usedVariable);

#if defined(_TREE_DUMP) && !defined(NDEBUG)

#define DUMP_TREE(node) \
    logPrint(L_ZERO, 0, "<h2>Node_t dump called from %s:%d %s</h2>\n", __FILE__, __LINE__, __PRETTY_FUNCTION__); \
    dumpTree(node);

#else

# define DUMP_TREE(node)

#endif


#endif

#ifndef EXPRESSION_TREE_H
#define EXPRESSION_TREE_H

#define DOTS_DIR "dot"
#define IMGS_DIR "img"

#define EXPR_TREE_DUMP_DOT_FORMAT "expr_%04zu.dot"
#define EXPR_TREE_DUMP_IMG_FORMAT "expr_dump_%04zu."

const char * const OPERATOR_COLOR       = "#EDE1CF";
const char * const VARIABLE_COLOR       = "#f3d4de";
const char * const NUMBER_COLOR         = "#107980";
const char * const DEFAULT_NODE_COLOR   = "#000000";
const size_t DUMP_BUFFER_SIZE = 128;
const size_t PARSER_BUFFER_SIZE = 32;

const double DOUBLE_EPSILON = 1e-7; //epsilon for comparing doubles

enum ElemType {
    OPERATOR,
    VARIABLE,
    NUMBER
};

enum OperatorType {
    ADD = 0,    ///< +
    SUB,        ///< -
    MUL,        ///< *
    DIV,        ///< /
    POW,        ///< ^
    SIN,        ///< sin
    COS,        ///< cos
    TAN,        ///< tan
    CTG,        ///< ctg
    LOG,        ///< log_x(y)
    LOGN,       ///< ln
};

typedef struct {
    enum OperatorType opCode;
    bool binary;
    const char *str;
    unsigned priority;
} Operator_t;

const Operator_t operators[] = {
    {ADD, 1, "+"   , 0},
    {SUB, 1, "-"   , 0},
    {MUL, 1, "*"   , 1},
    {DIV, 1, "/"   , 2}, // \frac{}{},
    {POW, 1, "^"   , 2},
    {SIN, 0, "sin" , 3},
    {COS, 0, "cos" , 3},
    {TAN, 0, "tg"  , 3},
    {CTG, 0, "ctg" , 3},
    {LOG, 1, "log" , 3},
    {LOGN, 0, "ln" , 3}
};

typedef enum TungstenStatus_t {
    TA_SUCCESS,
    TA_SYNTAX_ERROR,
    TA_MEMORY_ERROR,
    TA_NULL_PTR,
    TA_DUMP_ERROR
} TungstenStatus_t;

union NodeValue {
    double number;        ///< Floating point number
    enum OperatorType op; ///< +, -, /, etc.
    int var;              ///< x, y, z, etc.
};

typedef struct Node_t {
    Node_t *parent;

    enum ElemType type;

    union NodeValue value;

    Node_t *left;
    Node_t *right;
} Node_t;


Node_t *createNode(enum ElemType type, int iVal, double dVal, Node_t *left, Node_t *right);
TungstenStatus_t deleteTree(Node_t *node);

Node_t *copyTree(Node_t *node);


TungstenStatus_t verifyTree(Node_t *node);
TungstenStatus_t dumpTree(Node_t *node, bool minified);

int exprTexDump(Node_t *node);

Node_t *parseExpressionPrefix(const char *expression);

//Parse expression written if natural mathematical way
Node_t *parseExpression(const char *expression);

double evaluate(Node_t *node, bool *usedVariable);

/// @brief Fold constants in tree.
/// !NOTE: Do not assign return value of this function, all simplifications are done inplace
Node_t *foldConstants(Node_t *node, bool *changedTree);

Node_t *removeNeutralOperations(Node_t *node, bool *changedTree);

Node_t *simplifyExpression(Node_t *node);


#if defined(_TREE_DUMP) && !defined(NDEBUG)

#define DUMP_TREE(node, minified) \
    logPrint(L_ZERO, 0, "<h2>Node_t dump called from %s:%d %s</h2>\n", __FILE__, __LINE__, __PRETTY_FUNCTION__); \
    dumpTree(node, minified);

#else

# define DUMP_TREE(...)

#endif


#endif

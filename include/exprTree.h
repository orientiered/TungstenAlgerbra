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

const size_t EXPR_TREE_MAX_LIST_COUNT = 64;
const size_t EXPR_TREE_MAX_SUBST_COUNT = 64;
const double DOUBLE_EPSILON = 1e-12; //epsilon for comparing doubles

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
    bool commutative;
    const char *str;
    const char *texStr;
    unsigned priority;
} Operator_t;

const Operator_t operators[] = {
    {.opCode = ADD,  .binary = 1, .commutative = 1, .str = "+"   , .texStr = "+"        , .priority = 0},
    {.opCode = SUB,  .binary = 1, .commutative = 0, .str = "-"   , .texStr = "-"        , .priority = 0},
    {.opCode = MUL,  .binary = 1, .commutative = 1, .str = "*"   , .texStr = "\\cdot"   , .priority = 1},
    {.opCode = DIV,  .binary = 1, .commutative = 0, .str = "/"   , .texStr = "\\frac"   , .priority = 2},
    {.opCode = POW,  .binary = 1, .commutative = 0, .str = "^"   , .texStr = "^"        , .priority = 2},
    {.opCode = SIN,  .binary = 0, .commutative = 0, .str = "sin" , .texStr = "\\sin"    , .priority = 3},
    {.opCode = COS,  .binary = 0, .commutative = 0, .str = "cos" , .texStr = "\\cos"    , .priority = 3},
    {.opCode = TAN,  .binary = 0, .commutative = 0, .str = "tg"  , .texStr = "\\tan"    , .priority = 3},
    {.opCode = CTG,  .binary = 0, .commutative = 0, .str = "ctg" , .texStr = "\\ctg"    , .priority = 3},
    {.opCode = LOG,  .binary = 1, .commutative = 0, .str = "log" , .texStr = "\\log"    , .priority = 3},
    {.opCode = LOGN, .binary = 0, .commutative = 0, .str = "ln"  , .texStr = "\\ln"     , .priority = 3}
};

typedef enum TungstenStatus_t {
    TA_SUCCESS,
    TA_SYNTAX_ERROR,
    TA_MEMORY_ERROR,
    TA_NULL_PTR,
    TA_DUMP_ERROR
} TungstenStatus_t;

/*========Nametables==============*/
const double defaultVariableValue = 0.0;
const size_t VARIABLE_TABLE_SIZE = 64;
const size_t MAX_VARIABLE_LEN   = 64;

const int NULL_VARIABLE = -1;

typedef struct {
    double number;
    char *str;
} variable_t;

/*================================*/

typedef struct {
    // TexContext_t tex;
    hashTable_t variablesTable;
    variable_t variables[VARIABLE_TABLE_SIZE];
    size_t variablesCount;
} TungstenContext_t;

union NodeValue {
    double number;        ///< Floating point number
    enum OperatorType op; ///< +, -, /, etc.
    int var;              ///< x, y, z, etc.
};

typedef struct Node_t {
    Node_t *parent;

    enum ElemType type;

    union NodeValue value;

    bool isSubstitution;
    char substitutionSymbol;

    Node_t *left;
    Node_t *right;
} Node_t;

TungstenContext_t TungstenCtor();
TungstenStatus_t TungstenDtor(TungstenContext_t *context);

Node_t *createNode(enum ElemType type, int iVal, double dVal, Node_t *left, Node_t *right);
TungstenStatus_t deleteTree(Node_t *node);

Node_t *copyTree(Node_t *node);


TungstenStatus_t verifyTree(Node_t *node);
TungstenStatus_t dumpTree(TungstenContext_t *context, Node_t *node, bool minified);

int exprTexDump(TexContext_t *tex, TungstenContext_t *context, Node_t *node);

Node_t *parseExpressionPrefix(TungstenContext_t *context, const char *expression);

//Parse expression written if natural mathematical way
Node_t *parseExpression(TungstenContext_t *context, const char *expression);

double calculateOperation(enum OperatorType op, double left, double right);

double evaluate(TungstenContext_t *context, Node_t *node);

/// @brief Fold constants in tree.
Node_t *foldConstants(Node_t *node, bool *changedTree);

/// @brief Remove neutral operations such as *1, +0, ^1, etc.
Node_t *removeNeutralOperations(Node_t *node, bool *changedTree);

/// @brief Combine foldConstants() and removeNeutralOperations() while tree can be simplified
Node_t *simplifyExpression(TexContext_t *tex, TungstenContext_t *context, Node_t *node);



/*=====================NameTable functions==========================*/
size_t insertVariable(TungstenContext_t *tungsten, const char *buffer);

int findVariable(TungstenContext_t *tungsten, const char *variableName);

void setVariable(TungstenContext_t *tungsten, const char *variableName, double value);

double getVariable(TungstenContext_t *tungsten, int varIdx);

const char *getVariableName(TungstenContext_t *tungsten, int varIdx);

double getVariableByName(TungstenContext_t *tungsten, const char *variableName);
/*===================================================================*/

#if defined(_TREE_DUMP) && !defined(NDEBUG)

#define DUMP_TREE(context, node, minified) \
    logPrint(L_ZERO, 0, "<h2>Node_t dump called from %s:%d %s</h2>\n", __FILE__, __LINE__, __PRETTY_FUNCTION__); \
    dumpTree(context, node, minified);

#else

# define DUMP_TREE(...)

#endif


#endif

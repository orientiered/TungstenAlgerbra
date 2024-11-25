#define OPR_(op, left, right) createNode(OPERATOR, op, 0, left, right)
#define NUM_(num) createNode(NUMBER, 0, num, NULL, NULL)

// #define VAR_(variable)createNode(VARIABLE, variable, 0, NULL, NULL)

#define dL_(node) derivative(node->left)
#define dR_(node) derivative(node->right)
#define cL_(node) copyTree(node->left)
#define cR_(node) copyTree(node->right)


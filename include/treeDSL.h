static Node_t *createOperatorNode(enum OperatorType op, Node_t *left, Node_t *right) {
    return createNode(OPERATOR, op, 0, left, right);
}

static Node_t *createNumberNode(double number) {
    return createNode(NUMBER, 0, number, NULL, NULL);
}

#define OPR_(op, left, right) createOperatorNode(op, left, right)
#define NUM_(num) createNumberNode(num)

#define VAR_(variable)createNode(VARIABLE, variable, 0, NULL, NULL)




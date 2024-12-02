#define OPR_(op, left, right) createNode(OPERATOR, op, 0, left, right)
#define NUM_(num) createNode(NUMBER, 0, num, NULL, NULL)

#define VAR_(variable)createNode(VARIABLE, variable, 0, NULL, NULL)




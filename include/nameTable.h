#ifndef NAME_TABLE_H
#define NAME_TABLE_H

/*========Nametables==============*/
const double defaultVariableValue = 0.0;
const size_t VARIABLES_TABLE_LEN = 64;
const size_t MAX_VARIABLE_LEN = 8;

const int NULL_VARIABLE = -1;

typedef struct {
    double number;
    char str[MAX_VARIABLE_LEN];
} variable_t;

size_t insertVariable(const char *buffer);

int findVariable(const char *variableName);

void setVariable(const char *variableName, double value);

double getVariable(int varIdx);

const char *getVariableName(int varIdx);

double getVariableByName(const char *variableName);
/*================================*/

#endif

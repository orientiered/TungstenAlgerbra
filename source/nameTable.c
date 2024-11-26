#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "logger.h"
#include "nameTable.h"

static variable_t variables[VARIABLES_TABLE_LEN] = {0};
static size_t variablesCount = 0;

size_t insertVariable(const char *buffer) {
    assert(buffer);
    assert(strlen(buffer) < MAX_VARIABLE_LEN);

    logPrint(L_EXTRA, 0, "Inserting variable '%s'\nCurrent length = %d\n", buffer, variablesCount);
    int varIdx = findVariable(buffer);
    if (varIdx == NULL_VARIABLE) {
        strcpy(variables[variablesCount++].str, buffer);
        return variablesCount - 1;
    } else {
        return varIdx;
    }
}

int findVariable(const char *variableName) {
    for (size_t idx = 0; idx < variablesCount; idx++) {
        if (strcmp(variableName, variables[idx].str) == 0)
            return idx;
    }

    return NULL_VARIABLE;
}

void setVariable(const char *variableName, double value) {
    assert(variableName);

    int var = findVariable(variableName);
    if (var != NULL_VARIABLE) {
        variables[var].number = value;
    } else {
        logPrint(L_ZERO, 1, "Attempt to set unknown variable '%s'\n", variableName);
    }
}

double getVariableByName(const char *variableName) {
    assert(variableName);

    int var = findVariable(variableName);
     if (var != NULL_VARIABLE) {
        return variables[var].number;
    }
    logPrint(L_ZERO, 1, "Attempt to get unknown variable '%s'\n", variableName);

    return defaultVariableValue;
}

const char *getVariableName(int varIdx) {
    assert(varIdx < variablesCount);

    return variables[varIdx].str;
}


double getVariable(int varIdx) {
    assert(varIdx < variablesCount);

    return variables[varIdx].number;
}

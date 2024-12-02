#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "hashTable.h"
#include "tex.h"
#include "logger.h"
#include "exprTree.h"

size_t insertVariable(TungstenContext_t *tungsten, const char *variableName) {
    assert(variableName);
    logPrint(L_EXTRA, 0, "Inserting variable '%s'\nCurrent length = %d\n", variableName, tungsten->variablesCount);

    int varIdx = findVariable(tungsten, variableName);
    if (varIdx != NULL_VARIABLE)
        return varIdx;

    int index = (tungsten->variablesCount)++;
    tungsten->variables[index].str = hashTableInsert(&tungsten->variablesTable, variableName, &index, sizeof(int))->name;
    tungsten->variables[index].number = defaultVariableValue;
    return index;
}

int findVariable(TungstenContext_t *tungsten, const char *variableName) {
    int *varIdx = (int *)hashTableFind(&tungsten->variablesTable, variableName);
    if (varIdx)
        return *varIdx;
    else
        return NULL_VARIABLE;
}

void setVariable(TungstenContext_t *tungsten, const char *variableName, double value) {
    assert(variableName);

    int var = findVariable(tungsten, variableName);
    if (var != NULL_VARIABLE) {
        tungsten->variables[var].number = value;
    } else {
        logPrint(L_ZERO, 1, "Attempt to set unknown variable '%s'\n", variableName);
    }
}

double getVariableByName(TungstenContext_t *tungsten, const char *variableName) {
    assert(variableName);

    int var = findVariable(tungsten, variableName);
     if (var != NULL_VARIABLE) {
        return tungsten->variables[var].number;
    }
    logPrint(L_ZERO, 1, "Attempt to get unknown variable '%s'\n", variableName);

    return defaultVariableValue;
}

const char *getVariableName(TungstenContext_t *tungsten, int varIdx) {
    assert(varIdx < tungsten->variablesCount);

    return tungsten->variables[varIdx].str;
}


double getVariable(TungstenContext_t *tungsten, int varIdx) {
    assert(varIdx < tungsten->variablesCount);

    return tungsten->variables[varIdx].number;
}

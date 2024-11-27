#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include "logger.h"
#include "tex.h"

static FILE *texFile = NULL;

enum TexStatus texInit(const char *name) {
    if (texFile) {
        logPrint(L_ZERO, 1, "Tex file is already opened\n");
        return TEX_SUCCESS;
    }

    if (name)
        texFile = fopen(name, "w");
    else
        texFile = fopen(DEFAULT_TEX_FILE_NAME, "w");

    if (!texFile) {
        logPrint(L_ZERO, 1, "Failed to open tex file\n");
        return TEX_NO_FILE;
    }


    // texPrintf()

    return TEX_SUCCESS;
}

int texPrintf(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    int result = vfprintf(texFile, fmt, args);
    va_end(args);
    return result;
}

enum TexStatus texClose() {
    if (!texFile) {
        logPrint(L_ZERO, 1, "No tex file to close\n");
        return TEX_NO_FILE;
    }

    fclose(texFile);
    return TEX_SUCCESS;
}

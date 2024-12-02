#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include "logger.h"
#include "tex.h"

TexContext_t texInit(const char *name) {
    TexContext_t tex = {};
    if (name)
        strncpy(tex.fileName, name, TEX_MAX_FILENAME_SIZE-1);
    else
        strncpy(tex.fileName, DEFAULT_TEX_FILE_NAME, TEX_MAX_FILENAME_SIZE-1);

    tex.file = fopen(tex.fileName, "w");

    if (!tex.file) {
        logPrint(L_ZERO, 1, "Failed to open tex file\n");
        tex.status = TEX_NO_FILE;
        return tex;
    }

    texPrintf(&tex,
    "\\documentclass[11pt]{article}\n"
    "\\usepackage[utf8x]{inputenc}\n"
    "\\usepackage[russian]{babel}\n"
    "\\usepackage{multicol}\n"
    "\\usepackage{graphicx}\n"
    "\\usepackage[usenames]{color}\n"
    "\\usepackage{xcolor}\n"
    "\\usepackage[russian]{babel}\n"
    "\\usepackage{amsmath, amsfonts, amssymb, amsthm, mathtools, mwe, gensymb}\n"
    );

    texPrintf(&tex,
    "\\title{Пособие по дифференциальному исчислению}\n"
    "\\author{Лев Толстой}\n"
    "\\begin{document}\n"
    "\\maketitle\n"
    );

    tex.status = TEX_SUCCESS;
    return tex;
}

int texPrintf(TexContext_t *tex, const char *fmt, ...) {
    if (!tex->file) return -1;
    va_list args;
    va_start(args, fmt);
    int result = vfprintf(tex->file, fmt, args);
    va_end(args);
    return result;
}

enum TexStatus texClose(TexContext_t *tex) {
    if (!tex->file) {
        logPrint(L_ZERO, 1, "No tex file to close\n");
        return TEX_NO_FILE;
    }

    texPrintf(tex, "\\end{document}\n");
    fclose(tex->file);
    char command[TEX_COMMAND_BUFFER_SIZE] = "";
    sprintf(command, "pdflatex %s", tex->fileName);
    system(command);
    return TEX_SUCCESS;
}

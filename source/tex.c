#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <stdbool.h>

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
    tex.active = true;

    texPrintf(&tex,
    "\\documentclass[11pt]{article}\n"
    "\\usepackage[utf8x]{inputenc}\n"
    "\\usepackage[russian]{babel}\n"
    "\\usepackage{multicol}\n"
    "\\usepackage{graphicx}\n"
    "\\usepackage[usenames]{color}\n"
    "\\usepackage{xcolor}\n"
    "\\usepackage[russian]{babel}\n"
    "\\usepackage{pgfplots}\n"
    "\\pgfplotsset{compat=1.9}\n"
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
    if (!tex->active) return 0;

    if (!tex->file) return -1;
    va_list args;
    va_start(args, fmt);
    int result = vfprintf(tex->file, fmt, args);
    va_end(args);
    return result;
}

int texBeginGraph(TexContext_t *tex) {
    if (!tex->active) return 0;

    if (!tex->file) return -1;
    return fprintf(tex->file,
    "\n\\begin{tikzpicture}\n"
    "\\pgfplotsset{grid = both}\n"
    "\\begin{axis}\n"
    "\\addplot coordinates {\n"
    );
}

int texEndGraph(TexContext_t *tex) {
    if (!tex->active) return 0;

    if (!tex->file) return -1;
    return fprintf(tex->file,
    "};\n"
    "\\end{axis}\n"
    "\\end{tikzpicture}\n"
    );
}

int texAddCoordinates(TexContext_t *tex, double x, double y) {
    if (!tex->active) return 0;

    return texPrintf(tex, "(%.5lg, %.5lg) ", x, y);
}

int texBeginTable(TexContext_t *tex, unsigned columns) {
    if (!tex->active) return 0;

    texPrintf(tex,
    "\\begin{center}\n"
    "\\begin{tabular}\n"
    "{ |");
    for (unsigned idx = 0; idx < columns; idx++)
        texPrintf(tex, "c |");
    texPrintf(tex,
    " }\n"
    "\\hline\n");

    return 1;
}

int texAddTableLine(TexContext_t *tex, bool hLine, unsigned columns, ...) {
    if (!tex->active) return 0;

    va_list va;
    va_start(va, columns);
    int result = 0;

    for (unsigned colIdx = 0; colIdx < columns - 1; colIdx++) {
        const char *elem = va_arg(va, const char *);
        result += texPrintf(tex, "%s & ", elem);
    }
    result += texPrintf(tex,
                        "%s \\\\\n",
                        va_arg(va, const char *));
    if (hLine)
        result += texPrintf(tex, "\\hline\n");
    va_end(va);

    return result;
}

int texEndTable(TexContext_t *tex) {
    if (!tex->active) return 0;
    return texPrintf(tex,
    "\\hline\n"
    "\\end{tabular}\n"
    "\\end{center}\n"
    );
}

enum TexStatus texClose(TexContext_t *tex) {
    if (!tex->file) {
        logPrint(L_ZERO, 1, "No tex file to close\n");
        return TEX_NO_FILE;
    }

    tex->active = true;

    texPrintf(tex, "\\end{document}\n");
    fclose(tex->file);
    char command[TEX_COMMAND_BUFFER_SIZE] = "";
    sprintf(command, "pdflatex %s", tex->fileName);
    system(command);
    return TEX_SUCCESS;
}

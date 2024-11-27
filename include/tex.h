#ifndef TEX_H
#define TEX_H

enum TexStatus {
    TEX_SUCCESS = 0,
    TEX_NO_FILE,
    TEX_ERROR
};

const char * const DEFAULT_TEX_FILE_NAME = "article.tex";

enum TexStatus texInit(const char *name);

int texPrintf(const char *fmt, ...);
enum TexStatus texClose();


#endif

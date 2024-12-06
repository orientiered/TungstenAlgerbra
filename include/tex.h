#ifndef TEX_H
#define TEX_H

const size_t TEX_MAX_FILENAME_SIZE = 64;
const size_t TEX_COMMAND_BUFFER_SIZE = 64;
enum TexStatus {
    TEX_SUCCESS = 0,
    TEX_NO_FILE,
    TEX_ERROR
};

typedef struct {
    char fileName[TEX_MAX_FILENAME_SIZE];
    FILE *file;

    bool active;
    enum TexStatus status;
} TexContext_t;

const char * const DEFAULT_TEX_FILE_NAME = "article.tex";

TexContext_t texInit(const char *name);

int texPrintf(TexContext_t *tex, const char *fmt, ...);
enum TexStatus texClose(TexContext_t *tex);

int texBeginGraph(TexContext_t *tex, const char *xLabel, const char *yLabel, const char *graphTitle);
int texEndGraph(TexContext_t *tex);
int texAddGraph(TexContext_t *tex, double *x, double *y, int pointsCount);


int texBeginTable(TexContext_t *tex, unsigned columns);
int texEndTable(TexContext_t *tex);
int texAddTableLine(TexContext_t *tex, bool hLine, unsigned columns, ...);



#endif

#ifndef OPTIONS_H
#define OPTIONS_H

#include <stdbool.h>

struct Options {
    char *imageFilePath;
    char *tilesetPngFilePath;
    char *tilesetBppFilePath;
    char *tilemapFilePath;
    int bitDepth;
    int numTiles;
    bool isAffineMap;
};

void ParseOptions(struct Options *options, int argc, char **argv);

#endif // OPTIONS_H

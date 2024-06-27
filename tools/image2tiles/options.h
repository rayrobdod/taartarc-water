#ifndef OPTIONS_H
#define OPTIONS_H

#include <stdbool.h>

struct Slice {
    int width;
    int height;
};

struct Options {
    char *imageFilePath;
    char *prefixFilePath;
    char *tilesetPngFilePath;
    char *tilesetBppFilePath;
    char *tilemapFilePath;
    struct Slice slice;
    int bitDepth;
    int numTiles;
    bool isAffineMap;
};

void ParseOptions(struct Options *options, int argc, char **argv);

#endif // OPTIONS_H

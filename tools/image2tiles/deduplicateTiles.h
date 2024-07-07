#ifndef DEDUPLICATETILES_H
#define DEDUPLICATETILES_H

#include "../gbagfx/gfx.h"
#include "options.h"

struct Tileset {
    unsigned char *data;
    int size;
};

/// Creates a tileset and tilemap from an input image
void DeduplicateTiles(
      struct Image *inImage
    , struct Image *outImage
    , struct Tileset *outBpp
    , unsigned maxTilemapSize
    , bool isAffine
    , unsigned paletteBase
    , struct Slice slice
    );

#endif // DEDUPLICATETILES_H

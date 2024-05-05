
#include "deduplicateTiles.h"
#include "options.h"
#include "../gbagfx/convert_png.h"
#include "../gbagfx/gfx.h"
#include "../gbagfx/global.h"
#include "../gbagfx/util.h"

int main(int argc, char **argv)
{
    struct Options options;
    struct Image inImage = {0};
    struct Image outImage = {0};
    struct Tileset outBpp;

    ParseOptions(&options, argc, argv);

    if (NULL == options.imageFilePath)
    {
        FATAL_ERROR("No input file provided.\n");
    }

    inImage.bitDepth = 8;
    if (NULL != options.tilesetBppFilePath)
    {
        char *inputFileExtension = GetFileExtensionAfterDot(options.tilesetBppFilePath);
        inImage.bitDepth = inputFileExtension[0] - '0';
    }


    inImage.hasTransparency = false;
    inImage.tilemap.data.affine = NULL;
    outImage.bitDepth = inImage.bitDepth;

    ReadPng(options.imageFilePath, &inImage);
    ReadPngPalette(options.imageFilePath, &(inImage.palette));

    DeduplicateTiles(&inImage, &outImage, &outBpp, options.numTiles, options.isAffineMap, 0);

    if (NULL != options.tilesetPngFilePath)
    {
        WritePng(options.tilesetPngFilePath, &outImage);
    }

    if (NULL != options.tilesetBppFilePath)
    {
        WriteWholeFile(options.tilesetBppFilePath,
            outBpp.data,
            outBpp.size * sizeof(unsigned char));
    }

    if (NULL != options.tilemapFilePath)
    {
        if (outImage.isAffine)
        {
            WriteWholeFile(options.tilemapFilePath,
                outImage.tilemap.data.affine,
                outImage.tilemap.size * sizeof(unsigned char));
        }
        else
        {
            WriteWholeFile(options.tilemapFilePath,
                outImage.tilemap.data.non_affine,
                outImage.tilemap.size * sizeof(struct NonAffineTile));
        }
    }

    free(outBpp.data);
    FreeImage(&inImage);
    FreeImage(&outImage);

    return 0;
}

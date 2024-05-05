#include "options.h"

#include <string.h>
#include "../gbagfx/global.h"
#include "../gbagfx/util.h"

void ParseOptions(struct Options *options, int argc, char **argv)
{
    if (argc < 2)
        FATAL_ERROR("Usage: image2tiles INPUT_PATH "
            "[-t tilemap] [[-d bitdepth] -o tilesetBpp] [-O tilesetPng]\n"
            );

    options->imageFilePath = NULL;
    options->tilesetPngFilePath = NULL;
    options->tilesetBppFilePath = NULL;
    options->tilemapFilePath = NULL;
    options->numTiles = 256;
    options->isAffineMap = false;

    for (int i = 1; i < argc; i++)
    {
        char *option = argv[i];

        if (strcmp(option, "-tilemap") == 0)
        {
            if (i + 1 >= argc)
                FATAL_ERROR("No tilemap file following \"-tilemap\".\n");
            i++;
            options->tilemapFilePath = argv[i];
        }
        else if (strcmp(option, "-tileset") == 0)
        {
            if (i + 1 >= argc)
                FATAL_ERROR("No file following \"-tileset\".\n");
            i++;
            char *extension = GetFileExtensionAfterDot(argv[i]);
            if (0 != strcmp("8bpp", extension) && 0 != strcmp("4bpp", extension))
                FATAL_ERROR("Expected extension for for \"-tileset\" file to be either 8bpp or 4bpp.\n");
            options->tilesetBppFilePath = argv[i];
        }
        else if (strcmp(option, "-tileset_png") == 0)
        {
            if (i + 1 >= argc)
                FATAL_ERROR("No file following \"-tileset_png\".\n");
            i++;
            options->tilesetPngFilePath = argv[i];
        }
        else if (strcmp(option, "-num_tiles") == 0)
        {
            if (i + 1 >= argc)
                FATAL_ERROR("No value following \"-num_tiles\".\n");
            i++;

            if (!ParseNumber(argv[i], NULL, 10, &(options->numTiles)))
                FATAL_ERROR("Failed to parse data num_tiles.\n");

            if (options->numTiles < 1)
                FATAL_ERROR("num_tiles width must be positive.\n");
        }
        else if (strcmp(option, "-affine") == 0)
        {
            options->isAffineMap = true;
        }
        else if ('-' == option[0])
        {
            FATAL_ERROR("Unrecognized option \"%s\".\n", option);
        }
        else if (NULL == options->imageFilePath)
        {
            options->imageFilePath = argv[i];
            char *inputFileExtension = GetFileExtensionAfterDot(argv[i]);
            if (0 != strcmp(inputFileExtension, "png"))
                FATAL_ERROR("Expected input to have png extension. Was \"%s\".\n", inputFileExtension);
        }
        else
        {
            FATAL_ERROR("Expected no more than one input file.\n");
        }
    }
}

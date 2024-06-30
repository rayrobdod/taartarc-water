#include "options.h"

#include <limits.h>
#include <string.h>
#include "../gbagfx/global.h"
#include "../gbagfx/util.h"

void ParseOptions(struct Options *options, int argc, char **argv)
{
    if (argc < 2)
        FATAL_ERROR("Usage: image2tiles INPUT_PATH "
            "[-tilemap tilemap] "
            "[-tileset tileset.[48]bpp] "
            "[-tileset_png tileset.png] "
            "[-prefix tileset.png] "
            "[-num_tiles maximum_tile_count] "
            "[-affine] "
            "[-slice_width slice_width_in_tiles] "
            "[-slice_height slice_height_in_tiles] "
            "\n"
            );

    options->imageFilePath = NULL;
    options->tilesetPngFilePath = NULL;
    options->tilesetBppFilePath = NULL;
    options->tilemapFilePath = NULL;
    options->prefixFilePath = NULL;
    options->numTiles = 256;
    options->isAffineMap = false;
    options->slice.width = USHRT_MAX;
    options->slice.height = USHRT_MAX;

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
        else if (strcmp(option, "-prefix") == 0)
        {
            if (i + 1 >= argc)
                FATAL_ERROR("No prefix file following \"-prefix\".\n");
            i++;
            options->prefixFilePath = argv[i];
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
        else if (strcmp(option, "-slice_width") == 0)
        {
            if (i + 1 >= argc)
                FATAL_ERROR("No value following \"-slice_width\".\n");
            i++;

            if (!ParseNumber(argv[i], NULL, 10, &(options->slice.width)))
                FATAL_ERROR("Failed to parse data slice_width.\n");

            if (options->slice.width < 1)
                FATAL_ERROR("slice_width width must be positive.\n");
        }
        else if (strcmp(option, "-slice_height") == 0)
        {
            if (i + 1 >= argc)
                FATAL_ERROR("No value following \"-slice_height\".\n");
            i++;

            if (!ParseNumber(argv[i], NULL, 10, &(options->slice.height)))
                FATAL_ERROR("Failed to parse data slice_height.\n");

            if (options->slice.height < 1)
                FATAL_ERROR("slice_height height must be positive.\n");
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

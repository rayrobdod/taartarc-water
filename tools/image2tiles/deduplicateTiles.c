#include "deduplicateTiles.h"

#include <stdlib.h>
#include "../gbagfx/global.h"

#define BITS_PER_BYTE (8)
#define TILE_DIM (8)
#define TILESET_WIDTH (16)
#define NSWAP(x) ({ (((x) >> 4) & 0xF) | (((x) << 4) & 0xF0); })

void DeduplicateTiles(struct Image *inImage, struct Image *outImage, struct Tileset *outBpp, unsigned maxTilesetSize, bool isAffine, unsigned paletteBase)
{
    outImage->width = TILESET_WIDTH * TILE_DIM;
    outImage->height = ((maxTilesetSize % TILESET_WIDTH == 0 ? 0 : 1) + maxTilesetSize / TILESET_WIDTH) * TILE_DIM;
    outImage->bitDepth = inImage->bitDepth;
    outImage->pixels = calloc(outImage->width * outImage->height * outImage->bitDepth / TILE_DIM, sizeof(char));
    outImage->hasPalette = inImage->hasPalette;
    outImage->palette = inImage->palette;
    outImage->hasTransparency = inImage->hasTransparency;
    outImage->isAffine = isAffine;
    outImage->tilemap.size = (inImage->width * inImage->height) / (TILE_DIM * TILE_DIM);
    if (outImage->isAffine)
    {
        outImage->tilemap.data.affine = calloc(outImage->tilemap.size, sizeof(unsigned char));
    }
    else
    {
        outImage->tilemap.data.non_affine = calloc(outImage->tilemap.size, sizeof(struct NonAffineTile));
    }

    const int inputWidthInTiles = inImage->width / TILE_DIM;
    const int inputWidthInBytes = inImage->width * inImage->bitDepth / BITS_PER_BYTE;
    const int inputHeightInTiles = inImage->height / TILE_DIM;
    const int tileWidthInBytes = inImage->bitDepth * TILE_DIM / BITS_PER_BYTE;
    const int outputWidthInBytes = outImage->width * inImage->bitDepth / BITS_PER_BYTE;
    const int tileSizeInBytes = TILE_DIM * tileWidthInBytes;

    outBpp->size = maxTilesetSize * tileSizeInBytes;
    outBpp->data = calloc(maxTilesetSize, tileSizeInBytes * sizeof(unsigned char));
    unsigned char **tiles = calloc(maxTilesetSize, sizeof(unsigned char *));
    unsigned char *currentTile = calloc(tileSizeInBytes, sizeof(unsigned char));
    for (int i = 0; i < maxTilesetSize; i++)
    {
        tiles[i] = outBpp->data + i * tileSizeInBytes;
    }

    int tilesetSize = 0;
    for (int tile_y = 0; tile_y < inputHeightInTiles; tile_y++)
    for (int tile_x = 0; tile_x < inputWidthInTiles; tile_x++)
    {
        const int inputByteStart = tile_x * tileWidthInBytes + tile_y * inputWidthInBytes * TILE_DIM;
        const int tilemap_index = tile_y * inputWidthInTiles + tile_x;

        for (int pixel_y = 0; pixel_y < TILE_DIM; pixel_y++)
        for (int pixel_x = 0; pixel_x < tileWidthInBytes; pixel_x++)
        {
            switch (inImage->bitDepth)
            {
            case 4:
                currentTile[pixel_x + tileWidthInBytes * pixel_y] = NSWAP(inImage->pixels[inputByteStart + pixel_x + pixel_y * inputWidthInBytes]);
                break;
            case 8:
                currentTile[pixel_x + tileWidthInBytes * pixel_y] = inImage->pixels[inputByteStart + pixel_x + pixel_y * inputWidthInBytes];
                break;
            default:
                FATAL_ERROR("Non-implemented bit depth\n");
                break;
            }
        }

        int tileset_i;
        if (isAffine)
        {
            for (tileset_i = 0; tileset_i < tilesetSize; tileset_i++)
            {
                int byte;
                for (byte = 0; byte < tileSizeInBytes; byte++)
                {
                    if (tiles[tileset_i][byte] != currentTile[byte])
                        break;
                }

                if (byte == tileSizeInBytes)
                    break;
            }

            outImage->tilemap.data.affine[tilemap_index] = (unsigned char) (tileset_i % 256);
        }
        else
        {
            for (tileset_i = 0; tileset_i < tilesetSize; tileset_i++)
            {
                int byte;
                for (byte = 0; byte < tileSizeInBytes; byte++)
                {
                    if (tiles[tileset_i][byte] != currentTile[byte])
                        break;
                }

                if (byte == tileSizeInBytes)
                {
                    outImage->tilemap.data.non_affine[tilemap_index].index = (unsigned char) (tileset_i % 1024);
                    outImage->tilemap.data.non_affine[tilemap_index].hflip = 0;
                    outImage->tilemap.data.non_affine[tilemap_index].vflip = 0;
                    outImage->tilemap.data.non_affine[tilemap_index].palno = paletteBase;
                    break;
                }

                for (byte = 0; byte < tileSizeInBytes; byte++) {
                    int x = byte % tileWidthInBytes;
                    int y = byte / tileWidthInBytes;
                    int outbyte = (TILE_DIM - y - 1) * tileWidthInBytes + x;

                    if (tiles[tileset_i][byte] != currentTile[outbyte])
                        break;
                }

                if (byte == tileSizeInBytes) {
                    outImage->tilemap.data.non_affine[tilemap_index].index = (unsigned char) (tileset_i % 1024);
                    outImage->tilemap.data.non_affine[tilemap_index].hflip = 0;
                    outImage->tilemap.data.non_affine[tilemap_index].vflip = 1;
                    outImage->tilemap.data.non_affine[tilemap_index].palno = paletteBase;
                    break;
                }

                for (byte = 0; byte < tileSizeInBytes; byte++) {
                    int x = byte % tileWidthInBytes;
                    int y = byte / tileWidthInBytes;
                    int outbyte = y * tileWidthInBytes + (tileWidthInBytes - x - 1);

                    if (tiles[tileset_i][byte] != NSWAP(currentTile[outbyte]))
                        break;
                }

                if (byte == tileSizeInBytes) {
                    outImage->tilemap.data.non_affine[tilemap_index].index = (unsigned char) (tileset_i % 1024);
                    outImage->tilemap.data.non_affine[tilemap_index].hflip = 1;
                    outImage->tilemap.data.non_affine[tilemap_index].vflip = 0;
                    outImage->tilemap.data.non_affine[tilemap_index].palno = paletteBase;
                    break;
                }

                for (byte = 0; byte < tileSizeInBytes; byte++) {
                    int x = byte % tileWidthInBytes;
                    int y = byte / tileWidthInBytes;
                    int outbyte = (TILE_DIM - y - 1) * tileWidthInBytes + (tileWidthInBytes - x - 1);

                    if (tiles[tileset_i][byte] != NSWAP(currentTile[outbyte]))
                        break;
                }

                if (byte == tileSizeInBytes) {
                    outImage->tilemap.data.non_affine[tilemap_index].index = (unsigned char) (tileset_i % 1024);
                    outImage->tilemap.data.non_affine[tilemap_index].hflip = 1;
                    outImage->tilemap.data.non_affine[tilemap_index].vflip = 1;
                    outImage->tilemap.data.non_affine[tilemap_index].palno = paletteBase;
                    break;
                }
            }

            if (tileset_i == tilesetSize)
            {
                outImage->tilemap.data.non_affine[tilemap_index].index = (unsigned char) (tileset_i % 1024);
                outImage->tilemap.data.non_affine[tilemap_index].hflip = 0;
                outImage->tilemap.data.non_affine[tilemap_index].vflip = 0;
                outImage->tilemap.data.non_affine[tilemap_index].palno = paletteBase;
            }
        }

        if (tileset_i == tilesetSize)
        {
            if (tileset_i >= maxTilesetSize)
                FATAL_ERROR("Image contains more than %d tiles\n", maxTilesetSize);

            for (int i = 0; i < tileSizeInBytes; i++)
                tiles[tileset_i][i] = currentTile[i];

            const int outputByteStart = (tileset_i % TILESET_WIDTH) * tileWidthInBytes + (tileset_i / TILESET_WIDTH) * tileWidthInBytes * TILESET_WIDTH * TILE_DIM;

            for (int pixel_y = 0; pixel_y < TILE_DIM; pixel_y++)
            for (int pixel_x = 0; pixel_x < tileWidthInBytes; pixel_x++)
            {
                outImage->pixels[outputByteStart + pixel_x + pixel_y * outputWidthInBytes] = inImage->pixels[inputByteStart + pixel_x + pixel_y * inputWidthInBytes];
            }
            tilesetSize++;
        }
    }

    free(currentTile);
    free(tiles);
}

#include <stdio.h>

#define NDEBUG

#define PAL_SIZE (16)
typedef unsigned short int color16_t;

#define max(a, b) ((a) > (b) ? (a) : (b))
#define min(a, b) ((a) < (b) ? (a) : (b))

color16_t convert1(color16_t in)
{
    int /* [0, 31] */ r, g, b;
    int /* [0, 31] */ brightness;
    int /* [0, 31] */ delta;
    int /* [0, 6 * 256] fixpoint 256 hue */ hue;
    int /* [0, 31] */ saturation;
    int /* 0, 6 * 256] fixpoint 256 */ k;
    int /* 0, 256] fixpoint 256 */ y;
    int z;

    r = ((in >> 0) & 0x1F);
    g = ((in >> 5) & 0x1F);
    b = ((in >> 10) & 0x1F);

    brightness = max(r, max(g, b));
    delta = brightness - min(r, min(g, b));
    if (0 == delta)
        hue = 0;
    else if (brightness == r)
        hue = (((256 * (g - b)) / delta) + (6 * 256)) % (6 * 256);
    else if (brightness == g)
        hue = (((256 * (b - r)) / delta) + (2 * 256));
    else // if (brightness == b)
        hue = (((256 * (r - g)) / delta) + (4 * 256));
    saturation = (0 == brightness ? 0 : 31 * delta / brightness);

    //
    saturation /= 2;
    brightness = min(31, brightness + 6);

    // hsb to rgb
    k = ((5 * 256) + hue) % (6 * 256);
    y = max(0, min(256, min(k, 4 * 256 - k)));
    z = brightness * ((31 * 256) - (saturation * y));
    r = z / (256 * 31);

    k = ((3 * 256) + hue) % (6 * 256);
    y = max(0, min(256, min(k, 4 * 256 - k)));
    z = brightness * ((31 * 256) - (saturation * y));
    g = z / (256 * 31);

    k = ((1 * 256) + hue) % (6 * 256);
    y = max(0, min(256, min(k, 4 * 256 - k)));
    z = brightness * ((31 * 256) - (saturation * y));
    b = z / (256 * 31);

    //
    b = min(31, b + 3);

    return ((b & 0x1F) << 10) | ((g & 0x1F) << 5) | (r & 0x1F);
}

int main()
{
    color16_t pal[PAL_SIZE];

    int palsRead = fread(pal, sizeof(color16_t), PAL_SIZE, stdin);

#ifndef NDEBUG
    fprintf(stderr, "in:\n");
    for (int i = 0; i < palsRead; i++)
        fprintf(stderr, "  %d %d %d\n", ((pal[i] >> 0) & 0x1F) << 3, ((pal[i] >> 5) & 0x1F) << 3, ((pal[i] >> 10) & 0x1F) << 3);
#endif

    for (int i = 0; i < palsRead; i++)
        pal[i] = convert1(pal[i]);

#ifndef NDEBUG
    fprintf(stderr, "out:\n");
    for (int i = 0; i < palsRead; i++)
        fprintf(stderr, "  %d %d %d\n", ((pal[i] >> 0) & 0x1F) << 3, ((pal[i] >> 5) & 0x1F) << 3, ((pal[i] >> 10) & 0x1F) << 3);
#endif

    fwrite(pal, sizeof(color16_t), palsRead, stdout);

    return 0;
}

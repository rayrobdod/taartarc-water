#include "global.h"
#include "stamp_card.h"

#include "bg.h"
#include "gpu_regs.h"
#include "link.h"
#include "main.h"
#include "malloc.h"
#include "menu.h"
#include "palette.h"
#include "scanline_effect.h"
#include "sound.h"
#include "task.h"
#include "overworld.h"
#include "constants/rgb.h"
#include "constants/songs.h"

enum MainState {
    MAINSTATE_IDLE_FRONT,
    MAINSTATE_WAIT_FLIP_TO_BACK,
    MAINSTATE_IDLE_BACK,
    MAINSTATE_WAIT_FLIP_TO_FRONT,
    MAINSTATE_WAIT_CLOSE_CARD,
};

struct StampCardData
{
    void (*callbackOnUse)(void);
    void (*callback)(void);
    enum MainState mainState;
    bool32 allowDMACopy;
    u16 cardTiles[0x20 * 16 * 8];
    u16 cardTilemapBuffer[BG_SCREEN_SIZE];
    u16 bgTilemapBuffer[BG_SCREEN_SIZE];
    u16 _2TilemapBuffer[BG_SCREEN_SIZE];
    u16 _3TilemapBuffer[BG_SCREEN_SIZE];
};

static void CB2_InitStampCard(void);
static void ResetGpuRegs(void);
static void InitGpuRegs(void);
static void InitBgsAndWindows(void);

static void CB2_StampCard(void);
static void VBlankCB_StampCard(void);

static void Task_HandleStampCardInput(u8 taskId);

static void BeginCloseStampCard(u8 taskId);
static void Task_FreeDataAndExitStampCard(u8 taskId);

static void HblankCb_TrainerCard(void);
static void FlipTrainerCard();
static bool8 IsCardFlipTaskActive(void);
static void Task_DoCardFlipTask(u8 taskId);
static bool8 Task_BeginCardFlip(struct Task *task);
static bool8 Task_AnimateCardFlipDown(struct Task *task);
static bool8 Task_DrawFlippedCardSide(struct Task *task);
static bool8 Task_PlayCardFlippingSe(struct Task *task);
static bool8 Task_AnimateCardFlipUp(struct Task *task);
static bool8 Task_EndCardFlip(struct Task *task);


EWRAM_DATA static struct StampCardData *sData = NULL;


static const u32 sStampCard_Gfx[] = INCBIN_U32("graphics/stamp_card/_.4bpp.lz");
static const u16 sStampCard_Pal[] = INCBIN_U16("graphics/stamp_card/_.gbapal");

static const u16 sBackground_Tile = INCBIN_U16("graphics/stamp_card/background.tilemap");
static const u32 sFront_Tilemap[] = INCBIN_U32("graphics/stamp_card/front.tilemap.lz");
static const u32 sBack_Tilemap[] = INCBIN_U32("graphics/stamp_card/back.tilemap.lz");

enum {
    BG_CARD,
    BG_2,
    BG_3,
    BG_BACKGROUND,
};

static const struct BgTemplate sStampCardBgTemplates[4] =
{
    {
        .bg = BG_BACKGROUND,
        .charBaseIndex = 0,
        .mapBaseIndex = 28,
        .screenSize = 0,
        .paletteMode = 0,
        .priority = 3,
        .baseTile = 0
    },
    {
        .bg = BG_CARD,
        .charBaseIndex = 0,
        .mapBaseIndex = 29,
        .screenSize = 0,
        .paletteMode = 0,
        .priority = 2,
        .baseTile = 0
    },
    {
        .bg = BG_2,
        .charBaseIndex = 0,
        .mapBaseIndex = 30,
        .screenSize = 0,
        .paletteMode = 0,
        .priority = 1,
        .baseTile = 0
    },
    {
        .bg = BG_3,
        .charBaseIndex = 0,
        .mapBaseIndex = 31,
        .screenSize = 0,
        .paletteMode = 0,
        .priority = 0,
        .baseTile = 0
    },
};

static const struct WindowTemplate sStampCardWindowTemplates[] =
{
    DUMMY_WIN_TEMPLATE
};

static bool8 (*const sTrainerCardFlipTasks[])(struct Task *) =
{
    Task_BeginCardFlip,
    Task_AnimateCardFlipDown,
    Task_DrawFlippedCardSide,
    Task_PlayCardFlippingSe,
    Task_AnimateCardFlipUp,
    Task_EndCardFlip,
};


void ShowStampCard(void (*callback)(void))
{
    sData = AllocZeroed(sizeof(*sData));
    sData->callbackOnUse = NULL;
    sData->callback = callback;

    SetMainCallback2(CB2_InitStampCard);
}

static void CB2_InitStampCard(void)
{
    u8 taskId;

    switch (gMain.state)
    {
    case 0:
        ResetGpuRegs();
        ScanlineEffect_Stop();
        ResetTasks();
        gMain.state++;
        break;
    case 1:
        DmaClear32(3, (void *)OAM, OAM_SIZE);
        gMain.state++;
        break;
    case 2:
        DmaClear16(3, (void *)PLTT, PLTT_SIZE);
        gMain.state++;
        break;
    case 3:
        ResetSpriteData();
        FreeAllSpritePalettes();
        ResetPaletteFade();
        gMain.state++;
    case 4:
        InitBgsAndWindows();
        gMain.state++;
        break;
    case 5:
        SetBgTilemapBuffer(BG_BACKGROUND, sData->bgTilemapBuffer);
        SetBgTilemapBuffer(BG_CARD, sData->cardTilemapBuffer);
        SetBgTilemapBuffer(BG_2, sData->_2TilemapBuffer);
        SetBgTilemapBuffer(BG_3, sData->_3TilemapBuffer);
        gMain.state++;
        break;
    case 6:
        LoadPalette(sStampCard_Pal, BG_PLTT_ID(0), sizeof(sStampCard_Pal));
        gMain.state++;
        break;
    case 7:
        LZ77UnCompWram(sStampCard_Gfx, sData->cardTiles);
        LoadBgTiles(0, sData->cardTiles, ARRAY_COUNT(sData->cardTiles), 0);
        gMain.state++;
        break;
    case 8:
        FillBgTilemapBufferRect_Palette0(
            BG_BACKGROUND,
            sBackground_Tile,
            0,
            0,
            DISPLAY_WIDTH,
            DISPLAY_HEIGHT);
        CopyBgTilemapBufferToVram(BG_BACKGROUND);
        gMain.state++;
        break;
    case 9:
        LZ77UnCompWram(sFront_Tilemap, sData->cardTilemapBuffer);
        CopyBgTilemapBufferToVram(BG_CARD);
        gMain.state++;
        break;
    case 10:
        FillBgTilemapBufferRect_Palette0(
            BG_2,
            0,
            0,
            0,
            DISPLAY_WIDTH,
            DISPLAY_HEIGHT);
        CopyBgTilemapBufferToVram(BG_2);
        CopyBgTilemapBufferToVram(BG_3);
        gMain.state++;
        break;
    case 11:
        gMain.state++;
        break;
    case 12:
        InitGpuRegs();
        gMain.state++;
        break;
    case 13:
        taskId = CreateTask(Task_HandleStampCardInput, 0);
        gMain.state++;
        break;
    case 14:
        BlendPalettes(PALETTES_ALL, 16, RGB_BLACK);
        gMain.state++;
        break;
    case 15:
        BeginNormalPaletteFade(PALETTES_ALL, 0, 16, 0, RGB_BLACK);
        gPaletteFade.bufferTransferDisabled = FALSE;
        gMain.state++;
        break;
    default:
        SetVBlankCallback(VBlankCB_StampCard);
        SetMainCallback2(CB2_StampCard);
        break;
    }
}

static void ResetGpuRegs(void)
{
    SetVBlankCallback(NULL);
    SetHBlankCallback(NULL);
    SetGpuReg(REG_OFFSET_DISPCNT, 0);
    SetGpuReg(REG_OFFSET_BG0CNT, 0);
    SetGpuReg(REG_OFFSET_BG1CNT, 0);
    SetGpuReg(REG_OFFSET_BG2CNT, 0);
    SetGpuReg(REG_OFFSET_BG3CNT, 0);
}

static void InitGpuRegs(void)
{
    SetGpuReg(REG_OFFSET_DISPCNT, DISPCNT_WIN0_ON | DISPCNT_OBJ_ON | DISPCNT_OBJ_1D_MAP);
    ShowBg(0);
    ShowBg(1);
    ShowBg(2);
    ShowBg(3);
    SetGpuReg(REG_OFFSET_BLDCNT, BLDCNT_TGT1_BG0 | BLDCNT_EFFECT_DARKEN);
    SetGpuReg(REG_OFFSET_BLDY, 0);
    SetGpuReg(REG_OFFSET_WININ, WININ_WIN0_BG_ALL | WININ_WIN0_OBJ | WININ_WIN0_CLR);
    SetGpuReg(REG_OFFSET_WINOUT, WINOUT_WIN01_BG1 | WINOUT_WIN01_BG2 | WINOUT_WIN01_BG3 | WINOUT_WIN01_OBJ);
    SetGpuReg(REG_OFFSET_WIN0V, DISPLAY_HEIGHT);
    SetGpuReg(REG_OFFSET_WIN0H, DISPLAY_WIDTH);
    EnableInterrupts(INTR_FLAG_VBLANK | INTR_FLAG_HBLANK);
}

static void InitBgsAndWindows(void)
{
    ResetBgsAndClearDma3BusyFlags(0);
    InitBgsFromTemplates(0, sStampCardBgTemplates, ARRAY_COUNT(sStampCardBgTemplates));
    ChangeBgX(0, 0, BG_COORD_SET);
    ChangeBgY(0, 0, BG_COORD_SET);
    ChangeBgX(1, 0, BG_COORD_SET);
    ChangeBgY(1, 0, BG_COORD_SET);
    ChangeBgX(2, 0, BG_COORD_SET);
    ChangeBgY(2, 0, BG_COORD_SET);
    ChangeBgX(3, 0, BG_COORD_SET);
    ChangeBgY(3, 0, BG_COORD_SET);
    InitWindows(sStampCardWindowTemplates);
    DeactivateAllTextPrinters();
    LoadMessageBoxAndBorderGfx();
}

static void CB2_StampCard(void)
{
    RunTasks();
    AnimateSprites();
    BuildOamBuffer();
    UpdatePaletteFade();
}

static void VBlankCB_StampCard(void)
{
    LoadOam();
    ProcessSpriteCopyRequests();
    TransferPlttBuffer();
    if (sData->allowDMACopy)
        DmaCopy16(3, &gScanlineEffectRegBuffers[0], &gScanlineEffectRegBuffers[1], 0x140);
}


static void Task_HandleStampCardInput(u8 taskId)
{
    switch (sData->mainState) {
    case MAINSTATE_IDLE_FRONT:
        if (JOY_NEW(A_BUTTON))
        {
            FlipTrainerCard();
            PlaySE(SE_RG_CARD_FLIP);
            sData->mainState = MAINSTATE_WAIT_FLIP_TO_BACK;
        }
        else if (JOY_NEW(B_BUTTON))
        {
            BeginCloseStampCard(taskId);
        }
        break;
    case MAINSTATE_WAIT_FLIP_TO_BACK:
        if (IsCardFlipTaskActive() && Overworld_IsRecvQueueAtMax() != TRUE)
        {
            PlaySE(SE_RG_CARD_OPEN);
            sData->mainState = MAINSTATE_IDLE_BACK;
        }
        break;
    case MAINSTATE_IDLE_BACK:
        if (JOY_NEW(A_BUTTON))
        {
            BeginCloseStampCard(taskId);
        }
        else if (JOY_NEW(B_BUTTON))
        {
            FlipTrainerCard();
            PlaySE(SE_RG_CARD_FLIP);
            sData->mainState = MAINSTATE_WAIT_FLIP_TO_FRONT;
        }
        break;
    case MAINSTATE_WAIT_FLIP_TO_FRONT:
        if (IsCardFlipTaskActive() && Overworld_IsRecvQueueAtMax() != TRUE)
        {
            PlaySE(SE_RG_CARD_OPEN);
            sData->mainState = MAINSTATE_IDLE_FRONT;
        }
        break;
    }
}



static void BeginCloseStampCard(u8 taskId)
{
    BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 16, RGB_BLACK);
    gTasks[taskId].func = Task_FreeDataAndExitStampCard;
}

static void Task_FreeDataAndExitStampCard(u8 taskId)
{
    s16 *data = gTasks[taskId].data;

    if (!gPaletteFade.active)
    {
        ResetSpriteData();
        FreeAllSpritePalettes();

        if (sData->callbackOnUse != NULL)
            SetMainCallback2(sData->callbackOnUse);
        else
            SetMainCallback2(sData->callback);

        FreeAllWindowBuffers();
        Free(sData);
        DestroyTask(taskId);
    }
}



static void HblankCb_TrainerCard(void)
{
    u16 backup;
    u16 bgVOffset;

    backup = REG_IME;
    REG_IME = 0;
    bgVOffset = gScanlineEffectRegBuffers[1][REG_VCOUNT & 0xFF];
    REG_BG0VOFS = bgVOffset;
    REG_IME = backup;
}

#define tFlipState data[0]
#define tFlipDrawState data[1]
#define tCardTop data[2]

static void FlipTrainerCard()
{
    u8 taskId = CreateTask(Task_DoCardFlipTask, 0);
    Task_DoCardFlipTask(taskId);
    SetHBlankCallback(HblankCb_TrainerCard);
}

static bool8 IsCardFlipTaskActive(void)
{
    if (FindTaskIdByFunc(Task_DoCardFlipTask) == TASK_NONE)
        return TRUE;
    else
        return FALSE;
}

static void Task_DoCardFlipTask(u8 taskId)
{
    while(sTrainerCardFlipTasks[gTasks[taskId].tFlipState](&gTasks[taskId]))
        ;
}

static bool8 Task_BeginCardFlip(struct Task *task)
{
    u32 i;

    HideBg(BG_2);
    HideBg(BG_3);
    ScanlineEffect_Stop();
    ScanlineEffect_Clear();
    for (i = 0; i < DISPLAY_HEIGHT; i++)
        gScanlineEffectRegBuffers[0][i] = 0;
    task->tFlipState++;
    return FALSE;
}

// Note: Cannot be DISPLAY_HEIGHT / 2, or cardHeight will be 0
#define CARD_FLIP_Y ((DISPLAY_HEIGHT / 2) - 3)


static void UpdateCardFlipRegs(u16 cardTop)
{
    s8 blendY = (cardTop + 40) / 10;

    if (blendY <= 4)
        blendY = 0;
    SetGpuReg(REG_OFFSET_BLDY, blendY);
    SetGpuReg(REG_OFFSET_WIN0V, WIN_RANGE(cardTop, DISPLAY_HEIGHT - cardTop));
}

static void CalculateCardFlipScanlineEffect(struct Task *task)
{
    u32 cardHeight, r5, r10, cardTop, r6, var_24, cardBottom, var;
    s16 i;

    cardTop = task->tCardTop;
    cardBottom = DISPLAY_HEIGHT - cardTop;
    cardHeight = cardBottom - cardTop;
    r6 = -cardTop << 16;
    r5 = (DISPLAY_HEIGHT << 16) / cardHeight;
    r5 -= 1 << 16;
    var_24 = r6;
    var_24 += r5 * cardHeight;
    r10 = r5 / cardHeight;
    r5 *= 2;

    for (i = 0; i < cardTop; i++)
        gScanlineEffectRegBuffers[0][i] = -i;
    for (; i < (s16)cardBottom; i++)
    {
        var = r6 >> 16;
        r6 += r5;
        r5 -= r10;
        gScanlineEffectRegBuffers[0][i] = var;
    }
    var = var_24 >> 16;
    for (; i < DISPLAY_HEIGHT; i++)
        gScanlineEffectRegBuffers[0][i] = var;
}

static bool8 Task_AnimateCardFlipDown(struct Task *task)
{
    sData->allowDMACopy = FALSE;
    if (task->tCardTop >= CARD_FLIP_Y)
        task->tCardTop = CARD_FLIP_Y;
    else
        task->tCardTop += 7;

    UpdateCardFlipRegs(task->tCardTop);
    CalculateCardFlipScanlineEffect(task);

    sData->allowDMACopy = TRUE;
    if (task->tCardTop >= CARD_FLIP_Y)
        task->tFlipState++;

    return FALSE;
}

static bool8 Task_DrawFlippedCardSide(struct Task *task)
{
    const u32* tilemap;

    sData->allowDMACopy = FALSE;
    if (Overworld_IsRecvQueueAtMax() == TRUE)
        return FALSE;

    do
    {
        switch (task->tFlipDrawState)
        {
        case 0:
            if (MAINSTATE_WAIT_FLIP_TO_FRONT == sData->mainState)
                tilemap = sFront_Tilemap;
            else
                tilemap = sBack_Tilemap;
            LZ77UnCompWram(tilemap, sData->cardTilemapBuffer);
            CopyBgTilemapBufferToVram(BG_CARD);
            break;
        default:
            task->tFlipState++;
            sData->allowDMACopy = TRUE;
            task->tFlipDrawState = 0;
            return FALSE;
        }
        task->tFlipDrawState++;
    } while (gReceivedRemoteLinkPlayers == 0);

    return FALSE;
}

static bool8 Task_PlayCardFlippingSe(struct Task *task)
{
    PlaySE(SE_RG_CARD_FLIPPING);
    task->tFlipState++;
    return TRUE;
}

static bool8 Task_AnimateCardFlipUp(struct Task *task)
{
    sData->allowDMACopy = FALSE;
    if (task->tCardTop <= 5)
        task->tCardTop = 0;
    else
        task->tCardTop -= 5;

    UpdateCardFlipRegs(task->tCardTop);
    CalculateCardFlipScanlineEffect(task);

    sData->allowDMACopy = TRUE;
    if (task->tCardTop <= 0)
        task->tFlipState++;

    return FALSE;
}

static bool8 Task_EndCardFlip(struct Task *task)
{
    ShowBg(BG_2);
    ShowBg(BG_3);
    SetHBlankCallback(NULL);
    DestroyTask(FindTaskIdByFunc(Task_DoCardFlipTask));
    return FALSE;
}

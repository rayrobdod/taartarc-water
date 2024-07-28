#include "global.h"
#include "new_game.h"
#include "random.h"
#include "pokemon.h"
#include "roamer.h"
#include "pokemon_size_record.h"
#include "script.h"
#include "lottery_corner.h"
#include "play_time.h"
#include "mauville_old_man.h"
#include "match_call.h"
#include "lilycove_lady.h"
#include "load_save.h"
#include "pokeblock.h"
#include "dewford_trend.h"
#include "berry.h"
#include "rtc.h"
#include "easy_chat.h"
#include "event_data.h"
#include "field_player_avatar.h"
#include "money.h"
#include "trainer_hill.h"
#include "tv.h"
#include "coins.h"
#include "text.h"
#include "overworld.h"
#include "mail.h"
#include "battle_records.h"
#include "item.h"
#include "pokedex.h"
#include "apprentice.h"
#include "frontier_util.h"
#include "pokedex.h"
#include "save.h"
#include "link_rfu.h"
#include "main.h"
#include "contest.h"
#include "item_menu.h"
#include "pokemon_storage_system.h"
#include "pokemon_jump.h"
#include "decoration_inventory.h"
#include "secret_base.h"
#include "player_pc.h"
#include "field_specials.h"
#include "berry_powder.h"
#include "mystery_gift.h"
#include "union_room_chat.h"
#include "constants/items.h"
#include "constants/moves.h"
#include "constants/region_map_sections.h"
#include "strings.h"
#include "string_util.h"

extern const u8 EventScript_ResetAllMapFlags[];

static void ClearFrontierRecord(void);
static void WarpToAquariumEntrance(void);
static void ResetMiniGamesRecords(void);
static void GivePlayerStartingParty(void);
static void GivePlayerStartingItems(void);

EWRAM_DATA bool8 gDifferentSaveFile = FALSE;
EWRAM_DATA bool8 gEnableContestDebugging = FALSE;

static const struct ContestWinner sContestWinnerPicDummy =
{
    .monName = _(""),
    .trainerName = _("")
};

void SetTrainerId(u32 trainerId, u8 *dst)
{
    dst[0] = trainerId;
    dst[1] = trainerId >> 8;
    dst[2] = trainerId >> 16;
    dst[3] = trainerId >> 24;
}

u32 GetTrainerId(u8 *trainerId)
{
    return (trainerId[3] << 24) | (trainerId[2] << 16) | (trainerId[1] << 8) | (trainerId[0]);
}

void CopyTrainerId(u8 *dst, u8 *src)
{
    s32 i;
    for (i = 0; i < TRAINER_ID_LENGTH; i++)
        dst[i] = src[i];
}

static void InitPlayerTrainerId(void)
{
    u32 trainerId = (Random() << 16) | Random();
    SetTrainerId(trainerId, gSaveBlock2Ptr->playerTrainerId);
}

// L=A isnt set here for some reason.
static void SetDefaultOptions(void)
{
    gSaveBlock2Ptr->optionsTextSpeed = OPTIONS_TEXT_SPEED_MID;
    gSaveBlock2Ptr->optionsWindowFrameType = 0;
    gSaveBlock2Ptr->optionsSound = OPTIONS_SOUND_MONO;
    gSaveBlock2Ptr->optionsBattleStyle = OPTIONS_BATTLE_STYLE_SHIFT;
    gSaveBlock2Ptr->optionsBattleSceneOff = FALSE;
    gSaveBlock2Ptr->regionMapZoom = FALSE;
}

static void ClearPokedexFlags(void)
{
    gUnusedPokedexU8 = 0;
    memset(&gSaveBlock2Ptr->pokedex.owned, 0, sizeof(gSaveBlock2Ptr->pokedex.owned));
    memset(&gSaveBlock2Ptr->pokedex.seen, 0, sizeof(gSaveBlock2Ptr->pokedex.seen));
}

void ClearAllContestWinnerPics(void)
{
    s32 i;

    ClearContestWinnerPicsInContestHall();

    // Clear Museum paintings
    for (i = MUSEUM_CONTEST_WINNERS_START; i < NUM_CONTEST_WINNERS; i++)
        gSaveBlock1Ptr->contestWinners[i] = sContestWinnerPicDummy;
}

static void ClearFrontierRecord(void)
{
    CpuFill32(0, &gSaveBlock2Ptr->frontier, sizeof(gSaveBlock2Ptr->frontier));

    gSaveBlock2Ptr->frontier.opponentNames[0][0] = EOS;
    gSaveBlock2Ptr->frontier.opponentNames[1][0] = EOS;
}

PADDING_text(15)

__attribute__((section("newGameInitData"))) static void WarpToAquariumEntrance(void)
{
    SetWarpDestination(MAP_GROUP(AQUARIUM_GARDENS), MAP_NUM(AQUARIUM_GARDENS), WARP_ID_NONE, 8, 14);
    WarpIntoMap();
}

void Sav2_ClearSetDefault(void)
{
    ClearSav2();
    SetDefaultOptions();
}

void ResetMenuAndMonGlobals(void)
{
    gDifferentSaveFile = FALSE;
    ResetPokedexScrollPositions();
    ZeroPlayerPartyMons();
    ZeroEnemyPartyMons();
    ResetBagScrollPositions();
    ResetPokeblockScrollPositions();
}

static void GivePlayerStartingItems(void)
{
    // When this is called, the bag should be empty,
    // so this assumes that AddBagItem succeeds

    AddBagItem(ITEM_HYPER_POTION, 10);
    AddBagItem(ITEM_REVIVE, 5);
    AddBagItem(ITEM_FULL_HEAL, 5);
    AddBagItem(ITEM_FULL_RESTORE, 2);

    AddBagItem(ITEM_DRAGON_FANG, 1);
    AddBagItem(ITEM_SILK_SCARF, 1);
    //AddBagItem(ITEM_EXPERT_BELT, 1);
    //AddBagItem(ITEM_WIDE_LENS, 1);
    AddBagItem(ITEM_DRAGON_SCALE, 1);

    AddBagItem(ITEM_SITRUS_BERRY, 10);
    AddBagItem(ITEM_PERSIM_BERRY, 2);
    AddBagItem(ITEM_LUM_BERRY, 10);
    //AddBagItem(ITEM_YACHE_BERRY, 5);
    //AddBagItem(ITEM_WACAN_BERRY, 2);
    //AddBagItem(ITEM_CHARTI_BERRY, 2);
    //AddBagItem(ITEM_HABAN_BERRY, 2);
    //AddBagItem(ITEM_ROSELI_BERRY, 2);

    AddBagItem(ITEM_TM_HYPER_BEAM, 30);
    AddBagItem(ITEM_TM_DRAGON_CLAW, 2);
    AddBagItem(ITEM_TM_THUNDERBOLT, 1);
    AddBagItem(ITEM_TM_FLAMETHROWER, 1);
    AddBagItem(ITEM_TM_STEEL_WING, 1);
}

PADDING(".text", 156)

__attribute__((section("newGameInitData")))
void NewGameInitData(void)
{
    if (gSaveFileStatus == SAVE_STATUS_EMPTY || gSaveFileStatus == SAVE_STATUS_CORRUPT)
        RtcReset();

    gSaveBlock2Ptr->playerGender = MALE;
    StringCopy(gSaveBlock2Ptr->playerName, gText_DefaultNameLance);

    gDifferentSaveFile = TRUE;
    gSaveBlock2Ptr->encryptionKey = 0;
    ZeroPlayerPartyMons();
    ZeroEnemyPartyMons();
    ResetPokedex();
    ClearFrontierRecord();
    ClearSav1();
    ClearAllMail();
    gSaveBlock2Ptr->specialSaveWarpFlags = 0;
    gSaveBlock2Ptr->gcnLinkFlags = 0;
    InitPlayerTrainerId();
    PlayTimeCounter_Reset();
    ClearPokedexFlags();
    InitEventData();
    ClearTVShowData();
    ResetGabbyAndTy();
    ClearSecretBases();
    ClearBerryTrees();
    SetMoney(&gSaveBlock1Ptr->money, 3000);
    SetCoins(0);
    ResetLinkContestBoolean();
    ResetGameStats();
    ClearAllContestWinnerPics();
    ClearPlayerLinkBattleRecords();
    InitSeedotSizeRecord();
    InitLotadSizeRecord();
    gPlayerPartyCount = 0;
    ZeroPlayerPartyMons();
    ResetPokemonStorageSystem();
    ClearRoamerData();
    ClearRoamerLocationData();
    gSaveBlock1Ptr->registeredItem = ITEM_NONE;
    ClearBag();
    NewGameInitPCItems();
    ClearPokeblocks();
    ClearDecorationInventories();
    InitEasyChatPhrases();
    SetMauvilleOldMan();
    InitDewfordTrend();
    ResetFanClub();
    ResetLotteryCorner();
    WarpToAquariumEntrance();
    RunScriptImmediately(EventScript_ResetAllMapFlags);
    ResetMiniGamesRecords();
    InitUnionRoomChatRegisteredTexts();
    InitLilycoveLady();
    ResetAllApprenticeData();
    ClearRankingHallRecords();
    InitMatchCallCounters();
    ClearMysteryGift();
    WipeTrainerNameRecords();
    ResetTrainerHillResults();
    ResetContestLinkResults();
    GivePlayerStartingParty();
    GivePlayerStartingItems();
    EnableNationalPokedex();
}

static void ResetMiniGamesRecords(void)
{
    CpuFill16(0, &gSaveBlock2Ptr->berryCrush, sizeof(struct BerryCrush));
    SetBerryPowder(&gSaveBlock2Ptr->berryCrush.berryPowderAmount, 0);
    ResetPokemonJumpRecords();
    CpuFill16(0, &gSaveBlock2Ptr->berryPick, sizeof(struct BerryPickingResults));
}

__attribute__((section("added")))
static void GivePlayerStartingMon(
    int slot,
    u16 species,
    u8 level,
    u8 metLocation,
    u8 metLevel,
    u16 item,
    u16 move1,
    u16 move2,
    u16 move3,
    u16 move4
) {
    struct Pokemon *mon = &gPlayerParty[slot];
    CreateMon(mon, species, level, USE_RANDOM_IVS, FALSE, 0, OT_ID_PLAYER_ID, 0);
    SetMonData(mon, MON_DATA_MET_LOCATION, &metLocation);
    SetMonData(mon, MON_DATA_MET_LEVEL, &metLevel);
    SetMonData(mon, MON_DATA_HELD_ITEM, &item);
    SetMonMoveSlot(mon, move1, 0);
    SetMonMoveSlot(mon, move2, 1);
    SetMonMoveSlot(mon, move3, 2);
    SetMonMoveSlot(mon, move4, 3);
}

__attribute__((section("added")))
static void GivePlayerStartingParty(void)
{
    GivePlayerStartingMon(
        0,
        SPECIES_DRAGONITE,
        50,
        MAPSEC_DRAGONS_DEN,
        5,
        ITEM_SITRUS_BERRY, // ITEM_YACHE_BERRY,
        MOVE_OUTRAGE,
        MOVE_DRAGON_DANCE,
        MOVE_BARRIER,
        MOVE_THUNDER_PUNCH
    );

    GivePlayerStartingMon(
        1,
        SPECIES_AERODACTYL,
        48,
        MAPSEC_CINNABAR_ISLAND,
        20,
        ITEM_SITRUS_BERRY,
        MOVE_ROCK_SLIDE,
        MOVE_EARTHQUAKE,
        MOVE_STEEL_WING, // MOVE_IRON_HEAD,
        MOVE_HYPER_BEAM // MOVE_GIGA_IMPACT,
    );

    GivePlayerStartingMon(
        2,
        SPECIES_CHARIZARD,
        48,
        MAPSEC_CHARICIFIC_VALLEY,
        24,
        ITEM_SITRUS_BERRY, // ITEM_CHARTI_BERRY,
        MOVE_FIRE_BLAST,
        MOVE_WING_ATTACK, // MOVE_AIR_SLASH,
        MOVE_DRAGON_CLAW,
        MOVE_HYPER_BEAM
    );

    GivePlayerStartingMon(
        3,
        SPECIES_GYARADOS,
        48,
        MAPSEC_LAKE_OF_RAGE,
        8,
        ITEM_SITRUS_BERRY, // ITEM_WACAN_BERRY,
        MOVE_WATERFALL, // MOVE_AQUA_TAIL,
        MOVE_CRUNCH,
        MOVE_DRAGON_DANCE,
        MOVE_THRASH
    );
}

#include "global.h"
#include "constants/abilities.h"
#include "constants/flags.h"
#include "constants/items.h"
#include "constants/moves.h"
#include "constants/rgb.h"
#include "constants/region_map_sections.h"
#include "event_data.h"
#include "random.h"

#include "rogue_baked.h"
#include "rogue_colour_utils.h"
#include "rogue_gifts.h"
#include "rogue_hub.h"
#include "rogue_pokedex.h"
#include "rogue_query.h"
#include "rogue_quest.h"
#include "rogue_save.h"

struct CustomTrainerData
{
    u8 const* name;
    u32 trainerId;
    u8 trainerColour;
};

struct CustomMonData
{
    u8 const* nickname;
    u16 const* moves;
    u16 const* abilities;
    u16 types[2];
    u32 otId;
    u16 movesCount;
    u16 customTrainerId;
    u16 species;
    u16 pokeball;
    u16 heldItem;
    u16 isShiny : 1;
    u16 isDefaultSpawn : 1;
};

static u8 const sRarityToCustomTrainerIndex[] = 
{
    [UNIQUE_RARITY_COMMON]      = CUSTOM_TRAINER_COMMON,
    [UNIQUE_RARITY_RARE]        = CUSTOM_TRAINER_RARE,
    [UNIQUE_RARITY_EPIC]        = CUSTOM_TRAINER_EPIC,
    [UNIQUE_RARITY_EXOTIC]      = CUSTOM_TRAINER_EXOTIC,
};

static u16 const sDynamicCustomMonAbilities[] = 
{
#ifdef ROGUE_EXPANSION
    ABILITY_GOOD_AS_GOLD,
    ABILITY_WATER_ABSORB,
    ABILITY_VOLT_ABSORB,
    ABILITY_FLASH_FIRE,
    ABILITY_SPEED_BOOST,
    ABILITY_ADAPTABILITY,
    ABILITY_INTIMIDATE,
    ABILITY_DRIZZLE,
    ABILITY_DROUGHT,
    ABILITY_SNOW_WARNING,
    ABILITY_SAND_STREAM,
    ABILITY_QUICK_DRAW,
    ABILITY_NEUTRALIZING_GAS,
    ABILITY_ELECTRIC_SURGE,
    ABILITY_PSYCHIC_SURGE,
    ABILITY_MISTY_SURGE,
    ABILITY_GRASSY_SURGE,
    ABILITY_PROTEAN,
    ABILITY_MAGIC_BOUNCE,
    ABILITY_MOXIE,
    ABILITY_ILLUSION,
    ABILITY_REGENERATOR,
    ABILITY_SUPER_LUCK,
    ABILITY_CONTRARY,
    ABILITY_NO_GUARD,
    ABILITY_SKILL_LINK,
    ABILITY_SHELL_ARMOR,
    ABILITY_GUTS,
    ABILITY_THICK_FAT,
    ABILITY_HUGE_POWER,
    ABILITY_SERENE_GRACE,
    ABILITY_BATTLE_ARMOR,
    ABILITY_WEAK_ARMOR,
    ABILITY_TOXIC_DEBRIS,
    ABILITY_UNSEEN_FIST,
    ABILITY_GORILLA_TACTICS,
    ABILITY_TOUGH_CLAWS,
    ABILITY_FULL_METAL_BODY,
    ABILITY_TECHNICIAN,
    ABILITY_EMERGENCY_EXIT,
    ABILITY_PRANKSTER,
    ABILITY_POWER_OF_ALCHEMY,
    ABILITY_OPPORTUNIST,
    ABILITY_LEVITATE,
    ABILITY_MULTISCALE,
    ABILITY_COSTAR,
    ABILITY_POISON_HEAL,
    ABILITY_NEUROFORCE,
    ABILITY_MOODY,
    ABILITY_HOSPITALITY,
    ABILITY_COMATOSE,
    ABILITY_BERSERK,
    ABILITY_SIMPLE,
    ABILITY_UNAWARE,
    ABILITY_HARVEST,
    ABILITY_MOLD_BREAKER,
    ABILITY_SHEER_FORCE,
    ABILITY_OVERCOAT,
    ABILITY_IRON_BARBS,
    ABILITY_DEFIANT,
    ABILITY_STALL,
    ABILITY_SNIPER,
    ABILITY_MULTITYPE
#else
    ABILITY_DRIZZLE,
    ABILITY_SPEED_BOOST,
    ABILITY_BATTLE_ARMOR,
    ABILITY_LIMBER,
    ABILITY_SAND_VEIL,
    ABILITY_STATIC,
    ABILITY_VOLT_ABSORB,
    ABILITY_WATER_ABSORB,
    ABILITY_CLOUD_NINE,
    ABILITY_COMPOUND_EYES,
    ABILITY_COLOR_CHANGE,
    ABILITY_FLASH_FIRE,
    ABILITY_SHIELD_DUST,
    ABILITY_SUCTION_CUPS,
    ABILITY_INTIMIDATE,
    ABILITY_SHADOW_TAG,
    ABILITY_ROUGH_SKIN,
    ABILITY_LEVITATE,
    ABILITY_EFFECT_SPORE,
    ABILITY_CLEAR_BODY,
    ABILITY_SERENE_GRACE,
    ABILITY_SWIFT_SWIM,
    ABILITY_CHLOROPHYLL,
    ABILITY_HUGE_POWER,
    ABILITY_INNER_FOCUS,
    ABILITY_SOUNDPROOF,
    ABILITY_RAIN_DISH,
    ABILITY_SAND_STREAM,
    ABILITY_PRESSURE,
    ABILITY_THICK_FAT,
    ABILITY_FLAME_BODY,
    ABILITY_RUN_AWAY,
    ABILITY_PICKUP,
    ABILITY_HUSTLE,
    ABILITY_CUTE_CHARM,
    ABILITY_SHED_SKIN,
    ABILITY_GUTS,
    ABILITY_MARVEL_SCALE,
    ABILITY_ROCK_HEAD,
    ABILITY_DROUGHT,
    ABILITY_WHITE_SMOKE,
    ABILITY_PURE_POWER,
    ABILITY_SHELL_ARMOR,
    ABILITY_AIR_LOCK,
#endif
};

static u16 const sDynamicCustomMonMoves[] = 
{
#ifdef ROGUE_EXPANSION
    MOVE_HYDRO_STEAM,
    MOVE_CHILLING_WATER,
    MOVE_TORCH_SONG,
    MOVE_JET_PUNCH,
    MOVE_STONE_AXE,
    MOVE_THUNDEROUS_KICK,
    MOVE_SCORCHING_SANDS,
    MOVE_SURGING_STRIKES,
    MOVE_PARTING_SHOT,
    MOVE_GRASSY_GLIDE,
    MOVE_DYNAMAX_CANNON,
    MOVE_STOMPING_TANTRUM,
    MOVE_BANEFUL_BUNKER,
    MOVE_DIAMOND_STORM,
    MOVE_PLAY_ROUGH,
    MOVE_FREEZE_DRY,
    MOVE_PHANTOM_FORCE,
    MOVE_V_CREATE,
    MOVE_SECRET_SWORD,
    MOVE_TRICK_ROOM,
    MOVE_SPORE,
    MOVE_GUNK_SHOT,
    MOVE_NASTY_PLOT,
    MOVE_CLOSE_COMBAT,
    MOVE_U_TURN,
    MOVE_DRAGON_DANCE,
    MOVE_CALM_MIND,
    MOVE_LEAF_BLADE,
    MOVE_METEOR_MASH,
    MOVE_KNOCK_OFF,
    MOVE_SKILL_SWAP,
    MOVE_FAKE_OUT,
    MOVE_EXTREME_SPEED,
    MOVE_ANCIENT_POWER,
    MOVE_RAPID_SPIN,
    MOVE_MEGAHORN,
    MOVE_WISH,
    MOVE_THUNDER,
    MOVE_SWORDS_DANCE,
    MOVE_THUNDERCLAP,
    MOVE_GIGATON_HAMMER,
    MOVE_SALT_CURE,
    MOVE_WICKED_BLOW,
    MOVE_POLTERGEIST,
    MOVE_METEOR_BEAM,
    MOVE_SPIRIT_BREAK,
    MOVE_RECOVER,
    MOVE_NO_RETREAT,
    MOVE_SUBSTITUTE,
    MOVE_METRONOME,
    MOVE_TOPSY_TURVY,
    MOVE_QUIVER_DANCE,
    MOVE_BATON_PASS,
    MOVE_MOONGEIST_BEAM,
    MOVE_MAGMA_STORM,
    MOVE_RUINATION,
    MOVE_REVELATION_DANCE,
    MOVE_SUCKER_PUNCH,
    MOVE_COLLISION_COURSE,
    MOVE_ROLLOUT,
    MOVE_FURY_CUTTER,
    MOVE_LAST_RESPECTS,
    MOVE_ACROBATICS,
#else
    MOVE_PAY_DAY,
    MOVE_FIRE_PUNCH,
    MOVE_ICE_PUNCH,
    MOVE_THUNDER_PUNCH,
    MOVE_SWORDS_DANCE,
    MOVE_FLY,
    MOVE_BODY_SLAM,
    MOVE_SURF,
    MOVE_ICE_BEAM,
    MOVE_COUNTER,
    MOVE_DRAGON_RAGE,
    MOVE_EARTHQUAKE,
    MOVE_PSYCHIC,
    MOVE_METRONOME,
    MOVE_MIRROR_MOVE,
    MOVE_FIRE_BLAST,
    MOVE_SOFT_BOILED,
    MOVE_TRANSFORM,
    MOVE_SUBSTITUTE,
    MOVE_CURSE,
    MOVE_AEROBLAST,
    MOVE_MACH_PUNCH,
    MOVE_BELLY_DRUM,
    MOVE_ZAP_CANNON,
    MOVE_OUTRAGE,
    MOVE_GIGA_DRAIN,
    MOVE_ROLLOUT,
    MOVE_SACRED_FIRE,
    MOVE_MEGAHORN,
    MOVE_SYNTHESIS,
    MOVE_CRUNCH,
    MOVE_EXTREME_SPEED,
    MOVE_ANCIENT_POWER,
    MOVE_SHADOW_BALL,
    MOVE_FUTURE_SIGHT,
    MOVE_FAKE_OUT,
    MOVE_UPROAR,
    MOVE_WILL_O_WISP,
    MOVE_WISH,
    MOVE_ASSIST,
    MOVE_SUPERPOWER,
    MOVE_MAGIC_COAT,
    MOVE_BRICK_BREAK,
    MOVE_KNOCK_OFF,
    MOVE_SKILL_SWAP,
    MOVE_LUSTER_PURGE,
    MOVE_HYPER_VOICE,
    MOVE_BLAST_BURN,
    MOVE_HYDRO_CANNON,
    MOVE_OVERHEAT,
    MOVE_SIGNAL_BEAM,
    MOVE_AERIAL_ACE,
    MOVE_ICICLE_SPEAR,
    MOVE_FRENZY_PLANT,
    MOVE_BULK_UP,
    MOVE_VOLT_TACKLE,
    MOVE_CALM_MIND,
    MOVE_DRAGON_DANCE,
    MOVE_PSYCHO_BOOST,
    MOVE_ROCK_BLAST,
    MOVE_ERUPTION,
    MOVE_RECYCLE,
    MOVE_FACADE,
#endif
};

STATIC_ASSERT(ARRAY_COUNT(sDynamicCustomMonAbilities) <= 63, SizeOfDynamicCustomMonAbilities);
STATIC_ASSERT(ARRAY_COUNT(sDynamicCustomMonMoves) <= 63, SizeOfDynamicCustomMonMoves);

const u16 sTypeTintColors[NUMBER_OF_MON_TYPES] = 
{
//    [TYPE_NORMAL] = RGB_WHITE,
//    [TYPE_FIGHTING] = RGB(26, 8, 14),
//    [TYPE_FLYING] = RGB(31, 26, 7),
//    [TYPE_POISON] = RGB(26, 10, 25),
//    [TYPE_GROUND] = RGB(25, 23, 18),
//    [TYPE_ROCK] = RGB(18, 16, 8),
//    [TYPE_BUG] = RGB(18, 24, 6),
//    [TYPE_GHOST] = RGB(12, 10, 16),
//    [TYPE_STEEL] = RGB(19, 19, 20),
//    [TYPE_MYSTERY] = RGB_WHITE,
//    [TYPE_FIRE] = RGB(31, 20, 11),
//    [TYPE_WATER] = RGB(10, 18, 27),
//    [TYPE_GRASS] = RGB(12, 24, 11),
//    [TYPE_ELECTRIC] = RGB(30, 26, 7),
//    [TYPE_PSYCHIC] = RGB(31, 14, 15),
//    [TYPE_ICE] = RGB(14, 26, 25),
//    [TYPE_DRAGON] = RGB(10, 18, 27),
//    [TYPE_DARK] = RGB(6, 5, 8),
//#ifdef ROGUE_EXPANSION
//    [TYPE_FAIRY] = RGB(31, 15, 21),
//    [TYPE_STELLAR] = RGB(10, 18, 27),
//#endif


    [TYPE_NORMAL] = RGB_RANGE_255_TO_31(255, 236, 206),
    [TYPE_FIGHTING] = RGB_RANGE_255_TO_31(179, 39, 27),
    [TYPE_FLYING] = RGB_RANGE_255_TO_31(154, 217, 232),
    [TYPE_POISON] = RGB_RANGE_255_TO_31(254, 20, 255),
    [TYPE_GROUND] = RGB_RANGE_255_TO_31(142, 102, 59),
    [TYPE_ROCK] = RGB_RANGE_255_TO_31(202, 162, 86),
    [TYPE_BUG] = RGB_RANGE_255_TO_31(188, 202, 121),
    [TYPE_GHOST] = RGB_WHITE, //RGB_RANGE_255_TO_31(106, 46, 142),
    [TYPE_STEEL] = RGB_RANGE_255_TO_31(142, 142, 142),
    [TYPE_MYSTERY] = RGB_WHITE,
    [TYPE_FIRE] = RGB_RANGE_255_TO_31(255, 170, 71),
    [TYPE_WATER] = RGB_RANGE_255_TO_31(37, 178, 255),
    [TYPE_GRASS] = RGB_RANGE_255_TO_31(33, 255, 32),
    [TYPE_ELECTRIC] = RGB_RANGE_255_TO_31(255, 227, 20),
    [TYPE_PSYCHIC] = RGB_RANGE_255_TO_31(254, 80, 255),
    [TYPE_ICE] = RGB_RANGE_255_TO_31(163, 255, 253),
    [TYPE_DRAGON] = RGB_RANGE_255_TO_31(43, 40, 255),
    [TYPE_DARK] = RGB_RANGE_255_TO_31(91, 91, 91),
#ifdef ROGUE_EXPANSION
    [TYPE_FAIRY] = RGB_RANGE_255_TO_31(255, 192, 234),
    [TYPE_STELLAR] = RGB(10, 18, 27),
#endif
};

#include "data/rogue/custom_mons.h"

enum
{
    COMPRESSED_FORMAT_ORIGINAL = 0,
    COMPRESSED_FORMAT_MON_TYPE,
    COMPRESSED_FORMAT_UNUSED2,
    COMPRESSED_FORMAT_UNUSED3,
};

struct CompressedDynamicData
{
    u32 data1:21;
    u32 format:2; // COMPRESSED_FORMAT_[...]
    u32 data2:7;
    u32 reserved:2; // reserved for bitmask OTID_FLAG_CUSTOM_MON etc.
};
struct CompressedDynamicData_Original
{
    u32 move1:7; // 127 indices
    u32 move2:7; // 127 indices
    u32 move3:7; // 127 indices
    u32 format:2;
    u32 ability:7; // 127 indices
    u32 reserved:2;
};

struct CompressedDynamicData_MonType
{
    u32 type: 5;
    u32 typeSlot:1;
    u32 typeMoveFlip:1;
    u32 move1:7; // 127 indices
    u32 move2:7; // 127 indices
    u32 format:2;
    u32 ability:7; // 127 indices
    u32 reserved:2;
};


STATIC_ASSERT(sizeof(struct CompressedDynamicData) == sizeof(struct CompressedDynamicData_Original), SizeOfCompressedDynamicData_Original);
STATIC_ASSERT(sizeof(struct CompressedDynamicData) == sizeof(struct CompressedDynamicData_MonType), SizeOfCompressedDynamicData_MonType);

struct DynamicMonData
{
    u16 moves[MAX_MON_MOVES];
    u16 movesCount;
    u8 types[2];
    u16 ability;
};

STATIC_ASSERT(sizeof(struct CompressedDynamicData) == sizeof(u32), SizeOfDynamicCustomMonData);

static u16 SelectTypeBasedExtraMove(u8 type, u8 rng)
{
#ifdef ROGUE_EXPANSION
    switch (type)
    {        
    case TYPE_NORMAL:
        return rng ? MOVE_BOOMBURST : MOVE_EXTREME_SPEED;
    case TYPE_FIGHTING:
        return rng ? MOVE_FOCUS_BLAST : MOVE_CLOSE_COMBAT;
    case TYPE_FLYING:
        return rng ? MOVE_HURRICANE : MOVE_BRAVE_BIRD;
    case TYPE_POISON:
        return rng ? MOVE_SLUDGE_BOMB : MOVE_GUNK_SHOT;
    case TYPE_GROUND:
        return rng ? MOVE_EARTHQUAKE : MOVE_EARTH_POWER;
    case TYPE_ROCK:
        return rng ? MOVE_ROCK_SLIDE : MOVE_ANCIENT_POWER;
    case TYPE_BUG:
        return rng ? MOVE_LUNGE : MOVE_BUG_BUZZ;
    case TYPE_GHOST:
        return rng ? MOVE_SHADOW_BALL : MOVE_SHADOW_SNEAK;
    case TYPE_STEEL:
        return rng ? MOVE_IRON_HEAD : MOVE_MAGNET_BOMB;
    case TYPE_FIRE:
        return rng ? MOVE_OVERHEAT : MOVE_SACRED_FIRE;
    case TYPE_WATER:
        return rng ? MOVE_LIQUIDATION : MOVE_SURF;
    case TYPE_GRASS:
        return rng ? MOVE_GIGA_DRAIN : MOVE_LEAF_BLADE;
    case TYPE_ELECTRIC:
        return rng ? MOVE_THUNDERCLAP : MOVE_THUNDER;
    case TYPE_PSYCHIC:
        return rng ? MOVE_PSYCHIC : MOVE_PSYSHOCK;
    case TYPE_ICE:
        return rng ? MOVE_ICE_BEAM : MOVE_TRIPLE_AXEL;
    case TYPE_DRAGON:
        return rng ? MOVE_DRACO_METEOR : MOVE_DRAGON_RUSH;
    case TYPE_DARK:
        return rng ? MOVE_DARK_PULSE : MOVE_NIGHT_SLASH;
    case TYPE_FAIRY:
        return rng ? MOVE_MOONBLAST : MOVE_PLAY_ROUGH;
    }
#else

    switch (type)
    {        
    case TYPE_NORMAL:
        return rng ? MOVE_QUICK_ATTACK : MOVE_BODY_SLAM;
    case TYPE_FIGHTING:
        return rng ? MOVE_MACH_PUNCH : MOVE_FOCUS_PUNCH;
    case TYPE_FLYING:
        return rng ? MOVE_FLY : MOVE_AEROBLAST;
    case TYPE_POISON:
        return rng ? MOVE_TOXIC : MOVE_SLUDGE_BOMB;
    case TYPE_GROUND:
        return rng ? MOVE_EARTHQUAKE : MOVE_DIG;
    case TYPE_ROCK:
        return rng ? MOVE_ROCK_SLIDE : MOVE_SANDSTORM;
    case TYPE_BUG:
        return rng ? MOVE_BUG_BUZZ : MOVE_SIGNAL_BEAM;
    case TYPE_GHOST:
        return rng ? MOVE_SHADOW_BALL : MOVE_SHADOW_PUNCH;
    case TYPE_STEEL:
        return rng ? MOVE_METEOR_MASH : MOVE_IRON_DEFENSE;
    case TYPE_FIRE:
        return rng ? MOVE_FLAMETHROWER : MOVE_WILL_O_WISP;
    case TYPE_WATER:
        return rng ? MOVE_SURF : MOVE_WATER_PULSE;
    case TYPE_GRASS:
        return rng ? MOVE_TACKLE : MOVE_TACKLE;
    case TYPE_ELECTRIC:
        return rng ? MOVE_THUNDER : MOVE_VOLT_TACKLE;
    case TYPE_PSYCHIC:
        return rng ? MOVE_PSYCHIC : MOVE_CALM_MIND;
    case TYPE_ICE:
        return rng ? MOVE_ICE_BEAM : MOVE_BLIZZARD;
    case TYPE_DRAGON:
        return rng ? MOVE_DRAGON_BREATH : MOVE_DRAGON_CLAW;
    case TYPE_DARK:
        return rng ? MOVE_CRUNCH : MOVE_BITE;
    }
#endif

    return MOVE_RETURN;
}

static void UncompressDynamicMonData(u32 customMonId, struct DynamicMonData* outData)
{
    struct CompressedDynamicData* compressedUntyped = (struct CompressedDynamicData*)&customMonId;

    outData->ability = ABILITY_NONE;
    outData->movesCount = 0;
    outData->types[0] = TYPE_NONE;
    outData->types[1] = TYPE_NONE;

    if(compressedUntyped->format == COMPRESSED_FORMAT_ORIGINAL)
    {
        struct CompressedDynamicData_Original* compressedData = (struct CompressedDynamicData_Original*)compressedUntyped;

        outData->ability = ((compressedData->ability - 1) < ARRAY_COUNT(sDynamicCustomMonAbilities)) ? sDynamicCustomMonAbilities[compressedData->ability - 1] : ABILITY_NONE;

        if(compressedData->move1 != 0 && (compressedData->move1 - 1) < ARRAY_COUNT(sDynamicCustomMonMoves))
            outData->moves[outData->movesCount++] = sDynamicCustomMonMoves[compressedData->move1 - 1];

        if(compressedData->move2 != 0 && (compressedData->move2 - 1) < ARRAY_COUNT(sDynamicCustomMonMoves))
            outData->moves[outData->movesCount++] = sDynamicCustomMonMoves[compressedData->move2 - 1];

        if(compressedData->move3 != 0 && (compressedData->move3 - 1) < ARRAY_COUNT(sDynamicCustomMonMoves))
            outData->moves[outData->movesCount++] = sDynamicCustomMonMoves[compressedData->move3 - 1];

        //if(compressedData->move4 != 0 && (compressedData->move4 - 1) < ARRAY_COUNT(sDynamicCustomMonMoves))
        //    outData->moves[outData->movesCount++] = sDynamicCustomMonMoves[compressedData->move4 - 1];
    }
    else if(compressedUntyped->format == COMPRESSED_FORMAT_MON_TYPE)
    {
        struct CompressedDynamicData_MonType* compressedData = (struct CompressedDynamicData_MonType*)compressedUntyped;

        outData->ability = ((compressedData->ability - 1) < ARRAY_COUNT(sDynamicCustomMonAbilities)) ? sDynamicCustomMonAbilities[compressedData->ability - 1] : ABILITY_NONE;

        outData->types[compressedData->typeSlot] = compressedData->type;
        
        outData->moves[outData->movesCount++] = SelectTypeBasedExtraMove(compressedData->type, compressedData->typeMoveFlip);

        if(compressedData->move1 != 0 && (compressedData->move1 - 1) < ARRAY_COUNT(sDynamicCustomMonMoves))
            outData->moves[outData->movesCount++] = sDynamicCustomMonMoves[compressedData->move1 - 1];

        if(compressedData->move2 != 0 && (compressedData->move2 - 1) < ARRAY_COUNT(sDynamicCustomMonMoves))
            outData->moves[outData->movesCount++] = sDynamicCustomMonMoves[compressedData->move2 - 1];
    }
    else
    {
        AGB_ASSERT(FALSE);
    }
};

static u32 CompressedDynamicDataToCustomMonId(struct CompressedDynamicData* inData)
{
    u32 id;
    id = *((u32*)inData);
    return id;
}

u32 RogueGift_GetCustomMonId(struct Pokemon* mon)
{
    return RogueGift_GetCustomMonIdBySpecies(GetMonData(mon, MON_DATA_SPECIES), GetMonData(mon, MON_DATA_OT_ID));
}

u32 RogueGift_GetCustomBoxMonId(struct BoxPokemon* mon)
{
    return RogueGift_GetCustomMonIdBySpecies(GetBoxMonData(mon, MON_DATA_SPECIES), GetBoxMonData(mon, MON_DATA_OT_ID));
}

static u16 TransformCheckSpecies(u16 species)
{
#ifdef ROGUE_EXPANSION
    // Special case (probably should get a proper fix for this)
    if(species >= SPECIES_PIKACHU_COSPLAY && species <= SPECIES_PICHU_SPIKY_EARED)
        return species;

    if(!gSpeciesInfo[species].isAlolanForm && !gSpeciesInfo[species].isGalarianForm && !gSpeciesInfo[species].isHisuianForm && !gSpeciesInfo[species].isPaldeanForm)
    {
        // Fix for castform/other form changes
        return GET_BASE_SPECIES_ID(species);
    }
#endif

    return species;
}

u32 RogueGift_GetCustomMonIdBySpecies(u16 species, u32 otId)
{
    u32 i;

#ifdef ROGUE_EXPANSION
    // The mega itself has it's own ability so don't redirect
    if(species == SPECIES_PIKIN_MEGA)
        return CUSTOM_MON_NONE;
#endif

    species = TransformCheckSpecies(species);

    if((otId & OTID_FLAG_CUSTOM_MON) && (otId & OTID_FLAG_DYNAMIC_CUSTOM_MON))
    {
        // otId is the custom mon id, as it encodes the data into it
        return otId;
    }
    if(otId & OTID_FLAG_CUSTOM_MON)
    {
        species = Rogue_GetEggSpecies(species);

        for(i = CUSTOM_MON_NONE + 1; i < CUSTOM_MON_COUNT; ++i)
        {
            struct CustomMonData const* monData = &sCustomPokemon[i];
            
            if(monData->otId == otId && Rogue_GetEggSpecies(monData->species) == species)
                return i;
        }
    }

    return CUSTOM_MON_NONE;
}

u16 RogueGift_GetCustomMonMove(u32 id, u8 i)
{
    if(id & OTID_FLAG_DYNAMIC_CUSTOM_MON)
    {
        struct DynamicMonData dynamicData;
        UncompressDynamicMonData(id, &dynamicData);
        return i < dynamicData.movesCount ? dynamicData.moves[i] : MOVE_NONE;
    }
    else
    {
        AGB_ASSERT(id < CUSTOM_MON_COUNT);

        if(id != CUSTOM_MON_NONE)
        {
            struct CustomMonData const* monData = &sCustomPokemon[id];
            return i < monData->movesCount ? monData->moves[i] : MOVE_NONE;
        }
    }

    return MOVE_NONE;
}

u16 RogueGift_GetCustomMonMoveCount(u32 id)
{
    if(id & OTID_FLAG_DYNAMIC_CUSTOM_MON)
    {
        struct DynamicMonData dynamicData;
        UncompressDynamicMonData(id, &dynamicData);
        return dynamicData.movesCount;
    }
    else
    {
        AGB_ASSERT(id < CUSTOM_MON_COUNT);

        if(id != CUSTOM_MON_NONE)
        {
            struct CustomMonData const* monData = &sCustomPokemon[id];

            if(id == CUSTOM_MON_ABBIE_MAREEP && !FlagGet(FLAG_ROGUE_UNLOCKED_PIKIN_EASTER_EGG))
            {
                // Don't show last move until activated easter egg
                return monData->movesCount - 1;
            }

            return monData->movesCount;
        }
    }

    return 0;
}

u16 RogueGift_GetCustomMonAbility(u32 id, u8 i)
{
    if(id & OTID_FLAG_DYNAMIC_CUSTOM_MON)
    {
        struct DynamicMonData dynamicData;
        UncompressDynamicMonData(id, &dynamicData);
        return i == 0 ? dynamicData.ability : ABILITY_NONE;
    }
    else
    {
        AGB_ASSERT(id < CUSTOM_MON_COUNT);

        if(id != CUSTOM_MON_NONE)
        {
            struct CustomMonData const* monData = &sCustomPokemon[id];
            return (monData->abilities != NULL && i < NUM_ABILITY_SLOTS) ? monData->abilities[i] : ABILITY_NONE;
        }
    }

    return ABILITY_NONE;
}

u16 RogueGift_GetCustomMonAbilityCount(u32 id)
{
    if(id & OTID_FLAG_DYNAMIC_CUSTOM_MON)
    {
        struct DynamicMonData dynamicData;
        UncompressDynamicMonData(id, &dynamicData);
        return dynamicData.ability != ABILITY_NONE ? NUM_ABILITY_SLOTS : 0;
    }
    else
    {
        struct CustomMonData const* monData = &sCustomPokemon[id];
        AGB_ASSERT(id < CUSTOM_MON_COUNT);

        return (monData->abilities != NULL) ? NUM_ABILITY_SLOTS : 0;
    }
}

u8 RogueGift_GetCustomMonType(u32 id, u8 i)
{
    if(id & OTID_FLAG_DYNAMIC_CUSTOM_MON)
    {
        struct DynamicMonData dynamicData;
        UncompressDynamicMonData(id, &dynamicData);
        return dynamicData.types[i];
    }
    else
    {
        AGB_ASSERT(id < CUSTOM_MON_COUNT);

        if(id != CUSTOM_MON_NONE)
        {
            struct CustomMonData const* monData = &sCustomPokemon[id];
            return monData->types[i];
        }
    }

    return TYPE_NONE;
}

bool8 RogueGift_CanRenameCustomMon(u32 id)
{
    if(id & OTID_FLAG_DYNAMIC_CUSTOM_MON)
    {
        return TRUE;
    }
    else
    {
        // We're allowed to rename exotics, as they're technically dynamic custom mons too
        struct CustomMonData const* monData = &sCustomPokemon[id];
        AGB_ASSERT(id < CUSTOM_MON_COUNT);

        return monData->customTrainerId == CUSTOM_TRAINER_EXOTIC;
    }
}

bool8 RogueGift_DisplayCustomMonRarity(u32 id)
{
    if(id & OTID_FLAG_DYNAMIC_CUSTOM_MON)
        return TRUE;
    else
    {
        struct CustomMonData const* monData = &sCustomPokemon[id]; 
        if(monData->customTrainerId == CUSTOM_TRAINER_EXOTIC)
            return TRUE;
    }

    return FALSE;
}

u8 RogueGift_GetCustomMonRarity(u32 id)
{
    if(id & OTID_FLAG_DYNAMIC_CUSTOM_MON)
    {
        u16 moveCount = RogueGift_GetCustomMonMoveCount(id);
        u16 abilityCount = RogueGift_GetCustomMonAbilityCount(id);

        if(abilityCount != 0)
        {
            if(moveCount <= 1)
                return UNIQUE_RARITY_RARE;
            else
                return UNIQUE_RARITY_EPIC;
        }

        return UNIQUE_RARITY_COMMON;
    }
    else
    {
        struct CustomMonData const* monData = &sCustomPokemon[id]; 
        AGB_ASSERT(id < CUSTOM_MON_COUNT);

        switch (monData->customTrainerId)
        {
        case CUSTOM_TRAINER_COMMON:
            return UNIQUE_RARITY_COMMON;
        
        case CUSTOM_TRAINER_RARE:
            return UNIQUE_RARITY_RARE;
        
        case CUSTOM_TRAINER_EPIC:
            return UNIQUE_RARITY_EPIC;
        
        case CUSTOM_TRAINER_EXOTIC:
            return UNIQUE_RARITY_EXOTIC;
        }

        // We expect most/all of these to be classed as exotic
        return UNIQUE_RARITY_EXOTIC;
    }
}

u8 const* RogueGift_GetRarityName(u8 rarity)
{
    switch (rarity)
    {
    case UNIQUE_RARITY_COMMON:
        return sCustomTrainers[CUSTOM_TRAINER_COMMON].name;
    
    case UNIQUE_RARITY_RARE:
        return sCustomTrainers[CUSTOM_TRAINER_RARE].name;
    
    case UNIQUE_RARITY_EPIC:
        return sCustomTrainers[CUSTOM_TRAINER_EPIC].name;
    
    case UNIQUE_RARITY_EXOTIC:
        return sCustomTrainers[CUSTOM_TRAINER_EXOTIC].name;
    }

    AGB_ASSERT(FALSE);
    return sCustomTrainers[CUSTOM_TRAINER_QUESTS].name;
}

void RogueGift_CreateMon(u32 customMonId, struct Pokemon* mon, u16 species, u8 level, u8 fixedIV)
{
    struct CustomTrainerData const* trainerData = NULL;
    u32 temp = 0;
    u16 customMoveCount = RogueGift_GetCustomMonMoveCount(customMonId);
    u16 customAbilityCount = RogueGift_GetCustomMonAbilityCount(customMonId);
    
    if(customMonId & OTID_FLAG_DYNAMIC_CUSTOM_MON)
    {
        u8 customTrainerId = sRarityToCustomTrainerIndex[RogueGift_GetCustomMonRarity(customMonId)];
        trainerData = &sCustomTrainers[customTrainerId];

        ZeroMonData(mon);
        CreateMon(mon, species, level, fixedIV, 0, 0, OT_ID_CUSTOM_MON, customMonId);

        // Assign pokeball
        temp = ITEM_ROGUE_BALL;
        SetMonData(mon, MON_DATA_POKEBALL, &temp);
    }
    else
    {
        struct CustomMonData const* monData = &sCustomPokemon[customMonId]; 
        trainerData = &sCustomTrainers[monData->customTrainerId];

        AGB_ASSERT(customMonId < CUSTOM_MON_COUNT);
        AGB_ASSERT(Rogue_GetEggSpecies(species) == Rogue_GetEggSpecies(monData->species));
        AGB_ASSERT(trainerData->trainerId & OTID_FLAG_CUSTOM_MON);
        AGB_ASSERT(!(trainerData->trainerId & OTID_FLAG_DYNAMIC_CUSTOM_MON));

        ZeroMonData(mon);
        CreateMon(mon, species, level, fixedIV, 0, 0, OT_ID_CUSTOM_MON, trainerData->trainerId);

        // Update nickname
        if(monData->nickname != NULL)
        {
            SetMonData(mon, MON_DATA_NICKNAME, monData->nickname);
        }

        // Assign pokeball
        temp = monData->pokeball;
        SetMonData(mon, MON_DATA_POKEBALL, &temp);

        // Set shiny state
        temp = monData->isShiny ? 1 : 0;
        SetMonData(mon, MON_DATA_IS_SHINY, &temp);
    }

    // Met location
    temp = METLOC_FATEFUL_ENCOUNTER;
    SetMonData(mon, MON_DATA_MET_LOCATION, &temp);

    // Update OT name
    SetMonData(mon, MON_DATA_OT_NAME, trainerData->name);

    // Update OT colour
    temp = trainerData->trainerColour;
    SetMonData(mon, MON_DATA_OT_GENDER, &temp);

    // Default moveset is first 4 moves from custom moveset
    if(customMoveCount)
    {
        u8 m, j;
        u16 moves[MAX_MON_MOVES];

        for(m = 0; m < MAX_MON_MOVES && m < customMoveCount; ++m)
        {
            moves[m] = RogueGift_GetCustomMonMove(customMonId, m);
        }

        // Fill the rest of the moves with default moves
        for(j = 0; m < MAX_MON_MOVES; ++m, ++j)
        {
            moves[m] = GetMonData(mon, MON_DATA_MOVE1 + j);
        }

        // Give back moves
        for(m = 0; m < MAX_MON_MOVES; ++m)
        {
            temp = moves[m];
            SetMonData(mon, MON_DATA_MOVE1 + m, &temp);
            SetMonData(mon, MON_DATA_PP1 + m, &gBattleMoves[temp].pp);
        }
    }

    // Default custom ability
    if(customAbilityCount)
    {
        temp = (RogueGift_GetCustomMonAbility(customMonId, 1) != ABILITY_NONE) ? (Random() % 2) : 0;
        SetMonData(mon, MON_DATA_ABILITY_NUM, &temp);
    }
}

static u32 SelectNextMoveIndex(u16 species)
{
    if(RogueMiscQuery_AnyActiveElements())
    {
        u32 i;
        u16 moveId = RogueMiscQuery_SelectRandomElement(Random());
        RogueMiscQuery_EditElement(QUERY_FUNC_EXCLUDE, moveId);

        for (i = 0; i < ARRAY_COUNT(sDynamicCustomMonMoves); i++)
        {
            if(sDynamicCustomMonMoves[i] == moveId)
                return 1 + i;
        }

        // Should never get here
        AGB_ASSERT(FALSE);
        return 1;
    }

    // Can get here if we've ran out of move options, as everything else is already known
    return 0;
}

static u32 SelectNextAbilityIndex(u16 species)
{
    u8 i;

    // Give the mon a new ability for it
    while(TRUE)
    {
        u32 idx = (Random() % ARRAY_COUNT(sDynamicCustomMonAbilities));

        for(i = 0; i < NUM_ABILITY_SLOTS; ++i)
        {
            if(GetAbilityBySpecies(species, i, 0) == sDynamicCustomMonAbilities[idx])
            {
                idx = 10000;
                break;
            }
        }

        if(idx != 10000)
            return 1 + idx;
    }

    return 0;
}

static u32 SelectRandomType(u16 species, u8 index)
{
    u8 type;
    do
    {
        type = Random() % NUMBER_OF_MON_TYPES;
    } 
    while (!IS_STANDARD_TYPE(type) || GetTypeBySpecies(species, 0, 0) == type || GetTypeBySpecies(species, 1, 0) == type);

    return type;
}

u32 RogueGift_CreateDynamicMonId(u8 rarity, u16 species)
{
    u16 i;
    u32 temp;
    struct CompressedDynamicData compressedDataUntyped = {0};
    struct CustomTrainerData const* trainerData = &sCustomTrainers[sRarityToCustomTrainerIndex[rarity]];
    struct RoguePokemonProfile const* pokemonProfile = Rogue_GetPokemonProfile(species);

    switch (RogueRandom() % 2)
    {
    case 0:
        compressedDataUntyped.format = COMPRESSED_FORMAT_ORIGINAL;
        break;

    case 1:
        compressedDataUntyped.format = COMPRESSED_FORMAT_MON_TYPE;
        break;
    }

    // Lock this behind upgrade
    if(compressedDataUntyped.format == COMPRESSED_FORMAT_MON_TYPE && !RogueHub_HasUpgrade(HUB_UPGRADE_LAB_UNIQUE_TYPINGS))
    {
        compressedDataUntyped.format = COMPRESSED_FORMAT_ORIGINAL;
    }

    // Start query with moves which are valid
    RogueCustomQuery_Begin();

    for (i = 0; i < ARRAY_COUNT(sDynamicCustomMonMoves); i++)
    {
        RogueMiscQuery_EditElement(QUERY_FUNC_INCLUDE, sDynamicCustomMonMoves[i]);
    }

    for (i = 0; pokemonProfile->levelUpMoves[i].move != MOVE_NONE; i++)
    {
        RogueMiscQuery_EditElement(QUERY_FUNC_EXCLUDE, pokemonProfile->levelUpMoves[i].move);
    }

    for (i = 0; pokemonProfile->tutorMoves[i] != MOVE_NONE; i++)
    {
        RogueMiscQuery_EditElement(QUERY_FUNC_EXCLUDE, pokemonProfile->tutorMoves[i]);
    }

    if(compressedDataUntyped.format == COMPRESSED_FORMAT_ORIGINAL)
    {
        struct CompressedDynamicData_Original* compressedData = (struct CompressedDynamicData_Original*)&compressedDataUntyped;

        switch (rarity)
        {
        case UNIQUE_RARITY_COMMON:
            compressedData->move1 = SelectNextMoveIndex(species);
            compressedData->move2 = SelectNextMoveIndex(species);
            break;

        case UNIQUE_RARITY_RARE:
            compressedData->move1 = SelectNextMoveIndex(species);
            compressedData->ability = SelectNextAbilityIndex(species);
            break;

        case UNIQUE_RARITY_EPIC:
            compressedData->move1 = SelectNextMoveIndex(species);
            compressedData->move2 = SelectNextMoveIndex(species);
            compressedData->move3 = SelectNextMoveIndex(species);
            //compressedData.move4 = SelectNextMoveIndex(species);
            compressedData->ability = SelectNextAbilityIndex(species);
            break;

        default:
            AGB_ASSERT(FALSE);
            break;
        }
    }
    else if(compressedDataUntyped.format == COMPRESSED_FORMAT_MON_TYPE)
    {
        struct CompressedDynamicData_MonType* compressedData = (struct CompressedDynamicData_MonType*)&compressedDataUntyped;
        compressedData->typeSlot = Random() % 1;
        compressedData->typeMoveFlip = Random() % 1;

        switch (rarity)
        {
        case UNIQUE_RARITY_COMMON:
            compressedData->type = SelectRandomType(species, compressedData->typeSlot);
            break;

        case UNIQUE_RARITY_RARE:
            compressedData->type = SelectRandomType(species, compressedData->typeSlot);
            compressedData->move1 = SelectNextMoveIndex(species);
            compressedData->move2 = SelectNextMoveIndex(species);
            break;

        case UNIQUE_RARITY_EPIC:
            compressedData->type = SelectRandomType(species, compressedData->typeSlot);
            compressedData->move1 = SelectNextMoveIndex(species);
            compressedData->move2 = SelectNextMoveIndex(species);
            compressedData->ability = SelectNextAbilityIndex(species);
            break;

        default:
            AGB_ASSERT(FALSE);
            break;
        }
    }
    else
    {
        AGB_ASSERT(FALSE);
    }

    RogueCustomQuery_End();

    temp = CompressedDynamicDataToCustomMonId(&compressedDataUntyped);
    temp |= (OTID_FLAG_CUSTOM_MON | OTID_FLAG_DYNAMIC_CUSTOM_MON);

#ifdef ROGUE_DEBUG
    // Ensure data (un)compresses correctly
    if(compressedDataUntyped.format == COMPRESSED_FORMAT_ORIGINAL)
    {
        struct CompressedDynamicData_Original* compressedData = (struct CompressedDynamicData_Original*)&compressedDataUntyped;

        struct DynamicMonData dynamicData = {0};
        

        u16 ability = (compressedData->ability == 0) ? ABILITY_NONE : sDynamicCustomMonAbilities[compressedData->ability - 1];
        u16 moves[MAX_MON_MOVES] = 
        {
            (compressedData->move1 == 0) ? MOVE_NONE : sDynamicCustomMonMoves[compressedData->move1 - 1],
            (compressedData->move2 == 0) ? MOVE_NONE : sDynamicCustomMonMoves[compressedData->move2 - 1],
            (compressedData->move3 == 0) ? MOVE_NONE : sDynamicCustomMonMoves[compressedData->move3 - 1],
            MOVE_NONE
            //(compressedData.move4 == 0) ? MOVE_NONE : sDynamicCustomMonMoves[compressedData.move4 - 1],
        };

        UncompressDynamicMonData(temp, &dynamicData);

        AGB_ASSERT(moves[0] == dynamicData.moves[0]);
        AGB_ASSERT(moves[1] == dynamicData.moves[1]);
        AGB_ASSERT(moves[2] == dynamicData.moves[2]);
        AGB_ASSERT(moves[3] == dynamicData.moves[3]);

        AGB_ASSERT(ability == dynamicData.ability);
    }

#endif
    return temp;
}

static bool8 IsDynamicUniqueMonValid(struct UniqueMon* mon)
{
    return !(mon->species == SPECIES_NONE || mon->customMonId == 0 || mon->countDown == 0);
}

static u8 RandomRarity()
{
    u8 rarity;

    switch (Random() % 7)
    {
    case 0:
        rarity = UNIQUE_RARITY_EXOTIC;
        break;

    case 1:
    case 2:
        rarity = UNIQUE_RARITY_EPIC;
        break;

    case 3:
    case 4:
        rarity = UNIQUE_RARITY_RARE;
        break;
    
    case 5:
    case 6:
        rarity = UNIQUE_RARITY_COMMON;
        break;

    default:
        AGB_ASSERT(FALSE);
        rarity = UNIQUE_RARITY_COMMON;
        break;
    }

    if(rarity == UNIQUE_RARITY_EXOTIC && !RogueHub_HasUpgrade(HUB_UPGRADE_LAB_UNIQUE_MON_RARITY_EXOTIC))
        rarity = UNIQUE_RARITY_EPIC;

    if(rarity == UNIQUE_RARITY_EPIC && !RogueHub_HasUpgrade(HUB_UPGRADE_LAB_UNIQUE_MON_RARITY_EPIC))
        rarity = UNIQUE_RARITY_RARE;

    if(rarity == UNIQUE_RARITY_RARE && !RogueHub_HasUpgrade(HUB_UPGRADE_LAB_UNIQUE_MON_RARITY_RARE))
        rarity = UNIQUE_RARITY_COMMON;

    return rarity;
}

static bool8 IsSlotUnlocked(u8 slot)
{
    if(RogueHub_HasUpgrade(HUB_UPGRADE_LAB_UNIQUE_MON_LAB))
    {
        // 3 slots unlocked by default
        if(slot <= 2)
            return TRUE;

        if(slot == 3)
            return RogueHub_HasUpgrade(HUB_UPGRADE_LAB_UNIQUE_MON_SLOTS0);

        if(slot == 4)
            return RogueHub_HasUpgrade(HUB_UPGRADE_LAB_UNIQUE_MON_SLOTS1);
    }

    return FALSE;
}

static bool8 IsCustomMonInUse(u32 customMonId)
{
    u32 i;

    for(i = 0; i < DYNAMIC_UNIQUE_MON_COUNT; ++i)
    {
        if(IsDynamicUniqueMonValid(&gRogueSaveBlock->dynamicUniquePokemon[i]) && gRogueSaveBlock->dynamicUniquePokemon[i].customMonId == customMonId)
            return TRUE;
    }

    return FALSE;
}

static u32 SelectUnusedUnlockedExoticMon()
{
    struct RogueQuestReward const* reward;
    u32 currentList[CUSTOM_MON_COUNT];
    u32 listSize = 0;
    u32 questId, j;
    u32 questCount;

    for(j = 0; j < CUSTOM_MON_COUNT; ++j)
    {
        if(sCustomPokemon[j].isDefaultSpawn)
            currentList[listSize++] = j;
    }

    // Populate with exotic mons we have already unlocked
    for(questId = 0; questId < QUEST_ID_COUNT; ++questId)
    {
        if(RogueQuest_HasCollectedRewards(questId))
        {
            questCount = RogueQuest_GetRewardCount(questId);

            for(j = 0; j < questCount; ++j)
            {
                reward = RogueQuest_GetReward(questId, j);

                if(reward->type == QUEST_REWARD_POKEMON && reward->perType.pokemon.customMonId != 0)
                {
                    if(!IsCustomMonInUse(reward->perType.pokemon.customMonId))
                        currentList[listSize++] = reward->perType.pokemon.customMonId;
                }
            }
        }
    }

    // Pick from random options
    if(listSize != 0)
        return currentList[Random() % listSize];

    return 0;
}

void RogueGift_EnsureDynamicCustomMonsAreValid()
{
    u8 i, write;
    u16 newSpecies[DYNAMIC_UNIQUE_MON_COUNT];
    u8 dexVariant = RoguePokedex_GetDexVariant();

    // We use query below, so grab some new unique species now
    {
        // Override dex variant to our max nat dex
        u8 dexVariantToRestore = RoguePokedex_GetDexVariant();
        RoguePokedex_SetDexVariant(POKEDEX_VARIANT_DEFAULT);

        RogueMonQuery_Begin();
        RogueMonQuery_IsSpeciesActive();
        RogueMonQuery_TransformIntoEggSpecies();
        RogueMonQuery_IsLegendary(QUERY_FUNC_EXCLUDE);
        
        for(i = 0; i < DYNAMIC_UNIQUE_MON_COUNT; ++i)
        {
            if(IsDynamicUniqueMonValid(&gRogueSaveBlock->dynamicUniquePokemon[i]))
            {
                RogueMiscQuery_EditElement(QUERY_FUNC_EXCLUDE, gRogueSaveBlock->dynamicUniquePokemon[i].species);
            }
        }

        // Remove random entries until we can safely calcualte weights without going over
        while(RogueWeightQuery_IsOverSafeCapacity())
        {
            RogueMiscQuery_FilterByChance(Random(), QUERY_FUNC_INCLUDE, 50, DYNAMIC_UNIQUE_MON_COUNT);
        }

        RogueWeightQuery_Begin();
        RogueWeightQuery_FillWeights(1);

        for(i = 0; i < DYNAMIC_UNIQUE_MON_COUNT; ++i)
        {
            newSpecies[i] = RogueWeightQuery_SelectRandomFromWeightsWithUpdate(Random(), 0);
        }

        RogueWeightQuery_End();

        RogueMonQuery_End();

        RoguePokedex_SetDexVariant(dexVariantToRestore);
    }

    // Shift all the empty slots up 
    write = 0;
    for(i = 0; i < DYNAMIC_UNIQUE_MON_COUNT; ++i)
    {
        if(IsDynamicUniqueMonValid(&gRogueSaveBlock->dynamicUniquePokemon[i]))
        {
            if(write != i)
            {
                memcpy(&gRogueSaveBlock->dynamicUniquePokemon[write], &gRogueSaveBlock->dynamicUniquePokemon[i], sizeof(struct UniqueMon));
                memset(&gRogueSaveBlock->dynamicUniquePokemon[i], 0, sizeof(struct UniqueMon));
            }

            write++;
        }
        else
        {
            memset(&gRogueSaveBlock->dynamicUniquePokemon[i], 0, sizeof(struct UniqueMon));
        }
    }

    // Replace invalid mons
    for(i = 0; i < DYNAMIC_UNIQUE_MON_COUNT; ++i)
    {
        // We don't want to populate slots that aren't active
        if(!IsSlotUnlocked(i))
            continue;

        if(!IsDynamicUniqueMonValid(&gRogueSaveBlock->dynamicUniquePokemon[i]))
        {
            u8 rarity = RandomRarity();

            gRogueSaveBlock->dynamicUniquePokemon[i].countDown = 60 + 30 * i; // Time remaining is based on the slot

            if(rarity == UNIQUE_RARITY_EXOTIC)
            {
                u32 customMonId = SelectUnusedUnlockedExoticMon();

                if(customMonId != 0)
                {
                    struct CustomMonData const* monData = &sCustomPokemon[customMonId];
                    AGB_ASSERT(customMonId < CUSTOM_MON_COUNT);

                    gRogueSaveBlock->dynamicUniquePokemon[i].species = Rogue_GetEggSpecies(monData->species);
                    gRogueSaveBlock->dynamicUniquePokemon[i].customMonId = customMonId;
                    continue;
                }

                // Fallback to just have an epic mon in this slot
                rarity = UNIQUE_RARITY_EPIC;
            }

            gRogueSaveBlock->dynamicUniquePokemon[i].species = newSpecies[i];
            gRogueSaveBlock->dynamicUniquePokemon[i].customMonId = RogueGift_CreateDynamicMonId(rarity, newSpecies[i]);
        }
    }

}

void RogueGift_ClearDynamicCustomMons()
{
    memset(&gRogueSaveBlock->dynamicUniquePokemon[0], 0, sizeof(struct UniqueMon) * DYNAMIC_UNIQUE_MON_COUNT);
}

struct UniqueMon* RogueGift_GetDynamicUniqueMon(u8 slot)
{
    AGB_ASSERT(slot < DYNAMIC_UNIQUE_MON_COUNT);
    return &gRogueSaveBlock->dynamicUniquePokemon[slot];
}

void RogueGift_CountDownDynamicCustomMons()
{
    u8 i;

    for(i = 0; i < DYNAMIC_UNIQUE_MON_COUNT; ++i)
    {
        if(gRogueSaveBlock->dynamicUniquePokemon[i].countDown != 0)
            --gRogueSaveBlock->dynamicUniquePokemon[i].countDown;
    }
}

void RogueGift_RemoveDynamicCustomMon(u32 customMonId)
{
    u8 i;

    for(i = 0; i < DYNAMIC_UNIQUE_MON_COUNT; ++i)
    {
        if(gRogueSaveBlock->dynamicUniquePokemon[i].customMonId == customMonId)
        {
            memset(&gRogueSaveBlock->dynamicUniquePokemon[i], 0, sizeof(struct UniqueMon));
        }
    }
}

bool8 RogueGift_IsDynamicMonSlotEnabled(u8 slot)
{
    AGB_ASSERT(slot < DYNAMIC_UNIQUE_MON_COUNT);

    if(!IsSlotUnlocked(slot))
        return FALSE;

    return IsDynamicUniqueMonValid(&gRogueSaveBlock->dynamicUniquePokemon[slot]);
}

u32 RogueGift_TryFindEnabledDynamicCustomMonForSpecies(u16 species)
{
    u8 i;

    species = Rogue_GetEggSpecies(species);

    for(i = 0; i < DYNAMIC_UNIQUE_MON_COUNT; ++i)
    {
        if(RogueGift_IsDynamicMonSlotEnabled(i))
        {
            if(gRogueSaveBlock->dynamicUniquePokemon[i].species == species)
                return gRogueSaveBlock->dynamicUniquePokemon[i].customMonId;
        }
    }

    return 0;
}

static u8 ModifyShinyType(u8 type)
{
    if(type == TYPE_NONE)
        return type;

    do
    {
        type = (type + 5) % NUMBER_OF_MON_TYPES;
    } 
    while (!IS_STANDARD_TYPE(type));

    return type;
    
}

bool8 RogueGift_TryApplyPaletteModify(u32 id, bool8 isShiny, u16 const* inputPal, u16 const* layerRefPal, u16* outputPal)
{
    u8 type1 = RogueGift_GetCustomMonType(id, 0);
    u8 type2 = RogueGift_GetCustomMonType(id, 1);

    if(type1 != TYPE_NONE || type2 != TYPE_NONE)
    {
        u16 layerPal[16];
        u16 layerWhitePoints[PALETTE_MODIFY_LAYER_COUNT];
        u16 colorsToApply[PALETTE_MODIFY_LAYER_COUNT];

        if(isShiny)
        {
            // Give a psuedo random tint for shiny pal version
            type1 = ModifyShinyType(type1);
            type2 = ModifyShinyType(type2);
        }

        colorsToApply[0] = RGB_ALPHA;
        colorsToApply[1] = RGB_ALPHA;
        colorsToApply[2] = RGB_ALPHA;

        if(type1 != TYPE_NONE)
            colorsToApply[0] = sTypeTintColors[type1];

        if(type2 != TYPE_NONE)
            colorsToApply[1] = sTypeTintColors[type2];

        if(layerRefPal == NULL)
        {
            // Ref and Input are the same so generate layers and whitepoints together
            Rogue_GenerateLayerPaletteByHue(inputPal, inputPal, layerPal);
            Rogue_GenerateWhitePointsPerLayers(inputPal, layerPal, layerWhitePoints, gDefaultPaletteLayerMasks);
        }
        else
        {
            // Generate temp layers for ref so we can generate white points
            Rogue_GenerateLayerPaletteByHue(layerRefPal, layerRefPal, layerPal);
            Rogue_GenerateWhitePointsPerLayers(layerRefPal, layerPal, layerWhitePoints, gDefaultPaletteLayerMasks);

            // Now generate actual palette for input
            Rogue_GenerateLayerPaletteByHue(inputPal, layerRefPal, layerPal);
        }

        Rogue_ModifyPaletteByLayersHueShift(inputPal, layerPal, layerWhitePoints, outputPal, gDefaultPaletteLayerMasks, colorsToApply);

        return TRUE;
    }

    return FALSE;
}    u16 yEnd;
};

struct MapInfo
{
    u16 group;
    u16 num;
};

struct TileFixup
{
    u8 path : 1;
    u8 pond : 1;
    u8 mountain : 1;
    u8 trees : 1;
    u8 pathStyle : 1;
};

struct LayerInfo
{
    bool8 placeInBackground : 1;
    bool8 allowBackgroundTileOverlap : 1;
    bool8 allowSolidTileOverlap : 1;
    bool8 allowObjectOverlap : 1;
};

enum
{
    DECOR_LAYER_BACKGROUND,
    DECOR_LAYER_MOUNTAIN,
    DECOR_LAYER_SOLID_TILE,
    DECOR_LAYER_PLACEABLE_SURFACE,
    DECOR_LAYER_OBJECTS,
    DECOR_LAYER_COUNT,

    // Default that is used if not specified
    DECOR_LAYER_DEFAULT = DECOR_LAYER_SOLID_TILE,
};

struct LayerInfo const sDecorLayers[DECOR_LAYER_COUNT] = 
{
    [DECOR_LAYER_BACKGROUND] =
    {
        .placeInBackground = TRUE,
        .allowBackgroundTileOverlap = TRUE,
        .allowSolidTileOverlap = TRUE,
        .allowObjectOverlap = TRUE,
    },
    [DECOR_LAYER_MOUNTAIN] =
    {
        .placeInBackground = TRUE,
        .allowBackgroundTileOverlap = TRUE,
        .allowSolidTileOverlap = FALSE,
        .allowObjectOverlap = FALSE,
    },
    [DECOR_LAYER_SOLID_TILE] =
    {
        .placeInBackground = FALSE,
        .allowBackgroundTileOverlap = TRUE,
        .allowSolidTileOverlap = FALSE,
        .allowObjectOverlap = FALSE,
    },
    [DECOR_LAYER_PLACEABLE_SURFACE] =
    {
        .placeInBackground = FALSE,
        .allowBackgroundTileOverlap = TRUE,
        .allowSolidTileOverlap = FALSE,
        .allowObjectOverlap = TRUE,
    },
    [DECOR_LAYER_OBJECTS] =
    {
        .placeInBackground = FALSE,
        .allowBackgroundTileOverlap = TRUE,
        .allowSolidTileOverlap = FALSE,
        .allowObjectOverlap = FALSE,
    },
};

enum
{
    HOME_REGION_HOUSE,
    HOME_REGION_PLACEABLE_REGION,
    HOME_REGION_PLACEABLE_REGION_INTERIOR,
    HOME_REGION_COUNT
};

static struct RegionCoords const sHomeRegionCoords[HOME_REGION_COUNT] = 
{
    [HOME_REGION_HOUSE] =  { 15, 14, 19, 19 },
    [HOME_REGION_PLACEABLE_REGION] =  { 4, 4, 31, 31 },
    [HOME_REGION_PLACEABLE_REGION_INTERIOR] =  { 1, 1, 10, 10 },
};

static struct MapInfo const sHomeAreaStyles[HOME_AREA_STYLE_COUNT] = 
{
    [HOME_AREA_STYLE_OVERGROWN] = {}, // default
    [HOME_AREA_STYLE_FLOWERS] = { MAP_GROUP(ROGUE_TEMPLATE_HOME_FLOWERS), MAP_NUM(ROGUE_TEMPLATE_HOME_FLOWERS) },
    [HOME_AREA_STYLE_PLAIN] = { MAP_GROUP(ROGUE_TEMPLATE_HOME_GRASS), MAP_NUM(ROGUE_TEMPLATE_HOME_GRASS) },
};

enum
{
    DECOR_TYPE_TILE,
    DECOR_TYPE_OBJECT_EVENT,
};

struct RogueDecorationGroup
{
    u8 const* name;
    u16 const* decorationIds;
    u16 decorationCount;
};

struct RogueDecorationVariant
{
    u8 const* name;
    u8 type;
    u8 srcMapGroup;
    u8 srcMapNum;
    u8 layer;
    union
    {
        struct
        {
            u8 x;
            u8 y;
            u8 width;
            u8 height;
        } tile;
        struct
        {
            u8 localId;
            u8 editorLocalId;
            u8 capacityPerArea;
        } objectEvent;
    } perType;
};

struct RogueDecoration
{
    u8 const* name;
    u16 firstVariantId;
    u16 lastVariantId;
};

enum
{
    PATH_STYLE_GRASS,
    PATH_STYLE_SAND,
    PATH_STYLE_STONE,
    PATH_STYLE_PEBBLES,
    PATH_STYLE_MUDDY_TRACKS,
    PATH_STYLE_COUNT,
};

enum
{
    EXTERIOR_STYLE_CAVE,
    EXTERIOR_STYLE_TREES,
    EXTERIOR_STYLE_BRICK_HOUSE,
    EXTERIOR_STYLE_WOODEN_HOUSE,
    EXTERIOR_STYLE_GYM_BUILDING,
    EXTERIOR_STYLE_COUNT,
};

enum
{
    INTERIOR_STYLE_RED_CAVE,
    INTERIOR_STYLE_BLUE_CAVE,
    INTERIOR_STYLE_BROWN_CAVE,
    INTERIOR_STYLE_DESERT_CAVE,
    INTERIOR_STYLE_SHRUB,
    INTERIOR_STYLE_TREE,
    INTERIOR_STYLE_COUNT,
};

static u8 const sText_PathStyle_Grass[] = _("Grass");
static u8 const sText_PathStyle_Sand[] = _("Sand");
static u8 const sText_PathStyle_Stone[] = _("Stone");
static u8 const sText_PathStyle_Pebbles[] = _("Pebbles");
static u8 const sText_PathStyle_MuddyTracks[] = _("Muddy Tracks");

static u8 const* const sOptions_PathStyle[PATH_STYLE_COUNT] =
{
    [PATH_STYLE_GRASS] = sText_PathStyle_Grass,
    [PATH_STYLE_SAND] = sText_PathStyle_Sand,
    [PATH_STYLE_STONE] = sText_PathStyle_Stone,
    [PATH_STYLE_PEBBLES] = sText_PathStyle_Pebbles,
    [PATH_STYLE_MUDDY_TRACKS] = sText_PathStyle_MuddyTracks,
};

static u8 const sText_ExteriorStyle_Cave[] = _("Cave");
static u8 const sText_ExteriorStyle_Trees[] = _("Trees");
static u8 const sText_ExteriorStyle_BrickHouse[] = _("Brick House");
static u8 const sText_ExteriorStyle_WoodHouse[] = _("Wooden House");
static u8 const sText_ExteriorStyle_Gym[] = _("Gym Building");

static u8 const* const sOptions_ExteriorStyle[EXTERIOR_STYLE_COUNT] =
{
    [EXTERIOR_STYLE_CAVE] = sText_ExteriorStyle_Cave,
    [EXTERIOR_STYLE_TREES] = sText_ExteriorStyle_Trees,
    [EXTERIOR_STYLE_BRICK_HOUSE] = sText_ExteriorStyle_BrickHouse,
    [EXTERIOR_STYLE_WOODEN_HOUSE] = sText_ExteriorStyle_WoodHouse,
    [EXTERIOR_STYLE_GYM_BUILDING] = sText_ExteriorStyle_Gym,
};

static u8 const sText_InteriorStyle_BlueCave[] = _("Blue Cave");
static u8 const sText_InteriorStyle_BrownCave[] = _("Brown Cave");
static u8 const sText_InteriorStyle_RedCave[] = _("Red Cave");
static u8 const sText_InteriorStyle_DesertCave[] = _("Desert Cave");
static u8 const sText_InteriorStyle_Shrub[] = _("Shrub");
static u8 const sText_InteriorStyle_Tree[] = _("Tree");

static u8 const* const sOptions_InteriorStyle[INTERIOR_STYLE_COUNT] =
{
    [INTERIOR_STYLE_BLUE_CAVE] = sText_InteriorStyle_BlueCave,
    [INTERIOR_STYLE_BROWN_CAVE] = sText_InteriorStyle_BrownCave,
    [INTERIOR_STYLE_RED_CAVE] = sText_InteriorStyle_RedCave,
    [INTERIOR_STYLE_DESERT_CAVE] = sText_InteriorStyle_DesertCave,
    [INTERIOR_STYLE_SHRUB] = sText_InteriorStyle_Shrub,
    [INTERIOR_STYLE_TREE] = sText_InteriorStyle_Tree,
};

extern const struct Tileset gTileset_SecretBaseBlueCave;
extern const struct Tileset gTileset_SecretBaseBrownCave;
extern const struct Tileset gTileset_SecretBaseRedCave;
extern const struct Tileset gTileset_SecretBaseYellowCave;
extern const struct Tileset gTileset_SecretBaseShrub;
extern const struct Tileset gTileset_SecretBaseTree;

#include "data/rogue/decorations.h"

static void MetatileSet_Tile(u16 xStart, u16 yStart, u16 tile);
static void MetatileFill_Tile(u16 xStart, u16 yStart, u16 xEnd, u16 yEnd, u16 tile);
static void MetatileFill_TreesOverlapping(u16 xStart, u16 yStart, u16 xEnd, u16 yEnd, u8 treeType);
static void MetatileFill_TreeStumps(u16 xStart, u16 yStart, u16 xEnd, u8 treeType);
static void MetatileFill_TreeCaps(u16 xStart, u16 yStart, u16 xEnd);

static void MetatileFill_CommonWarpExitVertical(u16 xStart, u16 yStart);
static void MetatileFill_CommonWarpExitHorizontal(u16 xStart, u16 yStart);
static void MetatileFill_CommonPathRemoval(u16 xStart, u16 yStart, u16 xEnd, u16 yEnd);
static void MetatileFill_BlitMapRegion(u16 mapGroup, u16 mapNum, u16 destX1, u16 destY1, u16 destX2, u16 destY2, u16 srcX1, u16 srcY1);

static void RogueHub_UpdateLabsAreaMetatiles();
static void RogueHub_UpdateAdventureEntranceAreaMetatiles();
static void RogueHub_UpdateHomeAreaMetatiles();
static void RogueHub_PlaceHomeEnvironmentDecorations(bool8 placeTiles, bool8 placeObjects);
static void RogueHub_UpdateHomeInteriorMetatiles();
static void RogueHub_UpdateFarmingAreaMetatiles();
static void RogueHub_UpdateSafariAreaMetatiles();
static void RogueHub_UpdateRideTrainingAreaMetatiles();
static void RogueHub_UpdateMartsAreaMetatiles();
static void RogueHub_UpdateTownSquareAreaMetatiles();
static void RogueHub_UpdateMarketAreaMetatiles();
static void RogueHub_UpdateChallengeFrontierAreaMetatiles();
static void RogueHub_UpdateDayCareAreaMetatiles();

static void BuildAtRandomConnectionFrom(u8 fromArea, u8 buildArea);

static void FixupTileCommon(struct TileFixup* settings);

void RogueHub_Enter()
{
    //AGB_ASSERT(gRogueSaveBlock->hubMap == NULL);
    //gRogueSaveBlock->hubMap = AllocZeroed(sizeof(struct RogueHubMap));
}

void RogueHub_Exit()
{
    //AGB_ASSERT(gRogueSaveBlock->hubMap != NULL);
    //free(gRogueSaveBlock->hubMap);
    //gRogueSaveBlock->hubMap = NULL;
}

static struct RogueHubMap* GetActiveHubMap()
{
    if(RogueMP_IsActive() && !RogueMP_IsHost())
    {
        AGB_ASSERT(gRogueMultiplayer != NULL);
        return &gRogueMultiplayer->gameState.hub.hubMap;
    }

    return &gRogueSaveBlock->hubMap;
}

void RogueHub_ClearProgress()
{
    memset(&gRogueSaveBlock->hubMap, 0, sizeof(gRogueSaveBlock->hubMap));

    // Build default area at away from reserved coords (will recentre in a second)
    RogueHub_BuildArea(HUB_AREA_LABS, 10, 10);

    // Place required areas randomly (Order matters)
    BuildAtRandomConnectionFrom(HUB_AREA_LABS, HUB_AREA_ADVENTURE_ENTRANCE);
    BuildAtRandomConnectionFrom(HUB_AREA_ADVENTURE_ENTRANCE, HUB_AREA_SAFARI_ZONE);
    BuildAtRandomConnectionFrom(HUB_AREA_LABS, HUB_AREA_TOWN_SQUARE);

    // Now recenter so that the adventure enterance is actually at 0,0
    {
        u8 i;
        struct Coords8 offset = gRogueSaveBlock->hubMap.areaCoords[HUB_AREA_ADVENTURE_ENTRANCE];

        for(i = 0; i < HUB_AREA_COUNT; ++i)
        {
            if(RogueHub_HasAreaBuilt(i))
            {
                gRogueSaveBlock->hubMap.areaCoords[i].x -= offset.x;
                gRogueSaveBlock->hubMap.areaCoords[i].y -= offset.y;
            }
        }
    }

    // Place default decor
    gRogueSaveBlock->hubMap.homeDecorations[HOME_DECOR_OUTSIDE_OFFSET + 0].active = TRUE;
    gRogueSaveBlock->hubMap.homeDecorations[HOME_DECOR_OUTSIDE_OFFSET + 0].decorVariant = DECOR_VARIANT_OUTFIT_CHANGING_WARDROBE;
    gRogueSaveBlock->hubMap.homeDecorations[HOME_DECOR_OUTSIDE_OFFSET + 0].x = 21;
    gRogueSaveBlock->hubMap.homeDecorations[HOME_DECOR_OUTSIDE_OFFSET + 0].y = 19;
}

bool8 RogueHub_HasUpgrade(u16 upgradeId)
{
    u16 idx = upgradeId / 8;
    u16 bit = upgradeId % 8;

    u8 bitMask = 1 << bit;

    AGB_ASSERT(idx < ARRAY_COUNT(GetActiveHubMap()->upgradeFlags));
    return (GetActiveHubMap()->upgradeFlags[idx] & bitMask) != 0;
}

bool8 RogueHub_HasLocalUpgrade(u16 upgradeId)
{
    u16 idx = upgradeId / 8;
    u16 bit = upgradeId % 8;

    u8 bitMask = 1 << bit;

    AGB_ASSERT(idx < ARRAY_COUNT(GetActiveHubMap()->upgradeFlags));
    return (gRogueSaveBlock->hubMap.upgradeFlags[idx] & bitMask) != 0;
}

bool8 RogueHub_HasAllLocalUpgrades()
{
    u32 i = 0;

    for(i = 0; i < HUB_UPGRADE_COUNT; ++i)
    {
        // Ignore quest unlocks
        if(gRogueHubUpgrades[i].buildCost == 0)
            continue;

        if(!RogueHub_HasLocalUpgrade(i))
            return FALSE;
    }

    return TRUE;
}

void RogueHub_SetUpgrade(u16 upgradeId, bool8 state)
{
    u16 idx = upgradeId / 8;
    u16 bit = upgradeId % 8;

    u8 bitMask = 1 << bit;
    
    AGB_ASSERT(idx < ARRAY_COUNT(GetActiveHubMap()->upgradeFlags));
    if(state)
    {
        GetActiveHubMap()->upgradeFlags[idx] |= bitMask;
    }
    else
    {
        GetActiveHubMap()->upgradeFlags[idx] &= ~bitMask;
    }
}

bool8 RogueHub_HasUpgradeRequirements(u16 upgradeId)
{
    u8 i;
    u8 check;

    if(!RogueHub_HasAreaBuilt(gRogueHubUpgrades[upgradeId].targetArea))
        return FALSE;

    for(i = 0; i < HUB_UPGRADE_MAX_REQUIREMENTS; ++i)
    {
        check = gRogueHubUpgrades[upgradeId].requiredUpgrades[i];
        if(check == HUB_UPGRADE_NONE)
            break;

        if(!RogueHub_HasUpgrade(check))
            return FALSE;
    }

    return TRUE;
}

bool8 RogueHub_HasAreaBuilt(u8 area)
{
    u16 idx = area / 8;
    u16 bit = area % 8;

    u8 bitMask = 1 << bit;

    AGB_ASSERT(idx < ARRAY_COUNT(GetActiveHubMap()->areaBuiltFlags));
    return (GetActiveHubMap()->areaBuiltFlags[idx] & bitMask) != 0;
}

void RogueHub_BuildArea(u8 area, s8 x, s8 y)
{
    u16 idx = area / 8;
    u16 bit = area % 8;

    u8 bitMask = 1 << bit;
    
    AGB_ASSERT(idx < ARRAY_COUNT(GetActiveHubMap()->areaBuiltFlags));
    
    GetActiveHubMap()->areaBuiltFlags[idx] |= bitMask;
    GetActiveHubMap()->areaCoords[area].x = x;
    GetActiveHubMap()->areaCoords[area].y = y;
}

static void IncrementCoordsByDirection(struct Coords8* coords, u8 dir)
{
    switch (dir)
    {
    case HUB_AREA_CONN_SOUTH:
        --coords->y;
        break;
    case HUB_AREA_CONN_NORTH:
        ++coords->y;
        break;
    case HUB_AREA_CONN_WEST:
        --coords->x;
        break;
    case HUB_AREA_CONN_EAST:
        ++coords->x;
        break;
    }
}

void RogueHub_BuildAreaInConnDir(u8 area, u8 connDir)
{
    u8 currentArea = RogueHub_GetAreaFromCurrentMap();

    if(currentArea != HUB_AREA_NONE && !RogueHub_HasAreaBuilt(area) && connDir < HUB_AREA_CONN_COUNT)
    {
        struct Coords8 pos;
        pos.x = GetActiveHubMap()->areaCoords[currentArea].x;
        pos.y = GetActiveHubMap()->areaCoords[currentArea].y;
        IncrementCoordsByDirection(&pos, connDir);

        RogueHub_BuildArea(area, pos.x, pos.y);
    }
}

bool8 RogueHub_HasAreaBuildRequirements(u8 area)
{
    u8 i;
    u8 check;

    // Cannot build until post game
    if(area == HUB_AREA_CHALLENGE_FRONTIER)
    {
        if(!FlagGet(FLAG_ROGUE_MET_POKABBIE))
            return FALSE;
    }

    for(i = 0; i < HUB_UPGRADE_MAX_REQUIREMENTS; ++i)
    {
        check = gRogueHubAreas[area].requiredUpgrades[i];
        if(check == HUB_UPGRADE_NONE)
            break;

        if(!RogueHub_HasUpgrade(check))
            return FALSE;
    }

    return TRUE;
}

u8 RogueHub_FindAreaAtCoord(s8 x, s8 y)
{
    u8 i;

    for(i = 0; i < HUB_AREA_COUNT; ++i)
    {
        if(RogueHub_HasAreaBuilt(i))
        {
            if(GetActiveHubMap()->areaCoords[i].x == x && GetActiveHubMap()->areaCoords[i].y == y)
                return i;
        }
    }

    return HUB_AREA_NONE;
}

// Catch unbuildable coordinates
static bool8 IsReservedCoord(s8 x, s8 y)
{
    // Reserve north of adventure entrance
    if(GetActiveHubMap()->areaCoords[HUB_AREA_ADVENTURE_ENTRANCE].x == x && GetActiveHubMap()->areaCoords[HUB_AREA_ADVENTURE_ENTRANCE].y + 1 == y)
        return TRUE;

    // Reserve north of safari
    if(GetActiveHubMap()->areaCoords[HUB_AREA_SAFARI_ZONE].x == x && GetActiveHubMap()->areaCoords[HUB_AREA_SAFARI_ZONE].y + 1 == y)
        return TRUE;

    return FALSE;
}

struct Coords8 RogueHub_GetAreaCoords(u8 area)
{
    AGB_ASSERT(RogueHub_HasAreaBuilt(area));
    return GetActiveHubMap()->areaCoords[area];
}

u8 RogueHub_FindAreaInDir(u8 area, u8 connDir)
{
    if(RogueHub_HasAreaBuilt(area))
    {
        struct Coords8 pos;
        pos.x = GetActiveHubMap()->areaCoords[area].x;
        pos.y = GetActiveHubMap()->areaCoords[area].y;
        IncrementCoordsByDirection(&pos, connDir);

        return RogueHub_FindAreaAtCoord(pos.x, pos.y);
    }

    return HUB_AREA_NONE;
}

static bool8 CanAreaConnect(u8 area, u8 dir)
{
    // If both warps aren't set to 0, this is a valid warp
    return gRogueHubAreas[area].connectionWarps[dir][0] != 0 || gRogueHubAreas[area].connectionWarps[dir][1] != 0;
}

static u8 InvertConnDirection(u8 dir)
{
    switch (dir)
    {
    case HUB_AREA_CONN_SOUTH:
        return HUB_AREA_CONN_NORTH;
    case HUB_AREA_CONN_NORTH:
        return HUB_AREA_CONN_SOUTH;
    case HUB_AREA_CONN_WEST:
        return HUB_AREA_CONN_EAST;
    case HUB_AREA_CONN_EAST:
        return HUB_AREA_CONN_WEST;
    }

    return dir;
}

bool8 RogueHub_AreaHasFreeConnection(u8 area, u8 dir)
{
    if(CanAreaConnect(area, dir))
    {
        struct Coords8 pos;
        pos.x = GetActiveHubMap()->areaCoords[area].x;
        pos.y = GetActiveHubMap()->areaCoords[area].y;
        IncrementCoordsByDirection(&pos, dir);

        return !IsReservedCoord(pos.x, pos.y) && RogueHub_FindAreaAtCoord(pos.x, pos.y) == HUB_AREA_NONE;
    }

    return FALSE;
}

bool8 RogueHub_CanBuildConnectionBetween(u8 fromArea, u8 toArea, u8 dir)
{
    u8 invDir = InvertConnDirection(dir);
    return RogueHub_AreaHasFreeConnection(fromArea, dir) && CanAreaConnect(toArea, invDir);
}

u8 RogueHub_GetAreaAtConnection(u8 area, u8 dir)
{
    if(CanAreaConnect(area, dir))
    {
        struct Coords8 pos;
        pos.x = GetActiveHubMap()->areaCoords[area].x;
        pos.y = GetActiveHubMap()->areaCoords[area].y;
        IncrementCoordsByDirection(&pos, dir);

        return RogueHub_FindAreaAtCoord(pos.x, pos.y);
    }

    return HUB_AREA_NONE;
}

static u8 GetAreaForLayout(u16 layout)
{
    u8 area;

    switch (layout)
    {
    case LAYOUT_ROGUE_AREA_SAFARI_ZONE_TUTORIAL:
        layout = LAYOUT_ROGUE_AREA_SAFARI_ZONE;
        break;
    }

    for(area = HUB_AREA_FIRST; area < HUB_AREA_COUNT; ++area)
    {
        if(gRogueHubAreas[area].primaryMapLayout == layout)
            return area;
    }

    return HUB_AREA_NONE;
}

u8 RogueHub_GetAreaFromCurrentMap()
{
    return GetAreaForLayout(gMapHeader.mapLayoutId);
}

u16 RogueHub_GetWeatherState()
{
    return GetActiveHubMap()->weatherState;
}

static void BeginQueryForBoxMons(u8 boxId)
{
    u8 i;

    RogueMiscQuery_EditRange(QUERY_FUNC_EXCLUDE, 0, IN_BOX_COUNT - 1);

    for(i = 0; i < IN_BOX_COUNT; ++i)
    {
        if(GetBoxMonDataAt(boxId, i, MON_DATA_SPECIES) != SPECIES_NONE && GetBoxMonDataAt(boxId, i, MON_DATA_SPECIES_OR_EGG) != SPECIES_EGG)
        {
            RogueMiscQuery_EditElement(QUERY_FUNC_INCLUDE, i);
        }
    }
}

void RogueHub_UpdateWanderMons()
{
    u8 i;
    u16 pos;
    u16 boxId = 0;
    u8 monCount = 0;

    RogueCustomQuery_Begin();

    for(i = 0; i < TOTAL_BOXES_COUNT && monCount < HUB_WANDER_MON_COUNT; ++i)
    {
        // Prioritise mons in box 10 first then work right
        boxId = (TOTAL_BOXES_COUNT - 1 + i) % TOTAL_BOXES_COUNT;

        BeginQueryForBoxMons(boxId);

        for(; monCount < HUB_WANDER_MON_COUNT;)
        {
            if(!RogueMiscQuery_AnyActiveStates(0, IN_BOX_COUNT - 1))
                break;

            pos = RogueMiscQuery_SelectRandomElement(Random());
            RogueMiscQuery_EditElement(QUERY_FUNC_EXCLUDE, pos);

            BoxMonAtToMon(boxId, pos, &gEnemyParty[0]);
            gRogueSaveBlock->hubMap.homeWanderingMonSpecies[monCount++] = FollowMon_GetMonGraphics(&gEnemyParty[0]);
        }
    }

    // Fill remaining slots with empty
    for(; monCount < HUB_WANDER_MON_COUNT; ++monCount)
        gRogueSaveBlock->hubMap.homeWanderingMonSpecies[monCount] = SPECIES_NONE;

    RogueCustomQuery_End();
}

void RogueHub_OnNewDayStarted()
{
    //RogueHub_UpdateWeatherState
}

void RogueHub_UpdateWeatherState()
{
    // Ignore this if client
    if(RogueMP_IsActive() && !RogueMP_IsHost())
        return;

    gRogueSaveBlock->hubMap.weatherState = Random();
}

u8 const* RogueHub_GetHubName()
{
    if(RogueMP_IsActive() && !RogueMP_IsHost())
    {
        return RogueMP_GetPlayerHubName(RogueMP_GetRemotePlayerId());
    }

    return gSaveBlock2Ptr->pokemonHubName;
}

u8 RogueHub_GetHubVariantNumber()
{
    if(RogueMP_IsActive() && !RogueMP_IsHost())
    {
        return RogueMP_GetPlayerTrainerId(RogueMP_GetRemotePlayerId())[0];
    }

    return gSaveBlock2Ptr->playerTrainerId[0];
}

void RogueHub_ModifyMapWarpEvent(struct MapHeader *mapHeader, u8 warpId, struct WarpEvent *warp)
{
    u8 area = GetAreaForLayout(mapHeader->mapLayoutId);

    if(area != HUB_AREA_NONE)
    {
        u8 dir;
        u8 warpArea = HUB_AREA_NONE;
        u8 enterDir = HUB_AREA_CONN_SOUTH;

        for(dir = HUB_AREA_CONN_SOUTH; dir <= HUB_AREA_CONN_EAST; ++dir)
        {
            if(CanAreaConnect(area, dir))
            {
                if(gRogueHubAreas[area].connectionWarps[dir][0] == warpId || gRogueHubAreas[area].connectionWarps[dir][1] == warpId)
                    break;
            }
        }

        // We're trying to warp out into a valid direction
        if(dir <= HUB_AREA_CONN_EAST)
        {
            warpArea = RogueHub_GetAreaAtConnection(area, dir);
            enterDir = InvertConnDirection(dir);
        }

        // Warping into a valid area, so setup a valid warp
        if(warpArea != HUB_AREA_NONE)
        {
            warp->mapGroup = gRogueHubAreas[warpArea].primaryMapGroup;
            warp->mapNum = gRogueHubAreas[warpArea].primaryMapNum;
            warp->warpId = gRogueHubAreas[warpArea].connectionWarps[enterDir][0];

            if(gRogueHubAreas[warpArea].primaryMapLayout == LAYOUT_ROGUE_AREA_SAFARI_ZONE && VarGet(VAR_ROGUE_INTRO_STATE) == ROGUE_INTRO_STATE_CATCH_MON)
            {
                warp->mapGroup = MAP_GROUP(ROGUE_AREA_SAFARI_ZONE_TUTORIAL);
                warp->mapNum = MAP_NUM(ROGUE_AREA_SAFARI_ZONE_TUTORIAL);
            }
        }
        else
        {
            // TODO - We should do safe warp back to main area if we get here
        }
    }
}

bool8 RogueHub_AcceptMapConnection(struct MapHeader *mapHeader, const struct MapConnection *connection)
{
    // Convert from CONNECTION_ to HUB_AREA_CONN_
    u8 area = GetAreaForLayout(mapHeader->mapLayoutId);

    if(area != HUB_AREA_NONE)
    {
        u8 connDir = connection->direction - 1;
        
        // Hide any connections, unless we actually have the connection built
        return RogueHub_GetAreaAtConnection(area, connDir) != HUB_AREA_NONE;
    }

    return TRUE;
}

void RogueHub_ApplyMapMetatiles()
{
    bool8 applyCommonFixup = TRUE;

    switch (gMapHeader.mapLayoutId)
    {
    case LAYOUT_ROGUE_AREA_LABS:
        RogueHub_UpdateLabsAreaMetatiles();
        break;
    case LAYOUT_ROGUE_AREA_ADVENTURE_ENTRANCE:
        RogueHub_UpdateAdventureEntranceAreaMetatiles();
        break;

    case LAYOUT_ROGUE_AREA_HOME:
        applyCommonFixup = FALSE;
        RogueHub_UpdateHomeAreaMetatiles();
        break;
    case LAYOUT_ROGUE_INTERIOR_HOME:
        applyCommonFixup = FALSE;
        RogueHub_UpdateHomeInteriorMetatiles();
        break;

    case LAYOUT_ROGUE_AREA_FARMING_FIELD:
        RogueHub_UpdateFarmingAreaMetatiles();
        break;

    case LAYOUT_ROGUE_AREA_SAFARI_ZONE:
    case LAYOUT_ROGUE_AREA_SAFARI_ZONE_TUTORIAL:
        RogueHub_UpdateSafariAreaMetatiles();
        break;

    case LAYOUT_ROGUE_AREA_RIDE_TRAINING:
        RogueHub_UpdateRideTrainingAreaMetatiles();
        break;

    case LAYOUT_ROGUE_AREA_MARTS:
        RogueHub_UpdateMartsAreaMetatiles();
        break;

    case LAYOUT_ROGUE_AREA_TOWN_SQUARE:
        RogueHub_UpdateTownSquareAreaMetatiles();
        break;

    case LAYOUT_ROGUE_AREA_MARKET:
        RogueHub_UpdateMarketAreaMetatiles();
        break;

    case LAYOUT_ROGUE_AREA_CHALLENGE_FRONTIER:
        RogueHub_UpdateChallengeFrontierAreaMetatiles();
        break;

    case LAYOUT_ROGUE_AREA_DAY_CARE:
        RogueHub_UpdateDayCareAreaMetatiles();
        break;
    
    default:
        break;
    }

    if(applyCommonFixup)
    {
        struct TileFixup fixup;
        fixup.path = TRUE;
        fixup.pond = FALSE;
        fixup.mountain = FALSE;
        fixup.trees = FALSE;
        fixup.pathStyle = TRUE;
        FixupTileCommon(&fixup);
    }
}

static void UpdateStatueLevel()
{
    if(Rogue_Use200PercEffects())
        gRogueSaveBlock->hubMap.statueLevel = 3;
    else if(Rogue_Use100PercEffects())
        gRogueSaveBlock->hubMap.statueLevel = 1;
    else if(RogueQuest_HasCollectedRewards(QUEST_ID_CHAMPION))
        gRogueSaveBlock->hubMap.statueLevel = 1;
    else
        gRogueSaveBlock->hubMap.statueLevel = 0;
}

void RogueHub_UpdateWarpStates()
{
    UpdateStatueLevel();
}

u8 RogueHub_GetStatueLevel()
{
    UpdateStatueLevel();
    return GetActiveHubMap()->statueLevel;
}

static void RogueHub_UpdateLabsAreaMetatiles()
{
    // Remove connectionss
    if(RogueHub_GetAreaAtConnection(HUB_AREA_LABS, HUB_AREA_CONN_NORTH) == HUB_AREA_NONE)
    {
        MetatileFill_TreesOverlapping(11, 0, 16, 0, TREE_TYPE_DENSE);
        MetatileFill_TreeStumps(11, 1, 16, TREE_TYPE_DENSE);

        MetatileFill_CommonPathRemoval(12, 2, 15, 7);
    }

    if(RogueHub_GetAreaAtConnection(HUB_AREA_LABS, HUB_AREA_CONN_EAST) == HUB_AREA_NONE)
    {
        MetatileFill_CommonWarpExitHorizontal(26, 7);

        MetatileFill_CommonPathRemoval(22, 8, 25, 10);
    }

    if(RogueHub_GetAreaAtConnection(HUB_AREA_LABS, HUB_AREA_CONN_SOUTH) == HUB_AREA_NONE)
    {
        MetatileFill_CommonWarpExitVertical(12, 12);
        MetatileFill_TreeCaps(12, 13, 15);
        
        MetatileFill_TreesOverlapping(12, 14, 15, 23, TREE_TYPE_DENSE);

        MetatileFill_CommonPathRemoval(12, 11, 15, 11);
    }

    if(RogueHub_GetAreaAtConnection(HUB_AREA_LABS, HUB_AREA_CONN_WEST) == HUB_AREA_NONE)
    {
        MetatileFill_CommonWarpExitHorizontal(0, 7);

        MetatileFill_CommonPathRemoval(2, 8, 5, 10);
    }

    // Remove unique mon lab
    if(!RogueHub_HasUpgrade(HUB_UPGRADE_LAB_UNIQUE_MON_LAB))
    {
        MetatileFill_TreesOverlapping(0, 12, 9, 19, TREE_TYPE_DENSE);

        MetatileFill_TreesOverlapping(9, 12, 9, 12, TREE_TYPE_SPARSE);

        MetatileFill_TreeCaps(8, 11, 9);

        MetatileFill_Tile(10, 11, 10, 11, METATILE_General_Grass);
    }

#ifdef ROGUE_DEBUG
    MetatileSet_Tile(23, 6, METATILE_Petalburg_Door_BirchsLab);
#endif
}

static void RogueHub_UpdateAdventureEntranceAreaMetatiles()
{
    // Remove connectionss
    if(RogueHub_GetAreaAtConnection(HUB_AREA_ADVENTURE_ENTRANCE, HUB_AREA_CONN_EAST) == HUB_AREA_NONE)
    {
        MetatileFill_CommonWarpExitHorizontal(18, 9);

        MetatileFill_CommonPathRemoval(12, 10, 17, 12);
    }

    if(RogueHub_GetAreaAtConnection(HUB_AREA_ADVENTURE_ENTRANCE, HUB_AREA_CONN_SOUTH) == HUB_AREA_NONE)
    {
        MetatileFill_CommonWarpExitVertical(8, 14);
        MetatileFill_TreeCaps(8, 15, 11);

        MetatileFill_CommonPathRemoval(8, 13, 11, 13);
    }

    if(RogueHub_GetAreaAtConnection(HUB_AREA_ADVENTURE_ENTRANCE, HUB_AREA_CONN_WEST) == HUB_AREA_NONE)
    {
        MetatileFill_CommonWarpExitHorizontal(0, 9);

        MetatileFill_CommonPathRemoval(2, 10, 7, 12);
    }
}

static void BlitPlayerHomeRegion(u16 region, u16 style)
{
    AGB_ASSERT(region < HOME_REGION_COUNT);
    AGB_ASSERT(style < HOME_AREA_STYLE_COUNT);

    if(style != HOME_AREA_STYLE_OVERGROWN) // HOME_AREA_STYLE_OVERGROWN is default
    {
        MetatileFill_BlitMapRegion(
            sHomeAreaStyles[style].group, sHomeAreaStyles[style].num,
            sHomeRegionCoords[region].xStart, sHomeRegionCoords[region].yStart, sHomeRegionCoords[region].xEnd, sHomeRegionCoords[region].yEnd,
            sHomeRegionCoords[region].xStart, sHomeRegionCoords[region].yStart
        );
    }
}

static void BlitPlayerHouse(u16 style, bool8 isUpgraded)
{
    u16 const width = (sHomeRegionCoords[HOME_REGION_HOUSE].xEnd - sHomeRegionCoords[HOME_REGION_HOUSE].xStart + 1);

    if(RogueHub_HasUpgrade(HUB_UPGRADE_HOME_LOWER_FLOOR))
    {
        AGB_ASSERT(style < HOME_BUILDING_STYLE_COUNT);

        MetatileFill_BlitMapRegion(
            MAP_GROUP(ROGUE_TEMPLATE_HOMES), MAP_NUM(ROGUE_TEMPLATE_HOMES),
            sHomeRegionCoords[HOME_REGION_HOUSE].xStart, 
            sHomeRegionCoords[HOME_REGION_HOUSE].yStart, 
            sHomeRegionCoords[HOME_REGION_HOUSE].xEnd, 
            sHomeRegionCoords[HOME_REGION_HOUSE].yEnd - 1, // bottom tile is just to stop things being placed too close
            width * (1 + style * 2 + (isUpgraded ? 0 : 1)), 0
        );
    }
    else
    {
        // Blit the empty plot
        MetatileFill_BlitMapRegion(
            MAP_GROUP(ROGUE_TEMPLATE_HOMES), MAP_NUM(ROGUE_TEMPLATE_HOMES),
            sHomeRegionCoords[HOME_REGION_HOUSE].xStart, 
            sHomeRegionCoords[HOME_REGION_HOUSE].yStart, 
            sHomeRegionCoords[HOME_REGION_HOUSE].xEnd, 
            sHomeRegionCoords[HOME_REGION_HOUSE].yEnd - 1, // bottom tile is just to stop things being placed too close
            0,0
        );
    }
}

static u8 GetCurrentPlaceableRegion()
{
    switch (gMapHeader.mapLayoutId)
    {
    case LAYOUT_ROGUE_AREA_HOME:
        return HOME_REGION_PLACEABLE_REGION;
    
    case LAYOUT_ROGUE_INTERIOR_HOME:
        return HOME_REGION_PLACEABLE_REGION_INTERIOR;

    default:
        AGB_ASSERT(FALSE);
        return HOME_REGION_PLACEABLE_REGION;
        break;
    }
}

static void BlitPlayerHouseEnvDecor(s32 x, s32 y, u16 decorVariant)
{
    u8 placeableRegion = 0;
    u8 xStart = sDecorationVariants[decorVariant].perType.tile.x;
    u8 yStart = sDecorationVariants[decorVariant].perType.tile.y;
    u8 xEnd = xStart + sDecorationVariants[decorVariant].perType.tile.width - 1;
    u8 yEnd = yStart + sDecorationVariants[decorVariant].perType.tile.height - 1;

    AGB_ASSERT(decorVariant < DECOR_VARIANT_COUNT);
    AGB_ASSERT(sDecorationVariants[decorVariant].type == DECOR_TYPE_TILE);

    // Clip anything which is outside of the placeable region
    placeableRegion = GetCurrentPlaceableRegion();


    if(x < sHomeRegionCoords[placeableRegion].xStart)
    {
        u8 delta = sHomeRegionCoords[placeableRegion].xStart - x;
        x = sHomeRegionCoords[placeableRegion].xStart;
        xStart += delta;
    }

    if(y < sHomeRegionCoords[placeableRegion].yStart)
    {
        u8 delta = sHomeRegionCoords[placeableRegion].yStart - y;
        y = sHomeRegionCoords[placeableRegion].yStart;
        yStart += delta;
    }

    if(x + (xEnd - xStart) > sHomeRegionCoords[placeableRegion].xEnd)
    {
        u8 delta = x + (xEnd - xStart) - sHomeRegionCoords[placeableRegion].xEnd;
        xEnd -= delta;
    }

    if(y + (yEnd - yStart) > sHomeRegionCoords[placeableRegion].yEnd)
    {
        u8 delta = y + (yEnd - yStart) - sHomeRegionCoords[placeableRegion].yEnd;
        yEnd -= delta;
    }

    MetatileFill_BlitMapRegion(
        sDecorationVariants[decorVariant].srcMapGroup, sDecorationVariants[decorVariant].srcMapNum,
        x, y, 
        x + (xEnd - xStart), y + (yEnd - yStart),
        xStart, yStart
    );
}

static void RogueHub_UpdateHomeAreaMetatiles()
{
    // Blit map styles first
    u8 i;
    struct RogueHubMap* hubMap = GetActiveHubMap();

    RogueHub_PlaceHomeEnvironmentDecorations(TRUE, FALSE);

    // Remove connections
    if(RogueHub_GetAreaAtConnection(HUB_AREA_HOME, HUB_AREA_CONN_NORTH) == HUB_AREA_NONE)
    {
        MetatileFill_TreesOverlapping(15, 0, 20, 0, TREE_TYPE_DENSE);
        MetatileFill_TreeStumps(15, 1, 20, TREE_TYPE_DENSE);
    }

    if(RogueHub_GetAreaAtConnection(HUB_AREA_HOME, HUB_AREA_CONN_EAST) == HUB_AREA_NONE)
    {
        MetatileFill_TreesOverlapping(34, 15, 35, 20, TREE_TYPE_DENSE);
    }

    if(RogueHub_GetAreaAtConnection(HUB_AREA_HOME, HUB_AREA_CONN_SOUTH) == HUB_AREA_NONE)
    {
        MetatileFill_TreesOverlapping(15, 34, 20, 35, TREE_TYPE_DENSE);
        MetatileFill_TreeCaps(16, 33, 19);
    }

    if(RogueHub_GetAreaAtConnection(HUB_AREA_HOME, HUB_AREA_CONN_WEST) == HUB_AREA_NONE)
    {
        MetatileFill_TreesOverlapping(0, 15, 1, 20, TREE_TYPE_DENSE);
    }
}

static bool8 IsBottomLayerVariant(u16 decorVariant)
{
    AGB_ASSERT(sDecorationVariants[decorVariant].layer < DECOR_LAYER_COUNT);
    return sDecorLayers[sDecorationVariants[decorVariant].layer].placeInBackground;
}

static u16 GetCurrentDecorOffset(u16 i)
{
    switch (gMapHeader.mapLayoutId)
    {
    case LAYOUT_ROGUE_AREA_HOME:
        return HOME_DECOR_OUTSIDE_OFFSET + i;
    
    case LAYOUT_ROGUE_INTERIOR_HOME:
        return HOME_DECOR_INSIDE_OFFSET + i;
    }

    AGB_ASSERT(FALSE);
    return 0;
}

static u16 GetCurrentDecorCount()
{
    switch (gMapHeader.mapLayoutId)
    {
    case LAYOUT_ROGUE_AREA_HOME:
        return HOME_DECOR_OUTSIDE_COUNT;
    
    case LAYOUT_ROGUE_INTERIOR_HOME:
        return HOME_DECOR_INSIDE_COUNT;
    }

    AGB_ASSERT(FALSE);
    return 0;
}


static void RogueHub_PlaceHomeEnvironmentDecorations(bool8 placeTiles, bool8 placeObjects)
{
    u8 i;
    struct RogueHubMap* hubMap = GetActiveHubMap();
    u16 currDecorCount = GetCurrentDecorCount();

    if(placeTiles)
    {
        // Reset to default state
        switch (gMapHeader.mapLayoutId)
        {
        case LAYOUT_ROGUE_AREA_HOME:
            MetatileFill_BlitMapRegion(
                MAP_GROUP(ROGUE_AREA_HOME), MAP_NUM(ROGUE_AREA_HOME),
                sHomeRegionCoords[HOME_REGION_PLACEABLE_REGION].xStart, sHomeRegionCoords[HOME_REGION_PLACEABLE_REGION].yStart,
                sHomeRegionCoords[HOME_REGION_PLACEABLE_REGION].xEnd, sHomeRegionCoords[HOME_REGION_PLACEABLE_REGION].yEnd,
                sHomeRegionCoords[HOME_REGION_PLACEABLE_REGION].xStart, sHomeRegionCoords[HOME_REGION_PLACEABLE_REGION].yStart
            );
            break;

        case LAYOUT_ROGUE_INTERIOR_HOME:
            MetatileFill_BlitMapRegion(
                MAP_GROUP(ROGUE_INTERIOR_HOME), MAP_NUM(ROGUE_INTERIOR_HOME),
                sHomeRegionCoords[HOME_REGION_PLACEABLE_REGION_INTERIOR].xStart, sHomeRegionCoords[HOME_REGION_PLACEABLE_REGION_INTERIOR].yStart,
                sHomeRegionCoords[HOME_REGION_PLACEABLE_REGION_INTERIOR].xEnd, sHomeRegionCoords[HOME_REGION_PLACEABLE_REGION_INTERIOR].yEnd,
                sHomeRegionCoords[HOME_REGION_PLACEABLE_REGION_INTERIOR].xStart, sHomeRegionCoords[HOME_REGION_PLACEABLE_REGION_INTERIOR].yStart
            );
            break;
        
        default:
            AGB_ASSERT(FALSE);
            break;
        }

        // Place all of the bottom layers first
        for(i = 0; i < currDecorCount; ++i)
        {
            struct RogueHubDecoration* decor = &hubMap->homeDecorations[GetCurrentDecorOffset(i)];
            if(decor->active && sDecorationVariants[decor->decorVariant].type == DECOR_TYPE_TILE && IsBottomLayerVariant(decor->decorVariant))
                BlitPlayerHouseEnvDecor(decor->x, decor->y, decor->decorVariant);
        }

        // Place all others
        for(i = 0; i < currDecorCount; ++i)
        {
            struct RogueHubDecoration* decor = &hubMap->homeDecorations[GetCurrentDecorOffset(i)];
            if(decor->active && sDecorationVariants[decor->decorVariant].type == DECOR_TYPE_TILE && !IsBottomLayerVariant(decor->decorVariant))
                BlitPlayerHouseEnvDecor(decor->x, decor->y, decor->decorVariant);
        }

        // House specific behaviour
        if(gMapHeader.mapLayoutId == LAYOUT_ROGUE_AREA_HOME)
        {
            // Fill house area with grass to avoid overlap from stuff getting placed in this region
            MetatileFill_Tile(
                sHomeRegionCoords[HOME_REGION_HOUSE].xStart, sHomeRegionCoords[HOME_REGION_HOUSE].yStart,
                sHomeRegionCoords[HOME_REGION_HOUSE].xEnd, sHomeRegionCoords[HOME_REGION_HOUSE].yEnd,
                METATILE_General_Grass
            );

            // Replace this now, but need to tell fixup to ignore it
            BlitPlayerHouse(hubMap->homeStyles[HOME_STYLE_HOUSE_EXTERIOR], RogueHub_HasUpgrade(HUB_UPGRADE_HOME_UPPER_FLOOR));

            // Fixup connecting tiles
            {
                struct TileFixup fixup;
                fixup.path = TRUE;
                fixup.pond = TRUE;
                fixup.mountain = TRUE;
                fixup.trees = TRUE;
                fixup.pathStyle = TRUE;
                FixupTileCommon(&fixup);
            }
        }
    }

    if(placeObjects)
    {
        struct MapHeader const* baseMapHeader = Overworld_GetMapHeaderByGroupAndId(gSaveBlock1Ptr->location.mapGroup, gSaveBlock1Ptr->location.mapNum);

        // We only want to append objects, not overwrite
        gSaveBlock1Ptr->objectEventTemplatesCount = baseMapHeader->events->objectEventCount;
        RogueHub_ModifyPlayerBaseObjectEvents(gMapHeader.mapLayoutId, FALSE, gSaveBlock1Ptr->objectEventTemplates, &gSaveBlock1Ptr->objectEventTemplatesCount, ARRAY_COUNT(gSaveBlock1Ptr->objectEventTemplates));
        TrySpawnObjectEvents(0, 0);
    }
}

static void RogueHub_UpdateHomeInteriorMetatiles()
{
    RogueHub_PlaceHomeEnvironmentDecorations(TRUE, FALSE);
}

static void RogueHub_UpdateFarmingAreaMetatiles()
{
    // Remove connectionss
    if(RogueHub_GetAreaAtConnection(HUB_AREA_BERRY_FIELD, HUB_AREA_CONN_NORTH) == HUB_AREA_NONE)
    {
        MetatileFill_TreesOverlapping(17, 0, 22, 0, TREE_TYPE_DENSE);
        MetatileFill_TreeStumps(17, 1, 22, TREE_TYPE_DENSE);

        MetatileFill_CommonPathRemoval(18, 2, 21, 2);
    }

    if(RogueHub_GetAreaAtConnection(HUB_AREA_BERRY_FIELD, HUB_AREA_CONN_EAST) == HUB_AREA_NONE)
    {
        MetatileFill_TreesOverlapping(38, 5, 39, 9, TREE_TYPE_DENSE);

        MetatileFill_CommonPathRemoval(22, 6, 37, 8);
    }

    if(RogueHub_GetAreaAtConnection(HUB_AREA_BERRY_FIELD, HUB_AREA_CONN_SOUTH) == HUB_AREA_NONE)
    {
        MetatileFill_CommonPathRemoval(18, 9, 21, 9);

        MetatileFill_TreesOverlapping(15, 10, 24, 11, TREE_TYPE_DENSE);
        MetatileFill_TreeCaps(16, 9, 23);
    }

    if(RogueHub_GetAreaAtConnection(HUB_AREA_BERRY_FIELD, HUB_AREA_CONN_WEST) == HUB_AREA_NONE)
    {
        MetatileFill_TreesOverlapping(0, 1, 1, 8, TREE_TYPE_DENSE);

        MetatileFill_CommonPathRemoval(2, 3, 14, 5);
    }

    if(!RogueHub_HasUpgrade(HUB_UPGRADE_BERRY_FIELD_EXTRA_FIELD0))
    {
        MetatileFill_Tile(29, 3, 36, 5, METATILE_GeneralHub_Grass);
    }
    if(!RogueHub_HasUpgrade(HUB_UPGRADE_BERRY_FIELD_EXTRA_FIELD1))
    {
        MetatileFill_Tile(10, 6, 16, 8, METATILE_GeneralHub_Grass);
    }
    if(!RogueHub_HasUpgrade(HUB_UPGRADE_BERRY_FIELD_EXTRA_FIELD2))
    {
        MetatileFill_Tile(2, 6, 8, 8, METATILE_GeneralHub_Grass);
    }
}

static void RogueHub_UpdateSafariAreaMetatiles()
{
    // Remove connectionss
    if(RogueHub_GetAreaAtConnection(HUB_AREA_SAFARI_ZONE, HUB_AREA_CONN_EAST) == HUB_AREA_NONE)
    {
        MetatileFill_CommonWarpExitHorizontal(36, 13);
    }

    if(RogueHub_GetAreaAtConnection(HUB_AREA_SAFARI_ZONE, HUB_AREA_CONN_SOUTH) == HUB_AREA_NONE)
    {
        MetatileFill_CommonWarpExitVertical(16, 31);
        MetatileFill_TreeCaps(16, 31, 19);
    }

    if(RogueHub_GetAreaAtConnection(HUB_AREA_SAFARI_ZONE, HUB_AREA_CONN_WEST) == HUB_AREA_NONE)
    {
        MetatileFill_CommonWarpExitHorizontal(0, 13);
    }

    // Open cave
    if(RogueHub_HasUpgrade(HUB_UPGRADE_SAFARI_ZONE_LEGENDS_CAVE))
    {
        MetatileSet_Tile(18, 4, METATILE_Fallarbor_BrownCaveEntrance_Top);
        MetatileSet_Tile(18, 5, METATILE_Fallarbor_BrownCaveEntrance_Bottom);
    }
}

static void RogueHub_UpdateRideTrainingAreaMetatiles()
{
    // Remove connectionss
    if(RogueHub_GetAreaAtConnection(HUB_AREA_RIDE_TRAINING, HUB_AREA_CONN_NORTH) == HUB_AREA_NONE)
    {
        MetatileFill_TreesOverlapping(20, 0, 23, 8, TREE_TYPE_DENSE);
        MetatileFill_TreeStumps(20, 9, 23, TREE_TYPE_DENSE);

        MetatileFill_CommonPathRemoval(20, 10, 23, 17);
    }

    if(RogueHub_GetAreaAtConnection(HUB_AREA_RIDE_TRAINING, HUB_AREA_CONN_EAST) == HUB_AREA_NONE)
    {
        MetatileFill_CommonWarpExitHorizontal(32, 17);

        MetatileFill_CommonPathRemoval(24, 18, 31, 21);
    }

    if(RogueHub_GetAreaAtConnection(HUB_AREA_RIDE_TRAINING, HUB_AREA_CONN_SOUTH) == HUB_AREA_NONE)
    {
        MetatileFill_CommonWarpExitVertical(20, 34);
        MetatileFill_TreeCaps(20, 35, 23);

        MetatileFill_CommonPathRemoval(20, 22, 23, 33);
    }

    if(RogueHub_GetAreaAtConnection(HUB_AREA_RIDE_TRAINING, HUB_AREA_CONN_WEST) == HUB_AREA_NONE)
    {
        MetatileFill_CommonWarpExitHorizontal(0, 17);

        MetatileFill_CommonPathRemoval(2, 18, 19, 21);
    }
}

static void RogueHub_UpdateMartsAreaMetatiles()
{
    // Remove connectionss
    if(RogueHub_GetAreaAtConnection(HUB_AREA_MARTS, HUB_AREA_CONN_NORTH) == HUB_AREA_NONE)
    {
        MetatileFill_TreesOverlapping(7, 0, 12, 0, TREE_TYPE_DENSE);
        MetatileFill_TreeStumps(7, 1, 12, TREE_TYPE_DENSE);

        MetatileFill_CommonPathRemoval(8, 2, 11, 11);
    }

    if(RogueHub_GetAreaAtConnection(HUB_AREA_MARTS, HUB_AREA_CONN_EAST) == HUB_AREA_NONE)
    {
        MetatileFill_TreesOverlapping(20, 11, 21, 16, TREE_TYPE_DENSE);

        MetatileFill_CommonPathRemoval(17, 12, 19, 15);
    }

    if(RogueHub_GetAreaAtConnection(HUB_AREA_MARTS, HUB_AREA_CONN_SOUTH) == HUB_AREA_NONE)
    {
        MetatileFill_TreesOverlapping(5, 18, 14, 19, TREE_TYPE_DENSE);
        MetatileFill_TreeCaps(5, 17, 19);

        MetatileFill_CommonPathRemoval(8, 16, 11, 16);
    }

    if(RogueHub_GetAreaAtConnection(HUB_AREA_MARTS, HUB_AREA_CONN_WEST) == HUB_AREA_NONE)
    {
        MetatileFill_TreesOverlapping(0, 11, 1, 16, TREE_TYPE_DENSE);

        MetatileFill_CommonPathRemoval(2, 12, 7, 15);
    }

    if(!RogueHub_HasUpgrade(HUB_UPGRADE_MARTS_GENERAL_STOCK))
    {
        MetatileFill_Tile(13, 8, 13, 10, METATILE_GeneralHub_Grass);
    }

    if(!RogueHub_HasUpgrade(HUB_UPGRADE_MARTS_POKE_BALLS) && !RogueHub_HasUpgrade(HUB_UPGRADE_MARTS_TMS))
    {
        MetatileFill_TreesOverlapping(2, 1, 7, 10, TREE_TYPE_DENSE);
        MetatileFill_TreeStumps(1, 11, 6, TREE_TYPE_DENSE);
        MetatileFill_TreeStumps(7, 11, 7, TREE_TYPE_SPARSE);
    }
    else
    {
        if(!RogueHub_HasUpgrade(HUB_UPGRADE_MARTS_POKE_BALLS))
        {
            MetatileFill_TreesOverlapping(1, 8, 7, 10, TREE_TYPE_DENSE);
            MetatileFill_TreeCaps(2, 7, 7);
            MetatileFill_TreeStumps(1, 11, 6, TREE_TYPE_DENSE);
            MetatileFill_TreeStumps(7, 11, 7, TREE_TYPE_SPARSE);
        }
        else if(!RogueHub_HasUpgrade(HUB_UPGRADE_MARTS_POKE_BALLS_STOCK))
        {
            MetatileFill_Tile(2, 7, 3, 10, METATILE_GeneralHub_Grass);
        }

        if(!RogueHub_HasUpgrade(HUB_UPGRADE_MARTS_TMS))
        {
            MetatileFill_TreesOverlapping(2, 1, 7, 4, TREE_TYPE_DENSE);
            MetatileFill_TreeStumps(2, 5, 6, TREE_TYPE_DENSE);
            MetatileFill_TreeStumps(7, 5, 7, TREE_TYPE_SPARSE);

        }
        else if(!RogueHub_HasUpgrade(HUB_UPGRADE_MARTS_TMS_STOCK))
        {
            MetatileFill_Tile(2, 2, 3, 5, METATILE_GeneralHub_Grass);
            MetatileFill_Tile(4, 2, 7, 2, METATILE_GeneralHub_Grass);
        }
    }

    if(!RogueHub_HasUpgrade(HUB_UPGRADE_MARTS_TRAVELER_BATTLE_ENCHANCERS))
    {
        MetatileFill_Tile(13, 2, 18, 5, METATILE_GeneralHub_Grass);
    }
}

static void RogueHub_UpdateTownSquareAreaMetatiles()
{
    // Remove connections
    if(RogueHub_GetAreaAtConnection(HUB_AREA_TOWN_SQUARE, HUB_AREA_CONN_NORTH) == HUB_AREA_NONE)
    {
        MetatileFill_TreesOverlapping(13, 0, 18, 0, TREE_TYPE_DENSE);
        MetatileFill_TreeStumps(13, 1, 18, TREE_TYPE_DENSE);

        MetatileFill_CommonPathRemoval(14, 2, 17, 8);
    }

    if(RogueHub_GetAreaAtConnection(HUB_AREA_TOWN_SQUARE, HUB_AREA_CONN_EAST) == HUB_AREA_NONE)
    {
        MetatileFill_CommonWarpExitHorizontal(26, 9);

        MetatileFill_CommonPathRemoval(23, 10, 25, 12);
    }

    if(RogueHub_GetAreaAtConnection(HUB_AREA_TOWN_SQUARE, HUB_AREA_CONN_SOUTH) == HUB_AREA_NONE)
    {
        MetatileFill_CommonWarpExitVertical(14, 18);
        MetatileFill_TreeCaps(14, 19, 17);

        MetatileFill_CommonPathRemoval(13, 13, 17, 17);
    }

    if(RogueHub_GetAreaAtConnection(HUB_AREA_TOWN_SQUARE, HUB_AREA_CONN_WEST) == HUB_AREA_NONE)
    {
        MetatileFill_CommonWarpExitHorizontal(0, 7);

        MetatileFill_CommonPathRemoval(2, 9, 13, 11);
    }


    if(!RogueHub_HasUpgrade(HUB_UPGRADE_TOWN_SQUARE_SCHOOL))
    {
        MetatileFill_TreesOverlapping(1, 1, 5, 4, TREE_TYPE_DENSE);
        MetatileFill_TreeStumps(1, 5, 4, TREE_TYPE_DENSE);
        MetatileFill_TreeStumps(6, 1, 6, TREE_TYPE_DENSE);
        MetatileFill_TreeStumps(5, 5, 5, TREE_TYPE_SPARSE);

        MetatileFill_Tile(6, 2, 6, 5, METATILE_GeneralHub_Grass);
        MetatileFill_Tile(3, 6, 5, 6, METATILE_GeneralHub_Grass);
    }

    if(!RogueHub_HasUpgrade(HUB_UPGRADE_TOWN_SQUARE_TUTORS))
    {
        MetatileFill_Tile(6, 12, 12, 15, METATILE_GeneralHub_Grass);
        MetatileFill_Tile(9, 16, 12, 16, METATILE_GeneralHub_Grass);
    }
}

static void RogueHub_UpdateMarketAreaMetatiles()
{
    // Remove connections
    if(RogueHub_GetAreaAtConnection(HUB_AREA_MARKET, HUB_AREA_CONN_NORTH) == HUB_AREA_NONE)
    {
        MetatileFill_TreesOverlapping(16, 0, 20, 0, TREE_TYPE_DENSE);
        MetatileFill_TreeStumps(16, 1, 19, TREE_TYPE_DENSE);

        MetatileFill_CommonPathRemoval(16, 2, 19, 11);
    }

    if(RogueHub_GetAreaAtConnection(HUB_AREA_MARKET, HUB_AREA_CONN_EAST) == HUB_AREA_NONE)
    {
        MetatileFill_TreesOverlapping(32, 9, 33, 15, TREE_TYPE_DENSE);

        MetatileFill_CommonPathRemoval(28, 12, 31, 14);
    }

    if(RogueHub_GetAreaAtConnection(HUB_AREA_MARKET, HUB_AREA_CONN_SOUTH) == HUB_AREA_NONE)
    {
        MetatileFill_TreesOverlapping(15, 22, 20, 23, TREE_TYPE_DENSE);
        MetatileFill_TreeCaps(16, 21, 19);

        MetatileFill_CommonPathRemoval(16, 15, 19, 20);
    }

    if(RogueHub_GetAreaAtConnection(HUB_AREA_MARKET, HUB_AREA_CONN_WEST) == HUB_AREA_NONE)
    {
        MetatileFill_CommonWarpExitHorizontal(0, 11);

        MetatileFill_CommonPathRemoval(2, 12, 9, 14);
    }

    if(!RogueHub_HasUpgrade(HUB_UPGRADE_MARKET_BAKERY))
    {
        MetatileFill_TreesOverlapping(0, 1, 7, 8, TREE_TYPE_DENSE);
        MetatileFill_TreeStumps(0, 9, 7, TREE_TYPE_DENSE);

        if(RogueHub_GetAreaAtConnection(HUB_AREA_MARKET, HUB_AREA_CONN_WEST) == HUB_AREA_NONE)
        {
            MetatileFill_TreesOverlapping(0, 9, 7, 15, TREE_TYPE_DENSE);
        }
    }

    if(!RogueHub_HasUpgrade(HUB_UPGRADE_MARKET_TREAT_SHOP))
    {
        if(RogueHub_GetAreaAtConnection(HUB_AREA_MARKET, HUB_AREA_CONN_SOUTH) == HUB_AREA_NONE)
        {
            MetatileFill_TreesOverlapping(10, 16, 22, 21, TREE_TYPE_DENSE);
            MetatileFill_TreeCaps(10, 15, 21);
        }
        else
        {
            MetatileFill_TreesOverlapping(10, 16, 13, 21, TREE_TYPE_DENSE);
            MetatileFill_TreeCaps(10, 15, 13);

            MetatileFill_Tile(14, 16, 14, 19, METATILE_GeneralHub_Grass);
        }
    }

    if(!RogueHub_HasUpgrade(HUB_UPGRADE_MARKET_BANK))
    {
        MetatileFill_TreesOverlapping(22, 1, 38, 8, TREE_TYPE_DENSE);
        MetatileFill_TreeStumps(23, 9, 31, TREE_TYPE_DENSE);
        MetatileFill_TreeStumps(22, 9, 22, TREE_TYPE_SPARSE);

        MetatileFill_Tile(21, 2, 21, 10, METATILE_GeneralHub_Grass);
        MetatileFill_Tile(22, 10, 31, 10, METATILE_GeneralHub_Grass);
        MetatileFill_Tile(23, 11, 28, 11, METATILE_GeneralHub_Grass);

        if(RogueHub_GetAreaAtConnection(HUB_AREA_MARKET, HUB_AREA_CONN_EAST) == HUB_AREA_NONE)
        {
            MetatileFill_TreesOverlapping(22, 9, 31, 16, TREE_TYPE_DENSE);

            MetatileFill_CommonPathRemoval(20, 12, 21, 14);
        }
    }
}

static void RogueHub_UpdateChallengeFrontierAreaMetatiles()
{
    // Remove connectionss
    if(RogueHub_GetAreaAtConnection(HUB_AREA_CHALLENGE_FRONTIER, HUB_AREA_CONN_EAST) == HUB_AREA_NONE && RogueHub_GetAreaAtConnection(HUB_AREA_CHALLENGE_FRONTIER, HUB_AREA_CONN_NORTH) == HUB_AREA_NONE)
    {
        MetatileFill_TreesOverlapping(30, 7, 37, 19, TREE_TYPE_DENSE);

        MetatileFill_Tile(28, 12, 28, 17, 0x075 | MAPGRID_COLLISION_MASK);
        MetatileFill_Tile(29, 12, 29, 17, 0x072 | MAPGRID_COLLISION_MASK);

        MetatileFill_CommonPathRemoval(21, 13, 27, 15);
    }
    else
    {
        if(RogueHub_GetAreaAtConnection(HUB_AREA_CHALLENGE_FRONTIER, HUB_AREA_CONN_NORTH) == HUB_AREA_NONE)
        {
            MetatileFill_TreesOverlapping(32, 0, 35, 12, TREE_TYPE_DENSE);
            MetatileFill_TreeStumps(32, 13, 35, TREE_TYPE_DENSE);

            MetatileFill_CommonPathRemoval(32, 14, 35, 14);
        }

        if(RogueHub_GetAreaAtConnection(HUB_AREA_CHALLENGE_FRONTIER, HUB_AREA_CONN_EAST) == HUB_AREA_NONE)
        {
            MetatileFill_TreesOverlapping(38, 13, 39, 18, TREE_TYPE_DENSE);

            MetatileFill_CommonPathRemoval(36, 15, 37, 17);
        }
    }

    if(RogueHub_GetAreaAtConnection(HUB_AREA_CHALLENGE_FRONTIER, HUB_AREA_CONN_SOUTH) == HUB_AREA_NONE && RogueHub_GetAreaAtConnection(HUB_AREA_CHALLENGE_FRONTIER, HUB_AREA_CONN_WEST) == HUB_AREA_NONE)
    {
        MetatileFill_TreesOverlapping(2, 13, 9, 25, TREE_TYPE_DENSE);

        MetatileFill_Tile(10, 13, 10, 17, 0x070 | MAPGRID_COLLISION_MASK);
        MetatileFill_Tile(11, 13, 11, 17, 0x073 | MAPGRID_COLLISION_MASK);

        MetatileFill_CommonPathRemoval(12, 13, 18, 15);
    }
    else
    {
        if(RogueHub_GetAreaAtConnection(HUB_AREA_CHALLENGE_FRONTIER, HUB_AREA_CONN_SOUTH) == HUB_AREA_NONE)
        {
            MetatileFill_TreeCaps(4, 19, 7);
            MetatileFill_TreesOverlapping(4, 20, 7, 25, TREE_TYPE_DENSE);

            MetatileFill_CommonPathRemoval(4, 18, 7, 18);
        }

        if(RogueHub_GetAreaAtConnection(HUB_AREA_CHALLENGE_FRONTIER, HUB_AREA_CONN_WEST) == HUB_AREA_NONE)
        {
            MetatileFill_TreesOverlapping(0, 13, 1, 18, TREE_TYPE_DENSE);

            MetatileFill_CommonPathRemoval(2, 15, 3, 17);
        }
    }
}

static void RogueHub_UpdateDayCareAreaMetatiles()
{
    // Remove connectionss
    if(RogueHub_GetAreaAtConnection(HUB_AREA_DAY_CARE, HUB_AREA_CONN_NORTH) == HUB_AREA_NONE)
    {
        MetatileFill_TreesOverlapping(17, 0, 22, 0, TREE_TYPE_DENSE);
        MetatileFill_TreeStumps(17, 1, 22, TREE_TYPE_DENSE);

        MetatileFill_CommonPathRemoval(18, 2, 21, 11);
    }

    if(RogueHub_GetAreaAtConnection(HUB_AREA_DAY_CARE, HUB_AREA_CONN_EAST) == HUB_AREA_NONE)
    {
        MetatileFill_CommonWarpExitHorizontal(30, 11);

        MetatileFill_CommonPathRemoval(22, 12, 29, 14);
    }

    if(RogueHub_GetAreaAtConnection(HUB_AREA_DAY_CARE, HUB_AREA_CONN_SOUTH) == HUB_AREA_NONE)
    {
        MetatileFill_TreesOverlapping(17, 16, 22, 17, TREE_TYPE_DENSE);
        MetatileFill_TreeCaps(18, 15, 21);
    }

    if(RogueHub_GetAreaAtConnection(HUB_AREA_DAY_CARE, HUB_AREA_CONN_WEST) == HUB_AREA_NONE)
    {
        MetatileFill_CommonWarpExitHorizontal(0, 11);

        MetatileFill_CommonPathRemoval(2, 12, 4, 14);
    }

    if(!RogueHub_HasUpgrade(HUB_UPGRADE_DAY_CARE_BREEDER))
    {
        MetatileFill_Tile(10, 10, 10, 10, 0x291 | MAPGRID_COLLISION_MASK); // place wooden fence
    }

    if(!RogueHub_HasUpgrade(HUB_UPGRADE_DAY_CARE_TEA_SHOP))
    {
        // Default to remove and look nice-ish
        MetatileFill_Tile(24, 6, 29, 9, METATILE_GeneralHub_Grass);

        MetatileFill_TreesOverlapping(26, 5, 39, 6, TREE_TYPE_DENSE);
        MetatileFill_TreeStumps(27, 7, 29, TREE_TYPE_DENSE);
        MetatileFill_TreeStumps(26, 7, 26, TREE_TYPE_SPARSE);

        // Place big block of trees instead
        if(RogueHub_GetAreaAtConnection(HUB_AREA_DAY_CARE, HUB_AREA_CONN_NORTH) == HUB_AREA_NONE)
        {
            MetatileFill_TreesOverlapping(18, 0, 29, 8, TREE_TYPE_DENSE);
            MetatileFill_TreeStumps(18, 9, 29, TREE_TYPE_DENSE);
        }

        // Place big block of trees instead
        if(RogueHub_GetAreaAtConnection(HUB_AREA_DAY_CARE, HUB_AREA_CONN_EAST) == HUB_AREA_NONE)
        {
            MetatileFill_TreesOverlapping(24, 5, 31, 15, TREE_TYPE_DENSE);
        }
    }
}

// Metatile util functions
//

static void MetatileSet_Tile(u16 x, u16 y, u16 tile)
{
    MapGridSetMetatileIdAt(x + MAP_OFFSET, y + MAP_OFFSET, tile);
}

static void UNUSED MetatileFill_Tile(u16 xStart, u16 yStart, u16 xEnd, u16 yEnd, u16 tile)
{
    u16 x, y;

    for(x = xStart; x <= xEnd; ++x)
    {
        for(y = yStart; y <= yEnd; ++y)
        {
            MapGridSetMetatileIdAt(x + MAP_OFFSET, y + MAP_OFFSET, tile);
        }
    }
}

static void MetatileFill_TreesOverlapping(u16 xStart, u16 yStart, u16 xEnd, u16 yEnd, u8 treeType)
{
    u16 x, y, tile;

    for(x = xStart; x <= xEnd; ++x)
    {
        for(y = yStart; y <= yEnd; ++y)
        {
            if(treeType == TREE_TYPE_DENSE)
            {
                if((y % 2) == 0)
                {
                    tile = (x % 2) == 0 ? METATILE_GeneralHub_Tree_TopLeft_Dense : METATILE_GeneralHub_Tree_TopRight_Dense;
                }
                else
                {
                    tile = (x % 2) == 0 ? METATILE_GeneralHub_Tree_BottomLeft_Dense_Overlapped : METATILE_GeneralHub_Tree_BottomRight_Dense_Overlapped;
                }
            }
            else
            {
                if((y % 2) == 0)
                {
                    tile = (x % 2) == 0 ? METATILE_GeneralHub_Tree_TopLeft_Sparse : METATILE_GeneralHub_Tree_TopRight_Sparse;
                }
                else
                {
                    tile = (x % 2) == 0 ? METATILE_GeneralHub_Tree_BottomLeft_Sparse_Overlapped : METATILE_GeneralHub_Tree_BottomRight_Sparse_Overlapped;
                }
            }

            MapGridSetMetatileIdAt(x + MAP_OFFSET, y + MAP_OFFSET, tile | MAPGRID_COLLISION_MASK);
        }
    }
}

static void MetatileFill_TreeStumps(u16 xStart, u16 yStart, u16 xEnd, u8 treeType)
{
    u16 x, tile;

    for(x = xStart; x <= xEnd; ++x)
    {
        if(treeType == TREE_TYPE_DENSE)
            tile = (x % 2) == 0 ? METATILE_GeneralHub_Tree_BottomLeft_Dense : METATILE_GeneralHub_Tree_BottomRight_Dense;
        else
            tile = (x % 2) == 0 ? METATILE_GeneralHub_Tree_BottomLeft_Sparse : METATILE_GeneralHub_Tree_BottomRight_Sparse;

        MapGridSetMetatileIdAt(x + MAP_OFFSET, yStart + MAP_OFFSET, tile | MAPGRID_COLLISION_MASK);
    }
}

static void MetatileFill_TreeCaps(u16 xStart, u16 yStart, u16 xEnd)
{
    u16 x, tile;

    for(x = xStart; x <= xEnd; ++x)
    {
        tile = (x % 2) == 0 ? METATILE_GeneralHub_Tree_TopLeft_CapGrass : METATILE_GeneralHub_Tree_TopRight_CapGrass;
        MapGridSetMetatileIdAt(x + MAP_OFFSET, yStart + MAP_OFFSET, tile);
    }
}

static void MetatileFill_CommonWarpExit(u16 xStart, u16 yStart, u16 xEnd, u16 yEnd)
{
    u16 x, y;
    u32 tile;

    for(x = xStart; x <= xEnd; ++x)
    {
        for(y = yStart; y <= yEnd; ++y)
        {
            tile = MapGridGetMetatileIdAt(x + MAP_OFFSET, y + MAP_OFFSET);

            switch (tile)
            {
            // Tree tops
            case 0x00E:
            case 0x00F:
            case 0x03E:
            case 0x03F:
            case 0x047:
                MapGridSetMetatileIdAt(x + MAP_OFFSET, y + MAP_OFFSET, METATILE_GeneralHub_Grass);
                break;

            // Tree bottoms
            case 0x016:
            case 0x017:
            case 0x03D:
                MapGridSetMetatileIdAt(x + MAP_OFFSET, y + MAP_OFFSET, METATILE_GeneralHub_Grass);
                break;

            // Tree overlap tree left
            case 0x1F4:
                MapGridSetMetatileIdAt(x + MAP_OFFSET, y + MAP_OFFSET, 0x1CE);
                break;
            // Tree overlap tree right
            case 0x1F5:
                MapGridSetMetatileIdAt(x + MAP_OFFSET, y + MAP_OFFSET, 0x1CF);
                break;

            // Grass warps/shadows
            case METATILE_GeneralHub_GrassWarpNorth:
            case METATILE_GeneralHub_GrassWarpEast:
            case METATILE_GeneralHub_GrassWarpSouth:
            case METATILE_GeneralHub_GrassWarpWest:
            case METATILE_GeneralHub_GrassWarpShadowNorth:
            case METATILE_GeneralHub_GrassWarpShadowEast:
            case METATILE_GeneralHub_GrassWarpShadowSouth:
            case METATILE_GeneralHub_GrassWarpShadowWest:
                MapGridSetMetatileIdAt(x + MAP_OFFSET, y + MAP_OFFSET, METATILE_GeneralHub_Grass);
                break;
            
            default:
                break;
            }
        }
    }
}

static void MetatileFill_CommonWarpExitVertical(u16 xStart, u16 yStart)
{
    MetatileFill_CommonWarpExit(xStart, yStart, xStart + 3, yStart + 1);
}

static void MetatileFill_CommonWarpExitHorizontal(u16 xStart, u16 yStart)
{
    MetatileFill_CommonWarpExit(xStart, yStart, xStart + 1, yStart + 4);
}

static void MetatileFill_CommonPathRemoval(u16 xStart, u16 yStart, u16 xEnd, u16 yEnd)
{
    MetatileFill_Tile(xStart, yStart, xEnd, yEnd, METATILE_General_Grass);
}

static void MetatileFill_BlitMapRegion(u16 mapGroup, u16 mapNum, u16 destX1, u16 destY1, u16 destX2, u16 destY2, u16 srcX1, u16 srcY1)
{
    int i, dx, dy, sx, sy;
    struct MapHeader const * mapHeader = Overworld_GetMapHeaderByGroupAndId(mapGroup, mapNum);

    for(dy = destY1, sy = srcY1; dy <= destY2; ++dy, ++sy)
    {
        for(dx = destX1, sx = srcX1; dx <= destX2; ++dx, ++sx)
        {
            i = sx + mapHeader->mapLayout->width * sy;
            AGB_ASSERT(i < mapHeader->mapLayout->width * mapHeader->mapLayout->height);

            MapGridSetMetatileEntryAt(dx + MAP_OFFSET, dy + MAP_OFFSET, mapHeader->mapLayout->map[i]);
        }
    }
}

static void BuildAtRandomConnectionFrom(u8 fromArea, u8 buildArea)
{
    while (TRUE)
    {
        u8 dir = Random() % HUB_AREA_CONN_COUNT;
        u8 invDir = InvertConnDirection(dir);

        if(RogueHub_AreaHasFreeConnection(fromArea, dir) && CanAreaConnect(buildArea, invDir))
        {
            struct Coords8 pos;
            pos.x = GetActiveHubMap()->areaCoords[fromArea].x;
            pos.y = GetActiveHubMap()->areaCoords[fromArea].y;
            IncrementCoordsByDirection(&pos, dir);

            // We cannot place the lab next to the safari as it will bork up the tutorial sequence
            if(buildArea == HUB_AREA_TOWN_SQUARE)
            {
                if(
                    RogueHub_FindAreaAtCoord(pos.x + 1, pos.y) == HUB_AREA_SAFARI_ZONE ||
                    RogueHub_FindAreaAtCoord(pos.x - 1, pos.y) == HUB_AREA_SAFARI_ZONE ||
                    RogueHub_FindAreaAtCoord(pos.x, pos.y + 1) == HUB_AREA_SAFARI_ZONE ||
                    RogueHub_FindAreaAtCoord(pos.x, pos.y - 1) == HUB_AREA_SAFARI_ZONE
                )
                    continue;
            }

            RogueHub_BuildArea(buildArea, pos.x, pos.y);
            break;
        }
    }
}

bool8 RogueHub_IsPlayerBaseLayout(u16 layoutId)
{
    return layoutId == LAYOUT_ROGUE_AREA_HOME || layoutId == LAYOUT_ROGUE_INTERIOR_HOME || layoutId == LAYOUT_ROGUE_INTERIOR_HOME_UPPER;
}

#define DECOR_TO_LOCAL_ID(x) (x + 2)  // 1 is for the reserved work bench

static u8 SelectSourceVariantLocalId(u16 decorVariant)
{
    if(VarGet(VAR_ROGUE_SPECIAL_MODE) == ROGUE_SPECIAL_MODE_DECORATING)
        return sDecorationVariants[decorVariant].perType.objectEvent.editorLocalId;

    return sDecorationVariants[decorVariant].perType.objectEvent.localId;
}

static bool8 IsVariantHidden(u16 decorVariant, u8* wanderingPkmnCount)
{
    if(VarGet(VAR_ROGUE_SPECIAL_MODE) != ROGUE_SPECIAL_MODE_DECORATING)
    {
        // Only show mons if we have the slots active
        if(decorVariant == DECOR_VARIANT_PC_WANDERING_PKMN_DEFAULT)
        {
            struct RogueHubMap* hubMap = GetActiveHubMap();
            u16 baseIndex = HUB_WANDER_MON_EXTERIOR_SLOT1;

            u8 slot = (*wanderingPkmnCount)++;

            if(hubMap->homeWanderingMonSpecies[baseIndex + slot] == SPECIES_NONE || !FollowMon_IsSlotEnabled(slot))
                return TRUE;
        }
    }

    return FALSE;
}

static void SetupHomeAreaFollowMons(u16 layoutId, struct RogueHubMap* hubMap)
{
    u16 baseIndex = 0;

    switch (gMapHeader.mapLayoutId)
    {
    case LAYOUT_ROGUE_AREA_HOME:
        baseIndex = HUB_WANDER_MON_EXTERIOR_SLOT1;
        break;

    case LAYOUT_ROGUE_INTERIOR_HOME:
        baseIndex = HUB_WANDER_MON_INTERIOR_SLOT1;
        break;
    
    default:
        AGB_ASSERT(FALSE);
        break;
    }

    FollowMon_SetGraphicsRaw(0, hubMap->homeWanderingMonSpecies[baseIndex + 0], 0);
    FollowMon_SetGraphicsRaw(1, hubMap->homeWanderingMonSpecies[baseIndex + 1], 0);
    FollowMon_SetGraphicsRaw(2, hubMap->homeWanderingMonSpecies[baseIndex + 2], 0);
    FollowMon_SetGraphicsRaw(3, hubMap->homeWanderingMonSpecies[baseIndex + 3], 0);
    FollowMon_SetGraphicsRaw(4, hubMap->homeWanderingMonSpecies[baseIndex + 4], 0);
}

void RogueHub_ModifyPlayerBaseObjectEvents(u16 layoutId, bool8 loadingFromSave, struct ObjectEventTemplate *objectEvents, u8* objectEventCount, u8 objectEventCapacity)
{
    u8 i;
    u8 wanderingPkmnCount = 0;
    u16 currDecorCount = GetCurrentDecorCount();
    struct RogueHubMap* hubMap = GetActiveHubMap();

    if(loadingFromSave)
    {
        // Clear any saved templates
        struct MapHeader const* baseMapHeader = Overworld_GetMapHeaderByGroupAndId(gSaveBlock1Ptr->location.mapGroup, gSaveBlock1Ptr->location.mapNum);
        *objectEventCount = baseMapHeader->events->objectEventCount;
    }

    SetupHomeAreaFollowMons(layoutId, hubMap);

    for(i = 0; i < currDecorCount; ++i)
    {
        struct RogueHubDecoration* decor = &hubMap->homeDecorations[GetCurrentDecorOffset(i)];
        if(decor->active && sDecorationVariants[decor->decorVariant].type == DECOR_TYPE_OBJECT_EVENT && !IsVariantHidden(decor->decorVariant, &wanderingPkmnCount))
        {
            u8 srcLocalId = SelectSourceVariantLocalId(decor->decorVariant) - 1;
            struct MapHeader const* srcMapHeader = Overworld_GetMapHeaderByGroupAndId(sDecorationVariants[decor->decorVariant].srcMapGroup, sDecorationVariants[decor->decorVariant].srcMapNum);
        
            AGB_ASSERT(srcLocalId < srcMapHeader->events->objectEventCount);

            if(srcLocalId < srcMapHeader->events->objectEventCount)
            {
                u8 writeIndex = (*objectEventCount)++;
                memcpy(&objectEvents[writeIndex], &srcMapHeader->events->objectEvents[srcLocalId], sizeof(struct ObjectEventTemplate));

                objectEvents[writeIndex].localId = DECOR_TO_LOCAL_ID(GetCurrentDecorOffset(i));
                objectEvents[writeIndex].x = decor->x;
                objectEvents[writeIndex].y = decor->y;

                if(decor->decorVariant == DECOR_VARIANT_PC_WANDERING_PKMN_DEFAULT && objectEvents[writeIndex].graphicsId == OBJ_EVENT_GFX_FOLLOW_MON_0)
                {
                    objectEvents[writeIndex].graphicsId = OBJ_EVENT_GFX_FOLLOW_MON_0 + (wanderingPkmnCount - 1);
                }
            }
        }
    }
}

extern u8 const Rogue_Area_Home_InteractWithWorkbench[];
extern u8 const Rogue_Area_Home_ChooseDecoration[];

bool8 IsCoordInHomeRegion(s32 x, s32 y, u8 region)
{
    return (
        x >= sHomeRegionCoords[region].xStart &&
        x <= sHomeRegionCoords[region].xEnd &&
        y >= sHomeRegionCoords[region].yStart &&
        y <= sHomeRegionCoords[region].yEnd
    );
}

static void UpdatePlaceCoords(u8* placeX, u8* placeY, u8 decorVariant)
{
    u8 height = 1;
    u8 width = 1;
    u8 faceDir = GetPlayerFacingDirection();

    switch (sDecorationVariants[decorVariant].type)
    {
    case DECOR_TYPE_TILE:
        width = sDecorationVariants[decorVariant].perType.tile.width;
        height = sDecorationVariants[decorVariant].perType.tile.height;
        break;
    }

    // Coords are auto place from top left corner, so compensate for that and attempt to place in middle
    switch (faceDir)
    {
    case DIR_NORTH:
        *placeX -= width / 2;
        *placeY -= height - 1;
        break;
    
    case DIR_EAST:
        *placeY -= height / 2;
        break;

    case DIR_SOUTH:
        *placeX -= width / 2;
        break;

    case DIR_WEST:
        *placeX -= width - 1;
        *placeY -= height / 2;
        break;
    }
}

struct DecorBounds
{
    s32 x;
    s32 y;
    s32 width;
    s32 height;
};

static void GetDecorBounds(struct DecorBounds* outBounds, u8 decorVariant, u8 x, u8 y)
{
    outBounds->x = x;
    outBounds->y = y;

    switch(sDecorationVariants[decorVariant].type)
    {
        case DECOR_TYPE_TILE:
            outBounds->width = sDecorationVariants[decorVariant].perType.tile.width;
            outBounds->height = sDecorationVariants[decorVariant].perType.tile.height;
            break;

        case DECOR_TYPE_OBJECT_EVENT:
            outBounds->width = 1;
            outBounds->height = 1;
            break;
    }
}

static bool32 DoBoundsOverlap(struct DecorBounds* boundsA, struct DecorBounds* boundsB)
{
    s32 xMinA = boundsA->x;
    s32 xMaxA = boundsA->x + boundsA->width - 1;
    s32 yMinA = boundsA->y;
    s32 yMaxA = boundsA->y + boundsA->height - 1;

    s32 xMinB = boundsB->x;
    s32 xMaxB = boundsB->x + boundsB->width - 1;
    s32 yMinB = boundsB->y;
    s32 yMaxB = boundsB->y + boundsB->height - 1;
    
    return (xMaxA >= xMinB && xMaxB >= xMinA) && (yMaxA >= yMinB && yMaxB >= yMinA);
}

static bool32 DoesBoundsEncapsulate(struct DecorBounds* boundsA, struct DecorBounds* boundsB)
{
    s32 xMinA = boundsA->x;
    s32 xMaxA = boundsA->x + boundsA->width - 1;
    s32 yMinA = boundsA->y;
    s32 yMaxA = boundsA->y + boundsA->height - 1;

    s32 xMinB = boundsB->x;
    s32 xMaxB = boundsB->x + boundsB->width - 1;
    s32 yMinB = boundsB->y;
    s32 yMaxB = boundsB->y + boundsB->height - 1;

    return (xMinB >= xMinA && xMaxB <= xMaxA && yMinB >= yMinA && yMaxB <= yMaxA);
}

static bool32 CanPlaceDecorAt(struct RogueHubMap* hubMap, u8 decorVariant, u8 x, u8 y)
{
    // Avoid overlapping
    u32 i;
    u32 currDecorCount = GetCurrentDecorCount();
    struct DecorBounds placingBounds = {0};
    struct DecorBounds checkBounds  = {0};
    u8 inputLayer = sDecorationVariants[decorVariant].layer;
    u8 placeableRegion = GetCurrentPlaceableRegion();

    GetDecorBounds(&placingBounds, decorVariant, x, y);

    // Check we're not overlapping the placeable bounds
    checkBounds.x = sHomeRegionCoords[placeableRegion].xStart;
    checkBounds.y = sHomeRegionCoords[placeableRegion].yStart;
    checkBounds.width = 1 + sHomeRegionCoords[placeableRegion].xEnd - sHomeRegionCoords[placeableRegion].xStart;
    checkBounds.height = 1 + sHomeRegionCoords[placeableRegion].yEnd - sHomeRegionCoords[placeableRegion].yStart;

    if(!DoesBoundsEncapsulate(&checkBounds, &placingBounds))
        return FALSE;


    for(i = 0; i < currDecorCount; ++i)
    {
        struct RogueHubDecoration* decor = &hubMap->homeDecorations[GetCurrentDecorOffset(i)];
        if(decor->active)
        {
            u8 itCurrLayer = sDecorationVariants[decor->decorVariant].layer;

            switch(sDecorationVariants[decorVariant].type)
            {
                case DECOR_TYPE_TILE:
                    if(sDecorLayers[inputLayer].placeInBackground)
                    {
                        if(sDecorLayers[itCurrLayer].allowBackgroundTileOverlap)
                            continue;
                    }
                    else
                    {
                        if(sDecorLayers[itCurrLayer].allowSolidTileOverlap)
                            continue;
                    }
                    break;

                case DECOR_TYPE_OBJECT_EVENT:
                    if(sDecorLayers[itCurrLayer].allowObjectOverlap)
                        continue;
                    break;
            }

            GetDecorBounds(&checkBounds, decor->decorVariant, decor->x, decor->y);

            if(DoBoundsOverlap(&placingBounds, &checkBounds))
                return FALSE;
        }
    }

    return TRUE;
}

const struct Tileset* RogueHub_ModifyOverworldTileset(const struct Tileset* tileset)
{
    if(tileset == &gTileset_SecretBaseRedCave)
    {
        switch (GetActiveHubMap()->homeStyles[HOME_STYLE_HOUSE_INTERIOR])
        {
        case INTERIOR_STYLE_BLUE_CAVE:
            return &gTileset_SecretBaseBlueCave;

        case INTERIOR_STYLE_BROWN_CAVE:
            return &gTileset_SecretBaseBrownCave;

        case INTERIOR_STYLE_RED_CAVE:
            return &gTileset_SecretBaseRedCave;

        case INTERIOR_STYLE_DESERT_CAVE:
            return &gTileset_SecretBaseYellowCave;

        case INTERIOR_STYLE_SHRUB:
            return &gTileset_SecretBaseShrub;

        case INTERIOR_STYLE_TREE:
            return &gTileset_SecretBaseTree;
        }
    }

    return tileset;
}

// Scripts
//

// TODO - Keep in sync with data\maps\Rogue_Area_Home\scripts.pory
enum
{
    MENU_DEPTH_CHOOSE_GROUP,
    MENU_DEPTH_CHOOSE_DECOR,
    MENU_DEPTH_CHOOSE_VARIANT,
    MENU_DEPTH_PLACE_DECORATION,
};

#define VAR_MENU_DEPTH                  VAR_TEMP_F
#define VAR_SELECTED_GROUP              VAR_TEMP_E
#define VAR_SELECTED_DECOR_ID           VAR_TEMP_D
#define VAR_SELECTED_DECOR_VARIANT      VAR_TEMP_C
#define VAR_ACTIVE_DECOR_INDEX          VAR_TEMP_B

#define VAR_PLACE_X                     VAR_TEMP_A
#define VAR_PLACE_Y                     VAR_TEMP_9

static u8 const sText_Exit[] = _("Exit");
static u8 const sText_Back[] = _("Back");


const u8* RogueHub_GetDecoratingScriptFor(u16 layoutId, struct MapPosition *position, u16 metatileBehavior, u8 direction, u8 const* existingScript)
{
    // Always buffer coordinates so scripts known where we're interacting
    VarSet(VAR_PLACE_X, position->x - MAP_OFFSET);
    VarSet(VAR_PLACE_Y, position->y - MAP_OFFSET);

    if(existingScript == Rogue_Area_Home_InteractWithWorkbench)
    {
        return existingScript;
    }

    return Rogue_Area_Home_ChooseDecoration;
}

u8 const* RogueHub_GetDecorName(u16 decorId)
{
    AGB_ASSERT(decorId < DECOR_ID_COUNT);
    return sDecorations[decorId].name;
}

u8 const* RogueHub_GetDecorVariantName(u16 decorVariantId)
{
    AGB_ASSERT(decorVariantId < DECOR_VARIANT_COUNT);
    return sDecorationVariants[decorVariantId].name;
}

#define INTERNAL_ID_TO_MULTICHOICE_ID(id) (id >= MULTI_B_PRESSED ? (id + 1) : id)
#define MULTICHOICE_ID_TO_INTERNAL_ID(id) (id >= (MULTI_B_PRESSED + 1) ? (id - 1) : id)

void RogueHub_SetupDecorationMultichoice()
{
    u16 i;
    u16 menuDepth = VarGet(VAR_MENU_DEPTH);
    u16 selectedGroup = VarGet(VAR_SELECTED_GROUP);
    u16 selectedDecorId = VarGet(VAR_SELECTED_DECOR_ID);
    u16 selectedDecorVariant = VarGet(VAR_SELECTED_DECOR_VARIANT);

    // QoL hide variant menu if has only 1
    if(menuDepth == MENU_DEPTH_CHOOSE_VARIANT && sDecorations[selectedDecorId].firstVariantId == sDecorations[selectedDecorId].lastVariantId)
    {
        menuDepth = MENU_DEPTH_CHOOSE_DECOR;
        VarSet(VAR_MENU_DEPTH, menuDepth);
    }

    switch (menuDepth)
    {
    case MENU_DEPTH_CHOOSE_GROUP:
        for(i = 0; i < DECOR_GROUP_COUNT; ++i)
        {
            // Ignore illegal groups for this layout
            if(gMapHeader.mapLayoutId == LAYOUT_ROGUE_AREA_HOME)
            {
                switch (i)
                {
                case DECOR_GROUP_TILES_INTERIOR:
                    continue;
                }
            }
            else if(gMapHeader.mapLayoutId == LAYOUT_ROGUE_INTERIOR_HOME)
            {
                switch (i)
                {
                case DECOR_GROUP_ENVIRONMENT:
                case DECOR_GROUP_TILES_EXTERIOR:
                    continue;
                }
            }

            ScriptMenu_ScrollingMultichoiceDynamicAppendOption(sDecorationGroups[i].name, i);
        }
        ScriptMenu_ScrollingMultichoiceDynamicAppendOption(sText_Exit, MULTI_B_PRESSED);
        break;

    case MENU_DEPTH_CHOOSE_DECOR:
        {
            u16 questId, rewardCount;
            struct RogueQuestReward const* reward;

            // Setup the query to contain disabled decor
            RogueCustomQuery_Begin();

            for(questId = 0; questId < QUEST_ID_COUNT; ++questId)
            {
                if(!RogueQuest_HasCollectedRewards(questId))
                {
                    rewardCount = RogueQuest_GetRewardCount(questId);

                    for(i = 0; i < rewardCount; ++i)
                    {
                        reward = RogueQuest_GetReward(questId, i);
                        if(reward->type == QUEST_REWARD_DECOR)
                            RogueMiscQuery_EditElement(QUERY_FUNC_INCLUDE, reward->perType.decor.decorId);
                    }
                }
            }

            for(i = 0; i < sDecorationGroups[selectedGroup].decorationCount; ++i)
            {
                u16 decorId = sDecorationGroups[selectedGroup].decorationIds[i];

                switch (decorId)
                {
                #ifndef ROGUE_EXPANSION
                case DECOR_ID_APPLIANCES:
                    continue;
                #endif
                }

                // Hasn't unlocked yet
                if(RogueMiscQuery_CheckState(decorId))
                    continue;

                ScriptMenu_ScrollingMultichoiceDynamicAppendOption(sDecorations[decorId].name, INTERNAL_ID_TO_MULTICHOICE_ID(decorId));
            }

            ScriptMenu_ScrollingMultichoiceDynamicAppendOption(sText_Back, MULTI_B_PRESSED);
            RogueCustomQuery_End();
        }
        break;

    case MENU_DEPTH_CHOOSE_VARIANT:
        {
            u16 questId, rewardCount;
            struct RogueQuestReward const* reward;

            // Setup the query to contain disabled decor
            RogueCustomQuery_Begin();

            for(questId = 0; questId < QUEST_ID_COUNT; ++questId)
            {
                if(!RogueQuest_HasCollectedRewards(questId))
                {
                    rewardCount = RogueQuest_GetRewardCount(questId);

                    for(i = 0; i < rewardCount; ++i)
                    {
                        reward = RogueQuest_GetReward(questId, i);
                        if(reward->type == QUEST_REWARD_DECOR_VARIANT)
                            RogueMiscQuery_EditElement(QUERY_FUNC_INCLUDE, reward->perType.decorVariant.decorVariantId);
                    }
                }
            }

            for(i = sDecorations[selectedDecorId].firstVariantId; i <= sDecorations[selectedDecorId].lastVariantId; ++i)
            {
                u16 decorVariant = i;

                //switch (decorVariant)
                //{
                //}

                // Hasn't unlocked yet
                if(RogueMiscQuery_CheckState(decorVariant))
                    continue;

                ScriptMenu_ScrollingMultichoiceDynamicAppendOption(sDecorationVariants[decorVariant].name, INTERNAL_ID_TO_MULTICHOICE_ID(decorVariant));
            }

            ScriptMenu_ScrollingMultichoiceDynamicAppendOption(sText_Back, MULTI_B_PRESSED);
            RogueCustomQuery_End();
        }
        break;

    case MENU_DEPTH_PLACE_DECORATION:
        AGB_ASSERT(FALSE); // don't process here
        break;
    }
}
void RogueHub_HandleDecorationMultichoiceResult()
{
    u16 menuDepth = VarGet(VAR_MENU_DEPTH);
    u16 result = gSpecialVar_Result;

    // VAR_RESULT is "shouldContinueLooping" (Continue looping by default)
    gSpecialVar_Result = TRUE;

    switch (menuDepth)
    {
    case MENU_DEPTH_CHOOSE_GROUP:
        if(result == MULTI_B_PRESSED)
        {
            // stop looping
            gSpecialVar_Result = FALSE;
        }
        else
        {
            VarSet(VAR_SELECTED_GROUP, result);
            VarSet(VAR_MENU_DEPTH, MENU_DEPTH_CHOOSE_DECOR);
        }
        break;

    case MENU_DEPTH_CHOOSE_DECOR:
        if(result == MULTI_B_PRESSED)
        {
            VarSet(VAR_MENU_DEPTH, MENU_DEPTH_CHOOSE_GROUP);
        }
        else
        {
            result = MULTICHOICE_ID_TO_INTERNAL_ID(result);
            VarSet(VAR_SELECTED_DECOR_ID, result);

            // Has only 1 variant, so just start placing
            if(sDecorations[result].firstVariantId == sDecorations[result].lastVariantId)
            {
                VarSet(VAR_SELECTED_DECOR_VARIANT, sDecorations[result].firstVariantId);
                VarSet(VAR_MENU_DEPTH, MENU_DEPTH_PLACE_DECORATION);
                gSpecialVar_Result = FALSE; // stop choosing and begin decorating
            }
            else
            {
                VarSet(VAR_MENU_DEPTH, MENU_DEPTH_CHOOSE_VARIANT);
            }
        }
        break;

    case MENU_DEPTH_CHOOSE_VARIANT:
        if(result == MULTI_B_PRESSED)
        {
            VarSet(VAR_MENU_DEPTH, MENU_DEPTH_CHOOSE_DECOR);
        }
        else
        {
            result = MULTICHOICE_ID_TO_INTERNAL_ID(result);
            VarSet(VAR_SELECTED_DECOR_VARIANT, result);
            VarSet(VAR_MENU_DEPTH, MENU_DEPTH_PLACE_DECORATION);
            gSpecialVar_Result = FALSE; // stop choosing and begin decorating
        }
        break;

    case MENU_DEPTH_PLACE_DECORATION:
        AGB_ASSERT(FALSE); // don't process here
        break;
    }
}

void RogueHub_ClearAllDecorations()
{
    memset(gRogueSaveBlock->hubMap.homeDecorations, 0, sizeof(gRogueSaveBlock->hubMap.homeDecorations));
}

static bool8 IsCoordOnDecor(struct RogueHubDecoration* decor, u8 x, u8 y)
{
    u8 xStart = decor->x;
    u8 yStart = decor->y;
    u8 xEnd = xStart;
    u8 yEnd = yStart;

    switch (sDecorationVariants[decor->decorVariant].type)
    {
    case DECOR_TYPE_TILE:
        xEnd += sDecorationVariants[decor->decorVariant].perType.tile.width - 1;
        yEnd += sDecorationVariants[decor->decorVariant].perType.tile.height - 1;
        break;
    }

    return (
        x >= xStart &&
        x <= xEnd &&
        y >= yStart &&
        y <= yEnd
    );
}

#define MAX_OBJECTS_CHECK_RANGE_X       14
#define MAX_OBJECTS_CHECK_RANGE_Y       10
#define MAX_OBJECTS_CHECK_NEARBY_COUNT  (OBJECT_EVENTS_COUNT - 5)

static bool8 CanPlaceObjectEventInArea(u8 x, u8 y)
{
    //u16 i;
    //u8 counter = 0;
//
    //for(i = 0; i < OBJECT_EVENTS_COUNT; ++i)
    //{
    //    // Don't consider MP obects
    //    if(gObjectEvents[i].active && !(gObjectEvents[i].localId >= OBJ_EVENT_ID_MULTIPLAYER_FIRST && gObjectEvents[i].localId <= OBJ_EVENT_ID_MULTIPLAYER_LAST))
    //    {
    //        counter++;
    //        if(counter >= MAX_OBJECTS_CHECK_NEARBY_COUNT)
    //            return FALSE;
    //    }
    //}
//
    //return TRUE;


    u16 i;
    u16 currDecorCount = GetCurrentDecorCount();
    struct RogueHubMap* hubMap = &gRogueSaveBlock->hubMap;
    s16 xStart = x - MAX_OBJECTS_CHECK_RANGE_X;
    s16 yStart = y - MAX_OBJECTS_CHECK_RANGE_Y;
    s16 xEnd = x + MAX_OBJECTS_CHECK_RANGE_X;
    s16 yEnd = y + MAX_OBJECTS_CHECK_RANGE_Y;

    u8 counter = 0;

    for(i = 0; i < currDecorCount; ++i)
    {
        struct RogueHubDecoration* decor = &hubMap->homeDecorations[GetCurrentDecorOffset(i)];
        if(decor->active && sDecorationVariants[decor->decorVariant].type == DECOR_TYPE_OBJECT_EVENT)
        {
            if(
                (s16)decor->x >= xStart &&
                (s16)decor->x <= xEnd &&
                (s16)decor->y >= yStart &&
                (s16)decor->y <= yEnd
            )
            {
                counter++;
                if(counter >= MAX_OBJECTS_CHECK_NEARBY_COUNT)
                    return FALSE;
            }
        }
    }

    return TRUE;
}

bool8 HasReachedCapacityInArea(struct RogueHubMap* hubMap, u16 decorVariant)
{
    if(sDecorationVariants[decorVariant].type == DECOR_TYPE_OBJECT_EVENT && sDecorationVariants[decorVariant].perType.objectEvent.capacityPerArea != 0)
    {
        u16 i;
        u8 count = 0;
        u16 currDecorCount = GetCurrentDecorCount();

        for(i = 0; i < currDecorCount; ++i)
        {
            struct RogueHubDecoration* decor = &hubMap->homeDecorations[GetCurrentDecorOffset(i)];
            if(decor->active && decor->decorVariant == decorVariant)
            {
                count++;
                if(count >= sDecorationVariants[decorVariant].perType.objectEvent.capacityPerArea)
                    return TRUE;
            }
        }
    }

    return FALSE;
}

static bool8 IsCoordLegalForPlacement(u8 placeX, u8 placeY)
{
    if(gMapHeader.mapLayoutId == LAYOUT_ROGUE_AREA_HOME)
    {
        return IsCoordInHomeRegion(placeX, placeY, HOME_REGION_PLACEABLE_REGION) && !IsCoordInHomeRegion(placeX, placeY, HOME_REGION_HOUSE);
    }
    else
    {
        return IsCoordInHomeRegion(placeX, placeY, HOME_REGION_PLACEABLE_REGION_INTERIOR);
    }
}

u16 RogueHub_PlaceHomeDecor()
{
    u16 i;
    u16 currDecorCount = GetCurrentDecorCount();
    struct RogueHubMap* hubMap = &gRogueSaveBlock->hubMap;
    u16 decorVariant = VarGet(VAR_SELECTED_DECOR_VARIANT);
    u8 placeX = VarGet(VAR_PLACE_X);
    u8 placeY = VarGet(VAR_PLACE_Y);

    AGB_ASSERT(decorVariant < DECOR_VARIANT_COUNT);

    if(IsCoordLegalForPlacement(placeX, placeY))
    {
        // Special behaviour for removing
        if(decorVariant == DECOR_VARIANT_REMOVE_DECOR_DEFAULT)
        {
            // Attempt to remove the decor that's on top

            // object layer
            for(i = 0; i < currDecorCount; ++i)
            {
                u16 invI = currDecorCount - i - 1;

                struct RogueHubDecoration* decor = &hubMap->homeDecorations[GetCurrentDecorOffset(invI)];

                if(decor->active && sDecorationVariants[decor->decorVariant].type == DECOR_TYPE_OBJECT_EVENT && IsCoordOnDecor(decor, placeX, placeY))
                {
                    decor->active = FALSE; // deactivate for preview
                    RemoveObjectEventByLocalIdAndMap(DECOR_TO_LOCAL_ID(GetCurrentDecorOffset(invI)), gSaveBlock1Ptr->location.mapNum, gSaveBlock1Ptr->location.mapGroup);
                    RogueHub_PlaceHomeEnvironmentDecorations(FALSE, TRUE);
                    return GetCurrentDecorOffset(invI);
                }
            }
            
            // top layer
            for(i = 0; i < currDecorCount; ++i)
            {
                u16 invI = currDecorCount - i - 1;

                struct RogueHubDecoration* decor = &hubMap->homeDecorations[GetCurrentDecorOffset(invI)];

                if(decor->active && sDecorationVariants[decor->decorVariant].type == DECOR_TYPE_TILE && !IsBottomLayerVariant(decor->decorVariant) && IsCoordOnDecor(decor, placeX, placeY))
                {
                    decor->active = FALSE; // deactivate for preview
                    RogueHub_PlaceHomeEnvironmentDecorations(TRUE, FALSE);
                    return GetCurrentDecorOffset(invI);
                }
            }

            // bottom layer
            for(i = 0; i < currDecorCount; ++i)
            {
                u16 invI = currDecorCount - i - 1;

                struct RogueHubDecoration* decor = &hubMap->homeDecorations[GetCurrentDecorOffset(invI)];

                if(decor->active && sDecorationVariants[decor->decorVariant].type == DECOR_TYPE_TILE && IsBottomLayerVariant(decor->decorVariant) && IsCoordOnDecor(decor, placeX, placeY))
                {
                    decor->active = FALSE; // deactivate for preview
                    RogueHub_PlaceHomeEnvironmentDecorations(TRUE, FALSE);
                    return GetCurrentDecorOffset(invI);
                }
            }

            return HOME_DECOR_CANNOT_REMOVE;
        }
        else if(sDecorationVariants[decorVariant].type == DECOR_TYPE_OBJECT_EVENT && !CanPlaceObjectEventInArea(placeX, placeY))
        {
            return HOME_DECOR_TOO_MANY_OBJECTS_NEAR;
        }
        else if(sDecorationVariants[decorVariant].type == DECOR_TYPE_OBJECT_EVENT && HasReachedCapacityInArea(hubMap, decorVariant))
        {
            return HOME_DECOR_TOO_MANY_OF_TYPE;
        }
        else
        {
            UpdatePlaceCoords(&placeX, &placeY, decorVariant);

            if(CanPlaceDecorAt(hubMap, decorVariant, placeX, placeY))
            {
                for(i = 0; i < currDecorCount; ++i)
                {
                    struct RogueHubDecoration* decor = &hubMap->homeDecorations[GetCurrentDecorOffset(i)];
                    if(!decor->active)
                    {
                        decor->x = placeX;
                        decor->y = placeY;
                        decor->decorVariant = decorVariant;
                        decor->active = TRUE;

                        RogueHub_PlaceHomeEnvironmentDecorations(sDecorationVariants[decorVariant].type == DECOR_TYPE_TILE, sDecorationVariants[decorVariant].type == DECOR_TYPE_OBJECT_EVENT);
                        return GetCurrentDecorOffset(i);
                    }
                }
            }
            else
            {
                return HOME_DECOR_CODE_NOT_HERE;
            }
        }
    
        return HOME_DECOR_CODE_NO_ROOM;
    }

    return HOME_DECOR_CODE_NOT_HERE;
}

void RogueHub_RemoveHomeDecor()
{
    u16 index = VarGet(VAR_ACTIVE_DECOR_INDEX);
    u16 decorVariant = VarGet(VAR_SELECTED_DECOR_VARIANT);

    // This case falls through here, so ignore it
    if(index == HOME_DECOR_CANNOT_REMOVE)
        return;

    AGB_ASSERT(index < HOME_DECOR_TOTAL_COUNT);

    // Special behaviour for removing
    if(decorVariant == DECOR_VARIANT_REMOVE_DECOR_DEFAULT)
    {
        // We didn't want to remove it, so re-enable it
        gRogueSaveBlock->hubMap.homeDecorations[index].active = TRUE;
        RogueHub_PlaceHomeEnvironmentDecorations(TRUE, TRUE);
    }
    else
    {
        gRogueSaveBlock->hubMap.homeDecorations[index].active = FALSE;

        if(sDecorationVariants[decorVariant].type == DECOR_TYPE_OBJECT_EVENT)
            RemoveObjectEventByLocalIdAndMap(DECOR_TO_LOCAL_ID(index), gSaveBlock1Ptr->location.mapNum, gSaveBlock1Ptr->location.mapGroup);

        RogueHub_PlaceHomeEnvironmentDecorations(sDecorationVariants[decorVariant].type == DECOR_TYPE_TILE, sDecorationVariants[decorVariant].type == DECOR_TYPE_OBJECT_EVENT);
    }
}

u16 RogueHub_IsRemovingDecor()
{
    return VarGet(VAR_MENU_DEPTH) == MENU_DEPTH_PLACE_DECORATION && VarGet(VAR_SELECTED_DECOR_VARIANT) == DECOR_VARIANT_REMOVE_DECOR_DEFAULT;
}

#undef VAR_MENU_DEPTH
#undef VAR_SELECTED_GROUP
#undef VAR_SELECTED_DECOR_ID
#undef VAR_SELECTED_DECOR_VARIANT
#undef VAR_ACTIVE_DECOR_INDEX
#undef VAR_PLACE_X
#undef VAR_PLACE_Y

//

//#define HOME_STYLE_HOUSE_EXTERIOR   0
//#define HOME_STYLE_HOUSE_INTERIOR   1
//#define HOME_STYLE_PATH             2

static void AppendCommon(u8 const* const* options, u8 count)
{
    u8 i = 0;
    for(i = 0; i < count; ++i)
    {
        ScriptMenu_ScrollingMultichoiceDynamicAppendOption(options[i], i);
    }
}

void RogueHub_AppendChangeStyle_Paths()
{
    AppendCommon(sOptions_PathStyle, PATH_STYLE_COUNT);
}

void RogueHub_HandleChangeStyle_Paths()
{
    gRogueSaveBlock->hubMap.homeStyles[HOME_STYLE_PATH] = min(gSpecialVar_Result, PATH_STYLE_COUNT - 1);
}

void RogueHub_AppendChangeStyle_Exterior()
{
    AppendCommon(sOptions_ExteriorStyle, EXTERIOR_STYLE_COUNT);
}

void RogueHub_HandleChangeStyle_Exterior()
{
    gRogueSaveBlock->hubMap.homeStyles[HOME_STYLE_HOUSE_EXTERIOR] = min(gSpecialVar_Result, EXTERIOR_STYLE_COUNT - 1);
}

void RogueHub_AppendChangeStyle_Interior()
{
    AppendCommon(sOptions_InteriorStyle, INTERIOR_STYLE_COUNT);
}

void RogueHub_HandleChangeStyle_Interior()
{
    gRogueSaveBlock->hubMap.homeStyles[HOME_STYLE_HOUSE_INTERIOR] = min(gSpecialVar_Result, INTERIOR_STYLE_COUNT - 1);
}

//

static u32 GetCurrentAreaMetatileAt(s32 x, s32 y)
{
    u32 metaTile = MapGridGetMetatileIdAt(x + MAP_OFFSET, y + MAP_OFFSET);
    
    // Player home cannot auto join to player house
    if(gMapHeader.mapLayoutId == LAYOUT_ROGUE_AREA_HOME)
    {
        if(IsCoordInHomeRegion(x, y, HOME_REGION_HOUSE))
        {
            // Override these tiles to avoid fixup odd stuff
            metaTile = 0;
        }
    }

    return metaTile;
}

#define METATILE_GeneralHub_Tree_Class METATILE_GeneralHub_Tree_TopRight_Sparse

static bool8 IsCompatibleMetatile(u32 classTile,  u32 checkTile)
{
    if(classTile == METATILE_GeneralHub_Pond_Centre)
    {
        return (
            checkTile == METATILE_GeneralHub_Pond_Centre ||
            checkTile == METATILE_GeneralHub_Pond_Conn_EastWest_South ||
            checkTile == METATILE_GeneralHub_Pond_Conn_NorthEast ||
            checkTile == METATILE_GeneralHub_Pond_Conn_NorthSouth_East ||
            checkTile == METATILE_GeneralHub_Pond_Conn_NorthSouth_West ||
            checkTile == METATILE_GeneralHub_Pond_Conn_NorthWest ||
            checkTile == METATILE_GeneralHub_Pond_Conn_SouthEast ||
            checkTile == METATILE_GeneralHub_Pond_Conn_SouthWest
        );
    }
    else if(classTile == METATILE_GeneralHub_GrassPath_Centre)
    {
        return (
            checkTile == METATILE_GeneralHub_GrassPath_Centre ||
            checkTile == METATILE_GeneralHub_GrassPath_Conn_EastWest_North ||
            checkTile == METATILE_GeneralHub_GrassPath_Conn_EastWest_South ||
            checkTile == METATILE_GeneralHub_GrassPath_Conn_NorthEast ||
            checkTile == METATILE_GeneralHub_GrassPath_Conn_NorthSouth_East ||
            checkTile == METATILE_GeneralHub_GrassPath_Conn_NorthSouth_West ||
            checkTile == METATILE_GeneralHub_GrassPath_Conn_NorthWest ||
            checkTile == METATILE_GeneralHub_GrassPath_Conn_SouthEast ||
            checkTile == METATILE_GeneralHub_GrassPath_Conn_SouthWest
        );
    }
    else if(classTile == METATILE_GeneralHub_Mountain_Centre)
    {
        return (
            checkTile == METATILE_GeneralHub_Mountain_Centre ||
            checkTile == METATILE_GeneralHub_Mountain_Conn_EastWest_North ||
            checkTile == METATILE_GeneralHub_Mountain_Conn_EastWest_South ||
            checkTile == METATILE_GeneralHub_Mountain_Conn_NorthEast ||
            checkTile == METATILE_GeneralHub_Mountain_Conn_NorthSouth_East ||
            checkTile == METATILE_GeneralHub_Mountain_Conn_NorthSouth_West ||
            checkTile == METATILE_GeneralHub_Mountain_Conn_NorthWest ||
            checkTile == METATILE_GeneralHub_Mountain_Conn_SouthEast ||
            checkTile == METATILE_GeneralHub_Mountain_Conn_SouthEast_Inside ||
            checkTile == METATILE_GeneralHub_Mountain_Conn_SouthWest ||
            checkTile == METATILE_GeneralHub_Mountain_Conn_SouthWest_Inside ||
            checkTile == METATILE_GeneralHub_MountainRaised_Conn_EastWest_South
        );
    }
    else if(classTile == METATILE_GeneralHub_Tree_Class)
    {
        return (
            checkTile == METATILE_GeneralHub_Tree_BottomLeft_Dense ||
            checkTile == METATILE_GeneralHub_Tree_BottomLeft_Dense_Overlapped ||
            checkTile == METATILE_GeneralHub_Tree_BottomLeft_Sparse ||
            checkTile == METATILE_GeneralHub_Tree_BottomLeft_Sparse_Overlapped ||
            checkTile == METATILE_GeneralHub_Tree_BottomRight_Dense ||
            checkTile == METATILE_GeneralHub_Tree_BottomRight_Dense_Overlapped ||
            checkTile == METATILE_GeneralHub_Tree_BottomRight_Sparse ||
            checkTile == METATILE_GeneralHub_Tree_BottomRight_Sparse_Overlapped ||
            checkTile == METATILE_GeneralHub_Tree_TopLeft_CapGrass ||
            checkTile == METATILE_GeneralHub_Tree_TopLeft_Dense ||
            checkTile == METATILE_GeneralHub_Tree_TopLeft_Sparse ||
            checkTile == METATILE_GeneralHub_Tree_TopRight_CapGrass ||
            checkTile == METATILE_GeneralHub_Tree_TopRight_Dense ||
            checkTile == METATILE_GeneralHub_Tree_TopRight_Sparse
        );
    }

    return classTile == checkTile;
}

static bool8 IsCompatibleMetatileAt(s32 x, s32 y, u32 classTile)
{
    return IsCompatibleMetatile(classTile, GetCurrentAreaMetatileAt(x, y));
}

// Pond
//

static void FixupTile_Pond_Horizontal(s32 x, s32 y)
{
    bool8 west = IsCompatibleMetatileAt(x - 1, y + 0, METATILE_GeneralHub_Pond_Centre);
    bool8 east = IsCompatibleMetatileAt(x + 1, y + 0, METATILE_GeneralHub_Pond_Centre);

    if(!west && east)
    {
        MetatileSet_Tile(x, y, METATILE_GeneralHub_Pond_Conn_NorthSouth_East);
    }
    else if(west && !east)
    {
        MetatileSet_Tile(x, y, METATILE_GeneralHub_Pond_Conn_NorthSouth_West);
    }
}

static void FixupTile_Pond_Vertical(s32 x, s32 y)
{
    bool8 north = IsCompatibleMetatileAt(x + 0, y - 1, METATILE_GeneralHub_Pond_Centre);
    bool8 south = IsCompatibleMetatileAt(x + 0, y + 1, METATILE_GeneralHub_Pond_Centre);

    if(!north && south)
    {
        MetatileSet_Tile(x, y, METATILE_GeneralHub_Pond_Conn_EastWest_South);
    }
    else if(!north && !south)
    {
        // Also placed if nothing above or below
        MetatileSet_Tile(x, y, METATILE_GeneralHub_Pond_Conn_EastWest_South);
    }
}

static void FixupTile_Pond_Fixup(s32 x, s32 y, u32 centreTile)
{
    u32 northTile = GetCurrentAreaMetatileAt(x + 0, y - 1);
    u32 eastTile =  GetCurrentAreaMetatileAt(x + 1, y + 0);
    u32 southTile = GetCurrentAreaMetatileAt(x + 0, y + 1);
    u32 westTile =  GetCurrentAreaMetatileAt(x - 1, y + 0);

    if(centreTile == METATILE_GeneralHub_Pond_Centre)
    {
        // Inside Corners
        if(
            IsCompatibleMetatile(METATILE_GeneralHub_Pond_Centre, northTile) &&
            IsCompatibleMetatile(METATILE_GeneralHub_Pond_Centre, eastTile) &&
            IsCompatibleMetatile(METATILE_GeneralHub_Pond_Centre, westTile)
        )
        {
            if(!IsCompatibleMetatileAt(x - 1, y - 1, METATILE_GeneralHub_Pond_Centre))
                MetatileSet_Tile(x, y, METATILE_GeneralHub_Pond_Conn_NorthWest);
            else if(!IsCompatibleMetatileAt(x + 1, y - 1, METATILE_GeneralHub_Pond_Centre))
                MetatileSet_Tile(x, y, METATILE_GeneralHub_Pond_Conn_NorthEast);
        }
    }
    // Outside Corners
    else if(!IsCompatibleMetatile(METATILE_GeneralHub_Pond_Centre, northTile) && !IsCompatibleMetatile(METATILE_GeneralHub_Pond_Centre, westTile))
    {
        MetatileSet_Tile(x, y, METATILE_GeneralHub_Pond_Conn_SouthEast);
    }
    else if(!IsCompatibleMetatile(METATILE_GeneralHub_Pond_Centre, northTile) && !IsCompatibleMetatile(METATILE_GeneralHub_Pond_Centre, eastTile))
    {
        MetatileSet_Tile(x, y, METATILE_GeneralHub_Pond_Conn_SouthWest);
    }

    // Fix up grass
    if(southTile == METATILE_GeneralHub_Grass)
        MetatileSet_Tile(x, y + 1, METATILE_GeneralHub_Grass_NorthDrop);
}

// Grass Path
//

static void FixupTile_GrassPath_Horizontal(s32 x, s32 y)
{
    bool8 west = IsCompatibleMetatileAt(x - 1, y + 0, METATILE_GeneralHub_GrassPath_Centre);
    bool8 east = IsCompatibleMetatileAt(x + 1, y + 0, METATILE_GeneralHub_GrassPath_Centre);

    if(!west && east)
    {
        MetatileSet_Tile(x, y, METATILE_GeneralHub_GrassPath_Conn_NorthSouth_East);
    }
    else if(west && !east)
    {
        MetatileSet_Tile(x, y, METATILE_GeneralHub_GrassPath_Conn_NorthSouth_West);
    }
}

static void FixupTile_GrassPath_Vertical(s32 x, s32 y)
{
    bool8 north = IsCompatibleMetatileAt(x + 0, y - 1, METATILE_GeneralHub_GrassPath_Centre);
    bool8 south = IsCompatibleMetatileAt(x + 0, y + 1, METATILE_GeneralHub_GrassPath_Centre);

    if(!north && south)
    {
        MetatileSet_Tile(x, y, METATILE_GeneralHub_GrassPath_Conn_EastWest_South);
    }
    else if(north && !south)
    {
        MetatileSet_Tile(x, y, METATILE_GeneralHub_GrassPath_Conn_EastWest_North);
    }
}

static void FixupTile_GrassPath_Fixup(s32 x, s32 y, u32 centreTile)
{
    u32 northTile = GetCurrentAreaMetatileAt(x + 0, y - 1);
    u32 eastTile =  GetCurrentAreaMetatileAt(x + 1, y + 0);
    u32 southTile = GetCurrentAreaMetatileAt(x + 0, y + 1);
    u32 westTile =  GetCurrentAreaMetatileAt(x - 1, y + 0);

    // Outside Corners (North)
    if(!IsCompatibleMetatile(METATILE_GeneralHub_GrassPath_Centre, northTile) && !IsCompatibleMetatile(METATILE_GeneralHub_GrassPath_Centre, westTile))
    {
        MetatileSet_Tile(x, y, METATILE_GeneralHub_GrassPath_Conn_SouthEast);
    }
    else if(!IsCompatibleMetatile(METATILE_GeneralHub_GrassPath_Centre, northTile) && !IsCompatibleMetatile(METATILE_GeneralHub_GrassPath_Centre, eastTile))
    {
        MetatileSet_Tile(x, y, METATILE_GeneralHub_GrassPath_Conn_SouthWest);
    }
    // Outside Corners (South)
    if(!IsCompatibleMetatile(METATILE_GeneralHub_GrassPath_Centre, southTile) && !IsCompatibleMetatile(METATILE_GeneralHub_GrassPath_Centre, westTile))
    {
        MetatileSet_Tile(x, y, METATILE_GeneralHub_GrassPath_Conn_NorthEast);
    }
    else if(!IsCompatibleMetatile(METATILE_GeneralHub_GrassPath_Centre, southTile) && !IsCompatibleMetatile(METATILE_GeneralHub_GrassPath_Centre, eastTile))
    {
        MetatileSet_Tile(x, y, METATILE_GeneralHub_GrassPath_Conn_NorthWest);
    }
}

// Mountain Path
//

static void FixupTile_Mountain_Horizontal(s32 x, s32 y)
{
    bool8 west = IsCompatibleMetatileAt(x - 1, y + 0, METATILE_GeneralHub_Mountain_Centre);
    bool8 east = IsCompatibleMetatileAt(x + 1, y + 0, METATILE_GeneralHub_Mountain_Centre);

    if(!west && east)
    {
        MetatileSet_Tile(x, y, METATILE_GeneralHub_Mountain_Conn_NorthSouth_East | MAPGRID_COLLISION_MASK);
    }
    else if(west && !east)
    {
        MetatileSet_Tile(x, y, METATILE_GeneralHub_Mountain_Conn_NorthSouth_West | MAPGRID_COLLISION_MASK);
    }
}

static void FixupTile_Mountain_Vertical(s32 x, s32 y)
{
    bool8 north = IsCompatibleMetatileAt(x + 0, y - 1, METATILE_GeneralHub_Mountain_Centre);
    bool8 south = IsCompatibleMetatileAt(x + 0, y + 1, METATILE_GeneralHub_Mountain_Centre);

    if(!north && south)
    {
        MetatileSet_Tile(x, y, METATILE_GeneralHub_Mountain_Conn_EastWest_South | MAPGRID_COLLISION_MASK);
    }
    else if(north && !south)
    {
        MetatileSet_Tile(x, y, METATILE_GeneralHub_Mountain_Conn_EastWest_North | MAPGRID_COLLISION_MASK);
    }
}

static void FixupTile_Mountain_Fixup(s32 x, s32 y, u32 centreTile)
{
    u32 northTile = GetCurrentAreaMetatileAt(x + 0, y - 1);
    u32 eastTile =  GetCurrentAreaMetatileAt(x + 1, y + 0);
    u32 southTile = GetCurrentAreaMetatileAt(x + 0, y + 1);
    u32 westTile =  GetCurrentAreaMetatileAt(x - 1, y + 0);

    // Outside Corners (North)
    if(!IsCompatibleMetatile(METATILE_GeneralHub_Mountain_Centre, northTile) && !IsCompatibleMetatile(METATILE_GeneralHub_Mountain_Centre, westTile))
    {
        MetatileSet_Tile(x, y, METATILE_GeneralHub_Mountain_Conn_SouthEast | MAPGRID_COLLISION_MASK);
    }
    else if(!IsCompatibleMetatile(METATILE_GeneralHub_Mountain_Centre, northTile) && !IsCompatibleMetatile(METATILE_GeneralHub_Mountain_Centre, eastTile))
    {
        MetatileSet_Tile(x, y, METATILE_GeneralHub_Mountain_Conn_SouthWest | MAPGRID_COLLISION_MASK);
    }
    // Outside Corners (South)
    if(!IsCompatibleMetatile(METATILE_GeneralHub_Mountain_Centre, southTile) && !IsCompatibleMetatile(METATILE_GeneralHub_Mountain_Centre, westTile))
    {
        MetatileSet_Tile(x, y, METATILE_GeneralHub_Mountain_Conn_NorthEast | MAPGRID_COLLISION_MASK);
    }
    else if(!IsCompatibleMetatile(METATILE_GeneralHub_Mountain_Centre, southTile) && !IsCompatibleMetatile(METATILE_GeneralHub_Mountain_Centre, eastTile))
    {
        MetatileSet_Tile(x, y, METATILE_GeneralHub_Mountain_Conn_NorthWest | MAPGRID_COLLISION_MASK);
    }
    // Inside Corners (Noth)
    else if(centreTile == METATILE_GeneralHub_Mountain_Centre && IsCompatibleMetatile(METATILE_GeneralHub_Mountain_Centre, westTile) && IsCompatibleMetatile(METATILE_GeneralHub_Mountain_Centre, northTile) && !IsCompatibleMetatileAt(x - 1, y - 1, METATILE_GeneralHub_Mountain_Centre))
    {
        MetatileSet_Tile(x, y, METATILE_GeneralHub_MountainRaised_Conn_EastWest_South);
    }
    else if(centreTile == METATILE_GeneralHub_Mountain_Centre && IsCompatibleMetatile(METATILE_GeneralHub_Mountain_Centre, eastTile) && IsCompatibleMetatile(METATILE_GeneralHub_Mountain_Centre, northTile) && !IsCompatibleMetatileAt(x + 1, y - 1, METATILE_GeneralHub_Mountain_Centre))
    {
        MetatileSet_Tile(x, y, METATILE_GeneralHub_MountainRaised_Conn_EastWest_South);
    }
    // Inside Corners (South)
    else if(centreTile == METATILE_GeneralHub_Mountain_Centre && IsCompatibleMetatile(METATILE_GeneralHub_Mountain_Centre, westTile) && IsCompatibleMetatile(METATILE_GeneralHub_Mountain_Centre, southTile) && !IsCompatibleMetatileAt(x - 1, y + 1, METATILE_GeneralHub_Mountain_Centre))
    {
        MetatileSet_Tile(x, y, METATILE_GeneralHub_Mountain_Conn_SouthWest_Inside | MAPGRID_COLLISION_MASK);
    }
    else if(centreTile == METATILE_GeneralHub_Mountain_Centre && IsCompatibleMetatile(METATILE_GeneralHub_Mountain_Centre, eastTile) && IsCompatibleMetatile(METATILE_GeneralHub_Mountain_Centre, southTile) && !IsCompatibleMetatileAt(x + 1, y + 1, METATILE_GeneralHub_Mountain_Centre))
    {
        MetatileSet_Tile(x, y, METATILE_GeneralHub_Mountain_Conn_SouthEast_Inside | MAPGRID_COLLISION_MASK);
    }
}

// Mountain Path
//

static void FixupTile_Trees_Horizontal(s32 x, s32 y, u32 centreTile)
{
    bool8 west = IsCompatibleMetatileAt(x - 1, y + 0, METATILE_GeneralHub_Tree_Class);
    bool8 east = IsCompatibleMetatileAt(x + 1, y + 0, METATILE_GeneralHub_Tree_Class);

    switch (centreTile)
    {
    case METATILE_GeneralHub_Tree_TopLeft_Sparse:
        if(west)
            MetatileSet_Tile(x, y, METATILE_GeneralHub_Tree_TopLeft_Dense | MAPGRID_COLLISION_MASK);
        break;
    case METATILE_GeneralHub_Tree_BottomLeft_Sparse:
        if(west)
            MetatileSet_Tile(x, y, METATILE_GeneralHub_Tree_BottomLeft_Dense | MAPGRID_COLLISION_MASK);
        break;
    
    case METATILE_GeneralHub_Tree_TopRight_Sparse:
        if(east)
            MetatileSet_Tile(x, y, METATILE_GeneralHub_Tree_TopRight_Dense | MAPGRID_COLLISION_MASK);
        break;
    case METATILE_GeneralHub_Tree_BottomRight_Sparse:
        if(east)
            MetatileSet_Tile(x, y, METATILE_GeneralHub_Tree_BottomRight_Dense | MAPGRID_COLLISION_MASK);
        break;
    }
}

static void FixupTile_Trees_Vertical(s32 x, s32 y, u32 centreTile)
{
    u32 newTile = centreTile;

    bool8 north = IsCompatibleMetatileAt(x + 0, y - 1, METATILE_GeneralHub_Tree_Class);
    bool8 south = IsCompatibleMetatileAt(x + 0, y + 1, METATILE_GeneralHub_Tree_Class);

    // Decide on whether we're dense or sparse
    switch (centreTile)
    {
    case METATILE_GeneralHub_Tree_BottomLeft_Sparse:
        if(south)
            newTile = METATILE_GeneralHub_Tree_BottomLeft_Dense;
        break;
    case METATILE_GeneralHub_Tree_BottomRight_Sparse:
        if(south)
            newTile = METATILE_GeneralHub_Tree_BottomRight_Dense;
        break;

    case METATILE_GeneralHub_Tree_TopLeft_Sparse:
        if(north)
            newTile = METATILE_GeneralHub_Tree_TopLeft_Dense;
        break;
    case METATILE_GeneralHub_Tree_TopRight_Sparse:
        if(north)
            newTile = METATILE_GeneralHub_Tree_TopRight_Dense;
        break;
    }

    // Replace with cap variants
    if(south)
    {
        u32 southTile = GetCurrentAreaMetatileAt(x + 0, y + 1);

        switch (newTile)
        {
        case METATILE_GeneralHub_Tree_BottomLeft_Sparse:
            if(southTile == METATILE_GeneralHub_Tree_TopLeft_Dense || southTile == METATILE_GeneralHub_Tree_TopLeft_Sparse)
                newTile = METATILE_GeneralHub_Tree_BottomLeft_Sparse_Overlapped;
            else
                newTile = METATILE_GeneralHub_Tree_BottomLeft_Dense_Overlapped_Alt;
            break;
        case METATILE_GeneralHub_Tree_BottomRight_Sparse:
            if(southTile == METATILE_GeneralHub_Tree_TopRight_Dense || southTile == METATILE_GeneralHub_Tree_TopRight_Sparse)
                newTile = METATILE_GeneralHub_Tree_BottomRight_Sparse_Overlapped;
            else
                newTile = METATILE_GeneralHub_Tree_BottomRight_Dense_Overlapped_Alt;
            break;

        case METATILE_GeneralHub_Tree_BottomLeft_Dense:
            if(southTile == METATILE_GeneralHub_Tree_TopLeft_Dense || southTile == METATILE_GeneralHub_Tree_TopLeft_Sparse)
                newTile = METATILE_GeneralHub_Tree_BottomLeft_Dense_Overlapped;
            else
                newTile = METATILE_GeneralHub_Tree_BottomLeft_Dense_Overlapped_Alt;
            break;
        case METATILE_GeneralHub_Tree_BottomRight_Dense:
            if(southTile == METATILE_GeneralHub_Tree_TopRight_Dense || southTile == METATILE_GeneralHub_Tree_TopRight_Sparse)
                newTile = METATILE_GeneralHub_Tree_BottomRight_Dense_Overlapped;
            else
                newTile = METATILE_GeneralHub_Tree_BottomRight_Dense_Overlapped_Alt;
            break;
        }
    }

    if(newTile != centreTile)
        MetatileSet_Tile(x, y, newTile | MAPGRID_COLLISION_MASK);
}

static void FixupTile_Trees_Fixup(s32 x, s32 y, u32 centreTile)
{
    bool8 north = IsCompatibleMetatileAt(x + 0, y - 1, METATILE_GeneralHub_Tree_Class);

    // Set the tile above this to the tree caps
    switch (centreTile)
    {
    case METATILE_GeneralHub_Tree_TopLeft_Sparse:
    case METATILE_GeneralHub_Tree_TopLeft_Dense:
        if(!north)
        {
            u32 northTile = GetCurrentAreaMetatileAt(x + 0, y - 1);

            if(northTile == METATILE_GeneralHub_Grass)
                MetatileSet_Tile(x, y - 1, METATILE_GeneralHub_Tree_TopLeft_CapGrass);
            else if(northTile == METATILE_GeneralHub_TallGrass)
                MetatileSet_Tile(x, y - 1, METATILE_GeneralHub_Tree_TopLeft_CapTallGrass);
        }
        break;
    case METATILE_GeneralHub_Tree_TopRight_Sparse:
    case METATILE_GeneralHub_Tree_TopRight_Dense:
        if(!north)
        {
            u32 northTile = GetCurrentAreaMetatileAt(x + 0, y - 1);

            if(northTile == METATILE_GeneralHub_Grass)
                MetatileSet_Tile(x, y - 1, METATILE_GeneralHub_Tree_TopRight_CapGrass);
            else if(northTile == METATILE_GeneralHub_TallGrass)
                MetatileSet_Tile(x, y - 1, METATILE_GeneralHub_Tree_TopRight_CapTallGrass);
        }
        break;
    }
}

////

static void FixupTileCommon(struct TileFixup* settings)
{
    s32 fromX, fromY, toX, toY;
    u8 x, y;
    u16 metatileId;
    bool8 ignoreHouseTiles = FALSE;
    u8 pathStyle = GetActiveHubMap()->homeStyles[HOME_STYLE_PATH];

    switch (gMapHeader.mapLayoutId)
    {
    case LAYOUT_ROGUE_AREA_HOME:
        ignoreHouseTiles = TRUE;
        fromX = sHomeRegionCoords[HOME_REGION_PLACEABLE_REGION].xStart;
        fromY = sHomeRegionCoords[HOME_REGION_PLACEABLE_REGION].yStart;
        toX = sHomeRegionCoords[HOME_REGION_PLACEABLE_REGION].xEnd;
        toY = sHomeRegionCoords[HOME_REGION_PLACEABLE_REGION].yEnd;
        break;

    default:
        {
            struct MapHeader const * mapHeader = Overworld_GetMapHeaderByGroupAndId(gSaveBlock1Ptr->location.mapGroup, gSaveBlock1Ptr->location.mapNum);
            fromX = 2;
            fromY = 2;
            toX = mapHeader->mapLayout->width - 2;
            toY = mapHeader->mapLayout->height - 1;
        }
        break;
    }

    // 3 pass system, horizontal, vertical + fixup

    // Horizontal
    for(y = fromY; y <= toY; ++y)
    {
        for(x = fromX; x <= toX; ++x)
        {
            // Don't adjust home tiles 
            if(ignoreHouseTiles && IsCoordInHomeRegion(x, y, HOME_REGION_HOUSE))
                continue;

            metatileId = MapGridGetMetatileIdAt(x + MAP_OFFSET, y + MAP_OFFSET);

            if(settings->pond && metatileId == METATILE_GeneralHub_Pond_Centre)
                FixupTile_Pond_Horizontal(x, y);

            else if(settings->path && metatileId == METATILE_GeneralHub_GrassPath_Centre)
                FixupTile_GrassPath_Horizontal(x, y);

            else if(settings->mountain && metatileId == METATILE_GeneralHub_Mountain_Centre)
                FixupTile_Mountain_Horizontal(x, y);

            else if(settings->trees && (metatileId == METATILE_GeneralHub_Tree_TopLeft_Sparse || metatileId == METATILE_GeneralHub_Tree_TopRight_Sparse || metatileId == METATILE_GeneralHub_Tree_BottomLeft_Sparse || metatileId == METATILE_GeneralHub_Tree_BottomRight_Sparse))
                FixupTile_Trees_Horizontal(x, y, metatileId);
        }
    }

    // Vertical + Fixup
    for(x = fromX; x <= toX; ++x)
    {
        for(y = fromY; y <= toY; ++y)
        {
            // Don't adjust home tiles 
            if(ignoreHouseTiles && IsCoordInHomeRegion(x, y, HOME_REGION_HOUSE))
                continue;

            metatileId = MapGridGetMetatileIdAt(x + MAP_OFFSET, y + MAP_OFFSET);

            if(settings->pond)
            {
                if(metatileId == METATILE_GeneralHub_Pond_Centre)
                    FixupTile_Pond_Vertical(x, y);

                if(IsCompatibleMetatile(METATILE_GeneralHub_Pond_Centre, metatileId))
                    FixupTile_Pond_Fixup(x, y, metatileId);
            }

            if(settings->path)
            {
                if(metatileId == METATILE_GeneralHub_GrassPath_Centre)
                    FixupTile_GrassPath_Vertical(x, y);

                if(IsCompatibleMetatile(METATILE_GeneralHub_GrassPath_Centre, metatileId))
                    FixupTile_GrassPath_Fixup(x, y, metatileId);
            }

            if(settings->mountain)
            {
                if(metatileId == METATILE_GeneralHub_Mountain_Centre)
                    FixupTile_Mountain_Vertical(x, y);

                if(IsCompatibleMetatile(METATILE_GeneralHub_Mountain_Centre, metatileId))
                    FixupTile_Mountain_Fixup(x, y, metatileId);
            }

            if(settings->trees)
            {
                //if(metatileId == METATILE_GeneralHub_Mountain_Centre)
                //    FixupTile_Mountain_Vertical(x, y);

                if(IsCompatibleMetatile(METATILE_GeneralHub_Tree_Class, metatileId))
                {
                    FixupTile_Trees_Vertical(x, y, metatileId);
                    FixupTile_Trees_Fixup(x, y, MapGridGetMetatileIdAt(x + MAP_OFFSET, y + MAP_OFFSET));
                }
            }
        }
    }
    
    // Final fixup to do path replacement
    if(settings->pathStyle && pathStyle != PATH_STYLE_GRASS)
    {
        u16 oldMetatileId;

        for(x = fromX; x <= toX; ++x)
        {
            for(y = fromY; y <= toY; ++y)
            {
                // Don't adjust home tiles 
                if(ignoreHouseTiles && IsCoordInHomeRegion(x, y, HOME_REGION_HOUSE))
                    continue;

                metatileId = MapGridGetMetatileIdAt(x + MAP_OFFSET, y + MAP_OFFSET);
                oldMetatileId = metatileId;

                if(metatileId == METATILE_GeneralHub_GrassPath_Centre)
                {
                    switch (pathStyle)
                    {
                    case PATH_STYLE_SAND:
                        metatileId = METATILE_GeneralHub_SandPath_Centre;
                        break;

                    case PATH_STYLE_STONE:
                        metatileId = METATILE_GeneralHub_StonePath_Centre;
                        break;

                    case PATH_STYLE_PEBBLES:
                        metatileId = METATILE_GeneralHub_Pebbles;
                        break;

                    case PATH_STYLE_MUDDY_TRACKS:
                        metatileId = METATILE_GeneralHub_MuddyTracks;
                        break;
                    }
                }
                else if(metatileId == METATILE_GeneralHub_GrassPath_Conn_EastWest_North)
                {
                    switch (pathStyle)
                    {
                    case PATH_STYLE_SAND:
                        metatileId = METATILE_GeneralHub_SandPath_Conn_EastWest_North;
                        break;

                    case PATH_STYLE_STONE:
                        metatileId = METATILE_GeneralHub_StonePath_Conn_EastWest_North;
                        break;

                    case PATH_STYLE_PEBBLES:
                        metatileId = METATILE_GeneralHub_Pebbles;
                        break;

                    case PATH_STYLE_MUDDY_TRACKS:
                        metatileId = METATILE_GeneralHub_MuddyTracks;
                        break;
                    }
                }
                else if(metatileId == METATILE_GeneralHub_GrassPath_Conn_EastWest_South)
                {
                    switch (pathStyle)
                    {
                    case PATH_STYLE_SAND:
                        metatileId = METATILE_GeneralHub_SandPath_Conn_EastWest_South;
                        break;

                    case PATH_STYLE_STONE:
                        metatileId = METATILE_GeneralHub_StonePath_Conn_EastWest_South;
                        break;

                    case PATH_STYLE_PEBBLES:
                        metatileId = METATILE_GeneralHub_Pebbles;
                        break;

                    case PATH_STYLE_MUDDY_TRACKS:
                        metatileId = METATILE_GeneralHub_MuddyTracks;
                        break;
                    }
                }
                else if(metatileId == METATILE_GeneralHub_GrassPath_Conn_NorthEast)
                {
                    switch (pathStyle)
                    {
                    case PATH_STYLE_SAND:
                        metatileId = METATILE_GeneralHub_SandPath_Conn_NorthEast;
                        break;

                    case PATH_STYLE_STONE:
                        metatileId = METATILE_GeneralHub_StonePath_Conn_NorthEast;
                        break;

                    case PATH_STYLE_PEBBLES:
                        metatileId = METATILE_GeneralHub_Pebbles;
                        break;

                    case PATH_STYLE_MUDDY_TRACKS:
                        metatileId = METATILE_GeneralHub_MuddyTracks;
                        break;
                    }
                }
                else if(metatileId == METATILE_GeneralHub_GrassPath_Conn_NorthSouth_East)
                {
                    switch (pathStyle)
                    {
                    case PATH_STYLE_SAND:
                        metatileId = METATILE_GeneralHub_SandPath_Conn_NorthSouth_East;
                        break;

                    case PATH_STYLE_STONE:
                        metatileId = METATILE_GeneralHub_StonePath_Conn_NorthSouth_East;
                        break;

                    case PATH_STYLE_PEBBLES:
                        metatileId = METATILE_GeneralHub_Pebbles;
                        break;

                    case PATH_STYLE_MUDDY_TRACKS:
                        metatileId = METATILE_GeneralHub_MuddyTracks;
                        break;
                    }
                }
                else if(metatileId == METATILE_GeneralHub_GrassPath_Conn_NorthSouth_West)
                {
                    switch (pathStyle)
                    {
                    case PATH_STYLE_SAND:
                        metatileId = METATILE_GeneralHub_SandPath_Conn_NorthSouth_West;
                        break;

                    case PATH_STYLE_STONE:
                        metatileId = METATILE_GeneralHub_StonePath_Conn_NorthSouth_West;
                        break;

                    case PATH_STYLE_PEBBLES:
                        metatileId = METATILE_GeneralHub_Pebbles;
                        break;

                    case PATH_STYLE_MUDDY_TRACKS:
                        metatileId = METATILE_GeneralHub_MuddyTracks;
                        break;
                    }
                }
                else if(metatileId == METATILE_GeneralHub_GrassPath_Conn_NorthWest)
                {
                    switch (pathStyle)
                    {
                    case PATH_STYLE_SAND:
                        metatileId = METATILE_GeneralHub_SandPath_Conn_NorthWest;
                        break;

                    case PATH_STYLE_STONE:
                        metatileId = METATILE_GeneralHub_StonePath_Conn_NorthWest;
                        break;

                    case PATH_STYLE_PEBBLES:
                        metatileId = METATILE_GeneralHub_Pebbles;
                        break;

                    case PATH_STYLE_MUDDY_TRACKS:
                        metatileId = METATILE_GeneralHub_MuddyTracks;
                        break;
                    }
                }
                else if(metatileId == METATILE_GeneralHub_GrassPath_Conn_SouthEast)
                {
                    switch (pathStyle)
                    {
                    case PATH_STYLE_SAND:
                        metatileId = METATILE_GeneralHub_SandPath_Conn_SouthEast;
                        break;

                    case PATH_STYLE_STONE:
                        metatileId = METATILE_GeneralHub_StonePath_Conn_SouthEast;
                        break;

                    case PATH_STYLE_PEBBLES:
                        metatileId = METATILE_GeneralHub_Pebbles;
                        break;

                    case PATH_STYLE_MUDDY_TRACKS:
                        metatileId = METATILE_GeneralHub_MuddyTracks;
                        break;
                    }
                }
                else if(metatileId == METATILE_GeneralHub_GrassPath_Conn_SouthWest)
                {
                    switch (pathStyle)
                    {
                    case PATH_STYLE_SAND:
                        metatileId = METATILE_GeneralHub_SandPath_Conn_SouthWest;
                        break;

                    case PATH_STYLE_STONE:
                        metatileId = METATILE_GeneralHub_StonePath_Conn_SouthWest;
                        break;

                    case PATH_STYLE_PEBBLES:
                        metatileId = METATILE_GeneralHub_Pebbles;
                        break;

                    case PATH_STYLE_MUDDY_TRACKS:
                        metatileId = METATILE_GeneralHub_MuddyTracks;
                        break;
                    }
                }

                if(oldMetatileId != metatileId)
                    MapGridSetMetatileIdAt(x + MAP_OFFSET, y + MAP_OFFSET, metatileId);
            }
        }
    }
}

void RogueHub_ReloadObjectsAndTiles()
{
    if(RogueHub_IsPlayerBaseLayout(gMapHeader.mapLayoutId))
    {
        TrySpawnObjectEvents(0, 0);
    }
}

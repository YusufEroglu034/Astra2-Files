#pragma once

#include "CommonDefines.h"

enum EItemMisc
{
	ITEM_NAME_MAX_LEN			= 48,
	ITEM_VALUES_MAX_NUM			= 6,
	ITEM_SMALL_DESCR_MAX_LEN	= 256,
	ITEM_LIMIT_MAX_NUM			= 2,
	ITEM_APPLY_MAX_NUM			= 3,
	ITEM_SOCKET_MAX_NUM			= 6, // @correction144

	// @correction105 BEGIN
	ITEM_ATTRIBUTE_NORM_NUM		= 5,
	ITEM_ATTRIBUTE_RARE_NUM		= 2,

	ITEM_ATTRIBUTE_NORM_START	= 0,
	ITEM_ATTRIBUTE_NORM_END		= ITEM_ATTRIBUTE_NORM_START + ITEM_ATTRIBUTE_NORM_NUM,

	ITEM_ATTRIBUTE_RARE_START	= ITEM_ATTRIBUTE_NORM_END,
	ITEM_ATTRIBUTE_RARE_END		= ITEM_ATTRIBUTE_RARE_START + ITEM_ATTRIBUTE_RARE_NUM,

	ITEM_ATTRIBUTE_MAX_NUM		= ITEM_ATTRIBUTE_RARE_END,
	// @correction105 END
	ITEM_ATTRIBUTE_MAX_LEVEL	= 5,
	ITEM_AWARD_WHY_MAX_LEN		= 50,

	REFINE_MATERIAL_MAX_NUM		= 5,

	ITEM_ELK_VNUM				= 50026,
};

const uint8_t ITEM_SOCKET_REMAIN_SEC = 0;
enum EItemValueIdice
{
	ITEM_VALUE_DRAGON_SOUL_POLL_OUT_BONUS_IDX = 0,
	ITEM_VALUE_CHARGING_AMOUNT_IDX = 0,
	ITEM_VALUE_SECONDARY_COIN_UNIT_IDX = 0,
};
enum EItemDragonSoulSockets
{
	ITEM_SOCKET_DRAGON_SOUL_ACTIVE_IDX = 2,
	ITEM_SOCKET_CHARGING_AMOUNT_IDX = 2,
};
enum EItemUniqueSockets
{
	ITEM_SOCKET_UNIQUE_SAVE_TIME = ITEM_SOCKET_MAX_NUM - 2,
	ITEM_SOCKET_UNIQUE_REMAIN_TIME = ITEM_SOCKET_MAX_NUM - 1
};

enum EItemTypes
{
    ITEM_NONE,
    ITEM_WEAPON,
    ITEM_ARMOR,
    ITEM_USE,
    ITEM_AUTOUSE,
    ITEM_MATERIAL,
    ITEM_SPECIAL,
    ITEM_TOOL,
    ITEM_LOTTERY,
    ITEM_ELK,
    ITEM_METIN,
    ITEM_CONTAINER,
    ITEM_FISH,
    ITEM_ROD,
    ITEM_RESOURCE,
    ITEM_CAMPFIRE,
    ITEM_UNIQUE,
    ITEM_SKILLBOOK,
    ITEM_QUEST,
    ITEM_POLYMORPH,
    ITEM_TREASURE_BOX,
    ITEM_TREASURE_KEY,
    ITEM_SKILLFORGET,
    ITEM_GIFTBOX,
    ITEM_PICK,
    ITEM_HAIR,
    ITEM_TOTEM,
	ITEM_BLEND,
	ITEM_COSTUME,
	ITEM_DS,
	ITEM_SPECIAL_DS,
	ITEM_EXTRACT,
	ITEM_SECONDARY_COIN,
	ITEM_RING,
	ITEM_BELT,
	ITEM_PET,
#ifdef __SPECIAL_GACHA__
	ITEM_GACHA,
#endif
};

enum EPetSubTypes
{
#ifdef __GROWTH_PET__
	PET_EGG,
	PET_UPBRINGING,
	PET_BAG,
	PET_FEEDSTUFF,
	PET_SKILL,
	PET_SKILL_DEL_BOOK,
	PET_NAME_CHANGE,
	PET_EXPFOOD,
	PET_SKILL_ALL_DEL_BOOK,
	PET_EXPFOOD_PER,
	PET_ATTR_CHANGE,
	PET_ATTR_CHANGE_ITEMVNUM,
#endif
	PET_PAY
};

enum EMetinSubTypes
{
	METIN_NORMAL,
	METIN_GOLD,
};

enum EWeaponSubTypes
{
	WEAPON_SWORD,
	WEAPON_DAGGER,
	WEAPON_BOW,
	WEAPON_TWO_HANDED,
	WEAPON_BELL,
	WEAPON_FAN,
	WEAPON_ARROW,
	WEAPON_MOUNT_SPEAR,
#ifdef ENABLE_WOLFMAN_CHARACTER // @correction190
	WEAPON_CLAW,
#endif
#ifdef __QUIVER__ // @correction203
	WEAPON_QUIVER,
#endif
	WEAPON_NUM_TYPES,
};

enum EArmorSubTypes
{
	ARMOR_BODY,
	ARMOR_HEAD,
	ARMOR_SHIELD,
	ARMOR_WRIST,
	ARMOR_FOOTS,
	ARMOR_NECK,
	ARMOR_EAR,
#ifdef __PENDANT__
	ARMOR_PENDANT,
#endif
	ARMOR_NUM_TYPES
};

enum ECostumeSubTypes
{
	COSTUME_BODY = ARMOR_BODY,
	COSTUME_HAIR = ARMOR_HEAD,
#ifdef __COSTUME_MOUNT__
	COSTUME_MOUNT,
#endif
#ifdef __ACCE_SYSTEM__ // @correction191
	COSTUME_ACCE,
#endif
#ifdef __COSTUME_WEAPON__ // @correction204
	COSTUME_WEAPON,
#endif
#ifdef __COSTUME_EFFECT__
	COSTUME_EFFECT,
#endif
	COSTUME_NUM_TYPES,
};

enum EDragonSoulSubType
{
	DS_SLOT1,
	DS_SLOT2,
	DS_SLOT3,
	DS_SLOT4,
	DS_SLOT5,
	DS_SLOT6,
	DS_SLOT_MAX,
};

enum EDragonSoulGradeTypes
{
	DRAGON_SOUL_GRADE_NORMAL,
	DRAGON_SOUL_GRADE_BRILLIANT,
	DRAGON_SOUL_GRADE_RARE,
	DRAGON_SOUL_GRADE_ANCIENT,
	DRAGON_SOUL_GRADE_LEGENDARY,
#ifdef __DS_GRADE_MYTH__
	DRAGON_SOUL_GRADE_MYTH,
#endif
	DRAGON_SOUL_GRADE_MAX,

};

enum EDragonSoulStepTypes
{
	DRAGON_SOUL_STEP_LOWEST,
	DRAGON_SOUL_STEP_LOW,
	DRAGON_SOUL_STEP_MID,
	DRAGON_SOUL_STEP_HIGH,
	DRAGON_SOUL_STEP_HIGHEST,
	DRAGON_SOUL_STEP_MAX,
};
#define DRAGON_SOUL_STRENGTH_MAX 7

enum EDSInventoryMaxNum
{
	DRAGON_SOUL_INVENTORY_MAX_NUM = DS_SLOT_MAX * DRAGON_SOUL_GRADE_MAX * DRAGON_SOUL_BOX_SIZE,
};

enum EFishSubTypes
{
	FISH_ALIVE,
	FISH_DEAD,
};

enum EResourceSubTypes
{
	RESOURCE_FISHBONE,
	RESOURCE_WATERSTONEPIECE,
	RESOURCE_WATERSTONE,
	RESOURCE_BLOOD_PEARL,
	RESOURCE_BLUE_PEARL,
	RESOURCE_WHITE_PEARL,
	RESOURCE_BUCKET,
	RESOURCE_CRYSTAL,
	RESOURCE_GEM,
	RESOURCE_STONE,
	RESOURCE_METIN,
	RESOURCE_ORE,
};

enum EUniqueSubTypes
{
	UNIQUE_NONE,
	UNIQUE_BOOK,
	UNIQUE_SPECIAL_RIDE,
	UNIQUE_SPECIAL_MOUNT_RIDE,
};

enum EUseSubTypes
{
	USE_POTION,
	USE_TALISMAN,
	USE_TUNING,
	USE_MOVE,
	USE_TREASURE_BOX,
	USE_MONEYBAG,
	USE_BAIT,
	USE_ABILITY_UP,
	USE_AFFECT,
	USE_CREATE_STONE,
	USE_SPECIAL,
	USE_POTION_NODELAY,
	USE_CLEAR,
	USE_INVISIBILITY,
	USE_DETACHMENT,
	USE_BUCKET,
	USE_POTION_CONTINUE,
	USE_CLEAN_SOCKET,
	USE_CHANGE_ATTRIBUTE,
	USE_ADD_ATTRIBUTE,
	USE_ADD_ACCESSORY_SOCKET,
	USE_PUT_INTO_ACCESSORY_SOCKET,
	USE_ADD_ATTRIBUTE2,
	USE_RECIPE,
	USE_CHANGE_ATTRIBUTE2,
	USE_BIND,
	USE_UNBIND,
	USE_TIME_CHARGE_PER,
	USE_TIME_CHARGE_FIX,
	USE_PUT_INTO_BELT_SOCKET,
	USE_PUT_INTO_RING_SOCKET,
#ifdef __MOVE_COSTUME_ATTR__
	USE_ADD_COSTUME_ATTRIBUTE,
	USE_RESET_COSTUME_ATTR,
#endif
};

enum EExtractSubTypes
{
	EXTRACT_DRAGON_SOUL,
	EXTRACT_DRAGON_HEART,
};

enum EAutoUseSubTypes
{
	AUTOUSE_POTION,
	AUTOUSE_ABILITY_UP,
	AUTOUSE_BOMB,
	AUTOUSE_GOLD,
	AUTOUSE_MONEYBAG,
	AUTOUSE_TREASURE_BOX
};

enum EMaterialSubTypes
{
	MATERIAL_LEATHER,
	MATERIAL_BLOOD,
	MATERIAL_ROOT,
	MATERIAL_NEEDLE,
	MATERIAL_JEWEL,
	MATERIAL_DS_REFINE_NORMAL,
	MATERIAL_DS_REFINE_BLESSED,
	MATERIAL_DS_REFINE_HOLLY,
#ifdef __DS_REFINE_MASTER__
	MATERIAL_DS_REFINE_MASTER,
#endif
	MATERIAL_MAX_NUM,
};

enum ESpecialSubTypes
{
	SPECIAL_MAP,
	SPECIAL_KEY,
	SPECIAL_DOC,
	SPECIAL_SPIRIT,
};

enum EToolSubTypes
{
	TOOL_FISHING_ROD
};

enum ELotterySubTypes
{
	LOTTERY_TICKET,
	LOTTERY_INSTANT
};

enum EItemFlag
{
	ITEM_FLAG_REFINEABLE		= (1 << 0),
	ITEM_FLAG_SAVE			= (1 << 1),
	ITEM_FLAG_STACKABLE		= (1 << 2),
	ITEM_FLAG_COUNT_PER_1GOLD	= (1 << 3),
	ITEM_FLAG_SLOW_QUERY		= (1 << 4),
	ITEM_FLAG_RARE			= (1 << 5),
	ITEM_FLAG_UNIQUE		= (1 << 6),
	ITEM_FLAG_MAKECOUNT		= (1 << 7),
	ITEM_FLAG_IRREMOVABLE		= (1 << 8),
	ITEM_FLAG_CONFIRM_WHEN_USE	= (1 << 9),
	ITEM_FLAG_QUEST_USE		= (1 << 10),
	ITEM_FLAG_QUEST_USE_MULTIPLE	= (1 << 11),
	ITEM_FLAG_QUEST_GIVE		= (1 << 12),
	ITEM_FLAG_LOG			= (1 << 13),
	ITEM_FLAG_APPLICABLE		= (1 << 14),
};

enum EItemAntiFlag
{
	ITEM_ANTIFLAG_FEMALE	= (1 << 0),
	ITEM_ANTIFLAG_MALE		= (1 << 1),
	ITEM_ANTIFLAG_WARRIOR	= (1 << 2),
	ITEM_ANTIFLAG_ASSASSIN	= (1 << 3),
	ITEM_ANTIFLAG_SURA		= (1 << 4),
	ITEM_ANTIFLAG_SHAMAN	= (1 << 5),
	ITEM_ANTIFLAG_GET		= (1 << 6),
	ITEM_ANTIFLAG_DROP		= (1 << 7),
	ITEM_ANTIFLAG_SELL		= (1 << 8),
	ITEM_ANTIFLAG_EMPIRE_A	= (1 << 9),
	ITEM_ANTIFLAG_EMPIRE_B	= (1 << 10),
	ITEM_ANTIFLAG_EMPIRE_C	= (1 << 11),
	ITEM_ANTIFLAG_SAVE		= (1 << 12),
	ITEM_ANTIFLAG_GIVE		= (1 << 13),
	ITEM_ANTIFLAG_PKDROP	= (1 << 14),
	ITEM_ANTIFLAG_STACK		= (1 << 15),
	ITEM_ANTIFLAG_MYSHOP	= (1 << 16),
	ITEM_ANTIFLAG_SAFEBOX	= (1 << 17),
#ifdef ENABLE_WOLFMAN_CHARACTER // @correction190
	ITEM_ANTIFLAG_WOLFMAN	= (1 << 18),
#endif
};

enum EItemWearableFlag
{
	WEARABLE_BODY	= (1 << 0),
	WEARABLE_HEAD	= (1 << 1),
	WEARABLE_FOOTS	= (1 << 2),
	WEARABLE_WRIST	= (1 << 3),
	WEARABLE_WEAPON	= (1 << 4),
	WEARABLE_NECK	= (1 << 5),
	WEARABLE_EAR	= (1 << 6),
	WEARABLE_UNIQUE	= (1 << 7),
	WEARABLE_SHIELD	= (1 << 8),
	WEARABLE_ARROW	= (1 << 9),
	WEARABLE_HAIR	= (1 << 10),
	WEARABLE_ABILITY = (1 << 11),
#ifdef __PENDANT__
	WEARABLE_PENDANT = (1 << 12),
#endif
#ifdef __COSTUME_EFFECT__
	WEARABLE_COSTUME_EFFECT_ARMOR	= (1 << 13),
	WEARABLE_COSTUME_EFFECT_WEAPON	= (1 << 14),
#endif
};

enum ELimitTypes
{
	LIMIT_NONE,

	LIMIT_LEVEL,
	LIMIT_STR,
	LIMIT_DEX,
	LIMIT_INT,
	LIMIT_CON,
	LIMIT_PCBANG,

	LIMIT_REAL_TIME,

	LIMIT_REAL_TIME_START_FIRST_USE,

	LIMIT_TIMER_BASED_ON_WEAR,

	LIMIT_MAX_NUM
};

enum EAttrAddonTypes
{
	ATTR_ADDON_NONE,
	ATTR_DAMAGE_ADDON = -1,
};

enum ERefineType
{
	REFINE_TYPE_NORMAL,
	REFINE_TYPE_NOT_USED1,
	REFINE_TYPE_SCROLL,
	REFINE_TYPE_HYUNIRON,
	REFINE_TYPE_MONEY_ONLY,
	REFINE_TYPE_MUSIN,
	REFINE_TYPE_BDRAGON,
#ifdef __RITUEL_STONE__
	REFINE_TYPE_RITUALS_SCROLL,
#endif
};

#ifdef __ACCE_SYSTEM__ // @correction191
enum EAcceInfo
{
	ACCE_ABSORPTION_SOCKET = 0,
	ACCE_ABSORBED_SOCKET = 1,
	ACCE_GRADE_1_ABS = 1,
	ACCE_GRADE_2_ABS = 5,
	ACCE_GRADE_3_ABS = 10,
	ACCE_GRADE_4_ABS_MIN = 11,
	ACCE_GRADE_4_ABS_MAX = 25,
	ACCE_GRADE_NEW_ABS_30 = 30,
	ACCE_GRADE_4_ABS_MAX_COMB = 19,
	ACCE_GRADE_4_ABS_RANGE = 5,
	ACCE_EFFECT_FROM_ABS = 19,
	ACCE_CLEAN_ATTR_VALUE0 = 7,
	ACCE_WINDOW_MAX_MATERIALS = 2,
	ACCE_GRADE_1_PRICE = 100000,
	ACCE_GRADE_2_PRICE = 200000,
	ACCE_GRADE_3_PRICE = 300000,
	ACCE_GRADE_4_PRICE = 500000,
	ACCE_COMBINE_GRADE_1 = 80,
	ACCE_COMBINE_GRADE_2 = 70,
	ACCE_COMBINE_GRADE_3 = 50,
	ACCE_COMBINE_GRADE_4 = 30,
};
#endif

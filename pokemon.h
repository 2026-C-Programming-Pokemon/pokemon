#ifndef POKEMON_H
#define POKEMON_H

#define POKEMON_COUNT 81
#define TEAM_SIZE 6
#define MOVE_SLOT_COUNT 4
#define BATTLE_LEVEL 60
#define DEFAULT_INDIVIDUAL_VALUE 31

/* 포켓몬 타입을 숫자 상수로 다루기 위한 열거형입니다. */
typedef enum {
    TYPE_NORMAL,
    TYPE_FIRE,
    TYPE_WATER,
    TYPE_ELECTRIC,
    TYPE_GRASS,
    TYPE_ICE,
    TYPE_FIGHTING,
    TYPE_POISON,
    TYPE_GROUND,
    TYPE_FLYING,
    TYPE_PSYCHIC,
    TYPE_BUG,
    TYPE_ROCK,
    TYPE_GHOST,
    TYPE_DRAGON,
    TYPE_DARK,
    TYPE_STEEL,
    TYPE_FAIRY,
    TYPE_COUNT,
    TYPE_NONE
} PokemonType;

/* 도감에 저장되는 기본 포켓몬 정보입니다. */
typedef struct {
    int id;
    char name[32];
    PokemonType type1;
    PokemonType type2;
    int hp;
    int atk;
    int def;
    int spa;
    int spd;
    int agi;
} Pokemon;

/* 물리 기술인지 특수 기술인지 구분합니다. */
typedef enum {
    MOVE_PHYSICAL,
    MOVE_SPECIAL
} MoveCategory;

/* 랭크업/디버프 기술이 바꿀 수 있는 능력치 종류입니다. */
typedef enum {
    STAT_NONE,
    STAT_ATK,
    STAT_DEF,
    STAT_SPA,
    STAT_SPD,
    STAT_AGI,
    STAT_ACC,
    STAT_EVA
} StatType;

/* 전투 중 포켓몬에게 걸릴 수 있는 상태이상입니다. */
typedef enum {
    STATUS_NONE,
    STATUS_SLEEP,
    STATUS_PARALYSIS,
    STATUS_POISON,
    STATUS_BURN,
    STATUS_FREEZE
} StatusCondition;

/* 기술 하나의 타입, 분류, 위력, 능력치 변화 정보를 저장합니다. */
typedef struct {
    char name[32];
    PokemonType type;
    MoveCategory category;
    int power;
    StatType stat;
    int statChange;
} Move;

/* 전투 중 변하는 현재 HP, 기술, 랭크, 상태이상을 포함한 포켓몬 정보입니다. */
typedef struct {
    Pokemon pokemon;
    int level;
    int currentHp;
    Move moves[MOVE_SLOT_COUNT];
    int moveCount;
    int atkStage;
    int defStage;
    int spaStage;
    int spdStage;
    int agiStage;
    int accStage;
    int evaStage;
    StatusCondition status;
    int sleepTurns;
    int rechargeTurns;
} BattlePokemon;

/* 플레이어, 사천왕, 챔피언처럼 포켓몬 팀을 가진 대상을 표현합니다. */
typedef struct {
    char name[32];
    BattlePokemon team[TEAM_SIZE];
    int teamSize;
} Trainer;


#endif

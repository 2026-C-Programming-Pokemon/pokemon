#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

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

/* enum 타입 값을 화면에 출력할 문자열로 바꿔주는 배열입니다. */
const char *typeNames[] = {
    "normal",
    "fire",
    "water",
    "electric",
    "grass",
    "ice",
    "fighting",
    "poison",
    "ground",
    "flying",
    "psychic",
    "bug",
    "rock",
    "ghost",
    "dragon",
    "dark",
    "steel",
    "fairy"
};

/* typeChart[공격 타입][방어 타입] 형태의 타입 상성표입니다. */
float typeChart[TYPE_COUNT][TYPE_COUNT] = {
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0.5, 0, 1, 1, 0.5, 1},
    {1, 0.5, 0.5, 1, 2, 2, 1, 1, 1, 1, 1, 2, 0.5, 1, 0.5, 1, 2, 1},
    {1, 2, 0.5, 1, 0.5, 1, 1, 1, 2, 1, 1, 1, 2, 1, 0.5, 1, 1, 1},
    {1, 1, 2, 0.5, 0.5, 1, 1, 1, 0, 2, 1, 1, 1, 1, 0.5, 1, 1, 1},
    {1, 0.5, 2, 1, 0.5, 1, 1, 0.5, 2, 0.5, 1, 0.5, 2, 1, 0.5, 1, 0.5, 1},
    {1, 0.5, 0.5, 1, 2, 0.5, 1, 1, 2, 2, 1, 1, 1, 1, 2, 1, 0.5, 1},
    {2, 1, 1, 1, 1, 2, 1, 0.5, 1, 0.5, 0.5, 0.5, 2, 0, 1, 2, 2, 0.5},
    {1, 1, 1, 1, 2, 1, 1, 0.5, 0.5, 1, 1, 1, 0.5, 0.5, 1, 1, 0, 2},
    {1, 2, 1, 2, 0.5, 1, 1, 2, 1, 0, 1, 0.5, 2, 1, 1, 1, 2, 1},
    {1, 1, 1, 0.5, 2, 1, 2, 1, 1, 1, 1, 2, 0.5, 1, 1, 1, 0.5, 1},
    {1, 1, 1, 1, 1, 1, 2, 2, 1, 1, 0.5, 1, 1, 1, 1, 0, 0.5, 1},
    {1, 0.5, 1, 1, 2, 1, 0.5, 0.5, 1, 0.5, 2, 1, 1, 0.5, 1, 2, 0.5, 0.5},
    {1, 2, 1, 1, 1, 2, 0.5, 1, 0.5, 2, 1, 2, 1, 1, 1, 1, 0.5, 1},
    {0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 1, 1, 2, 1, 0.5, 1, 1},
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 1, 0.5, 0},
    {1, 1, 1, 1, 1, 1, 0.5, 1, 1, 1, 2, 1, 1, 2, 1, 0.5, 1, 0.5},
    {1, 0.5, 0.5, 0.5, 1, 2, 1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 0.5, 2},
    {1, 0.5, 1, 1, 1, 1, 2, 0.5, 1, 1, 1, 1, 1, 1, 2, 2, 0.5, 1}
};

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
} BattlePokemon;

/* 플레이어, 사천왕, 챔피언처럼 포켓몬 팀을 가진 대상을 표현합니다. */
typedef struct {
    char name[32];
    BattlePokemon team[TEAM_SIZE];
    int teamSize;
} Trainer;

/* 현재 게임에서 사용할 포켓몬 도감입니다. */
Pokemon pokedex[POKEMON_COUNT] = {
    {1, "이상해꽃", TYPE_GRASS, TYPE_POISON, 80, 100, 123, 122, 120, 80},
    {2, "리자몽", TYPE_FIRE, TYPE_FLYING, 78, 84, 78, 109, 85, 100},
    {3, "거북왕", TYPE_WATER, TYPE_NONE, 79, 83, 100, 85, 105, 78},
    {4, "버터풀", TYPE_BUG, TYPE_FLYING, 60, 45, 50, 90, 80, 70},
    {5, "독침붕", TYPE_BUG, TYPE_POISON, 65, 90, 40, 45, 80, 75},
    {6, "피죤투", TYPE_NORMAL, TYPE_FLYING, 83, 80, 75, 70, 70, 101},
    {7, "레트라", TYPE_NORMAL, TYPE_NONE, 55, 81, 60, 50, 70, 97},
    {8, "깨비드릴조", TYPE_NORMAL, TYPE_FLYING, 65, 90, 65, 61, 61, 100},
    {9, "아보크", TYPE_POISON, TYPE_NONE, 60, 95, 69, 65, 79, 80},
    {10, "라이츄", TYPE_ELECTRIC, TYPE_NONE, 60, 90, 55, 90, 80, 110},
    {11, "고지", TYPE_GROUND, TYPE_NONE, 75, 100, 110, 45, 55, 65},
    {12, "니드퀸", TYPE_GROUND, TYPE_POISON, 90, 92, 87, 75, 85, 76},
    {13, "니드킹", TYPE_GROUND, TYPE_POISON, 81, 102, 77, 85, 75, 85},
    {14, "픽시", TYPE_NORMAL, TYPE_NONE, 95, 70, 73, 95, 90, 60},
    {15, "나인테일", TYPE_FIRE, TYPE_NONE, 73, 76, 75, 81, 100, 100},
    {16, "푸크린", TYPE_NORMAL, TYPE_NONE, 140, 70, 45, 85, 50, 45},
    {17, "골벳", TYPE_FLYING, TYPE_POISON, 75, 80, 70, 65, 75, 90},
    {18, "라플레시아", TYPE_GRASS, TYPE_POISON, 75, 80, 85, 110, 90, 50},
    {19, "파라섹트", TYPE_BUG, TYPE_GRASS, 60, 95, 80, 60, 80, 30},
    {20, "도나리", TYPE_BUG, TYPE_POISON, 70, 65, 60, 90, 75, 90},
    {21, "닥트리오", TYPE_GROUND, TYPE_NONE, 35, 100, 50, 50, 70, 120},
    {22, "페르시온", TYPE_NORMAL, TYPE_NONE, 65, 70, 60, 65, 65, 115},
    {23, "골덕", TYPE_WATER, TYPE_NONE, 80, 82, 78, 95, 80, 85},
    {24, "성원숭", TYPE_FIGHTING, TYPE_NONE, 65, 105, 60, 60, 70, 95},
    {25, "윈디", TYPE_FIRE, TYPE_NONE, 90, 110, 80, 100, 80, 95},
    {26, "강챙이", TYPE_WATER, TYPE_FIGHTING, 90, 95, 95, 70, 90, 70},
    {27, "후딘", TYPE_PSYCHIC, TYPE_NONE, 55, 50, 45, 135, 95, 120},
    {28, "괴력몬", TYPE_FIGHTING, TYPE_NONE, 90, 130, 80, 65, 85, 55},
    {29, "우츠보트", TYPE_GRASS, TYPE_POISON, 80, 105, 65, 100, 70, 70},
    {30, "독파리", TYPE_WATER, TYPE_POISON, 80, 70, 65, 80, 120, 100},
    {31, "딱구리", TYPE_ROCK, TYPE_GROUND, 80, 120, 130, 55, 65, 45},
    {32, "날쌩마", TYPE_FIRE, TYPE_NONE, 65, 100, 70, 80, 80, 105},
    {33, "야도란", TYPE_WATER, TYPE_PSYCHIC, 95, 100, 95, 100, 70, 30},
    {34, "레어코일", TYPE_STEEL, TYPE_ELECTRIC, 50, 60, 95, 120, 70, 70},
    {35, "파오리", TYPE_NORMAL, TYPE_FLYING, 52, 90, 55, 58, 62, 60},
    {36, "두트리오", TYPE_NORMAL, TYPE_FLYING, 60, 110, 70, 60, 60, 110},
    {37, "쥬레곤", TYPE_WATER, TYPE_ICE, 90, 70, 80, 70, 95, 70},
    {38, "질뻐기", TYPE_POISON, TYPE_NONE, 105, 105, 75, 65, 100, 50},
    {39, "파르셀", TYPE_WATER, TYPE_ICE, 50, 95, 180, 85, 45, 70},
    {40, "팬텀", TYPE_GHOST, TYPE_POISON, 60, 65, 60, 130, 75, 110},
    {41, "롱스톤", TYPE_ROCK, TYPE_GROUND, 35, 45, 160, 30, 45, 70},
    {42, "슬리퍼", TYPE_PSYCHIC, TYPE_NONE, 85, 73, 70, 73, 115, 67},
    {43, "킹크랩", TYPE_WATER, TYPE_NONE, 55, 130, 115, 50, 50, 75},
    {44, "붐볼", TYPE_ELECTRIC, TYPE_NONE, 60, 50, 70, 80, 80, 150},
    {45, "나시", TYPE_GRASS, TYPE_PSYCHIC, 95, 95, 85, 125, 75, 55},
    {46, "텅구리", TYPE_GROUND, TYPE_NONE, 60, 80, 110, 50, 80, 45},
    {47, "시라소몬", TYPE_FIGHTING, TYPE_NONE, 50, 120, 53, 35, 110, 87},
    {48, "홍수몬", TYPE_FIGHTING, TYPE_NONE, 50, 105, 79, 35, 110, 78},
    {49, "내루미", TYPE_NORMAL, TYPE_NONE, 90, 55, 75, 60, 75, 30},
    {50, "또도가스", TYPE_POISON, TYPE_NONE, 65, 90, 120, 85, 70, 60},
    {51, "코뿌리", TYPE_ROCK, TYPE_GROUND, 105, 130, 120, 45, 45, 40},
    {52, "럭키", TYPE_NORMAL, TYPE_NONE, 250, 5, 5, 35, 105, 50},
    {53, "덩쿠리", TYPE_GRASS, TYPE_NONE, 65, 55, 115, 100, 40, 60},
    {54, "캥카", TYPE_NORMAL, TYPE_NONE, 105, 95, 80, 40, 80, 90},
    {55, "시드라", TYPE_WATER, TYPE_NONE, 55, 65, 95, 95, 45, 85},
    {56, "왕콘치", TYPE_WATER, TYPE_NONE, 80, 92, 65, 65, 80, 68},
    {57, "아쿠스타", TYPE_WATER, TYPE_PSYCHIC, 60, 75, 85, 100, 85, 115},
    {58, "마임맨", TYPE_PSYCHIC, TYPE_NONE, 40, 45, 65, 100, 120, 90},
    {59, "스라크", TYPE_BUG, TYPE_FLYING, 70, 110, 80, 55, 80, 105},
    {60, "루주라", TYPE_ICE, TYPE_PSYCHIC, 65, 50, 35, 115, 85, 95},
    {61, "에레브", TYPE_ELECTRIC, TYPE_NONE, 65, 83, 57, 95, 85, 105},
    {62, "마그마", TYPE_FIRE, TYPE_NONE, 65, 95, 57, 100, 85, 93},
    {63, "쁘사이저", TYPE_BUG, TYPE_NONE, 65, 125, 100, 55, 70, 85},
    {64, "켄타로스", TYPE_NORMAL, TYPE_NONE, 75, 100, 95, 40, 70, 110},
    {65, "갸라도스", TYPE_WATER, TYPE_FLYING, 95, 125, 79, 60, 100, 81},
    {66, "라프라스", TYPE_WATER, TYPE_ICE, 130, 85, 80, 85, 95, 60},
    {67, "메타몽", TYPE_NORMAL, TYPE_NONE, 40, 45, 65, 100, 120, 90},
    {68, "샤미드", TYPE_WATER, TYPE_NONE, 130, 65, 60, 110, 95, 65},
    {69, "쥬피썬더", TYPE_ELECTRIC, TYPE_NONE, 65, 65, 60, 110, 95, 130},
    {70, "부스터", TYPE_FIRE, TYPE_NONE, 65, 130, 60, 95, 110, 65},
    {71, "폴리곤", TYPE_NORMAL, TYPE_NONE, 65, 60, 70, 85, 75, 40},
    {72, "암스타", TYPE_ROCK, TYPE_WATER, 70, 60, 125, 115, 70, 55},
    {73, "투구푸스", TYPE_ROCK, TYPE_WATER, 60, 115, 105, 65, 70, 80},
    {74, "프테라", TYPE_ROCK, TYPE_FLYING, 80, 105, 65, 60, 75, 130},
    {75, "잠만보", TYPE_NORMAL, TYPE_NONE, 160, 110, 65, 65, 110, 30},
    {76, "프리져", TYPE_ICE, TYPE_FLYING, 90, 85, 100, 95, 125, 85},
    {77, "썬더", TYPE_ELECTRIC, TYPE_FLYING, 90, 90, 85, 125, 90, 100},
    {78, "파이어", TYPE_FIRE, TYPE_FLYING, 90, 100, 90, 125, 85, 90},
    {79, "망나뇽", TYPE_DRAGON, TYPE_FLYING, 91, 134, 95, 100, 100, 80},
    {80, "뮤츠", TYPE_PSYCHIC, TYPE_NONE, 106, 110, 90, 154, 90, 130},
    {81, "뮤", TYPE_PSYCHIC, TYPE_NONE, 100, 100, 100, 100, 100, 100}
};

/* 플레이어 포켓몬은 각 후보 배열에서 최대 4개의 기술을 랜덤으로 받습니다. */
Move moveCandidates1[] = {{"수면가루", TYPE_GRASS, MOVE_SPECIAL, 0, STAT_NONE, 0}, {"독가루", TYPE_POISON, MOVE_PHYSICAL, 0, STAT_NONE, 0}, {"울음소리", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_ATK, -1}, {"성장", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_ATK, 1}, {"솔라빔", TYPE_GRASS, MOVE_SPECIAL, 120, STAT_NONE, 0}};
Move moveCandidates2[] = {{"째려보기", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_DEF, -1}, {"울음소리", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_ATK, -1}, {"화염방사", TYPE_FIRE, MOVE_SPECIAL, 90, STAT_NONE, 0}, {"베어가르기", TYPE_NORMAL, MOVE_PHYSICAL, 70, STAT_NONE, 0}};
Move moveCandidates3[] = {{"꼬리흔들기", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_DEF, -1}, {"하이드로펌프", TYPE_WATER, MOVE_SPECIAL, 110, STAT_NONE, 0}, {"껍질에숨기", TYPE_WATER, MOVE_SPECIAL, 0, STAT_DEF, 1}, {"로켓박치기", TYPE_NORMAL, MOVE_PHYSICAL, 130, STAT_NONE, 0}, {"거품", TYPE_WATER, MOVE_SPECIAL, 40, STAT_AGI, -1}};
Move moveCandidates4[] = {{"수면가루", TYPE_GRASS, MOVE_SPECIAL, 0, STAT_NONE, 0}, {"저리가루", TYPE_GRASS, MOVE_SPECIAL, 0, STAT_NONE, 0}, {"독가루", TYPE_POISON, MOVE_PHYSICAL, 0, STAT_NONE, 0}, {"실뿜기", TYPE_BUG, MOVE_PHYSICAL, 0, STAT_AGI, -2}, {"단단해지기", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_DEF, 1}};
Move moveCandidates5[] = {{"실뿜기", TYPE_BUG, MOVE_PHYSICAL, 0, STAT_AGI, -2}, {"단단해지기", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_DEF, 1}, {"고속이동", TYPE_PSYCHIC, MOVE_SPECIAL, 0, STAT_AGI, 2}};
Move moveCandidates6[] = {{"모래뿌리기", TYPE_GROUND, MOVE_PHYSICAL, 0, STAT_ACC, -1}, {"고속이동", TYPE_PSYCHIC, MOVE_SPECIAL, 0, STAT_AGI, 2}};
Move moveCandidates7[] = {{"꼬리흔들기", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_DEF, -1}, {"필살앞니", TYPE_NORMAL, MOVE_PHYSICAL, 80, STAT_NONE, 0}};
Move moveCandidates8[] = {{"째려보기", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_DEF, -1}, {"울음소리", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_ATK, -1}, {"회전부리", TYPE_FLYING, MOVE_PHYSICAL, 80, STAT_NONE, 0}, {"고속이동", TYPE_PSYCHIC, MOVE_SPECIAL, 0, STAT_AGI, 2}};
Move moveCandidates9[] = {{"째려보기", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_DEF, -1}, {"용해액", TYPE_POISON, MOVE_PHYSICAL, 40, STAT_SPD, -1}, {"싫은소리", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_DEF, -2}};
Move moveCandidates10[] = {{"울음소리", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_ATK, -1}, {"번개", TYPE_ELECTRIC, MOVE_SPECIAL, 110, STAT_NONE, 0}, {"고속이동", TYPE_PSYCHIC, MOVE_SPECIAL, 0, STAT_AGI, 2}};
Move moveCandidates11[] = {{"모래뿌리기", TYPE_GROUND, MOVE_PHYSICAL, 0, STAT_ACC, -1}, {"베어가르기", TYPE_NORMAL, MOVE_PHYSICAL, 70, STAT_NONE, 0}};
Move moveCandidates12[] = {{"꼬리흔들기", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_DEF, -1}, {"울음소리", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_ATK, -1}, {"누르기", TYPE_NORMAL, MOVE_PHYSICAL, 85, STAT_NONE, 0}};
Move moveCandidates13[] = {{"째려보기", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_DEF, -1}, {"난동부리기", TYPE_NORMAL, MOVE_PHYSICAL, 120, STAT_NONE, 0}};
Move moveCandidates14[] = {{"울음소리", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_ATK, -1}, {"작아지기", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_EVA, 1}, {"웅크리기", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_DEF, 1}};
Move moveCandidates15[] = {{"꼬리흔들기", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_DEF, -1}, {"화염방사", TYPE_FIRE, MOVE_SPECIAL, 90, STAT_NONE, 0}};
Move moveCandidates16[] = {{"누르기", TYPE_NORMAL, MOVE_PHYSICAL, 85, STAT_NONE, 0}, {"이판사판태클", TYPE_NORMAL, MOVE_PHYSICAL, 120, STAT_NONE, 0}, {"웅크리기", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_DEF, 1}};
Move moveCandidates17[] = {{"흡혈", TYPE_BUG, MOVE_PHYSICAL, 80, STAT_NONE, 0}, {"싫은소리", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_DEF, -2}};
Move moveCandidates18[] = {{"수면가루", TYPE_GRASS, MOVE_SPECIAL, 0, STAT_NONE, 0}, {"저리가루", TYPE_GRASS, MOVE_SPECIAL, 0, STAT_NONE, 0}, {"독가루", TYPE_POISON, MOVE_PHYSICAL, 0, STAT_NONE, 0}, {"용해액", TYPE_POISON, MOVE_PHYSICAL, 40, STAT_SPD, -1}, {"솔라빔", TYPE_GRASS, MOVE_SPECIAL, 120, STAT_NONE, 0}, {"꽃잎댄스", TYPE_GRASS, MOVE_SPECIAL, 120, STAT_NONE, 0}};
Move moveCandidates19[] = {{"버섯포자", TYPE_GRASS, MOVE_SPECIAL, 0, STAT_NONE, 0}, {"저리가루", TYPE_GRASS, MOVE_SPECIAL, 0, STAT_NONE, 0}, {"독가루", TYPE_POISON, MOVE_PHYSICAL, 0, STAT_NONE, 0}, {"성장", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_ATK, 1}, {"흡혈", TYPE_BUG, MOVE_PHYSICAL, 80, STAT_NONE, 0}, {"베어가르기", TYPE_NORMAL, MOVE_PHYSICAL, 70, STAT_NONE, 0}};
Move moveCandidates20[] = {{"수면가루", TYPE_GRASS, MOVE_SPECIAL, 0, STAT_NONE, 0}, {"저리가루", TYPE_GRASS, MOVE_SPECIAL, 0, STAT_NONE, 0}, {"독가루", TYPE_POISON, MOVE_PHYSICAL, 0, STAT_NONE, 0}, {"사이코키네시스", TYPE_PSYCHIC, MOVE_SPECIAL, 90, STAT_SPD, -1}, {"흡혈", TYPE_BUG, MOVE_PHYSICAL, 80, STAT_NONE, 0}};
Move moveCandidates21[] = {{"모래뿌리기", TYPE_GROUND, MOVE_PHYSICAL, 0, STAT_ACC, -1}, {"울음소리", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_ATK, -1}, {"지진", TYPE_GROUND, MOVE_PHYSICAL, 100, STAT_NONE, 0}, {"구멍파기", TYPE_GROUND, MOVE_PHYSICAL, 80, STAT_NONE, 0}, {"베어가르기", TYPE_NORMAL, MOVE_PHYSICAL, 70, STAT_NONE, 0}};
Move moveCandidates22[] = {{"울음소리", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_ATK, -1}, {"싫은소리", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_DEF, -2}, {"베어가르기", TYPE_NORMAL, MOVE_PHYSICAL, 70, STAT_NONE, 0}};
Move moveCandidates23[] = {{"꼬리흔들기", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_DEF, -1}, {"하이드로펌프", TYPE_WATER, MOVE_SPECIAL, 110, STAT_NONE, 0}};
Move moveCandidates24[] = {{"난동부리기", TYPE_NORMAL, MOVE_PHYSICAL, 120, STAT_NONE, 0}, {"째려보기", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_DEF, -1}};
Move moveCandidates25[] = {{"돌진", TYPE_NORMAL, MOVE_PHYSICAL, 90, STAT_NONE, 0}, {"째려보기", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_DEF, -1}, {"화염방사", TYPE_FIRE, MOVE_SPECIAL, 90, STAT_NONE, 0}, {"고속이동", TYPE_PSYCHIC, MOVE_SPECIAL, 0, STAT_AGI, 2}};
Move moveCandidates26[] = {{"누르기", TYPE_NORMAL, MOVE_PHYSICAL, 85, STAT_NONE, 0}, {"하이드로펌프", TYPE_WATER, MOVE_SPECIAL, 110, STAT_NONE, 0}, {"망각술", TYPE_PSYCHIC, MOVE_SPECIAL, 0, STAT_SPD, 2}, {"거품", TYPE_WATER, MOVE_SPECIAL, 40, STAT_AGI, -1}};
Move moveCandidates27[] = {{"사이코키네시스", TYPE_PSYCHIC, MOVE_SPECIAL, 90, STAT_SPD, -1}};
Move moveCandidates28[] = {{"째려보기", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_DEF, -1}, {"지옥의바퀴", TYPE_FIGHTING, MOVE_PHYSICAL, 80, STAT_NONE, 0}};
Move moveCandidates29[] = {{"힘껏치기", TYPE_NORMAL, MOVE_PHYSICAL, 80, STAT_NONE, 0}, {"용해액", TYPE_POISON, MOVE_PHYSICAL, 40, STAT_SPD, -1}, {"성장", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_ATK, 1}};
Move moveCandidates30[] = {{"용해액", TYPE_POISON, MOVE_PHYSICAL, 40, STAT_SPD, -1}, {"하이드로펌프", TYPE_WATER, MOVE_SPECIAL, 110, STAT_NONE, 0}, {"싫은소리", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_DEF, -2}, {"배리어", TYPE_PSYCHIC, MOVE_SPECIAL, 0, STAT_DEF, 2}, {"휘감기", TYPE_NORMAL, MOVE_PHYSICAL, 10, STAT_AGI, -1}};
Move moveCandidates31[] = {{"지진", TYPE_GROUND, MOVE_PHYSICAL, 100, STAT_NONE, 0}, {"단단해지기", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_DEF, 1}, {"웅크리기", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_DEF, 1}, {"자폭", TYPE_NORMAL, MOVE_PHYSICAL, 200, STAT_NONE, 0}, {"대폭발", TYPE_NORMAL, MOVE_PHYSICAL, 250, STAT_NONE, 0}};
Move moveCandidates32[] = {{"돌진", TYPE_NORMAL, MOVE_PHYSICAL, 90, STAT_NONE, 0}, {"꼬리흔들기", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_DEF, -1}, {"울음소리", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_ATK, -1}, {"고속이동", TYPE_PSYCHIC, MOVE_SPECIAL, 0, STAT_AGI, 2}};
Move moveCandidates33[] = {{"박치기", TYPE_NORMAL, MOVE_PHYSICAL, 70, STAT_NONE, 0}, {"울음소리", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_ATK, -1}, {"사이코키네시스", TYPE_PSYCHIC, MOVE_SPECIAL, 90, STAT_SPD, -1}, {"망각술", TYPE_PSYCHIC, MOVE_SPECIAL, 0, STAT_SPD, 2}, {"껍질에숨기", TYPE_WATER, MOVE_SPECIAL, 0, STAT_DEF, 1}};
Move moveCandidates34[] = {{"싫은소리", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_DEF, -2}};
Move moveCandidates35[] = {{"칼춤", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_ATK, 2}, {"모래뿌리기", TYPE_GROUND, MOVE_PHYSICAL, 0, STAT_ACC, -1}, {"째려보기", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_DEF, -1}, {"고속이동", TYPE_PSYCHIC, MOVE_SPECIAL, 0, STAT_AGI, 2}, {"베어가르기", TYPE_NORMAL, MOVE_PHYSICAL, 70, STAT_NONE, 0}};
Move moveCandidates36[] = {{"울음소리", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_ATK, -1}, {"회전부리", TYPE_FLYING, MOVE_PHYSICAL, 80, STAT_NONE, 0}, {"고속이동", TYPE_PSYCHIC, MOVE_SPECIAL, 0, STAT_AGI, 2}, {"트라이어택", TYPE_NORMAL, MOVE_PHYSICAL, 80, STAT_NONE, 0}};
Move moveCandidates37[] = {{"박치기", TYPE_NORMAL, MOVE_PHYSICAL, 70, STAT_NONE, 0}, {"돌진", TYPE_NORMAL, MOVE_PHYSICAL, 90, STAT_NONE, 0}, {"울음소리", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_ATK, -1}, {"냉동빔", TYPE_ICE, MOVE_SPECIAL, 90, STAT_NONE, 0}, {"오로라빔", TYPE_ICE, MOVE_SPECIAL, 65, STAT_ATK, -1}};
Move moveCandidates38[] = {{"싫은소리", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_DEF, -2}, {"단단해지기", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_DEF, 1}, {"작아지기", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_EVA, 1}, {"녹기", TYPE_POISON, MOVE_PHYSICAL, 0, STAT_DEF, 2}};
Move moveCandidates39[] = {{"째려보기", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_DEF, -1}, {"냉동빔", TYPE_ICE, MOVE_SPECIAL, 90, STAT_NONE, 0}, {"오로라빔", TYPE_ICE, MOVE_SPECIAL, 65, STAT_ATK, -1}, {"껍질에숨기", TYPE_WATER, MOVE_SPECIAL, 0, STAT_DEF, 1}};
Move moveCandidates40[] = {{"최면술", TYPE_PSYCHIC, MOVE_SPECIAL, 0, STAT_NONE, 0}, {"꿈먹기", TYPE_PSYCHIC, MOVE_SPECIAL, 100, STAT_NONE, 0}};
Move moveCandidates41[] = {{"힘껏치기", TYPE_NORMAL, MOVE_PHYSICAL, 80, STAT_NONE, 0}, {"싫은소리", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_DEF, -2}, {"단단해지기", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_DEF, 1}};
Move moveCandidates42[] = {{"최면술", TYPE_PSYCHIC, MOVE_SPECIAL, 0, STAT_NONE, 0}, {"박치기", TYPE_NORMAL, MOVE_PHYSICAL, 70, STAT_NONE, 0}, {"사이코키네시스", TYPE_PSYCHIC, MOVE_SPECIAL, 90, STAT_SPD, -1}, {"요가포즈", TYPE_PSYCHIC, MOVE_SPECIAL, 0, STAT_ATK, 1}};
Move moveCandidates43[] = {{"째려보기", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_DEF, -1}, {"단단해지기", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_DEF, 1}, {"거품", TYPE_WATER, MOVE_SPECIAL, 40, STAT_AGI, -1}, {"집게해머", TYPE_WATER, MOVE_SPECIAL, 100, STAT_NONE, 0}};
Move moveCandidates44[] = {{"싫은소리", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_DEF, -2}, {"자폭", TYPE_NORMAL, MOVE_PHYSICAL, 200, STAT_NONE, 0}, {"대폭발", TYPE_NORMAL, MOVE_PHYSICAL, 250, STAT_NONE, 0}};
Move moveCandidates45[] = {{"최면술", TYPE_PSYCHIC, MOVE_SPECIAL, 0, STAT_NONE, 0}, {"저리가루", TYPE_GRASS, MOVE_SPECIAL, 0, STAT_NONE, 0}, {"독가루", TYPE_POISON, MOVE_PHYSICAL, 0, STAT_NONE, 0}, {"솔라빔", TYPE_GRASS, MOVE_SPECIAL, 120, STAT_NONE, 0}};
Move moveCandidates46[] = {{"난동부리기", TYPE_NORMAL, MOVE_PHYSICAL, 120, STAT_NONE, 0}, {"째려보기", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_DEF, -1}, {"울음소리", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_ATK, -1}};
Move moveCandidates47[] = {{"메가톤킥", TYPE_NORMAL, MOVE_PHYSICAL, 120, STAT_NONE, 0}, {"점프킥", TYPE_FIGHTING, MOVE_PHYSICAL, 100, STAT_NONE, 0}, {"요가포즈", TYPE_PSYCHIC, MOVE_SPECIAL, 0, STAT_ATK, 1}, {"무릎차기", TYPE_FIGHTING, MOVE_PHYSICAL, 130, STAT_NONE, 0}};
Move moveCandidates48[] = {{"메가톤펀치", TYPE_NORMAL, MOVE_PHYSICAL, 80, STAT_NONE, 0}, {"불꽃펀치", TYPE_FIRE, MOVE_SPECIAL, 75, STAT_NONE, 0}, {"냉동펀치", TYPE_ICE, MOVE_SPECIAL, 75, STAT_NONE, 0}, {"번개펀치", TYPE_ELECTRIC, MOVE_SPECIAL, 75, STAT_NONE, 0}, {"고속이동", TYPE_PSYCHIC, MOVE_SPECIAL, 0, STAT_AGI, 2}};
Move moveCandidates49[] = {{"핥기", TYPE_GHOST, MOVE_PHYSICAL, 30, STAT_NONE, 0}, {"힘껏치기", TYPE_NORMAL, MOVE_PHYSICAL, 80, STAT_NONE, 0}, {"싫은소리", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_DEF, -2}, {"웅크리기", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_DEF, 1}};
Move moveCandidates50[] = {{"독가스", TYPE_POISON, MOVE_PHYSICAL, 0, STAT_NONE, 0}, {"연막", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_ACC, -1}, {"자폭", TYPE_NORMAL, MOVE_PHYSICAL, 200, STAT_NONE, 0}, {"대폭발", TYPE_NORMAL, MOVE_PHYSICAL, 250, STAT_NONE, 0}};
Move moveCandidates51[] = {{"돌진", TYPE_NORMAL, MOVE_PHYSICAL, 90, STAT_NONE, 0}, {"꼬리흔들기", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_DEF, -1}, {"째려보기", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_DEF, -1}};
Move moveCandidates52[] = {{"이판사판태클", TYPE_NORMAL, MOVE_PHYSICAL, 120, STAT_NONE, 0}, {"울음소리", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_ATK, -1}, {"작아지기", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_EVA, 1}, {"웅크리기", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_DEF, 1}};
Move moveCandidates53[] = {{"힘껏치기", TYPE_NORMAL, MOVE_PHYSICAL, 80, STAT_NONE, 0}, {"성장", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_ATK, 1}, {"휘감기", TYPE_NORMAL, MOVE_PHYSICAL, 10, STAT_AGI, -1}};
Move moveCandidates54[] = {{"메가톤펀치", TYPE_NORMAL, MOVE_PHYSICAL, 80, STAT_NONE, 0}, {"꼬리흔들기", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_DEF, -1}, {"째려보기", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_DEF, -1}, {"잼잼펀치", TYPE_NORMAL, MOVE_PHYSICAL, 70, STAT_NONE, 0}};
Move moveCandidates55[] = {{"째려보기", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_DEF, -1}, {"하이드로펌프", TYPE_WATER, MOVE_SPECIAL, 110, STAT_NONE, 0}, {"고속이동", TYPE_PSYCHIC, MOVE_SPECIAL, 0, STAT_AGI, 2}, {"연막", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_ACC, -1}, {"거품", TYPE_WATER, MOVE_SPECIAL, 40, STAT_AGI, -1}};
Move moveCandidates56[] = {{"꼬리흔들기", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_DEF, -1}, {"고속이동", TYPE_PSYCHIC, MOVE_SPECIAL, 0, STAT_AGI, 2}, {"폭포오르기", TYPE_WATER, MOVE_SPECIAL, 80, STAT_NONE, 0}};
Move moveCandidates57[] = {{"하이드로펌프", TYPE_WATER, MOVE_SPECIAL, 110, STAT_NONE, 0}, {"단단해지기", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_DEF, 1}, {"작아지기", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_EVA, 1}};
Move moveCandidates58[] = {{"요가포즈", TYPE_PSYCHIC, MOVE_SPECIAL, 0, STAT_ATK, 1}, {"배리어", TYPE_PSYCHIC, MOVE_SPECIAL, 0, STAT_DEF, 2}};
Move moveCandidates59[] = {{"칼춤", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_ATK, 2}, {"째려보기", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_DEF, -1}, {"고속이동", TYPE_PSYCHIC, MOVE_SPECIAL, 0, STAT_AGI, 2}, {"그림자분신", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_EVA, 1}, {"베어가르기", TYPE_NORMAL, MOVE_PHYSICAL, 70, STAT_NONE, 0}};
Move moveCandidates60[] = {{"냉동펀치", TYPE_ICE, MOVE_SPECIAL, 75, STAT_NONE, 0}, {"누르기", TYPE_NORMAL, MOVE_PHYSICAL, 85, STAT_NONE, 0}, {"난동부리기", TYPE_NORMAL, MOVE_PHYSICAL, 120, STAT_NONE, 0}, {"눈보라", TYPE_ICE, MOVE_SPECIAL, 110, STAT_NONE, 0}};
Move moveCandidates61[] = {{"번개펀치", TYPE_ELECTRIC, MOVE_SPECIAL, 75, STAT_NONE, 0}, {"째려보기", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_DEF, -1}, {"번개", TYPE_ELECTRIC, MOVE_SPECIAL, 110, STAT_NONE, 0}, {"싫은소리", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_DEF, -2}};
Move moveCandidates62[] = {{"불꽃펀치", TYPE_FIRE, MOVE_SPECIAL, 75, STAT_NONE, 0}, {"째려보기", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_DEF, -1}, {"화염방사", TYPE_FIRE, MOVE_SPECIAL, 90, STAT_NONE, 0}, {"연막", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_ACC, -1}};
Move moveCandidates63[] = {{"칼춤", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_ATK, 2}, {"단단해지기", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_DEF, 1}, {"베어가르기", TYPE_NORMAL, MOVE_PHYSICAL, 70, STAT_NONE, 0}};
Move moveCandidates64[] = {{"돌진", TYPE_NORMAL, MOVE_PHYSICAL, 90, STAT_NONE, 0}, {"꼬리흔들기", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_DEF, -1}, {"째려보기", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_DEF, -1}};
Move moveCandidates65[] = {{"째려보기", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_DEF, -1}, {"하이드로펌프", TYPE_WATER, MOVE_SPECIAL, 110, STAT_NONE, 0}, {"파괴광선", TYPE_NORMAL, MOVE_PHYSICAL, 150, STAT_NONE, 0}};
Move moveCandidates66[] = {{"노래하기", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_NONE, 0}, {"누르기", TYPE_NORMAL, MOVE_PHYSICAL, 85, STAT_NONE, 0}, {"울음소리", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_ATK, -1}, {"하이드로펌프", TYPE_WATER, MOVE_SPECIAL, 110, STAT_NONE, 0}, {"냉동빔", TYPE_ICE, MOVE_SPECIAL, 90, STAT_NONE, 0}};
Move moveCandidates67[] = {};
Move moveCandidates68[] = {{"모래뿌리기", TYPE_GROUND, MOVE_PHYSICAL, 0, STAT_ACC, -1}, {"돌진", TYPE_NORMAL, MOVE_PHYSICAL, 90, STAT_NONE, 0}, {"꼬리흔들기", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_DEF, -1}, {"하이드로펌프", TYPE_WATER, MOVE_SPECIAL, 110, STAT_NONE, 0}, {"녹기", TYPE_POISON, MOVE_PHYSICAL, 0, STAT_DEF, 2}};
Move moveCandidates69[] = {{"모래뿌리기", TYPE_GROUND, MOVE_PHYSICAL, 0, STAT_ACC, -1}, {"돌진", TYPE_NORMAL, MOVE_PHYSICAL, 90, STAT_NONE, 0}, {"꼬리흔들기", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_DEF, -1}, {"번개", TYPE_ELECTRIC, MOVE_SPECIAL, 110, STAT_NONE, 0}, {"고속이동", TYPE_PSYCHIC, MOVE_SPECIAL, 0, STAT_AGI, 2}};
Move moveCandidates70[] = {{"모래뿌리기", TYPE_GROUND, MOVE_PHYSICAL, 0, STAT_ACC, -1}, {"돌진", TYPE_NORMAL, MOVE_PHYSICAL, 90, STAT_NONE, 0}, {"꼬리흔들기", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_DEF, -1}, {"째려보기", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_DEF, -1}, {"화염방사", TYPE_FIRE, MOVE_SPECIAL, 90, STAT_NONE, 0}};
Move moveCandidates71[] = {{"고속이동", TYPE_PSYCHIC, MOVE_SPECIAL, 0, STAT_AGI, 2}, {"각지기", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_ATK, 1}, {"트라이어택", TYPE_NORMAL, MOVE_PHYSICAL, 80, STAT_NONE, 0}};
Move moveCandidates72[] = {{"째려보기", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_DEF, -1}, {"하이드로펌프", TYPE_WATER, MOVE_SPECIAL, 110, STAT_NONE, 0}, {"껍질에숨기", TYPE_WATER, MOVE_SPECIAL, 0, STAT_DEF, 1}};
Move moveCandidates73[] = {{"째려보기", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_DEF, -1}, {"하이드로펌프", TYPE_WATER, MOVE_SPECIAL, 110, STAT_NONE, 0}, {"단단해지기", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_DEF, 1}, {"베어가르기", TYPE_NORMAL, MOVE_PHYSICAL, 70, STAT_NONE, 0}};
Move moveCandidates74[] = {{"돌진", TYPE_NORMAL, MOVE_PHYSICAL, 90, STAT_NONE, 0}, {"파괴광선", TYPE_NORMAL, MOVE_PHYSICAL, 150, STAT_NONE, 0}, {"고속이동", TYPE_PSYCHIC, MOVE_SPECIAL, 0, STAT_AGI, 2}};
Move moveCandidates75[] = {{"박치기", TYPE_NORMAL, MOVE_PHYSICAL, 70, STAT_NONE, 0}, {"누르기", TYPE_NORMAL, MOVE_PHYSICAL, 85, STAT_NONE, 0}, {"이판사판태클", TYPE_NORMAL, MOVE_PHYSICAL, 120, STAT_NONE, 0}, {"파괴광선", TYPE_NORMAL, MOVE_PHYSICAL, 150, STAT_NONE, 0}, {"단단해지기", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_DEF, 1}, {"망각술", TYPE_PSYCHIC, MOVE_SPECIAL, 0, STAT_SPD, 2}};
Move moveCandidates76[] = {{"냉동빔", TYPE_ICE, MOVE_SPECIAL, 90, STAT_NONE, 0}, {"눈보라", TYPE_ICE, MOVE_SPECIAL, 110, STAT_NONE, 0}, {"고속이동", TYPE_PSYCHIC, MOVE_SPECIAL, 0, STAT_AGI, 2}};
Move moveCandidates77[] = {{"회전부리", TYPE_FLYING, MOVE_PHYSICAL, 80, STAT_NONE, 0}, {"번개", TYPE_ELECTRIC, MOVE_SPECIAL, 110, STAT_NONE, 0}, {"고속이동", TYPE_PSYCHIC, MOVE_SPECIAL, 0, STAT_AGI, 2}};
Move moveCandidates78[] = {{"째려보기", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_DEF, -1}, {"고속이동", TYPE_PSYCHIC, MOVE_SPECIAL, 0, STAT_AGI, 2}, {"불새", TYPE_FLYING, MOVE_PHYSICAL, 140, STAT_NONE, 0}};
Move moveCandidates79[] = {{"힘껏치기", TYPE_NORMAL, MOVE_PHYSICAL, 80, STAT_NONE, 0}, {"째려보기", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_DEF, -1}, {"파괴광선", TYPE_NORMAL, MOVE_PHYSICAL, 150, STAT_NONE, 0}, {"고속이동", TYPE_PSYCHIC, MOVE_SPECIAL, 0, STAT_AGI, 2}};
Move moveCandidates80[] = {{"사이코키네시스", TYPE_PSYCHIC, MOVE_SPECIAL, 90, STAT_SPD, -1}, {"배리어", TYPE_PSYCHIC, MOVE_SPECIAL, 0, STAT_DEF, 2}, {"망각술", TYPE_PSYCHIC, MOVE_SPECIAL, 0, STAT_SPD, 2}};
Move moveCandidates81[] = {{"메가톤펀치", TYPE_NORMAL, MOVE_PHYSICAL, 80, STAT_NONE, 0}, {"사이코키네시스", TYPE_PSYCHIC, MOVE_SPECIAL, 90, STAT_SPD, -1}};

/* 사천왕과 챔피언은 원작 레드/블루 기준 기술을 고정으로 사용합니다. */
Move loreleiDewgongMoves[] = {{"울음소리", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_ATK, -1}, {"오로라빔", TYPE_ICE, MOVE_SPECIAL, 65, STAT_ATK, -1}, {"잠자기", TYPE_PSYCHIC, MOVE_SPECIAL, 0, STAT_NONE, 0}, {"돌진", TYPE_NORMAL, MOVE_PHYSICAL, 90, STAT_NONE, 0}};
Move loreleiCloysterMoves[] = {{"오로라빔", TYPE_ICE, MOVE_SPECIAL, 65, STAT_ATK, -1}, {"껍질에숨기", TYPE_WATER, MOVE_SPECIAL, 0, STAT_DEF, 1}, {"껍질끼우기", TYPE_WATER, MOVE_SPECIAL, 35, STAT_NONE, 0}, {"가시대포", TYPE_NORMAL, MOVE_PHYSICAL, 20, STAT_NONE, 0}};
Move loreleiSlowbroMoves[] = {{"울음소리", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_ATK, -1}, {"물대포", TYPE_WATER, MOVE_SPECIAL, 40, STAT_NONE, 0}, {"껍질에숨기", TYPE_WATER, MOVE_SPECIAL, 0, STAT_DEF, 1}, {"망각술", TYPE_PSYCHIC, MOVE_SPECIAL, 0, STAT_SPD, 2}};
Move loreleiJynxMoves[] = {{"연속뺨치기", TYPE_NORMAL, MOVE_PHYSICAL, 15, STAT_NONE, 0}, {"냉동펀치", TYPE_ICE, MOVE_SPECIAL, 75, STAT_NONE, 0}, {"누르기", TYPE_NORMAL, MOVE_PHYSICAL, 85, STAT_NONE, 0}, {"난동부리기", TYPE_NORMAL, MOVE_PHYSICAL, 120, STAT_NONE, 0}};
Move loreleiLaprasMoves[] = {{"누르기", TYPE_NORMAL, MOVE_PHYSICAL, 85, STAT_NONE, 0}, {"이상한빛", TYPE_GHOST, MOVE_PHYSICAL, 0, STAT_NONE, 0}, {"냉동빔", TYPE_ICE, MOVE_SPECIAL, 90, STAT_NONE, 0}, {"하이드로펌프", TYPE_WATER, MOVE_SPECIAL, 110, STAT_NONE, 0}};
Move brunoOnix53Moves[] = {{"돌떨구기", TYPE_ROCK, MOVE_PHYSICAL, 50, STAT_NONE, 0}, {"분노", TYPE_NORMAL, MOVE_PHYSICAL, 20, STAT_NONE, 0}, {"힘껏치기", TYPE_NORMAL, MOVE_PHYSICAL, 80, STAT_NONE, 0}, {"단단해지기", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_DEF, 1}};
Move brunoHitmonchanMoves[] = {{"냉동펀치", TYPE_ICE, MOVE_SPECIAL, 75, STAT_NONE, 0}, {"번개펀치", TYPE_ELECTRIC, MOVE_SPECIAL, 75, STAT_NONE, 0}, {"메가톤펀치", TYPE_NORMAL, MOVE_PHYSICAL, 80, STAT_NONE, 0}, {"카운터", TYPE_FIGHTING, MOVE_PHYSICAL, 0, STAT_NONE, 0}};
Move brunoHitmonleeMoves[] = {{"점프킥", TYPE_FIGHTING, MOVE_PHYSICAL, 70, STAT_NONE, 0}, {"기충전", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_NONE, 0}, {"무릎차기", TYPE_FIGHTING, MOVE_PHYSICAL, 85, STAT_NONE, 0}, {"메가톤킥", TYPE_NORMAL, MOVE_PHYSICAL, 120, STAT_NONE, 0}};
Move brunoOnix56Moves[] = {{"돌떨구기", TYPE_ROCK, MOVE_PHYSICAL, 50, STAT_NONE, 0}, {"분노", TYPE_NORMAL, MOVE_PHYSICAL, 20, STAT_NONE, 0}, {"힘껏치기", TYPE_NORMAL, MOVE_PHYSICAL, 80, STAT_NONE, 0}, {"단단해지기", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_DEF, 1}};
Move brunoMachampMoves[] = {{"안다리걸기", TYPE_FIGHTING, MOVE_PHYSICAL, 50, STAT_NONE, 0}, {"기충전", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_NONE, 0}, {"지구던지기", TYPE_FIGHTING, MOVE_PHYSICAL, 0, STAT_NONE, 0}, {"지옥의바퀴", TYPE_FIGHTING, MOVE_PHYSICAL, 80, STAT_NONE, 0}};
Move agathaGengar56Moves[] = {{"이상한빛", TYPE_GHOST, MOVE_PHYSICAL, 0, STAT_NONE, 0}, {"핥기", TYPE_GHOST, MOVE_PHYSICAL, 30, STAT_NONE, 0}, {"최면술", TYPE_PSYCHIC, MOVE_SPECIAL, 0, STAT_NONE, 0}, {"꿈먹기", TYPE_PSYCHIC, MOVE_SPECIAL, 100, STAT_NONE, 0}};
Move agathaGolbatMoves[] = {{"초음파", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_NONE, 0}, {"이상한빛", TYPE_GHOST, MOVE_PHYSICAL, 0, STAT_NONE, 0}, {"날개치기", TYPE_FLYING, MOVE_PHYSICAL, 60, STAT_NONE, 0}, {"흰안개", TYPE_ICE, MOVE_SPECIAL, 0, STAT_NONE, 0}};
Move agathaHaunterMoves[] = {{"이상한빛", TYPE_GHOST, MOVE_PHYSICAL, 0, STAT_NONE, 0}, {"핥기", TYPE_GHOST, MOVE_PHYSICAL, 30, STAT_NONE, 0}, {"최면술", TYPE_PSYCHIC, MOVE_SPECIAL, 0, STAT_NONE, 0}, {"꿈먹기", TYPE_PSYCHIC, MOVE_SPECIAL, 100, STAT_NONE, 0}};
Move agathaArbokMoves[] = {{"물기", TYPE_NORMAL, MOVE_SPECIAL, 60, STAT_NONE, 0}, {"뱀눈초리", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_NONE, 0}, {"싫은소리", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_DEF, -2}, {"용해액", TYPE_POISON, MOVE_PHYSICAL, 40, STAT_SPD, -1}};
Move agathaGengar60Moves[] = {{"이상한빛", TYPE_GHOST, MOVE_PHYSICAL, 0, STAT_NONE, 0}, {"핥기", TYPE_GHOST, MOVE_PHYSICAL, 30, STAT_NONE, 0}, {"최면술", TYPE_PSYCHIC, MOVE_SPECIAL, 0, STAT_NONE, 0}, {"꿈먹기", TYPE_PSYCHIC, MOVE_SPECIAL, 100, STAT_NONE, 0}};
Move lanceGyaradosMoves[] = {{"물기", TYPE_NORMAL, MOVE_SPECIAL, 60, STAT_NONE, 0}, {"하이드로펌프", TYPE_WATER, MOVE_SPECIAL, 110, STAT_NONE, 0}, {"용의분노", TYPE_DRAGON, MOVE_SPECIAL, 1, STAT_NONE, 0}, {"파괴광선", TYPE_NORMAL, MOVE_PHYSICAL, 150, STAT_NONE, 0}};
Move lanceDragonairMoves[] = {{"고속이동", TYPE_PSYCHIC, MOVE_SPECIAL, 0, STAT_AGI, 2}, {"힘껏치기", TYPE_NORMAL, MOVE_PHYSICAL, 80, STAT_NONE, 0}, {"용의분노", TYPE_DRAGON, MOVE_SPECIAL, 1, STAT_NONE, 0}, {"파괴광선", TYPE_NORMAL, MOVE_PHYSICAL, 150, STAT_NONE, 0}};
Move lanceAerodactylMoves[] = {{"초음파", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_NONE, 0}, {"물기", TYPE_NORMAL, MOVE_SPECIAL, 60, STAT_NONE, 0}, {"돌진", TYPE_NORMAL, MOVE_PHYSICAL, 90, STAT_NONE, 0}, {"파괴광선", TYPE_NORMAL, MOVE_PHYSICAL, 150, STAT_NONE, 0}};
Move lanceDragoniteMoves[] = {{"고속이동", TYPE_PSYCHIC, MOVE_SPECIAL, 0, STAT_AGI, 2}, {"힘껏치기", TYPE_NORMAL, MOVE_PHYSICAL, 80, STAT_NONE, 0}, {"용의분노", TYPE_DRAGON, MOVE_SPECIAL, 1, STAT_NONE, 0}, {"파괴광선", TYPE_NORMAL, MOVE_PHYSICAL, 150, STAT_NONE, 0}};
Move championPidgeotMoves[] = {{"날려버리기", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_NONE, 0}, {"날개치기", TYPE_FLYING, MOVE_PHYSICAL, 60, STAT_NONE, 0}, {"고속이동", TYPE_PSYCHIC, MOVE_SPECIAL, 0, STAT_AGI, 2}, {"따라하기", TYPE_FLYING, MOVE_PHYSICAL, 0, STAT_NONE, 0}};
Move championAlakazamMoves[] = {{"환상빔", TYPE_PSYCHIC, MOVE_SPECIAL, 65, STAT_NONE, 0}, {"HP회복", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_NONE, 0}, {"사이코키네시스", TYPE_PSYCHIC, MOVE_SPECIAL, 90, STAT_SPD, -1}, {"리플렉터", TYPE_PSYCHIC, MOVE_SPECIAL, 0, STAT_NONE, 0}};
Move championRhydonMoves[] = {{"마구찌르기", TYPE_NORMAL, MOVE_PHYSICAL, 15, STAT_NONE, 0}, {"꼬리흔들기", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_DEF, -1}, {"뿔드릴", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_NONE, 0}, {"째려보기", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_DEF, -1}};
Move championArcanineMoves[] = {{"돌진", TYPE_NORMAL, MOVE_PHYSICAL, 90, STAT_NONE, 0}, {"째려보기", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_DEF, -1}, {"울부짖기", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_NONE, 0}, {"불꽃세례", TYPE_FIRE, MOVE_SPECIAL, 40, STAT_NONE, 0}};
Move championExeggutorMoves[] = {{"최면술", TYPE_PSYCHIC, MOVE_SPECIAL, 0, STAT_NONE, 0}, {"구슬던지기", TYPE_NORMAL, MOVE_PHYSICAL, 15, STAT_NONE, 0}, {"짓밟기", TYPE_NORMAL, MOVE_PHYSICAL, 65, STAT_NONE, 0}};
Move championBlastoiseMoves[] = {{"물기", TYPE_NORMAL, MOVE_SPECIAL, 60, STAT_NONE, 0}, {"껍질에숨기", TYPE_WATER, MOVE_SPECIAL, 0, STAT_DEF, 1}, {"로켓박치기", TYPE_NORMAL, MOVE_PHYSICAL, 130, STAT_NONE, 0}, {"하이드로펌프", TYPE_WATER, MOVE_SPECIAL, 110, STAT_NONE, 0}};

const char *getStatusName(StatusCondition status);

/* 포켓몬 한 마리의 기본 정보를 출력합니다. */
void printPokemon(Pokemon pokemon)
{
    printf("No.%03d %s\n", pokemon.id, pokemon.name);
    printf("Type: %s", typeNames[pokemon.type1]);
    if (pokemon.type2 != TYPE_NONE) {
        printf(", %s", typeNames[pokemon.type2]);
    }
    printf("\n");
    printf("HP:%d ATK:%d DEF:%d SPA:%d SPD:%d AGI:%d\n",
           pokemon.hp, pokemon.atk, pokemon.def,
           pokemon.spa, pokemon.spd, pokemon.agi);
}

/* 트레이너의 전체 팀, 현재 HP, 상태이상, 기술 목록을 출력합니다. */
void printTrainerTeam(Trainer trainer)
{
    printf("\n[%s의 팀]\n", trainer.name);

    for (int i = 0; i < trainer.teamSize; i++) {
        printf("%d. ", i + 1);
        printPokemon(trainer.team[i].pokemon);
        printf("Level:%d Current HP:%d/%d\n",
               trainer.team[i].level,
               trainer.team[i].currentHp,
               trainer.team[i].pokemon.hp);
        printf("Status: %s\n", getStatusName(trainer.team[i].status));
        printf("Moves: ");
        if (trainer.team[i].moveCount == 0) {
            printf("없음");
        }
        for (int j = 0; j < trainer.team[i].moveCount; j++) {
            printf("%s", trainer.team[i].moves[j].name);
            if (j + 1 < trainer.team[i].moveCount) {
                printf(", ");
            }
        }
        printf("\n");
    }
}

/* 공격 타입이 방어 포켓몬의 두 타입에 몇 배로 들어가는지 계산합니다. */
float getTypeEffectiveness(PokemonType attackType, PokemonType defenderType1, PokemonType defenderType2)
{
    float effectiveness = typeChart[attackType][defenderType1];

    if (defenderType2 != TYPE_NONE) {
        effectiveness *= typeChart[attackType][defenderType2];
    }

    return effectiveness;
}

/* HP 종족값, 개체값, 레벨로 전투에 사용할 최종 HP를 계산합니다. */
int calculateFinalHp(int baseHp, int individualValue, int level)
{
    return ((baseHp * 2 + individualValue) * level) / 100 + level + 10;
}

/* HP 이외의 종족값, 개체값, 레벨로 전투에 사용할 최종 능력치를 계산합니다. */
int calculateFinalNonHpStat(int baseStat, int individualValue, int level)
{
    return ((baseStat * 2 + individualValue) * level) / 100 + 5;
}

/* 도감의 종족값 포켓몬을 레벨과 개체값이 반영된 전투용 능력치로 바꿉니다. */
Pokemon calculateFinalPokemonStats(Pokemon basePokemon, int level, int individualValue)
{
    Pokemon finalPokemon = basePokemon;

    finalPokemon.hp = calculateFinalHp(basePokemon.hp, individualValue, level);
    finalPokemon.atk = calculateFinalNonHpStat(basePokemon.atk, individualValue, level);
    finalPokemon.def = calculateFinalNonHpStat(basePokemon.def, individualValue, level);
    finalPokemon.spa = calculateFinalNonHpStat(basePokemon.spa, individualValue, level);
    finalPokemon.spd = calculateFinalNonHpStat(basePokemon.spd, individualValue, level);
    finalPokemon.agi = calculateFinalNonHpStat(basePokemon.agi, individualValue, level);

    return finalPokemon;
}

/* 도감 포켓몬을 전투용 포켓몬으로 바꾸고 전투 상태를 초기화합니다. */
BattlePokemon createBattlePokemon(Pokemon pokemon, int level)
{
    BattlePokemon battlePokemon;
    Pokemon finalPokemon;

    if (level <= 0) {
        level = BATTLE_LEVEL;
    }

    finalPokemon = calculateFinalPokemonStats(pokemon, level, DEFAULT_INDIVIDUAL_VALUE);

    battlePokemon.pokemon = finalPokemon;
    battlePokemon.level = level;
    battlePokemon.currentHp = finalPokemon.hp;
    battlePokemon.moveCount = 0;
    battlePokemon.atkStage = 0;
    battlePokemon.defStage = 0;
    battlePokemon.spaStage = 0;
    battlePokemon.spdStage = 0;
    battlePokemon.agiStage = 0;
    battlePokemon.accStage = 0;
    battlePokemon.evaStage = 0;
    battlePokemon.status = STATUS_NONE;
    battlePokemon.sleepTurns = 0;

    return battlePokemon;
}

/* 포켓몬의 기술칸을 비웁니다. */
void clearMoves(BattlePokemon *pokemon)
{
    pokemon->moveCount = 0;
}

/* 기술칸이 남아 있으면 기술 하나를 추가합니다. */
void addMove(BattlePokemon *pokemon, Move move)
{
    if (pokemon->moveCount < MOVE_SLOT_COUNT) {
        pokemon->moves[pokemon->moveCount] = move;
        pokemon->moveCount++;
    }
}

/* Fisher-Yates 방식으로 정수 배열을 섞습니다. */
void shuffleIntArray(int array[], int size)
{
    for (int i = size - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        int temp = array[i];
        array[i] = array[j];
        array[j] = temp;
    }
}

/* 후보 기술 목록에서 중복 없이 최대 4개를 랜덤으로 골라 배치합니다. */
void setRandomMoves(BattlePokemon *pokemon, Move candidates[], int candidateCount)
{
    int indexes[64];
    int selectCount;

    clearMoves(pokemon);

    if (candidateCount > 64) {
        candidateCount = 64;
    }

    selectCount = candidateCount;
    if (selectCount > MOVE_SLOT_COUNT) {
        selectCount = MOVE_SLOT_COUNT;
    }

    for (int i = 0; i < candidateCount; i++) {
        indexes[i] = i;
    }

    shuffleIntArray(indexes, candidateCount);

    for (int i = 0; i < selectCount; i++) {
        addMove(pokemon, candidates[indexes[i]]);
    }
}

/* 포켓몬 id에 맞는 레벨업 기술 후보 배열을 찾아 랜덤 기술배치를 만듭니다. */
void setRandomLevelUpMoves(BattlePokemon *pokemon)
{
    switch (pokemon->pokemon.id) {
        case 1: setRandomMoves(pokemon, moveCandidates1, 5); break;
        case 2: setRandomMoves(pokemon, moveCandidates2, 4); break;
        case 3: setRandomMoves(pokemon, moveCandidates3, 5); break;
        case 4: setRandomMoves(pokemon, moveCandidates4, 5); break;
        case 5: setRandomMoves(pokemon, moveCandidates5, 3); break;
        case 6: setRandomMoves(pokemon, moveCandidates6, 2); break;
        case 7: setRandomMoves(pokemon, moveCandidates7, 2); break;
        case 8: setRandomMoves(pokemon, moveCandidates8, 4); break;
        case 9: setRandomMoves(pokemon, moveCandidates9, 3); break;
        case 10: setRandomMoves(pokemon, moveCandidates10, 3); break;
        case 11: setRandomMoves(pokemon, moveCandidates11, 2); break;
        case 12: setRandomMoves(pokemon, moveCandidates12, 3); break;
        case 13: setRandomMoves(pokemon, moveCandidates13, 2); break;
        case 14: setRandomMoves(pokemon, moveCandidates14, 3); break;
        case 15: setRandomMoves(pokemon, moveCandidates15, 2); break;
        case 16: setRandomMoves(pokemon, moveCandidates16, 3); break;
        case 17: setRandomMoves(pokemon, moveCandidates17, 2); break;
        case 18: setRandomMoves(pokemon, moveCandidates18, 6); break;
        case 19: setRandomMoves(pokemon, moveCandidates19, 6); break;
        case 20: setRandomMoves(pokemon, moveCandidates20, 5); break;
        case 21: setRandomMoves(pokemon, moveCandidates21, 5); break;
        case 22: setRandomMoves(pokemon, moveCandidates22, 3); break;
        case 23: setRandomMoves(pokemon, moveCandidates23, 2); break;
        case 24: setRandomMoves(pokemon, moveCandidates24, 2); break;
        case 25: setRandomMoves(pokemon, moveCandidates25, 4); break;
        case 26: setRandomMoves(pokemon, moveCandidates26, 4); break;
        case 27: setRandomMoves(pokemon, moveCandidates27, 1); break;
        case 28: setRandomMoves(pokemon, moveCandidates28, 2); break;
        case 29: setRandomMoves(pokemon, moveCandidates29, 3); break;
        case 30: setRandomMoves(pokemon, moveCandidates30, 5); break;
        case 31: setRandomMoves(pokemon, moveCandidates31, 5); break;
        case 32: setRandomMoves(pokemon, moveCandidates32, 4); break;
        case 33: setRandomMoves(pokemon, moveCandidates33, 5); break;
        case 34: setRandomMoves(pokemon, moveCandidates34, 1); break;
        case 35: setRandomMoves(pokemon, moveCandidates35, 5); break;
        case 36: setRandomMoves(pokemon, moveCandidates36, 4); break;
        case 37: setRandomMoves(pokemon, moveCandidates37, 5); break;
        case 38: setRandomMoves(pokemon, moveCandidates38, 4); break;
        case 39: setRandomMoves(pokemon, moveCandidates39, 4); break;
        case 40: setRandomMoves(pokemon, moveCandidates40, 2); break;
        case 41: setRandomMoves(pokemon, moveCandidates41, 3); break;
        case 42: setRandomMoves(pokemon, moveCandidates42, 4); break;
        case 43: setRandomMoves(pokemon, moveCandidates43, 4); break;
        case 44: setRandomMoves(pokemon, moveCandidates44, 3); break;
        case 45: setRandomMoves(pokemon, moveCandidates45, 4); break;
        case 46: setRandomMoves(pokemon, moveCandidates46, 3); break;
        case 47: setRandomMoves(pokemon, moveCandidates47, 4); break;
        case 48: setRandomMoves(pokemon, moveCandidates48, 5); break;
        case 49: setRandomMoves(pokemon, moveCandidates49, 4); break;
        case 50: setRandomMoves(pokemon, moveCandidates50, 4); break;
        case 51: setRandomMoves(pokemon, moveCandidates51, 3); break;
        case 52: setRandomMoves(pokemon, moveCandidates52, 4); break;
        case 53: setRandomMoves(pokemon, moveCandidates53, 3); break;
        case 54: setRandomMoves(pokemon, moveCandidates54, 4); break;
        case 55: setRandomMoves(pokemon, moveCandidates55, 5); break;
        case 56: setRandomMoves(pokemon, moveCandidates56, 3); break;
        case 57: setRandomMoves(pokemon, moveCandidates57, 3); break;
        case 58: setRandomMoves(pokemon, moveCandidates58, 2); break;
        case 59: setRandomMoves(pokemon, moveCandidates59, 5); break;
        case 60: setRandomMoves(pokemon, moveCandidates60, 4); break;
        case 61: setRandomMoves(pokemon, moveCandidates61, 4); break;
        case 62: setRandomMoves(pokemon, moveCandidates62, 4); break;
        case 63: setRandomMoves(pokemon, moveCandidates63, 3); break;
        case 64: setRandomMoves(pokemon, moveCandidates64, 3); break;
        case 65: setRandomMoves(pokemon, moveCandidates65, 3); break;
        case 66: setRandomMoves(pokemon, moveCandidates66, 5); break;
        case 67: setRandomMoves(pokemon, moveCandidates67, 0); break;
        case 68: setRandomMoves(pokemon, moveCandidates68, 5); break;
        case 69: setRandomMoves(pokemon, moveCandidates69, 5); break;
        case 70: setRandomMoves(pokemon, moveCandidates70, 5); break;
        case 71: setRandomMoves(pokemon, moveCandidates71, 3); break;
        case 72: setRandomMoves(pokemon, moveCandidates72, 3); break;
        case 73: setRandomMoves(pokemon, moveCandidates73, 4); break;
        case 74: setRandomMoves(pokemon, moveCandidates74, 3); break;
        case 75: setRandomMoves(pokemon, moveCandidates75, 6); break;
        case 76: setRandomMoves(pokemon, moveCandidates76, 3); break;
        case 77: setRandomMoves(pokemon, moveCandidates77, 3); break;
        case 78: setRandomMoves(pokemon, moveCandidates78, 3); break;
        case 79: setRandomMoves(pokemon, moveCandidates79, 4); break;
        case 80: setRandomMoves(pokemon, moveCandidates80, 3); break;
        case 81: setRandomMoves(pokemon, moveCandidates81, 2); break;
        default: clearMoves(pokemon); break;
    }
}

/* 도감에서 중복 없이 6마리를 뽑아 플레이어용 랜덤 팀을 만듭니다. */
Trainer createRandomTrainer(char name[], int level)
{
    Trainer trainer;
    int indexes[POKEMON_COUNT];

    snprintf(trainer.name, sizeof(trainer.name), "%s", name);
    trainer.teamSize = TEAM_SIZE;

    for (int i = 0; i < POKEMON_COUNT; i++) {
        indexes[i] = i;
    }

    shuffleIntArray(indexes, POKEMON_COUNT);

    for (int i = 0; i < TEAM_SIZE; i++) {
        trainer.team[i] = createBattlePokemon(pokedex[indexes[i]], level);
        setRandomLevelUpMoves(&trainer.team[i]);
    }

    return trainer;
}

/* 포켓몬 id로 전투 포켓몬을 만들고 랜덤 기술까지 붙입니다. */
BattlePokemon createBattlePokemonById(int pokemonId, int level)
{
    BattlePokemon battlePokemon = createBattlePokemon(pokedex[pokemonId - 1], level);
    setRandomLevelUpMoves(&battlePokemon);

    return battlePokemon;
}

/* 이름만 있는 빈 트레이너를 만듭니다. 이후 포켓몬을 하나씩 추가합니다. */
Trainer createEmptyTrainer(char name[])
{
    Trainer trainer;

    snprintf(trainer.name, sizeof(trainer.name), "%s", name);
    trainer.teamSize = 0;

    return trainer;
}

/* 랜덤 기술배치를 쓰는 포켓몬을 트레이너 팀에 추가합니다. */
void addPokemonToTrainer(Trainer *trainer, int pokemonId, int level)
{
    if (trainer->teamSize >= TEAM_SIZE) {
        return;
    }

    trainer->team[trainer->teamSize] = createBattlePokemonById(pokemonId, level);
    trainer->teamSize++;
}

/* 지정한 기술 배열을 그대로 포켓몬에게 복사합니다. */
void setFixedMoves(BattlePokemon *pokemon, Move moves[], int moveCount)
{
    clearMoves(pokemon);

    if (moveCount > MOVE_SLOT_COUNT) {
        moveCount = MOVE_SLOT_COUNT;
    }

    for (int i = 0; i < moveCount; i++) {
        addMove(pokemon, moves[i]);
    }
}

/* 원작 기술배치를 가진 포켓몬을 트레이너 팀에 추가합니다. */
void addPokemonToTrainerWithMoves(Trainer *trainer, int pokemonId, int level, Move moves[], int moveCount)
{
    if (trainer->teamSize >= TEAM_SIZE) {
        return;
    }

    trainer->team[trainer->teamSize] = createBattlePokemon(pokedex[pokemonId - 1], level);
    setFixedMoves(&trainer->team[trainer->teamSize], moves, moveCount);
    trainer->teamSize++;
}

/* 사천왕 1번 칸나의 원작 파티를 만듭니다. */
Trainer createLorelei(void)
{
    Trainer trainer = createEmptyTrainer("칸나");

    addPokemonToTrainerWithMoves(&trainer, 37, 54, loreleiDewgongMoves, 4);
    addPokemonToTrainerWithMoves(&trainer, 39, 53, loreleiCloysterMoves, 4);
    addPokemonToTrainerWithMoves(&trainer, 33, 54, loreleiSlowbroMoves, 4);
    addPokemonToTrainerWithMoves(&trainer, 60, 56, loreleiJynxMoves, 4);
    addPokemonToTrainerWithMoves(&trainer, 66, 56, loreleiLaprasMoves, 4);

    return trainer;
}

/* 사천왕 2번 시바의 원작 파티를 만듭니다. */
Trainer createBruno(void)
{
    Trainer trainer = createEmptyTrainer("시바");

    addPokemonToTrainerWithMoves(&trainer, 41, 53, brunoOnix53Moves, 4);
    addPokemonToTrainerWithMoves(&trainer, 48, 55, brunoHitmonchanMoves, 4);
    addPokemonToTrainerWithMoves(&trainer, 47, 55, brunoHitmonleeMoves, 4);
    addPokemonToTrainerWithMoves(&trainer, 41, 56, brunoOnix56Moves, 4);
    addPokemonToTrainerWithMoves(&trainer, 28, 58, brunoMachampMoves, 4);

    return trainer;
}

/* 사천왕 3번 국화의 원작 파티를 만듭니다. */
Trainer createAgatha(void)
{
    Trainer trainer = createEmptyTrainer("국화");

    addPokemonToTrainerWithMoves(&trainer, 40, 56, agathaGengar56Moves, 4);
    addPokemonToTrainerWithMoves(&trainer, 17, 56, agathaGolbatMoves, 4);
    addPokemonToTrainerWithMoves(&trainer, 40, 55, agathaHaunterMoves, 4);
    addPokemonToTrainerWithMoves(&trainer, 9, 58, agathaArbokMoves, 4);
    addPokemonToTrainerWithMoves(&trainer, 40, 60, agathaGengar60Moves, 4);

    return trainer;
}

/* 사천왕 4번 목호의 원작 파티를 만듭니다. */
Trainer createLance(void)
{
    Trainer trainer = createEmptyTrainer("목호");

    addPokemonToTrainerWithMoves(&trainer, 65, 58, lanceGyaradosMoves, 4);
    addPokemonToTrainerWithMoves(&trainer, 79, 56, lanceDragonairMoves, 4);
    addPokemonToTrainerWithMoves(&trainer, 79, 56, lanceDragonairMoves, 4);
    addPokemonToTrainerWithMoves(&trainer, 74, 60, lanceAerodactylMoves, 4);
    addPokemonToTrainerWithMoves(&trainer, 79, 62, lanceDragoniteMoves, 4);

    return trainer;
}

/* 챔피언 파티를 만듭니다. 현재는 거북왕 루트 기준입니다. */
Trainer createChampion(void)
{
    Trainer trainer = createEmptyTrainer("챔피언");

    addPokemonToTrainerWithMoves(&trainer, 6, 61, championPidgeotMoves, 4);
    addPokemonToTrainerWithMoves(&trainer, 27, 59, championAlakazamMoves, 4);
    addPokemonToTrainerWithMoves(&trainer, 51, 61, championRhydonMoves, 4);
    addPokemonToTrainerWithMoves(&trainer, 25, 61, championArcanineMoves, 4);
    addPokemonToTrainerWithMoves(&trainer, 45, 61, championExeggutorMoves, 3);
    addPokemonToTrainerWithMoves(&trainer, 3, 65, championBlastoiseMoves, 4);

    return trainer;
}

/* 칸나 -> 시바 -> 국화 -> 목호 -> 챔피언 순서로 상대 목록을 채웁니다. */
void createEliteFourChallenge(Trainer opponents[], int *opponentCount)
{
    opponents[0] = createLorelei();
    opponents[1] = createBruno();
    opponents[2] = createAgatha();
    opponents[3] = createLance();
    opponents[4] = createChampion();
    *opponentCount = 5;
}

/* 현재 HP가 0이면 쓰러진 것으로 판정합니다. */
int isFainted(BattlePokemon pokemon)
{
    return pokemon.currentHp <= 0;
}

/* 상태이상 enum 값을 출력용 한글 문자열로 바꿉니다. */
const char *getStatusName(StatusCondition status)
{
    if (status == STATUS_SLEEP) {
        return "수면";
    } else if (status == STATUS_PARALYSIS) {
        return "마비";
    } else if (status == STATUS_POISON) {
        return "독";
    } else if (status == STATUS_BURN) {
        return "화상";
    } else if (status == STATUS_FREEZE) {
        return "얼음";
    }

    return "정상";
}

/* 팀에 아직 싸울 수 있는 포켓몬이 남아 있는지 확인합니다. */
int hasUsablePokemon(Trainer trainer)
{
    for (int i = 0; i < trainer.teamSize; i++) {
        if (!isFainted(trainer.team[i])) {
            return 1;
        }
    }

    return 0;
}

/* 팀에서 처음으로 쓰러지지 않은 포켓몬의 인덱스를 찾습니다. */
int getFirstUsablePokemonIndex(Trainer trainer)
{
    for (int i = 0; i < trainer.teamSize; i++) {
        if (!isFainted(trainer.team[i])) {
            return i;
        }
    }

    return -1;
}

/* 기술 타입이 공격자 타입과 같으면 자속 보정 1.5배를 줍니다. */
float getSameTypeAttackBonus(Pokemon attacker, PokemonType moveType)
{
    if (attacker.type1 == moveType || attacker.type2 == moveType) {
        return 1.5;
    }

    return 1.0;
}

/* 랭크 변화값(-6~+6)을 실제 능력치 배율로 바꿉니다. */
float getStageMultiplier(int stage)
{
    if (stage > 6) {
        stage = 6;
    } else if (stage < -6) {
        stage = -6;
    }

    if (stage >= 0) {
        return (2.0 + stage) / 2.0;
    }

    return 2.0 / (2.0 - stage);
}

/* 랭크 변화와 상태이상까지 반영한 전투용 능력치를 계산합니다. */
int getBattleStat(BattlePokemon pokemon, StatType stat)
{
    if (stat == STAT_ATK) {
        int attack = (int)(pokemon.pokemon.atk * getStageMultiplier(pokemon.atkStage));
        /* 화상 상태에서는 물리 공격을 절반으로 낮춥니다. */
        if (pokemon.status == STATUS_BURN) {
            attack /= 2;
        }
        return attack;
    } else if (stat == STAT_DEF) {
        return (int)(pokemon.pokemon.def * getStageMultiplier(pokemon.defStage));
    } else if (stat == STAT_SPA) {
        return (int)(pokemon.pokemon.spa * getStageMultiplier(pokemon.spaStage));
    } else if (stat == STAT_SPD) {
        return (int)(pokemon.pokemon.spd * getStageMultiplier(pokemon.spdStage));
    } else if (stat == STAT_AGI) {
        int speed = (int)(pokemon.pokemon.agi * getStageMultiplier(pokemon.agiStage));
        /* 마비 상태에서는 스피드를 1/4로 낮춥니다. */
        if (pokemon.status == STATUS_PARALYSIS) {
            speed /= 4;
        }
        return speed;
    } else if (stat == STAT_ACC) {
        return (int)(100 * getStageMultiplier(pokemon.accStage));
    } else if (stat == STAT_EVA) {
        return (int)(100 * getStageMultiplier(pokemon.evaStage));
    }

    return 1;
}

/* 특정 능력치 랭크를 바꾸고 -6~+6 범위 안에 고정합니다. */
void changeStatStage(BattlePokemon *pokemon, StatType stat, int change)
{
    int *stage = NULL;

    if (stat == STAT_ATK) {
        stage = &pokemon->atkStage;
    } else if (stat == STAT_DEF) {
        stage = &pokemon->defStage;
    } else if (stat == STAT_SPA) {
        stage = &pokemon->spaStage;
    } else if (stat == STAT_SPD) {
        stage = &pokemon->spdStage;
    } else if (stat == STAT_AGI) {
        stage = &pokemon->agiStage;
    } else if (stat == STAT_ACC) {
        stage = &pokemon->accStage;
    } else if (stat == STAT_EVA) {
        stage = &pokemon->evaStage;
    }

    if (stage == NULL) {
        return;
    }

    *stage += change;

    if (*stage > 6) {
        *stage = 6;
    } else if (*stage < -6) {
        *stage = -6;
    }
}

/* 기술 위력, 공격/방어, 레벨, 자속, 타입 상성을 반영해 데미지를 계산합니다. */
int calculateDamage(BattlePokemon attacker, BattlePokemon defender, Move move)
{
    int attackStat;
    int defenseStat;
    float stab;
    float effectiveness;
    float damage;

    /* 일부 1세대 고정 데미지/일격기 기술은 일반 공식 대신 별도로 처리합니다. */
    if (strcmp(move.name, "용의분노") == 0) {
        return 40;
    } else if (strcmp(move.name, "지구던지기") == 0 || strcmp(move.name, "나이트헤드") == 0) {
        return attacker.level;
    } else if (strcmp(move.name, "소닉붐") == 0) {
        return 20;
    } else if (strcmp(move.name, "뿔드릴") == 0) {
        return defender.currentHp;
    }

    if (move.power <= 0) {
        return 0;
    }

    if (move.category == MOVE_PHYSICAL) {
        attackStat = getBattleStat(attacker, STAT_ATK);
        defenseStat = getBattleStat(defender, STAT_DEF);
    } else {
        attackStat = getBattleStat(attacker, STAT_SPA);
        defenseStat = getBattleStat(defender, STAT_SPD);
    }

    stab = getSameTypeAttackBonus(attacker.pokemon, move.type);
    effectiveness = getTypeEffectiveness(move.type, defender.pokemon.type1, defender.pokemon.type2);

    if (effectiveness == 0) {
        return 0;
    }

    damage = (((2 * attacker.level / 5.0 + 2) * move.power * attackStat / defenseStat) / 50 + 2);
    damage *= stab * effectiveness;

    if (damage < 1) {
        return 1;
    }

    return (int)damage;
}

/* 타입 상성 배율에 맞는 전투 메시지를 출력합니다. */
void printEffectiveness(float effectiveness)
{
    if (effectiveness == 0) {
        printf("효과가 없다!\n");
    } else if (effectiveness >= 2) {
        printf("효과가 굉장했다!\n");
    } else if (effectiveness < 1) {
        printf("효과가 별로인 듯하다...\n");
    }
}

/* 현재 HP에서 데미지를 빼고, 0 밑으로 내려가지 않게 합니다. */
void takeDamage(BattlePokemon *defender, int damage)
{
    defender->currentHp -= damage;

    if (defender->currentHp < 0) {
        defender->currentHp = 0;
    }
}

/* 명중률과 회피율 랭크를 반영해 기술이 맞는지 판정합니다. */
int checkMoveHit(BattlePokemon attacker, BattlePokemon defender)
{
    int accuracy = getBattleStat(attacker, STAT_ACC);
    int evasion = getBattleStat(defender, STAT_EVA);
    int hitRate = accuracy * 100 / evasion;

    if (hitRate > 100) {
        hitRate = 100;
    } else if (hitRate < 30) {
        hitRate = 30;
    }

    return rand() % 100 < hitRate;
}

/* 이미 상태이상이 없을 때만 새 상태이상을 부여합니다. */
void setStatusCondition(BattlePokemon *pokemon, StatusCondition status)
{
    if (pokemon->status != STATUS_NONE || isFainted(*pokemon)) {
        return;
    }

    pokemon->status = status;

    if (status == STATUS_SLEEP) {
        pokemon->sleepTurns = rand() % 3 + 1;
    }

    printf("%s는 %s 상태가 되었다!\n", pokemon->pokemon.name, getStatusName(status));
}

/* 기술 이름을 보고 어떤 상태이상을 유발하는지 반환합니다. */
StatusCondition getMoveStatus(Move move)
{
    if (strcmp(move.name, "수면가루") == 0 || strcmp(move.name, "최면술") == 0 ||
        strcmp(move.name, "버섯포자") == 0 || strcmp(move.name, "노래하기") == 0) {
        return STATUS_SLEEP;
    } else if (strcmp(move.name, "저리가루") == 0 || strcmp(move.name, "전기자석파") == 0 ||
               strcmp(move.name, "뱀눈초리") == 0) {
        return STATUS_PARALYSIS;
    } else if (strcmp(move.name, "독가루") == 0 || strcmp(move.name, "독가스") == 0) {
        return STATUS_POISON;
    } else if (strcmp(move.name, "누르기") == 0 || strcmp(move.name, "번개") == 0 ||
               strcmp(move.name, "번개펀치") == 0 || strcmp(move.name, "핥기") == 0) {
        return STATUS_PARALYSIS;
    } else if (strcmp(move.name, "화염방사") == 0 || strcmp(move.name, "불꽃펀치") == 0) {
        return STATUS_BURN;
    } else if (strcmp(move.name, "냉동빔") == 0 || strcmp(move.name, "눈보라") == 0 ||
               strcmp(move.name, "냉동펀치") == 0) {
        return STATUS_FREEZE;
    }

    return STATUS_NONE;
}

/* 기술별 상태이상 발생 확률을 반환합니다. */
int getMoveStatusChance(Move move)
{
    if (strcmp(move.name, "수면가루") == 0 || strcmp(move.name, "최면술") == 0 ||
        strcmp(move.name, "버섯포자") == 0 || strcmp(move.name, "노래하기") == 0 ||
        strcmp(move.name, "저리가루") == 0 || strcmp(move.name, "전기자석파") == 0 ||
        strcmp(move.name, "뱀눈초리") == 0 || strcmp(move.name, "독가루") == 0 ||
        strcmp(move.name, "독가스") == 0) {
        return 100;
    } else if (strcmp(move.name, "누르기") == 0) {
        return 30;
    } else if (strcmp(move.name, "번개") == 0 || strcmp(move.name, "번개펀치") == 0 ||
               strcmp(move.name, "화염방사") == 0 || strcmp(move.name, "불꽃펀치") == 0 ||
               strcmp(move.name, "냉동빔") == 0 || strcmp(move.name, "눈보라") == 0 ||
               strcmp(move.name, "냉동펀치") == 0) {
        return 10;
    }

    return 0;
}

/* 기술의 부가 효과로 상태이상을 걸 수 있으면 확률 판정을 합니다. */
void tryApplyMoveStatus(BattlePokemon *defender, Move move)
{
    StatusCondition status = getMoveStatus(move);
    int chance = getMoveStatusChance(move);

    if (status == STATUS_NONE || chance <= 0 || defender->status != STATUS_NONE || isFainted(*defender)) {
        return;
    }

    if (rand() % 100 < chance) {
        setStatusCondition(defender, status);
    }
}

/* 수면/마비/얼음 때문에 이번 턴 행동 가능한지 확인합니다. */
int canPokemonMove(BattlePokemon *pokemon)
{
    if (pokemon->status == STATUS_SLEEP) {
        if (pokemon->sleepTurns > 0) {
            pokemon->sleepTurns--;
            printf("%s는 잠들어 있다!\n", pokemon->pokemon.name);
            return 0;
        }

        pokemon->status = STATUS_NONE;
        printf("%s는 잠에서 깨어났다!\n", pokemon->pokemon.name);
    } else if (pokemon->status == STATUS_PARALYSIS) {
        if (rand() % 100 < 25) {
            printf("%s는 몸이 저려 움직일 수 없다!\n", pokemon->pokemon.name);
            return 0;
        }
    } else if (pokemon->status == STATUS_FREEZE) {
        if (rand() % 100 < 20) {
            pokemon->status = STATUS_NONE;
            printf("%s의 얼음이 녹았다!\n", pokemon->pokemon.name);
        } else {
            printf("%s는 얼어붙어 움직일 수 없다!\n", pokemon->pokemon.name);
            return 0;
        }
    }

    return 1;
}

/* 턴 종료 시 독/화상 데미지를 적용합니다. */
void applyEndTurnStatusDamage(BattlePokemon *pokemon)
{
    int damage;

    if (isFainted(*pokemon)) {
        return;
    }

    if (pokemon->status != STATUS_POISON && pokemon->status != STATUS_BURN) {
        return;
    }

    damage = pokemon->pokemon.hp / 8;
    if (damage < 1) {
        damage = 1;
    }

    takeDamage(pokemon, damage);
    printf("%s는 %s으로 %d의 데미지를 입었다!\n",
           pokemon->pokemon.name,
           getStatusName(pokemon->status),
           damage);

    if (isFainted(*pokemon)) {
        printf("%s는 쓰러졌다!\n", pokemon->pokemon.name);
    }
}

/* 위력이 0인 회복, 상태이상, 랭크 변화 기술을 처리합니다. */
void useStatusMove(BattlePokemon *attacker, BattlePokemon *defender, Move move)
{
    BattlePokemon *target = attacker;
    StatusCondition status = getMoveStatus(move);
    int heal;

    /* 회복/특수 보조 기술은 이름으로 별도 처리합니다. */
    if (strcmp(move.name, "HP회복") == 0) {
        heal = attacker->pokemon.hp / 2;
        attacker->currentHp += heal;
        if (attacker->currentHp > attacker->pokemon.hp) {
            attacker->currentHp = attacker->pokemon.hp;
        }
        printf("%s는 체력을 회복했다!\n", attacker->pokemon.name);
        return;
    } else if (strcmp(move.name, "잠자기") == 0) {
        attacker->currentHp = attacker->pokemon.hp;
        attacker->status = STATUS_SLEEP;
        attacker->sleepTurns = 2;
        printf("%s는 잠들고 체력을 모두 회복했다!\n", attacker->pokemon.name);
        return;
    } else if (strcmp(move.name, "리플렉터") == 0) {
        changeStatStage(attacker, STAT_DEF, 2);
        printf("%s의 방어가 크게 올랐다!\n", attacker->pokemon.name);
        return;
    } else if (strcmp(move.name, "흰안개") == 0) {
        attacker->atkStage = attacker->defStage = attacker->spaStage = attacker->spdStage = 0;
        attacker->agiStage = attacker->accStage = attacker->evaStage = 0;
        defender->atkStage = defender->defStage = defender->spaStage = defender->spdStage = 0;
        defender->agiStage = defender->accStage = defender->evaStage = 0;
        printf("모든 능력 변화가 사라졌다!\n");
        return;
    }

    if (status != STATUS_NONE) {
        tryApplyMoveStatus(defender, move);
        return;
    }

    if (move.stat == STAT_NONE || move.statChange == 0) {
        printf("하지만 아무 일도 일어나지 않았다.\n");
        return;
    }

    if (move.statChange < 0) {
        /* 음수 변화량은 상대에게 거는 디버프입니다. */
        target = defender;
    }

    changeStatStage(target, move.stat, move.statChange);

    if (move.statChange > 0) {
        printf("%s의 능력치가 올랐다!\n", target->pokemon.name);
    } else {
        printf("%s의 능력치가 떨어졌다!\n", target->pokemon.name);
    }
}

/* 실제 공격 한 번을 처리합니다. 상태 체크, 명중, 데미지, 부가 효과 순서입니다. */
int attackPokemon(BattlePokemon *attacker, BattlePokemon *defender, Move move)
{
    int damage;
    float effectiveness;

    if (isFainted(*attacker)) {
        return 0;
    }

    /* 상태이상 때문에 움직이지 못하면 기술 사용 자체가 취소됩니다. */
    if (!canPokemonMove(attacker)) {
        return 0;
    }

    printf("%s의 %s!\n", attacker->pokemon.name, move.name);

    if (!checkMoveHit(*attacker, *defender)) {
        printf("공격이 빗나갔다!\n");
        return 0;
    }

    /* 위력이 0인 기술은 데미지 계산 대신 보조 기술 처리로 넘깁니다. */
    if (move.power <= 0) {
        useStatusMove(attacker, defender, move);
        return 0;
    }

    damage = calculateDamage(*attacker, *defender, move);
    effectiveness = getTypeEffectiveness(move.type, defender->pokemon.type1, defender->pokemon.type2);

    takeDamage(defender, damage);
    printf("%s에게 %d의 데미지!\n", defender->pokemon.name, damage);
    printEffectiveness(effectiveness);
    tryApplyMoveStatus(defender, move);
    printf("%s HP: %d/%d\n", defender->pokemon.name, defender->currentHp, defender->pokemon.hp);

    if (isFainted(*defender)) {
        printf("%s는 쓰러졌다!\n", defender->pokemon.name);
    }

    return damage;
}

/* 기술이 없는 포켓몬이 사용할 기본 기술입니다. */
Move getFallbackMove(void)
{
    Move move = {"발버둥", TYPE_NORMAL, MOVE_PHYSICAL, 50, STAT_NONE, 0};
    return move;
}

/* AI가 보조 기술을 고를 때 사용할 간단한 점수입니다. */
int scoreStatusMove(BattlePokemon attacker, BattlePokemon defender, Move move)
{
    int score = 0;
    StatusCondition status = getMoveStatus(move);

    /* 상대가 정상 상태면 수면/마비/독 같은 상태이상 기술을 높게 평가합니다. */
    if (status != STATUS_NONE) {
        if (defender.status == STATUS_NONE) {
            return 45;
        }

        return 0;
    }

    if (move.stat == STAT_NONE || move.statChange == 0) {
        return 0;
    }

    if (move.statChange > 0) {
        if (move.stat == STAT_ATK && attacker.atkStage < 3) {
            score = 30 + move.statChange * 10;
        } else if (move.stat == STAT_SPA && attacker.spaStage < 3) {
            score = 30 + move.statChange * 10;
        } else if (move.stat == STAT_DEF && attacker.defStage < 3) {
            score = 20 + move.statChange * 10;
        } else if (move.stat == STAT_SPD && attacker.spdStage < 3) {
            score = 20 + move.statChange * 10;
        } else if (move.stat == STAT_AGI && attacker.agiStage < 3) {
            score = 15 + move.statChange * 10;
        } else if (move.stat == STAT_EVA && attacker.evaStage < 3) {
            score = 15 + move.statChange * 10;
        }
    } else {
        if (move.stat == STAT_DEF && defender.defStage > -3) {
            score = 25 - move.statChange * 10;
        } else if (move.stat == STAT_ATK && defender.atkStage > -3) {
            score = 20 - move.statChange * 10;
        } else if (move.stat == STAT_SPD && defender.spdStage > -3) {
            score = 20 - move.statChange * 10;
        } else if (move.stat == STAT_AGI && defender.agiStage > -3) {
            score = 15 - move.statChange * 10;
        } else if (move.stat == STAT_ACC && defender.accStage > -3) {
            score = 15 - move.statChange * 10;
        }
    }

    return score;
}

/* AI는 가장 큰 데미지를 내거나 유리한 보조 효과를 주는 기술을 선택합니다. */
Move chooseAIMove(BattlePokemon attacker, BattlePokemon defender)
{
    Move bestMove;
    int bestScore = -1;

    if (attacker.moveCount == 0) {
        return getFallbackMove();
    }

    bestMove = attacker.moves[0];

    for (int i = 0; i < attacker.moveCount; i++) {
        Move move = attacker.moves[i];
        int score;

        if (move.power > 0) {
            score = calculateDamage(attacker, defender, move);
        } else {
            score = scoreStatusMove(attacker, defender, move);
        }

        if (score > bestScore) {
            bestScore = score;
            bestMove = move;
        }
    }

    return bestMove;
}

/* 1대1 전투 상태를 간단히 출력합니다. */
void printBattleStatus(BattlePokemon first, BattlePokemon second)
{
    printf("[%s] HP %d/%d\n", first.pokemon.name, first.currentHp, first.pokemon.hp);
    printf("[%s] HP %d/%d\n", second.pokemon.name, second.currentHp, second.pokemon.hp);
}

/* 한 턴을 진행합니다. 스피드가 높은 쪽이 먼저 움직입니다. */
void battleTurn(BattlePokemon *first, Move firstMove, BattlePokemon *second, Move secondMove)
{
    BattlePokemon *fastPokemon = first;
    BattlePokemon *slowPokemon = second;
    Move fastMove = firstMove;
    Move slowMove = secondMove;

    /* 마비로 낮아진 스피드도 getBattleStat에서 반영됩니다. */
    if (getBattleStat(*second, STAT_AGI) > getBattleStat(*first, STAT_AGI)) {
        fastPokemon = second;
        slowPokemon = first;
        fastMove = secondMove;
        slowMove = firstMove;
    }

    attackPokemon(fastPokemon, slowPokemon, fastMove);

    if (!isFainted(*slowPokemon)) {
        attackPokemon(slowPokemon, fastPokemon, slowMove);
    }

    /* 양쪽 행동이 끝난 뒤 독/화상 데미지를 처리합니다. */
    applyEndTurnStatusDamage(first);
    applyEndTurnStatusDamage(second);
}

/* 두 포켓몬 중 하나가 쓰러질 때까지 1대1 배틀을 반복합니다. */
BattlePokemon *battleOneOnOne(BattlePokemon *first, Move firstMove, BattlePokemon *second, Move secondMove)
{
    int turn = 1;

    while (!isFainted(*first) && !isFainted(*second)) {
        printf("\n--- %d턴 ---\n", turn);
        battleTurn(first, firstMove, second, secondMove);
        printBattleStatus(*first, *second);
        turn++;
    }

    if (isFainted(*first)) {
        printf("\n%s의 승리!\n", second->pokemon.name);
        return second;
    }

    printf("\n%s의 승리!\n", first->pokemon.name);
    return first;
}

int main(void)
{
    /* 랜덤 팀과 랜덤 기술배치를 매 실행마다 다르게 만들기 위한 시드입니다. */
    srand((unsigned int)time(NULL));

    /* 현재 main은 플레이어 랜덤 팀을 생성하고 출력하는 테스트 진입점입니다. */
    Trainer player = createRandomTrainer("플레이어", 50);

    printTrainerTeam(player);

    return 0;
}

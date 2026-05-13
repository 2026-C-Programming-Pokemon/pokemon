#include <stdio.h>

#include "dogam.h"

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
    battlePokemon.rechargeTurns = 0;

    return battlePokemon;
}

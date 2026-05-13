#ifndef DOGAM_H
#define DOGAM_H

#include "../pokemon.h"

extern const char *typeNames[];
extern float typeChart[TYPE_COUNT][TYPE_COUNT];
extern Pokemon pokedex[POKEMON_COUNT];

void printPokemon(Pokemon pokemon);
float getTypeEffectiveness(PokemonType attackType, PokemonType defenderType1, PokemonType defenderType2);
int calculateFinalHp(int baseHp, int individualValue, int level);
int calculateFinalNonHpStat(int baseStat, int individualValue, int level);
Pokemon calculateFinalPokemonStats(Pokemon basePokemon, int level, int individualValue);
BattlePokemon createBattlePokemon(Pokemon pokemon, int level);

#endif

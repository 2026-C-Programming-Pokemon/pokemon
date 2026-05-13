#ifndef ENTRY_H
#define ENTRY_H

#include "../pokemon.h"

void printTrainerTeam(Trainer trainer);
Trainer createRandomTrainer(char name[], int level);
BattlePokemon createBattlePokemonById(int pokemonId, int level);
Trainer createEmptyTrainer(char name[]);
void addPokemonToTrainer(Trainer *trainer, int pokemonId, int level);
void addPokemonToTrainerWithMoves(Trainer *trainer, int pokemonId, int level, Move moves[], int moveCount);
Trainer createLorelei(void);
Trainer createBruno(void);
Trainer createAgatha(void);
Trainer createLance(void);
Trainer createChampion(void);
void createEliteFourChallenge(Trainer opponents[], int *opponentCount);
int hasUsablePokemon(Trainer trainer);
int getFirstUsablePokemonIndex(Trainer trainer);

#endif

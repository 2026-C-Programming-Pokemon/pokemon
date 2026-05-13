#ifndef SKILL_H
#define SKILL_H

#include "../pokemon.h"

extern Move loreleiDewgongMoves[];
extern Move loreleiCloysterMoves[];
extern Move loreleiSlowbroMoves[];
extern Move loreleiJynxMoves[];
extern Move loreleiLaprasMoves[];
extern Move brunoOnix53Moves[];
extern Move brunoHitmonchanMoves[];
extern Move brunoHitmonleeMoves[];
extern Move brunoOnix56Moves[];
extern Move brunoMachampMoves[];
extern Move agathaGengar56Moves[];
extern Move agathaGolbatMoves[];
extern Move agathaHaunterMoves[];
extern Move agathaArbokMoves[];
extern Move agathaGengar60Moves[];
extern Move lanceGyaradosMoves[];
extern Move lanceDragonairMoves[];
extern Move lanceAerodactylMoves[];
extern Move lanceDragoniteMoves[];
extern Move championPidgeotMoves[];
extern Move championAlakazamMoves[];
extern Move championRhydonMoves[];
extern Move championArcanineMoves[];
extern Move championExeggutorMoves[];
extern Move championBlastoiseMoves[];

void clearMoves(BattlePokemon *pokemon);
void addMove(BattlePokemon *pokemon, Move move);
void shuffleIntArray(int array[], int size);
void setRandomMoves(BattlePokemon *pokemon, Move candidates[], int candidateCount);
void setRandomLevelUpMoves(BattlePokemon *pokemon);
void setFixedMoves(BattlePokemon *pokemon, Move moves[], int moveCount);
StatusCondition getMoveStatus(Move move);
int getMoveStatusChance(Move move);
Move getFallbackMove(void);

#endif

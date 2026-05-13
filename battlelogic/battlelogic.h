#ifndef BATTLELOGIC_H
#define BATTLELOGIC_H

#include "../pokemon.h"

int isFainted(BattlePokemon pokemon);
const char *getStatusName(StatusCondition status);
float getSameTypeAttackBonus(Pokemon attacker, PokemonType moveType);
float getStageMultiplier(int stage);
int getBattleStat(BattlePokemon pokemon, StatType stat);
void changeStatStage(BattlePokemon *pokemon, StatType stat, int change);
int calculateDamage(BattlePokemon attacker, BattlePokemon defender, Move move);
void printEffectiveness(float effectiveness);
void takeDamage(BattlePokemon *defender, int damage);
int checkMoveHit(BattlePokemon attacker, BattlePokemon defender);
void setStatusCondition(BattlePokemon *pokemon, StatusCondition status);
void tryApplyMoveStatus(BattlePokemon *defender, Move move);
int canPokemonMove(BattlePokemon *pokemon);
void applyEndTurnStatusDamage(BattlePokemon *pokemon);
void useStatusMove(BattlePokemon *attacker, BattlePokemon *defender, Move move);
int attackPokemon(BattlePokemon *attacker, BattlePokemon *defender, Move move);
int scoreStatusMove(BattlePokemon attacker, BattlePokemon defender, Move move);
Move chooseAIMove(BattlePokemon attacker, BattlePokemon defender);
void printBattleStatus(BattlePokemon first, BattlePokemon second);
void battleTurn(BattlePokemon *first, Move firstMove, BattlePokemon *second, Move secondMove);
BattlePokemon *battleOneOnOne(BattlePokemon *first, Move firstMove, BattlePokemon *second, Move secondMove);

#endif

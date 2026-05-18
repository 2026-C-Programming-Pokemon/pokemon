#include <stdio.h>

#include "../pokemon.h"

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

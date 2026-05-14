#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "pokemon.h"

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


void clearScreen(void)
{
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

void renderHpBar(int current, int max)
{
    int barSize = 20;
    int filled = (current * barSize) / max;

    printf("[");

    for (int i = 0; i < barSize; i++) {
        if (i < filled)
            printf("#");
        else
            printf("-");
    }

    printf("]");
}

void renderBattleScreen(BattlePokemon player, BattlePokemon enemy)
{
    clearScreen();

    printf("\n");
    printf("=====================================\n");
    printf("            포켓몬 배틀\n");
    printf("=====================================\n");

    printf("\n[상대 포켓몬]\n");

    printf("%s  Lv.%d\n",
           enemy.pokemon.name,
           enemy.level);

    renderHpBar(enemy.currentHp, enemy.pokemon.hp);

    printf(" %d/%d\n",
           enemy.currentHp,
           enemy.pokemon.hp);

    printf("\n-------------------------------------\n");

    printf("\n[내 포켓몬]\n");

    printf("%s  Lv.%d\n",
           player.pokemon.name,
           player.level);

    renderHpBar(player.currentHp, player.pokemon.hp);

    printf(" %d/%d\n",
           player.currentHp,
           player.pokemon.hp);

    printf("\n=====================================\n");
}

int selectMove(BattlePokemon pokemon)
{
    int choice;

    printf("\n사용할 기술을 선택하세요.\n\n");

    for (int i = 0; i < pokemon.moveCount; i++) {

        printf("%d. %s",
               i + 1,
               pokemon.moves[i].name);

        if (pokemon.moves[i].power > 0)
            printf(" (위력 %d)", pokemon.moves[i].power);

        printf(" [명중 %d%%]", getMoveAccuracy(pokemon.moves[i]));

        printf("\n");
    }

    printf("\n입력: ");
    if (scanf("%d", &choice) != 1) {
        return -2;
    }

    return choice - 1;
}

int selectBattleAction(void)
{
    int choice;

    printf("\n행동을 선택하세요.\n");
    printf("1. 싸운다\n");
    printf("2. 포켓몬 교체\n");
    printf("\n입력: ");
    if (scanf("%d", &choice) != 1) {
        return -2;
    }

    return choice;
}

void printSwitchOptions(Trainer trainer, int activeIndex)
{
    printf("\n교체할 포켓몬을 선택하세요.\n\n");

    for (int i = 0; i < trainer.teamSize; i++) {
        printf("%d. %s HP %d/%d",
               i + 1,
               trainer.team[i].pokemon.name,
               trainer.team[i].currentHp,
               trainer.team[i].pokemon.hp);

        if (i == activeIndex) {
            printf(" (현재)");
        } else if (isFainted(trainer.team[i])) {
            printf(" (기절)");
        }

        printf("\n");
    }
}

int selectSwitchPokemon(Trainer trainer, int activeIndex)
{
    int choice;

    printSwitchOptions(trainer, activeIndex);
    printf("\n입력: ");
    if (scanf("%d", &choice) != 1) {
        return -2;
    }

    return choice - 1;
}

int forcePlayerSwitch(Trainer *player, int *activeIndex)
{
    int newIndex;

    if (!hasUsablePokemon(*player)) {
        return 0;
    }

    while (1) {
        printf("\n%s는 더 이상 싸울 수 없다!\n",
               player->team[*activeIndex].pokemon.name);
        newIndex = selectSwitchPokemon(*player, *activeIndex);

        if (newIndex == -2) {
            return 0;
        }

        if (canSwitchToPokemon(*player, *activeIndex, newIndex)) {
            switchPokemon(player, activeIndex, newIndex);
            return 1;
        }

        printf("\n그 포켓몬으로는 교체할 수 없습니다.\n");
    }
}

void switchEnemyIfNeeded(Trainer *rival, int *activeIndex)
{
    int newIndex;

    if (!isFainted(rival->team[*activeIndex])) {
        return;
    }

    newIndex = getFirstUsablePokemonIndex(*rival);
    if (newIndex < 0) {
        return;
    }

    resetBattleStages(&rival->team[*activeIndex]);
    resetBattleStages(&rival->team[newIndex]);
    *activeIndex = newIndex;

    printf("\n%s는 %s을(를) 내보냈다!\n",
           rival->name,
           rival->team[*activeIndex].pokemon.name);
}

void waitEnter(void)
{
    printf("\n계속하려면 엔터를 누르세요...");
    getchar();
    getchar();
}

/* =========================
   완전히 교체할 main 함수
   ========================= */

int main(void)
{
    srand((unsigned int)time(NULL));


    Trainer player = createRandomTrainer("플레이어", 50);
    Trainer rival  = createRandomTrainer("라이벌", 50);
    int playerActiveIndex = 0;
    int enemyActiveIndex = 0;
    int inputClosed = 0;

    printf("\n내 팀\n");
    printTrainerTeam(player);

    printf("\n상대 팀 생성 완료!\n");

    while (hasUsablePokemon(player) &&
           hasUsablePokemon(rival))
    {
        BattlePokemon *playerPokemon = &player.team[playerActiveIndex];
        BattlePokemon *enemyPokemon = &rival.team[enemyActiveIndex];
        int action;

        if (isFainted(*playerPokemon)) {
            if (!forcePlayerSwitch(&player, &playerActiveIndex)) {
                if (hasUsablePokemon(player)) {
                    inputClosed = 1;
                }
                break;
            }
            playerPokemon = &player.team[playerActiveIndex];
        }

        if (isFainted(*enemyPokemon)) {
            switchEnemyIfNeeded(&rival, &enemyActiveIndex);
            enemyPokemon = &rival.team[enemyActiveIndex];
            waitEnter();
            continue;
        }

        renderBattleScreen(*playerPokemon,
                           *enemyPokemon);

        action = selectBattleAction();

        if (action == -2) {
            inputClosed = 1;
            break;
        }

        if (action == 2) {
            int switchIndex = selectSwitchPokemon(player, playerActiveIndex);
            Move enemyMove;

            if (switchIndex == -2) {
                inputClosed = 1;
                break;
            }

            if (!switchPokemon(&player, &playerActiveIndex, switchIndex)) {
                printf("\n그 포켓몬으로는 교체할 수 없습니다.\n");
                waitEnter();
                continue;
            }

            playerPokemon = &player.team[playerActiveIndex];
            enemyMove = chooseAIMove(*enemyPokemon, *playerPokemon);

            printf("\n%s의 %s 사용!\n",
                   enemyPokemon->pokemon.name,
                   enemyMove.name);

            attackPokemon(enemyPokemon, playerPokemon, enemyMove);
            applyEndTurnStatusDamage(playerPokemon);
            applyEndTurnStatusDamage(enemyPokemon);

            printf("\n턴 종료!\n");

            printBattleStatus(*playerPokemon,
                              *enemyPokemon);

            waitEnter();
            continue;
        }

        if (action != 1) {
            printf("\n잘못된 입력입니다.\n");
            waitEnter();
            continue;
        }

        int moveIndex = selectMove(*playerPokemon);

        if (moveIndex == -2) {
            inputClosed = 1;
            break;
        }

        if (moveIndex < 0 ||
            moveIndex >= playerPokemon->moveCount)
        {
            printf("\n잘못된 입력입니다.\n");
            waitEnter();
            continue;
        }

        Move playerMove =
            playerPokemon->moves[moveIndex];

        Move enemyMove =
            chooseAIMove(*enemyPokemon, *playerPokemon);

        printf("\n%s의 %s 사용!\n",
               playerPokemon->pokemon.name,
               playerMove.name);

        printf("%s의 %s 사용!\n",
               enemyPokemon->pokemon.name,
               enemyMove.name);

        battleTurn(playerPokemon,
                   playerMove,
                   enemyPokemon,
                   enemyMove);

        printf("\n턴 종료!\n");

        printBattleStatus(*playerPokemon,
                          *enemyPokemon);

        waitEnter();
    }

    clearScreen();

    printf("\n=========================\n");

    if (inputClosed) {

        printf("\n게임을 종료합니다.\n");

    } else if (!hasUsablePokemon(player)) {

        printf("\n패배했습니다...\n");

    } else {

        printf("\n승리했습니다!!\n");
    }

    printf("\n=========================\n");

    

    return 0;
}

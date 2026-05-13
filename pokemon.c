#include <stdlib.h>
#include <time.h>

#include "entry/entry.h"

int main(void)
{
    /* 랜덤 팀과 랜덤 기술배치를 매 실행마다 다르게 만들기 위한 시드입니다. */
    srand((unsigned int)time(NULL));

    /* 현재 main은 플레이어 랜덤 팀을 생성하고 출력하는 테스트 진입점입니다. */
    Trainer player = createRandomTrainer("플레이어", 50);

    printTrainerTeam(player);

    return 0;
}

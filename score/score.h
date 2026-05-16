#ifndef SCORE_H
#define SCORE_H

/*
 * 전투 점수 계산 함수입니다.
 * turnsTaken: 상대를 한 명 클리어하는 데 걸린 턴 수.
 * remainingPokemon: 승리 시 남아 있는 내 포켓몬 수.
 */
int calculateScore(int turnsTaken, int remainingPokemon);

void addScoreToLeaderboard(const char *name, int score);
void showLeaderboard(void);

#endif

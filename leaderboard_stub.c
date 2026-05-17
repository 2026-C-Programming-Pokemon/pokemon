#include <stdio.h>

int calculateScore(int wins, int losses, int turns) {
    return wins * 100 - losses * 50 - turns;
}

void addScoreToLeaderboard(const char* name, int score) {
    printf("Score saved: %s (%d)\n", name, score);
}

void showLeaderboard() {
    printf("===== LEADERBOARD =====\n");
    printf("Leaderboard unavailable.\n");
    printf("=======================\n");
}

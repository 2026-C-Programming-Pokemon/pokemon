#include "score.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <direct.h>
#else
#include <sys/stat.h>
#endif

#define MAX_LEADERBOARD_ENTRIES 5
#define MAX_LEADERBOARD_HISTORY 100
#define LEADERBOARD_PATH "score/ranking.txt"

typedef struct {
    char name[32];
    int score;
} ScoreEntry;

static int compareScoreEntries(const void *a, const void *b)
{
    const ScoreEntry *first = a;
    const ScoreEntry *second = b;
    return second->score - first->score;
}

static int parseRankingLine(const char *line, ScoreEntry *entry)
{
    char buffer[128];
    size_t length = strlen(line);
    while (length > 0 && (line[length - 1] == '\n' || line[length - 1] == '\r')) {
        length--;
    }
    if (length == 0 || length >= sizeof(buffer)) {
        return 0;
    }

    memcpy(buffer, line, length);
    buffer[length] = '\0';

    char *lastSpace = strrchr(buffer, ' ');
    if (lastSpace == NULL) {
        return 0;
    }

    *lastSpace = '\0';
    entry->score = atoi(lastSpace + 1);
    if (entry->score < 0) {
        return 0;
    }

    strncpy(entry->name, buffer, sizeof(entry->name));
    entry->name[sizeof(entry->name) - 1] = '\0';
    return 1;
}

static int loadLeaderboard(ScoreEntry entries[], int maxEntries)
{
    FILE *fp = fopen(LEADERBOARD_PATH, "r");
    if (fp == NULL) {
        return 0;
    }

    ScoreEntry buffer[MAX_LEADERBOARD_HISTORY];
    int total = 0;
    char line[128];

    while (total < MAX_LEADERBOARD_HISTORY && fgets(line, sizeof(line), fp)) {
        if (parseRankingLine(line, &buffer[total])) {
            total++;
        }
    }
    fclose(fp);

    if (total == 0) {
        return 0;
    }

    qsort(buffer, total, sizeof(buffer[0]), compareScoreEntries);
    int count = total < maxEntries ? total : maxEntries;
    for (int i = 0; i < count; i++) {
        entries[i] = buffer[i];
    }
    return count;
}

static void saveLeaderboard(const ScoreEntry entries[], int count)
{
#ifdef _WIN32
    _mkdir("score");
#else
    mkdir("score", 0755);
#endif

    FILE *fp = fopen(LEADERBOARD_PATH, "w");
    if (fp == NULL) {
        return;
    }

    for (int i = 0; i < count; i++) {
        fprintf(fp, "%s %d\n", entries[i].name, entries[i].score);
    }
    fclose(fp);
}

int calculateScore(int turnsTaken, int remainingPokemon)
{
    if (turnsTaken < 1) {
        turnsTaken = 1;
    }
    if (remainingPokemon < 0) {
        remainingPokemon = 0;
    }

    int score = 0;
    score += remainingPokemon * 200;
    score += 1000 / turnsTaken;
    return score;
}

void addScoreToLeaderboard(const char *name, int score)
{
    if (score < 0) {
        return;
    }

    ScoreEntry entries[MAX_LEADERBOARD_ENTRIES + 1];
    int count = loadLeaderboard(entries, MAX_LEADERBOARD_ENTRIES + 1);

    if (count < MAX_LEADERBOARD_ENTRIES || score > entries[count - 1].score) {
        if (count < MAX_LEADERBOARD_ENTRIES) {
            count++;
        }

        entries[count - 1].score = score;
        strncpy(entries[count - 1].name, name, sizeof(entries[count - 1].name));
        entries[count - 1].name[sizeof(entries[count - 1].name) - 1] = '\0';

        qsort(entries, count, sizeof(entries[0]), compareScoreEntries);
        if (count > MAX_LEADERBOARD_ENTRIES) {
            count = MAX_LEADERBOARD_ENTRIES;
        }
        saveLeaderboard(entries, count);
    }
}

void showLeaderboard(void)
{
    ScoreEntry entries[MAX_LEADERBOARD_ENTRIES];
    int count = loadLeaderboard(entries, MAX_LEADERBOARD_ENTRIES);

    printf("\n=====랭킹=====\n");
    if (count == 0) {
        printf("아직 기록이 없습니다.\n");
    } else {
        for (int i = 0; i < count; i++) {
            printf("%d위 %s : %d점\n", i + 1, entries[i].name, entries[i].score);
        }
    }
    printf("==============\n");
}

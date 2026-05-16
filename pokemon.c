#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <locale.h>

#ifdef _WIN32
#include <windows.h>
#include <io.h>
#else
#include <signal.h>
#include <unistd.h>
#endif

#include "pokemon.h"
#include "score/score.h"

#ifdef _WIN32
static PROCESS_INFORMATION bgmProcess;
#else
static int bgmPid = -1;
#endif

static float musicVolume = 1.0f;

static void stopBackgroundMusic(void);

static void configureConsole(void)
{
#ifdef _WIN32
    if (setlocale(LC_ALL, ".UTF-8") == NULL) {
        setlocale(LC_ALL, "");
    }
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    HANDLE output = GetStdHandle(STD_OUTPUT_HANDLE);
    if (output != INVALID_HANDLE_VALUE) {
        DWORD mode = 0;
        if (GetConsoleMode(output, &mode)) {
            SetConsoleMode(output, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
        }
    }
#else
    setlocale(LC_ALL, "");
#endif
}

static void flushInputLine(void)
{
    int ch;
    while ((ch = getchar()) != '\n' && ch != EOF) {}
}

static void playMusic(const char *path, float volume)
{
    if (volume <= 0.0f) {
        stopBackgroundMusic();
        return;
    }
    if (volume > 1.0f) {
        volume = 1.0f;
    }

#ifdef _WIN32
    char fullPath[MAX_PATH];
    if (_access(path, 0) != 0 || GetFullPathNameA(path, sizeof(fullPath), fullPath, NULL) == 0) {
        return;
    }

    stopBackgroundMusic();

    STARTUPINFO si;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);

    char command[4096];
    snprintf(command, sizeof(command),
             "powershell.exe -STA -NoProfile -ExecutionPolicy Bypass -Command \"Add-Type -AssemblyName PresentationCore; $player = New-Object System.Windows.Media.MediaPlayer; $player.Open([System.Uri]::new('%s')); $player.Volume = %.2f; $player.Play(); while ($true) { Start-Sleep -Seconds 1 }\"",
             fullPath, volume);

    ZeroMemory(&bgmProcess, sizeof(bgmProcess));
    if (!CreateProcess(NULL, command, NULL, NULL, FALSE, CREATE_NO_WINDOW | CREATE_NEW_PROCESS_GROUP, NULL, NULL, &si, &bgmProcess)) {
        bgmProcess.hProcess = NULL;
        bgmProcess.hThread = NULL;
    }
#else
    if (access(path, F_OK) != 0) {
        return;
    }

    char command[1024];
    snprintf(command, sizeof(command), "afplay -v %.2f \"%s\" >/dev/null 2>&1 & echo $!", volume, path);
    FILE *pipe = popen(command, "r");
    if (pipe == NULL) {
        return;
    }

    if (fscanf(pipe, "%d", &bgmPid) != 1) {
        bgmPid = -1;
    }
    pclose(pipe);
#endif
}

static void startTitleMusic(void)
{
    playMusic("sound/1-01. Title Screen.mp3", musicVolume);
}

static void startBackgroundMusic(void)
{
#ifdef _WIN32
    playMusic("sound\\1-01. Title Screen.mp3", musicVolume);
#else
    playMusic("sound/1-01. Title Screen.mp3", musicVolume);
#endif
}

static void startBattleMusic(void)
{
#ifdef _WIN32
    playMusic("sound\\1-28. Battle! (Gym Leader Battle).mp3", musicVolume);
#else
    playMusic("sound/1-28. Battle! (Gym Leader Battle).mp3", musicVolume);
#endif
}

static void startVictoryMusic(void)
{
#ifdef _WIN32
    playMusic("sound\\1-29. Victory! (Gym Leader Battle).mp3", musicVolume);
#else
    playMusic("sound/1-29. Victory! (Gym Leader Battle).mp3", musicVolume);
#endif
}

static void startHallOfFameMusic(void)
{
#ifdef _WIN32
    playMusic("sound\\1-44. Hall of Fame.mp3", musicVolume);
#else
    playMusic("sound/1-44. Hall of Fame.mp3", musicVolume);
#endif
}

static void stopBackgroundMusic(void)
{
#ifdef _WIN32
    if (bgmProcess.hProcess == NULL) {
        return;
    }

    TerminateProcess(bgmProcess.hProcess, 0);
    CloseHandle(bgmProcess.hProcess);
    CloseHandle(bgmProcess.hThread);
    bgmProcess.hProcess = NULL;
    bgmProcess.hThread = NULL;
#else
    if (bgmPid <= 0) {
        return;
    }

    kill(bgmPid, SIGTERM);
    bgmPid = -1;
#endif
}

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
    /* system("clear")는 VS Code/일부 터미널에서 TERM 오류가 나서 ANSI 코드로 처리합니다. */
    printf("\033[2J\033[H");
}







static void printLineSlow(const char *text)
{
    /* 너무 느리지 않게 한 줄씩 출력해서 레드/그린 텍스트 박스 느낌을 냅니다. */
    printf("%s\n", text);
}

void pressEnter(void)
{
    int c;
    printf("\n▼ 엔터를 누르세요...");
    while ((c = getchar()) != '\n' && c != EOF) {}
    getchar();
}

void waitEnter(void)
{
    printf("\n▼ 엔터를 누르세요...");
    flushInputLine();
}

static int getRealDexNumber(const char *name)
{
    for (int i = 0; i < POKEMON_COUNT; i++) {
        if (strcmp(name, pokedex[i].name) == 0) {
            return pokedex[i].id;
        }
    }

    struct NameDex { const char *name; int dex; } table[] = {
        {"이상해꽃",3},{"리자몽",6},{"거북왕",9},{"버터풀",12},{"독침붕",15},{"피죤투",18},
        {"레트라",20},{"깨비드릴조",22},{"아보크",24},{"라이츄",26},{"고지",28},{"니드퀸",31},
        {"니드킹",34},{"픽시",36},{"나인테일",38},{"푸크린",40},{"골벳",42},{"라플레시아",45},
        {"파라섹트",47},{"도나리",49},{"닥트리오",51},{"페르시온",53},{"골덕",55},{"성원숭",57},
        {"윈디",59},{"강챙이",62},{"후딘",65},{"괴력몬",68},{"우츠보트",71},{"독파리",73},
        {"딱구리",76},{"날쌩마",78},{"야도란",80},{"레어코일",82},{"파오리",83},{"두트리오",85},
        {"쥬레곤",87},{"질뻐기",89},{"파르셀",91},{"팬텀",94},{"롱스톤",95},{"슬리퍼",97},
        {"킹크랩",99},{"붐볼",101},{"나시",103},{"텅구리",105},{"시라소몬",106},{"홍수몬",107},
        {"내루미",108},{"또도가스",110},{"코뿌리",112},{"럭키",113},{"덩쿠리",114},{"캥카",115},
        {"시드라",117},{"왕콘치",119},{"아쿠스타",121},{"마임맨",122},{"스라크",123},{"루주라",124},
        {"에레브",125},{"마그마",126},{"쁘사이저",127},{"켄타로스",128},{"갸라도스",130},{"라프라스",131},
        {"메타몽",132},{"샤미드",134},{"쥬피썬더",135},{"부스터",136},{"폴리곤",137},{"암스타",139},
        {"투구푸스",141},{"프테라",142},{"잠만보",143},{"프리져",144},{"썬더",145},{"파이어",146},
        {"망나뇽",149},{"뮤츠",150},{"뮤",151}
    };
    int count = (int)(sizeof(table) / sizeof(table[0]));
    for (int i = 0; i < count; i++) {
        if (strcmp(name, table[i].name) == 0) return table[i].dex;
    }
    return 25;
}

static FILE *openResourceFile(const char *relativePath, char *resolvedPath, size_t resolvedPathSize)
{
    FILE *fp = fopen(relativePath, "r");
    if (fp != NULL) {
        snprintf(resolvedPath, resolvedPathSize, "%s", relativePath);
        return fp;
    }

#ifdef _WIN32
    char exePath[MAX_PATH];
    DWORD len = GetModuleFileNameA(NULL, exePath, sizeof(exePath));
    if (len > 0 && len < sizeof(exePath)) {
        char *lastSlash = strrchr(exePath, '\\');
        if (lastSlash != NULL) {
            *(lastSlash + 1) = '\0';
            snprintf(resolvedPath, resolvedPathSize, "%s%s", exePath, relativePath);
            return fopen(resolvedPath, "r");
        }
    }
#endif

    resolvedPath[0] = '\0';
    return NULL;
}

static int shouldPrintConflictLine(const char *line, int *hasConflict, int *inSelectedSide)
{
    if (strstr(line, "<<<<<<<") != NULL) {
        *hasConflict = 1;
        *inSelectedSide = 0;
        return 0;
    }
    if (*hasConflict && strstr(line, "=======") != NULL) {
        *inSelectedSide = 1;
        return 0;
    }
    if (*hasConflict && strstr(line, ">>>>>>>") != NULL) {
        *hasConflict = 0;
        *inSelectedSide = 0;
        return 0;
    }

    return !*hasConflict || *inSelectedSide;
}

static void printSpriteFileLine(const char *line)
{
    const char *first = strchr(line, '"');
    const char *last = strrchr(line, '"');
    if (first == NULL || last == NULL || first == last) return;
    for (const char *p = first + 1; p < last; p++) {
        if (*p == '\\' && p + 1 < last) {
            p++;
            if (*p == 'n') putchar('\n');
            else if (*p == 't') putchar('\t');
            else putchar(*p);
        } else {
            putchar(*p);
        }
    }
    putchar('\n');
}

static int printTextArtFile(FILE *fp)
{
    char line[512];
    int hasConflict = 0;
    int inSelectedSide = 0;
    int printed = 0;

    while (fgets(line, sizeof(line), fp)) {
        if (!shouldPrintConflictLine(line, &hasConflict, &inSelectedSide)) {
            continue;
        }
        fputs(line, stdout);
        printed = 1;
    }

    return printed;
}

static int printGeneratedArtFile(FILE *fp)
{
    char line[512];
    int hasConflict = 0;
    int inSelectedSide = 0;
    int printed = 0;

    while (fgets(line, sizeof(line), fp)) {
        if (!shouldPrintConflictLine(line, &hasConflict, &inSelectedSide)) {
            continue;
        }
        printSpriteFileLine(line);
        printed = 1;
    }

    return printed;
}

static void showPokemonArtByDexNumber(int dexNumber, const char *name)
{
    const char *formats[] = {
        "tools/ascii_converter/output/%03d.txt",
        "tools/ascii_converter/output/bin/%03d.txt",
        "tools/ascii_converter/generated/%03d.inc",
        "tools/ascii_converter/generated/bin/%03d.inc"
    };
    char relativePath[256];
    char resolvedPath[512];
    FILE *fp = NULL;

    for (int i = 0; i < (int)(sizeof(formats) / sizeof(formats[0])); i++) {
        snprintf(relativePath, sizeof(relativePath), formats[i], dexNumber);
        fp = openResourceFile(relativePath, resolvedPath, sizeof(resolvedPath));
        if (fp != NULL) {
            int printed = strstr(relativePath, ".inc") != NULL
                ? printGeneratedArtFile(fp)
                : printTextArtFile(fp);
            fclose(fp);
            if (printed) {
                return;
            }
        }
    }

    printf("\n        [No.%03d %s]\n", dexNumber, name);
}

void showPokemonArtByName(const char *name)
{
    showPokemonArtByDexNumber(getRealDexNumber(name), name);
}

void showPokemonArt(Pokemon pokemon)
{
    showPokemonArtByDexNumber(getPokemonDexNumber(pokemon), pokemon.name);
}

void showTitleScreen(void)
{
    clearScreen();
    printf("\n");
    printf("  ================================================\n");
    printf("  |                                              |\n");
    printf("  |        P O K E M O N   R E D / G R E E N    |\n");
    printf("  |              C CONSOLE EDITION               |\n");
    printf("  |                                              |\n");
    printf("  ================================================\n\n");
    showPokemonArtByName("피카츄");
    printf("\n  1. 처음부터 시작\n");
    printf("  2. 조작법 보기\n");
    printf("  3. 옵션\n");
    printf("  4. 종료\n");
    printf("\n  입력: ");
}

void showHowToPlay(void)
{
    clearScreen();
    printLineSlow("오박사: 포켓몬 세계에 온 걸 환영한다!");
    printLineSlow("전투는 원작처럼 싸운다/포켓몬 교체를 골라 진행한다.");
    printLineSlow("상대는 칸나, 시바, 국화, 목호, 챔피언 순서로 등장한다.");
    printLineSlow("HP가 0이 되면 다음 포켓몬으로 교체해야 한다.");
    printLineSlow("모든 상대를 쓰러뜨리면 전당등록이다!");
    waitEnter();
}

void showOptionsScreen(void)
{
    int choice;
    while (1) {
        int volumePercent = (int)(musicVolume * 100.0f + 0.5f);
        clearScreen();
        printf("\n  ================================================\n");
        printf("  |                    옵션                      |\n");
        printf("  ================================================\n\n");
        printf("  배경음악 볼륨: %3d%% [", volumePercent);
        for (int i = 0; i < 10; i++) {
            printf(i < volumePercent / 10 ? "#" : "-");
        }
        printf("]\n\n");
        printf("  1. 배경음악 볼륨 설정\n");
        printf("  2. 이전 화면으로 돌아가기\n");
        printf("\n입력: ");

        if (scanf("%d", &choice) != 1) {
            flushInputLine();
            continue;
        }
        flushInputLine();

        if (choice == 1) {
            int newVolume;
            printf("\n볼륨 값을 0부터 100 사이로 입력하세요: ");
            if (scanf("%d", &newVolume) != 1) {
                flushInputLine();
                continue;
            }
            flushInputLine();
            if (newVolume < 0) newVolume = 0;
            if (newVolume > 100) newVolume = 100;
            musicVolume = newVolume / 100.0f;
            stopBackgroundMusic();
            startTitleMusic();
            printf("\n볼륨이 %d%%로 설정되었습니다.\n", newVolume);
            waitEnter();
        } else if (choice == 2) {
            break;
        }
    }
}

void showOpeningStory(void)
{
    clearScreen();
    printLineSlow("오박사: 자! 이제부터 너의 이야기가 시작된다!");
    printLineSlow("포켓몬 리그의 문이 열렸다.");
    printLineSlow("강한 트레이너들과 싸워 챔피언이 되어라!");
    waitEnter();
}

void showEndingScreen(int won)
{
    clearScreen();
    printf("\n===============================================\n");
    if (won) {
        printf("             전당등록! 축하합니다!\n");
        printf("        당신은 포켓몬 리그 챔피언입니다.\n");
        showPokemonArtByName("뮤");
    } else {
        printf("              눈앞이 깜깜해졌다...\n");
        printf("        포켓몬센터에서 다시 도전하세요.\n");
        showPokemonArtByName("잠만보");
    }
    printf("===============================================\n");
}


void renderHpBar(int current, int max)
{
    int barSize = 20;
    int filled;
    if (max <= 0) max = 1;
    if (current < 0) current = 0;
    filled = (current * barSize) / max;
    printf("[");
    for (int i = 0; i < barSize; i++) printf(i < filled ? "#" : "-");
    printf("]");
}

void renderBattleScreen(BattlePokemon player, BattlePokemon enemy)
{
    clearScreen();
    printf("\n============================================================\n");
    printf("  %s  Lv.%d\n", enemy.pokemon.name, enemy.level);
    printf("  HP ");
    renderHpBar(enemy.currentHp, enemy.pokemon.hp);
    printf("\n");
    showPokemonArt(enemy.pokemon);
    printf("------------------------------------------------------------\n");
    showPokemonArt(player.pokemon);
    printf("  %s  Lv.%d\n", player.pokemon.name, player.level);
    printf("  HP ");
    renderHpBar(player.currentHp, player.pokemon.hp);
    printf(" %d/%d  상태:%s\n", player.currentHp, player.pokemon.hp, getStatusName(player.status));
    printf("============================================================\n");
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

/* =========================
   완전히 교체할 main 함수
   ========================= */


static void healTrainer(Trainer *trainer)
{
    for (int i = 0; i < trainer->teamSize; i++) {
        trainer->team[i].currentHp = trainer->team[i].pokemon.hp;
        trainer->team[i].status = STATUS_NONE;
        trainer->team[i].sleepTurns = 0;
        resetBattleStages(&trainer->team[i]);
    }
}

static int countRemainingPokemon(const Trainer *trainer)
{
    int count = 0;
    for (int i = 0; i < trainer->teamSize; i++) {
        if (!isFainted(trainer->team[i])) {
            count++;
        }
    }
    return count;
}

static void promptPlayerName(char *name, int size)
{
    printf("\n이름을 입력하세요: ");
    if (fgets(name, size, stdin) == NULL) {
        name[0] = '\0';
        return;
    }
    if (name[0] == '\n') {
        if (fgets(name, size, stdin) == NULL) {
            name[0] = '\0';
            return;
        }
    }

    size_t len = strcspn(name, "\r\n");
    name[len] = '\0';
    if (len == 0) {
        strncpy(name, "익명의 트레이너", size);
        name[size - 1] = '\0';
    }
}

static int battleTrainer(Trainer *player, Trainer rival, int *turnsTakenOut)
{
    int playerActiveIndex = getFirstUsablePokemonIndex(*player);
    int enemyActiveIndex = getFirstUsablePokemonIndex(rival);
    int inputClosed = 0;
    int turnsTaken = 0;

    startBattleMusic();
    clearScreen();
    printf("\n%s가 승부를 걸어왔다!\n", rival.name);
    printf("%s는 %s을(를) 내보냈다!\n", rival.name, rival.team[enemyActiveIndex].pokemon.name);
    printf("가랏! %s!\n", player->team[playerActiveIndex].pokemon.name);
    waitEnter();

    while (hasUsablePokemon(*player) && hasUsablePokemon(rival)) {
        BattlePokemon *playerPokemon = &player->team[playerActiveIndex];
        BattlePokemon *enemyPokemon = &rival.team[enemyActiveIndex];
        int action;

        if (isFainted(*playerPokemon)) {
            if (!forcePlayerSwitch(player, &playerActiveIndex)) {
                inputClosed = 1;
                break;
            }
            playerPokemon = &player->team[playerActiveIndex];
        }

        if (isFainted(*enemyPokemon)) {
            switchEnemyIfNeeded(&rival, &enemyActiveIndex);
            if (!hasUsablePokemon(rival)) break;
            enemyPokemon = &rival.team[enemyActiveIndex];
            waitEnter();
            continue;
        }

        renderBattleScreen(*playerPokemon, *enemyPokemon);
        action = selectBattleAction();

        if (action == -2) { inputClosed = 1; break; }

        if (action == 2) {
            int switchIndex = selectSwitchPokemon(*player, playerActiveIndex);
            Move enemyMove;

            if (switchIndex == -2) { inputClosed = 1; break; }
            if (!switchPokemon(player, &playerActiveIndex, switchIndex)) {
                printf("\n그 포켓몬으로는 교체할 수 없습니다.\n");
                waitEnter();
                continue;
            }

            playerPokemon = &player->team[playerActiveIndex];
            enemyMove = chooseAIMove(*enemyPokemon, *playerPokemon);
            printf("\n%s의 %s!\n", enemyPokemon->pokemon.name, enemyMove.name);
            attackPokemon(enemyPokemon, playerPokemon, enemyMove);
            applyEndTurnStatusDamage(playerPokemon);
            applyEndTurnStatusDamage(enemyPokemon);
            printBattleStatus(*playerPokemon, *enemyPokemon);
            turnsTaken++;
            waitEnter();
            continue;
        }

        if (action != 1) {
            printf("\n잘못된 입력입니다.\n");
            waitEnter();
            continue;
        }

        int moveIndex = selectMove(*playerPokemon);
        if (moveIndex == -2) { inputClosed = 1; break; }
        if (moveIndex < 0 || moveIndex >= playerPokemon->moveCount) {
            printf("\n잘못된 입력입니다.\n");
            waitEnter();
            continue;
        }

        Move playerMove = playerPokemon->moves[moveIndex];
        Move enemyMove = chooseAIMove(*enemyPokemon, *playerPokemon);

        battleTurn(playerPokemon, playerMove, enemyPokemon, enemyMove);
        turnsTaken++;
        printBattleStatus(*playerPokemon, *enemyPokemon);
        waitEnter();
    }

    if (inputClosed) return -1;
    if (!hasUsablePokemon(*player)) return 0;

    if (turnsTakenOut) {
        *turnsTakenOut = turnsTaken;
    }

    clearScreen();
    printf("\n%s와의 승부에서 이겼다!\n", rival.name);
    waitEnter();
    return 1;
}

int main(void)
{
    int menu = 0;
    configureConsole();
    srand((unsigned int)time(NULL));
    atexit(stopBackgroundMusic);
    startTitleMusic();

    while (1) {
        showTitleScreen();
        if (scanf("%d", &menu) != 1) {
            flushInputLine();
            continue;
        }
        flushInputLine();
        if (menu == 2) { showHowToPlay(); continue; }
        if (menu == 3) { showOptionsScreen(); continue; }
        if (menu == 4) { stopBackgroundMusic(); showEndingScreen(0); return 0; }
        if (menu == 1) break;
    }

    stopBackgroundMusic();
    showOpeningStory();
    startBackgroundMusic();

    Trainer player = createRandomTrainer("레드", BATTLE_LEVEL);
    Trainer opponents[5];
    int opponentCount = 0;
    createEliteFourChallenge(opponents, &opponentCount);
    healTrainer(&player);

    clearScreen();
    printf("\n[레드의 포켓몬]\n");
    printTrainerTeam(player);
    waitEnter();

    int totalScore = 0;

    for (int i = 0; i < opponentCount; i++) {
        int turns = 0;
        int result = battleTrainer(&player, opponents[i], &turns);
        if (result <= 0) {
            char playerName[32] = "";
            clearScreen();
            printf("\n최종 점수: %d점\n", totalScore);
            promptPlayerName(playerName, sizeof(playerName));
            addScoreToLeaderboard(playerName, totalScore);
            showLeaderboard();
            waitEnter();
            showEndingScreen(0);
            return 0;
        }

        int remainingPokemon = countRemainingPokemon(&player);
        int battleScore = calculateScore(turns, remainingPokemon);
        totalScore += battleScore;

        clearScreen();
        printf("\n%s를 쓰러뜨렸다!\n", opponents[i].name);
        printf("이번 점수: %d점 (턴: %d, 남은 포켓몬: %d)\n", battleScore, turns, remainingPokemon);
        stopBackgroundMusic();
        startVictoryMusic();
        waitEnter();
        stopBackgroundMusic();

        if (i + 1 < opponentCount) {
            healTrainer(&player);
            clearScreen();
            printf("\n승리 후 포켓몬들이 회복되었다!\n");
            printf("포켓몬 리그 다음 방으로 이동했다...\n");
            waitEnter();
        }
    }

    char playerName[32] = "";
    clearScreen();
    stopBackgroundMusic();
    startHallOfFameMusic();
    printf("\n모든 배틀 종료!\n최종 점수: %d점\n", totalScore);
    promptPlayerName(playerName, sizeof(playerName));
    addScoreToLeaderboard(playerName, totalScore);
    showLeaderboard();
    waitEnter();
    stopBackgroundMusic();
    showEndingScreen(1);
    return 0;
}

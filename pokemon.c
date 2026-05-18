#ifndef _WIN32
#define _XOPEN_SOURCE 700
#define _POSIX_C_SOURCE 200809L
#endif
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <locale.h>
#include <stdarg.h>

#ifdef _WIN32
#include <windows.h>
#include <io.h>
#else
#include <signal.h>
#include <unistd.h>
#endif
#include <locale.h>
#include <wchar.h>

#include "pokemon.h"
#include "score/score.h"

#ifdef _WIN32
static PROCESS_INFORMATION bgmProcess;
#else
static int bgmPid = -1;
#endif

static float musicVolume = 1.0f;

static void stopBackgroundMusic(void);

#define SCREEN_WIDTH 136
#define SCREEN_HEIGHT 160
#define FRAME_INNER_WIDTH (SCREEN_WIDTH - 4)

static void makeBorder(char *out, size_t outSize)
{
    if (outSize == 0) return;
    int pos = 0;
    if (pos < (int)outSize - 1) out[pos++] = '+';
    for (int i = 0; i < SCREEN_WIDTH - 2 && pos < (int)outSize - 1; i++) out[pos++] = '=';
    if (pos < (int)outSize - 1) out[pos++] = '+';
    out[pos] = '\0';
}

static void printFrameBorder(void)
{
    char border[SCREEN_WIDTH + 2];
    makeBorder(border, sizeof(border));
    printf("%s\n", border);
}

static void trimNewline(char *text)
{
    size_t len = strlen(text);
    while (len > 0 && (text[len - 1] == '\n' || text[len - 1] == '\r')) {
        text[--len] = '\0';
    }
}

static int visualWidth(const char *s)
{
    mbstate_t state;
    memset(&state, 0, sizeof(state));

    int width = 0;
    wchar_t wc;
    size_t len;

    while (*s) {
        len = mbrtowc(&wc, s, MB_CUR_MAX, &state);

        if (len == (size_t)-1 || len == (size_t)-2) {
            width++;
            s++;
            memset(&state, 0, sizeof(state));
            continue;
        }

#ifdef _WIN32
        /* Windows/MSVC에는 POSIX wcwidth()가 없어서, 한글/일본어/중국어 계열 문자는
           콘솔에서 대체로 2칸으로 잡는 간단한 대체 함수를 사용합니다. */
        int w = ((wc >= 0x1100 && wc <= 0x11FF) ||
                 (wc >= 0x2E80 && wc <= 0xA4CF) ||
                 (wc >= 0xAC00 && wc <= 0xD7A3) ||
                 (wc >= 0xF900 && wc <= 0xFAFF) ||
                 (wc >= 0xFE10 && wc <= 0xFE6F) ||
                 (wc >= 0xFF00 && wc <= 0xFF60)) ? 2 : 1;
#else
        int w = wcwidth(wc);
#endif
        width += (w > 0) ? w : 1;
        s += len;
    }

    return width;
}

static void printFrameLineRaw(const char *text)
{
    if (text == NULL) text = "";

    int padding = FRAME_INNER_WIDTH - visualWidth(text);
    if (padding < 0) padding = 0;

    printf("| %s", text);

    for (int i = 0; i < padding; i++) {
        putchar(' ');
    }

    printf(" |\n");
}

static void printFrameLine(const char *fmt, ...)
{
    char line[512];
    va_list args;
    va_start(args, fmt);
    vsnprintf(line, sizeof(line), fmt, args);
    va_end(args);
    trimNewline(line);
    printFrameLineRaw(line);
}

static void drawFrameTop(const char *title)
{
    printFrameBorder();
    if (title && title[0]) {
        printFrameLine("%s", title);
        printFrameBorder();
    }
}

static void drawFrameBottom(void)
{
    printFrameBorder();
}

static void drawTextBox(const char *line1, const char *line2)
{
    printFrameBorder();
    printFrameLine("%s", line1 ? line1 : "");
    if (line2 && line2[0]) printFrameLine("%s", line2);
    printFrameBorder();
}

static void drawMenuBox(const char *a, const char *b, const char *c, const char *d)
{
    printFrameBorder();
    printFrameLine("선택");
    if (a) printFrameLine("  %s", a);
    if (b) printFrameLine("  %s", b);
    if (c) printFrameLine("  %s", c);
    if (d) printFrameLine("  %s", d);
    printFrameBorder();
}


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

        /* 포켓몬 ASCII 그림을 축소하지 않고 대각선 배치로 넣을 수 있도록
           콘솔 버퍼와 창 너비/높이를 게임 화면 기준에 맞게 넓힙니다. */
        {
            CONSOLE_SCREEN_BUFFER_INFO info;
            COORD bufferSize;
            SMALL_RECT windowRect;
            COORD maxWindow;

            if (GetConsoleScreenBufferInfo(output, &info)) {
                bufferSize.X = (SHORT)(SCREEN_WIDTH > info.dwSize.X ? SCREEN_WIDTH : info.dwSize.X);
                bufferSize.Y = (SHORT)(SCREEN_HEIGHT > info.dwSize.Y ? SCREEN_HEIGHT : info.dwSize.Y);

                /* 창보다 버퍼가 먼저 넓어져야 창 크기 변경이 실패하지 않습니다. */
                SetConsoleScreenBufferSize(output, bufferSize);

                maxWindow = GetLargestConsoleWindowSize(output);
                windowRect.Left = 0;
                windowRect.Top = 0;
                windowRect.Right = (SHORT)((SCREEN_WIDTH <= maxWindow.X ? SCREEN_WIDTH : maxWindow.X) - 1);
                windowRect.Bottom = (SHORT)((SCREEN_HEIGHT <= maxWindow.Y ? SCREEN_HEIGHT : maxWindow.Y) - 1);
                if (windowRect.Right >= windowRect.Left && windowRect.Bottom >= windowRect.Top) {
                    SetConsoleWindowInfo(output, TRUE, &windowRect);
                }

                bufferSize.X = (SHORT)(SCREEN_WIDTH > info.dwSize.X ? SCREEN_WIDTH : info.dwSize.X);
                bufferSize.Y = (SHORT)(SCREEN_HEIGHT > info.dwSize.Y ? SCREEN_HEIGHT : info.dwSize.Y);
                SetConsoleScreenBufferSize(output, bufferSize);
            }
        }
    }
#else
    /* 먼저 환경 기본 로케일을 적용한다. 단 개발컨테이너 등에서는 LANG 이 비어
       있어 C/POSIX 로케일이 잡히는데, 그러면 mbrtowc/wcwidth 가 UTF-8 을 못 읽어
       박스 테두리·한글 폭 계산이 깨진다. 그 경우 UTF-8 로케일을 명시적으로 강제. */
    setlocale(LC_ALL, "");
    if (MB_CUR_MAX <= 1) {
        if (setlocale(LC_ALL, "C.UTF-8") == NULL &&
            setlocale(LC_ALL, "C.utf8") == NULL &&
            setlocale(LC_ALL, "en_US.UTF-8") == NULL) {
            setlocale(LC_ALL, "");  /* UTF-8 로케일이 하나도 없으면 원래대로 폴백 */
        }
    }
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
             "powershell.exe -STA -NoProfile -ExecutionPolicy Bypass -Command \"Add-Type -AssemblyName PresentationCore; $player = New-Object System.Windows.Media.MediaPlayer; $player.Open([System.Uri]::new('%s')); $player.Volume = %.2f; Register-ObjectEvent -InputObject $player -EventName MediaEnded -Action { $Event.Sender.Position = [TimeSpan]::Zero; $Event.Sender.Play() } | Out-Null; $player.Play(); while ($true) { Start-Sleep -Seconds 1 }\"",
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

    stopBackgroundMusic();

    char command[1024];
    snprintf(command, sizeof(command), "while :; do afplay -v %.2f \"%s\"; done >/dev/null 2>&1 & echo $!", volume, path);
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

    char command[64];
    snprintf(command, sizeof(command), "pkill -TERM -P %d >/dev/null 2>&1", bgmPid);
    system(command);
    kill(bgmPid, SIGTERM);
    bgmPid = -1;
#endif
}

void printTrainerTeam(Trainer trainer)
{
    drawFrameTop("나의 포켓몬");
    for (int i = 0; i < trainer.teamSize; i++) {
        printf("|  %d. %-16s Lv.%-3d HP %4d/%-4d 상태:%-8s |\n",
               i + 1,
               trainer.team[i].pokemon.name,
               trainer.team[i].level,
               trainer.team[i].currentHp,
               trainer.team[i].pokemon.hp,
               getStatusName(trainer.team[i].status));
    }
    drawFrameBottom();
}



void clearScreen(void)
{
#ifdef _WIN32
    HANDLE output = GetStdHandle(STD_OUTPUT_HANDLE);
    if (output != INVALID_HANDLE_VALUE) {
        CONSOLE_SCREEN_BUFFER_INFO info;
        if (GetConsoleScreenBufferInfo(output, &info)) {
            DWORD written = 0;
            COORD home = {0, 0};
            DWORD cellCount = (DWORD)info.dwSize.X * (DWORD)info.dwSize.Y;

            /* Windows 콘솔의 전체 화면 버퍼를 전부 공백으로 밀어 지웁니다.
               이전 화면이 반쯤 남는 현상을 막기 위해 화면 일부가 아니라
               현재 버퍼 전체를 지운 뒤 커서를 반드시 좌상단으로 되돌립니다. */
            FillConsoleOutputCharacterA(output, ' ', cellCount, home, &written);
            FillConsoleOutputAttribute(output, info.wAttributes, cellCount, home, &written);
            SetConsoleCursorPosition(output, home);
        }
    }

    /* Windows Terminal/일부 콘솔 호스트에서는 API 삭제 후에도 스크롤백이나
       잔상이 남는 경우가 있어 ANSI 삭제까지 한 번 더 수행합니다. */
    printf("\x1b[2J\x1b[3J\x1b[H");
    fflush(stdout);
    return;
#else
    printf("\033[2J\033[3J\033[H");
    fflush(stdout);
#endif
}

static int readIntAndClearScreen(int *value)
{
    int ok = scanf("%d", value);
    flushInputLine();
    clearScreen();
    return ok == 1;
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
    fflush(stdout);
    while ((c = getchar()) != '\n' && c != EOF) {}
    clearScreen();
}

void waitEnter(void)
{
    printf("\n▼ 엔터를 누르세요...");
    fflush(stdout);
    flushInputLine();
    clearScreen();
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

static int utf8CharLen(unsigned char c)
{
    if (c < 0x80) return 1;
    if ((c & 0xE0) == 0xC0) return 2;
    if ((c & 0xF0) == 0xE0) return 3;
    if ((c & 0xF8) == 0xF0) return 4;
    return 1;
}

static void printArtLine(const char *src)
{
    char cleaned[1024];
    const char *glyph[512];
    int glyphLen[512];
    int glyphCount = 0;
    int byteLen;

    if (src == NULL) src = "";
    snprintf(cleaned, sizeof(cleaned), "%s", src);
    trimNewline(cleaned);
    byteLen = (int)strlen(cleaned);

    for (int i = 0; i < byteLen && glyphCount < 512; ) {
        int len = utf8CharLen((unsigned char)cleaned[i]);
        if (i + len > byteLen) break;
        glyph[glyphCount] = &cleaned[i];
        glyphLen[glyphCount] = len;
        glyphCount++;
        i += len;
    }

    printf("| ");
    if (glyphCount <= FRAME_INNER_WIDTH) {
        for (int i = 0; i < glyphCount; i++) fwrite(glyph[i], 1, glyphLen[i], stdout);
        for (int i = glyphCount; i < FRAME_INNER_WIDTH; i++) putchar(' ');
    } else {
        for (int i = 0; i < FRAME_INNER_WIDTH; i++) {
            int idx = (i * glyphCount) / FRAME_INNER_WIDTH;
            fwrite(glyph[idx], 1, glyphLen[idx], stdout);
        }
    }
    printf(" |\n");
}


static void printSpriteFileLine(const char *line)
{
    const char *first = strchr(line, '"');
    const char *last = strrchr(line, '"');
    char out[1024];
    int pos = 0;

    if (first == NULL || last == NULL || first == last) return;
    for (const char *p = first + 1; p < last && pos < (int)sizeof(out) - 1; p++) {
        if (*p == '\\' && p + 1 < last) {
            p++;
            if (*p == 'n') break;
            else if (*p == 't') out[pos++] = ' ';
            else out[pos++] = *p;
        } else {
            out[pos++] = *p;
        }
    }
    out[pos] = '\0';
    printArtLine(out);
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
        printArtLine(line);
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

    drawFrameTop("POKEMON");
    printFrameLine("        [No.%03d %s]", dexNumber, name);
    drawFrameBottom();
}

void showPokemonArtByName(const char *name)
{
    showPokemonArtByDexNumber(getRealDexNumber(name), name);
}

void showPokemonArt(Pokemon pokemon)
{
    showPokemonArtByDexNumber(getPokemonDexNumber(pokemon), pokemon.name);
}

static void showDefaultPokemonArt(void)
{
    showPokemonArtByName("피카츄");
}


#define BATTLE_LEFT_WIDTH 64
#define BATTLE_RIGHT_WIDTH 64
#define BATTLE_GAP_WIDTH 4
#define BATTLE_ART_DRAW_WIDTH 64
#define BATTLE_ART_LINES 64
#define BATTLE_MAX_ART_LINES 64
#define BATTLE_RAW_LINE_SIZE 1024

static int isBattleArtLineVisible(const char *line);
static void normalizeBattleArtMargins(char lines[BATTLE_MAX_ART_LINES][BATTLE_RAW_LINE_SIZE], int count);

static int splitGlyphs(const char *src, const char **glyph, int *glyphLen, int maxGlyphs)
{
    char dummy[BATTLE_RAW_LINE_SIZE];
    int byteLen;
    int glyphCount = 0;

    if (src == NULL) src = "";
    snprintf(dummy, sizeof(dummy), "%s", src);
    trimNewline(dummy);
    byteLen = (int)strlen(dummy);

    for (int i = 0; i < byteLen && glyphCount < maxGlyphs; ) {
        int len = utf8CharLen((unsigned char)dummy[i]);
        if (i + len > byteLen) break;
        glyph[glyphCount] = src + i;
        glyphLen[glyphCount] = len;
        glyphCount++;
        i += len;
    }

    return glyphCount;
}

static void copyIncSpriteLineToBuffer(const char *line, char *out, size_t outSize)
{
    const char *first = strchr(line, '"');
    const char *last = strrchr(line, '"');
    int pos = 0;

    if (outSize == 0) return;
    out[0] = '\0';
    if (first == NULL || last == NULL || first == last) return;

    for (const char *p = first + 1; p < last && pos < (int)outSize - 1; p++) {
        if (*p == '\\' && p + 1 < last) {
            p++;
            if (*p == 'n') break;
            else if (*p == 't') out[pos++] = ' ';
            else out[pos++] = *p;
        } else {
            out[pos++] = *p;
        }
    }
    out[pos] = '\0';
    trimNewline(out);
}

static int loadBattleArtByDexNumber(int dexNumber, char lines[BATTLE_MAX_ART_LINES][BATTLE_RAW_LINE_SIZE])
{
    const char *formats[] = {
        "tools/ascii_converter/output/%03d.txt",
        "tools/ascii_converter/output/bin/%03d.txt",
        "tools/ascii_converter/generated/%03d.inc",
        "tools/ascii_converter/generated/bin/%03d.inc"
    };
    char relativePath[256];
    char resolvedPath[512];
    char raw[BATTLE_RAW_LINE_SIZE];
    int hasConflict;
    int inSelectedSide;
    int count;

    for (int f = 0; f < (int)(sizeof(formats) / sizeof(formats[0])); f++) {
        FILE *fp;
        snprintf(relativePath, sizeof(relativePath), formats[f], dexNumber);
        fp = openResourceFile(relativePath, resolvedPath, sizeof(resolvedPath));
        if (fp == NULL) continue;

        hasConflict = 0;
        inSelectedSide = 0;
        count = 0;
        while (fgets(raw, sizeof(raw), fp) && count < BATTLE_MAX_ART_LINES) {
            char cleaned[BATTLE_RAW_LINE_SIZE];
            if (!shouldPrintConflictLine(raw, &hasConflict, &inSelectedSide)) {
                continue;
            }
            if (strstr(relativePath, ".inc") != NULL) {
                copyIncSpriteLineToBuffer(raw, cleaned, sizeof(cleaned));
            } else {
                snprintf(cleaned, sizeof(cleaned), "%s", raw);
                trimNewline(cleaned);
            }
            if (cleaned[0] == '\0' && count == 0) {
                continue;
            }
            snprintf(lines[count], BATTLE_RAW_LINE_SIZE, "%s", cleaned);
            count++;
        }
        fclose(fp);
        if (count > 0) {
            normalizeBattleArtMargins(lines, count);
            return count;
        }
    }

    snprintf(lines[0], BATTLE_RAW_LINE_SIZE, "[No.%03d]", dexNumber);
    return 1;
}

static int loadBattleArt(Pokemon pokemon, char lines[BATTLE_MAX_ART_LINES][BATTLE_RAW_LINE_SIZE])
{
    return loadBattleArtByDexNumber(getPokemonDexNumber(pokemon), lines);
}

static int isBattleArtLineVisible(const char *line)
{
    if (line == NULL) return 0;
    for (const char *p = line; *p; p++) {
        if (*p != ' ' && *p != '\t' && *p != '\r' && *p != '\n') {
            return 1;
        }
    }
    return 0;
}


static void rtrimBattleArtLine(char *line)
{
    int len;
    if (line == NULL) return;
    len = (int)strlen(line);
    while (len > 0 && (line[len - 1] == ' ' || line[len - 1] == '\t' || line[len - 1] == '\r' || line[len - 1] == '\n')) {
        line[--len] = '\0';
    }
}

static int countLeadingAsciiSpaces(const char *line)
{
    int count = 0;
    if (line == NULL) return 0;
    while (line[count] == ' ') count++;
    return count;
}

static void normalizeBattleArtMargins(char lines[BATTLE_MAX_ART_LINES][BATTLE_RAW_LINE_SIZE], int count)
{
    int minLeft = BATTLE_RAW_LINE_SIZE;
    int found = 0;

    for (int i = 0; i < count; i++) {
        rtrimBattleArtLine(lines[i]);
        if (isBattleArtLineVisible(lines[i])) {
            int left = countLeadingAsciiSpaces(lines[i]);
            if (left < minLeft) minLeft = left;
            found = 1;
        }
    }

    if (!found || minLeft <= 0 || minLeft >= BATTLE_RAW_LINE_SIZE) return;

    for (int i = 0; i < count; i++) {
        if ((int)strlen(lines[i]) > minLeft) {
            memmove(lines[i], lines[i] + minLeft, strlen(lines[i] + minLeft) + 1);
        }
    }
}

static int findFirstVisibleArtLine(char lines[BATTLE_MAX_ART_LINES][BATTLE_RAW_LINE_SIZE], int count)
{
    for (int i = 0; i < count; i++) {
        if (isBattleArtLineVisible(lines[i])) {
            return i;
        }
    }
    return 0;
}

static int findLastVisibleArtLine(char lines[BATTLE_MAX_ART_LINES][BATTLE_RAW_LINE_SIZE], int count)
{
    for (int i = count - 1; i >= 0; i--) {
        if (isBattleArtLineVisible(lines[i])) {
            return i;
        }
    }
    return 0;
}

static int getVisibleBattleArtLineCount(int visibleStart, int visibleEnd)
{
    if (visibleStart < 0 || visibleEnd < visibleStart) return 0;
    return visibleEnd - visibleStart + 1;
}

static const char *getOriginalBattleArtLine(char lines[BATTLE_MAX_ART_LINES][BATTLE_RAW_LINE_SIZE], int count, int visibleStart, int visibleEnd, int outputLine)
{
    int sourceIndex;

    if (count <= 0) return "";
    if (visibleStart < 0) visibleStart = 0;
    if (visibleEnd < visibleStart) return "";

    sourceIndex = visibleStart + outputLine;
    if (sourceIndex > visibleEnd || sourceIndex >= count) return "";
    return lines[sourceIndex];
}

static void makeBattleHpBarText(char *out, size_t outSize, int current, int max)
{
    int barSize = 16;
    int filled;
    int pos = 0;
    if (outSize == 0) return;
    if (max <= 0) max = 1;
    if (current < 0) current = 0;
    if (current > max) current = max;
    filled = (current * barSize) / max;

    if (pos < (int)outSize - 1) out[pos++] = '[';
    for (int i = 0; i < barSize && pos < (int)outSize - 1; i++) {
        out[pos++] = (i < filled ? '#' : ' ');
    }
    if (pos < (int)outSize - 1) out[pos++] = ']';
    out[pos] = '\0';
}

static int glyphVisualWidth(const char *p, int len)
{
    char buf[8];
    if (p == NULL || len <= 0) return 0;
    if (len >= (int)sizeof(buf)) len = (int)sizeof(buf) - 1;
    memcpy(buf, p, len);
    buf[len] = '\0';
    return visualWidth(buf);
}

static void writeBattleField(const char *text, int width)
{
    const char *glyph[1024];
    int glyphLen[1024];
    int glyphCount;
    int textWidth;

    if (width <= 0) return;
    if (text == NULL) text = "";

    textWidth = visualWidth(text);
    glyphCount = splitGlyphs(text, glyph, glyphLen, 1024);

    if (textWidth <= width) {
        fwrite(text, 1, strlen(text), stdout);
        for (int i = textWidth; i < width; i++) putchar(' ');
        return;
    }

    int used = 0;
    int previousIdx = -1;
    for (int i = 0; i < width && used < width; i++) {
        int idx = (i * glyphCount) / width;
        int gw;
        if (idx < 0) idx = 0;
        if (idx >= glyphCount) idx = glyphCount - 1;
        if (idx == previousIdx) continue;
        gw = glyphVisualWidth(glyph[idx], glyphLen[idx]);
        if (gw <= 0) gw = 1;
        if (used + gw > width) break;
        fwrite(glyph[idx], 1, glyphLen[idx], stdout);
        used += gw;
        previousIdx = idx;
    }
    for (int i = used; i < width; i++) putchar(' ');
}

static void writeBattleArtField(const char *text, int fieldWidth)
{
    int artWidth = BATTLE_ART_DRAW_WIDTH;
    int leftPad;
    int rightPad;

    if (artWidth > fieldWidth) artWidth = fieldWidth;
    leftPad = (fieldWidth - artWidth) / 2;
    rightPad = fieldWidth - artWidth - leftPad;

    for (int i = 0; i < leftPad; i++) putchar(' ');
    writeBattleField(text, artWidth);
    for (int i = 0; i < rightPad; i++) putchar(' ');
}

static void printBattleLayoutLine(const char *left, const char *right)
{
    printf("| ");
    writeBattleField(left ? left : "", BATTLE_LEFT_WIDTH);
    for (int i = 0; i < BATTLE_GAP_WIDTH; i++) putchar(' ');
    writeBattleField(right ? right : "", BATTLE_RIGHT_WIDTH);
    printf(" |\n");
}

static void printBattleLayoutLineEnemyArt(const char *leftInfo, const char *rightArt)
{
    printf("| ");
    writeBattleField(leftInfo ? leftInfo : "", BATTLE_LEFT_WIDTH);
    for (int i = 0; i < BATTLE_GAP_WIDTH; i++) putchar(' ');
    writeBattleArtField(rightArt ? rightArt : "", BATTLE_RIGHT_WIDTH);
    printf(" |\n");
}

static void printBattleLayoutLinePlayerArt(const char *leftArt, const char *rightInfo)
{
    printf("| ");
    writeBattleArtField(leftArt ? leftArt : "", BATTLE_LEFT_WIDTH);
    for (int i = 0; i < BATTLE_GAP_WIDTH; i++) putchar(' ');
    writeBattleField(rightInfo ? rightInfo : "", BATTLE_RIGHT_WIDTH);
    printf(" |\n");
}

static void makeEnemyInfoLine(char *out, size_t outSize, BattlePokemon enemy, int lineIndex)
{
    char hpbar[32];
    makeBattleHpBarText(hpbar, sizeof(hpbar), enemy.currentHp, enemy.pokemon.hp);

    switch (lineIndex) {
        case 1:
            snprintf(out, outSize, "  상대 %s Lv.%d", enemy.pokemon.name, enemy.level);
            break;
        case 2:
            snprintf(out, outSize, "  HP %s", hpbar);
            break;
        case 3:
            snprintf(out, outSize, "     %4d / %-4d", enemy.currentHp, enemy.pokemon.hp);
            break;
        default:
            snprintf(out, outSize, "");
            break;
    }
}

static void makePlayerInfoLine(char *out, size_t outSize, BattlePokemon player, int lineIndex)
{
    char hpbar[32];
    makeBattleHpBarText(hpbar, sizeof(hpbar), player.currentHp, player.pokemon.hp);

    switch (lineIndex) {
        case 8:
            snprintf(out, outSize, "  %s Lv.%d", player.pokemon.name, player.level);
            break;
        case 9:
            snprintf(out, outSize, "  HP %s", hpbar);
            break;
        case 10:
            snprintf(out, outSize, "     %4d / %-4d", player.currentHp, player.pokemon.hp);
            break;
        case 11:
            snprintf(out, outSize, "  상태:%s", getStatusName(player.status));
            break;
        default:
            snprintf(out, outSize, "");
            break;
    }
}


void showTitleScreen(void)
{
    clearScreen();
    printf("\n");
    drawFrameTop("POKEMON RED / GREEN  -  C CONSOLE EDITION");
    printFrameLine("");
    printFrameLine("                 P O K E M O N   L E A G U E");
    printFrameLine("");
    drawFrameBottom();
    showPokemonArtByName("피카츄");
    printf("\n");
    drawTextBox("오박사: 포켓몬 리그에 도전할 준비는 되었나?", "번호를 입력해서 메뉴를 고르세요.");
    drawMenuBox("1. 처음부터 시작", "2. 조작법 보기", "3. 옵션", "4. 종료");
    printf("▶ 입력: ");
}


void showHowToPlay(void)
{
    clearScreen();
    drawFrameTop("조작법");
    printFrameLine("  1을 입력하면 싸운다, 2를 입력하면 포켓몬을 교체합니다.");
    printFrameLine("  기술 선택 화면에서는 사용할 기술 번호를 입력합니다.");
    printFrameLine("  HP가 0이 된 포켓몬은 전투에 나설 수 없습니다.");
    printFrameLine("  사천왕과 챔피언을 모두 이기면 전당등록입니다.");
    drawFrameBottom();
    showDefaultPokemonArt();
    drawTextBox("오박사: 실제 배틀처럼 아래 메뉴를 보고 행동을 고르렴!", "");
    waitEnter();
}


void showOptionsScreen(void)
{
    int choice;
    while (1) {
        int volumePercent = (int)(musicVolume * 100.0f + 0.5f);
        clearScreen();
        drawFrameTop("옵션");
        {
            char volumeLine[128];
            char bar[21];
            for (int i = 0; i < 20; i++) bar[i] = (i < volumePercent / 5 ? '#' : ' ');
            bar[20] = '\0';
            snprintf(volumeLine, sizeof(volumeLine), "  배경음악 볼륨  %3d%%  [%s]", volumePercent, bar);
            printFrameLine("%s", volumeLine);
        }
        drawFrameBottom();
        showDefaultPokemonArt();
        drawMenuBox("1. 배경음악 볼륨 설정", "2. 이전 화면으로 돌아가기", NULL, NULL);
        printf("▶ 입력: ");

        if (!readIntAndClearScreen(&choice)) {
            continue;
        }

        if (choice == 1) {
            int newVolume;
            showDefaultPokemonArt();
            drawTextBox("볼륨 값을 0부터 100 사이로 입력하세요.", "");
            printf("▶ 입력: ");
            if (!readIntAndClearScreen(&newVolume)) {
                continue;
            }
            if (newVolume < 0) newVolume = 0;
            if (newVolume > 100) newVolume = 100;
            musicVolume = newVolume / 100.0f;
            stopBackgroundMusic();
            startTitleMusic();
            showDefaultPokemonArt();
            drawTextBox("설정이 저장되었습니다.", "");
            waitEnter();
        } else if (choice == 2) {
            break;
        }
    }
}


void showOpeningStory(void)
{
    clearScreen();
    drawFrameTop("오프닝");
    printFrameLine("");
    printFrameLine("                 포켓몬 리그의 문이 열렸다!");
    printFrameLine("");
    drawFrameBottom();
    showPokemonArtByName("피카츄");
    drawTextBox("오박사: 자! 이제부터 너의 이야기가 시작된다!", "강한 트레이너들과 싸워 챔피언이 되어라!");
    waitEnter();
}


void showEndingScreen(int won)
{
    clearScreen();
    if (won) {
        drawFrameTop("HALL OF FAME");
        printFrameLine("               전당등록! 축하합니다!");
        printFrameLine("               당신은 포켓몬 리그 챔피언입니다.");
        drawFrameBottom();
        showPokemonArtByName("뮤");
    } else {
        drawFrameTop("GAME OVER");
        printFrameLine("               눈앞이 깜깜해졌다...");
        printFrameLine("               포켓몬센터에서 다시 도전하세요.");
        drawFrameBottom();
        showPokemonArtByName("잠만보");
    }
}



void renderHpBar(int current, int max)
{
    int barSize = 18;
    int filled;
    if (max <= 0) max = 1;
    if (current < 0) current = 0;
    filled = (current * barSize) / max;
    printf("[");
    for (int i = 0; i < barSize; i++) printf(i < filled ? "#" : " ");
    printf("]");
}


static void makeHpBarText(char *out, size_t outSize, int current, int max)
{
    int barSize = 18;
    int filled;
    int pos = 0;
    if (max <= 0) max = 1;
    if (current < 0) current = 0;
    filled = (current * barSize) / max;
    if (pos < (int)outSize - 1) out[pos++] = '[';
    for (int i = 0; i < barSize && pos < (int)outSize - 1; i++) out[pos++] = (i < filled ? '#' : ' ');
    if (pos < (int)outSize - 1) out[pos++] = ']';
    out[pos] = '\0';
}

void renderBattleScreen(BattlePokemon player, BattlePokemon enemy)
{
    char enemyArt[BATTLE_MAX_ART_LINES][BATTLE_RAW_LINE_SIZE];
    char playerArt[BATTLE_MAX_ART_LINES][BATTLE_RAW_LINE_SIZE];
    int enemyCount = loadBattleArt(enemy.pokemon, enemyArt);
    int playerCount = loadBattleArt(player.pokemon, playerArt);
    int enemyStart = findFirstVisibleArtLine(enemyArt, enemyCount);
    int enemyEnd = findLastVisibleArtLine(enemyArt, enemyCount);
    int playerStart = findFirstVisibleArtLine(playerArt, playerCount);
    int playerEnd = findLastVisibleArtLine(playerArt, playerCount);

    clearScreen();
    drawFrameTop("BATTLE");

    int enemyVisibleLines = getVisibleBattleArtLineCount(enemyStart, enemyEnd);
    int playerVisibleLines = getVisibleBattleArtLineCount(playerStart, playerEnd);
    if (enemyVisibleLines > BATTLE_ART_LINES) enemyVisibleLines = BATTLE_ART_LINES;
    if (playerVisibleLines > BATTLE_ART_LINES) playerVisibleLines = BATTLE_ART_LINES;

    for (int i = 0; i < enemyVisibleLines; i++) {
        char info[BATTLE_RAW_LINE_SIZE];
        const char *art;
        makeEnemyInfoLine(info, sizeof(info), enemy, i);
        art = getOriginalBattleArtLine(enemyArt, enemyCount, enemyStart, enemyEnd, i);
        printBattleLayoutLineEnemyArt(info, art);
    }

    printBattleLayoutLine("", "                         VS");

    for (int i = 0; i < playerVisibleLines; i++) {
        char info[BATTLE_RAW_LINE_SIZE];
        const char *art;
        makePlayerInfoLine(info, sizeof(info), player, i);
        art = getOriginalBattleArtLine(playerArt, playerCount, playerStart, playerEnd, i);
        printBattleLayoutLinePlayerArt(art, info);
    }

    drawFrameBottom();
}


int selectMove(BattlePokemon pokemon)
{
    int choice;

    drawTextBox("무엇을 사용할까?", "기술 번호를 입력하세요.");
    printFrameBorder();
    printFrameLine("기술");
    for (int i = 0; i < pokemon.moveCount; i++) {
        char powerText[24];
        if (pokemon.moves[i].power > 0) snprintf(powerText, sizeof(powerText), "위력:%3d", pokemon.moves[i].power);
        else snprintf(powerText, sizeof(powerText), "변화기술");
        printFrameLine("  %d. %s  %s  명중:%3d%%",
               i + 1,
               pokemon.moves[i].name,
               powerText,
               getMoveAccuracy(pokemon.moves[i]));
    }
    printFrameBorder();
    printf("▶ 입력: ");
    if (!readIntAndClearScreen(&choice)) {
        return -2;
    }

    return choice - 1;
}


int selectBattleAction(void)
{
    int choice;

    drawTextBox("무엇을 할까?", "");
    drawMenuBox("1. 싸운다", "2. 포켓몬", NULL, NULL);
    printf("▶ 입력: ");
    if (!readIntAndClearScreen(&choice)) {
        return -2;
    }

    return choice;
}


void printSwitchOptions(Trainer trainer, int activeIndex)
{
    if (activeIndex >= 0 && activeIndex < trainer.teamSize) {
        showPokemonArt(trainer.team[activeIndex].pokemon);
    } else {
        showDefaultPokemonArt();
    }
    drawTextBox("어느 포켓몬으로 교체할까?", "");
    printFrameBorder();
    printFrameLine("포켓몬 선택");
    for (int i = 0; i < trainer.teamSize; i++) {
        const char *mark = "";
        if (i == activeIndex) mark = "현재";
        else if (isFainted(trainer.team[i])) mark = "기절";
        printFrameLine("  %d. %s HP %4d/%-4d %s",
               i + 1,
               trainer.team[i].pokemon.name,
               trainer.team[i].currentHp,
               trainer.team[i].pokemon.hp,
               mark);
    }
    printFrameBorder();
}


int selectSwitchPokemon(Trainer trainer, int activeIndex)
{
    int choice;

    printSwitchOptions(trainer, activeIndex);
    printf("\n입력: ");
    if (!readIntAndClearScreen(&choice)) {
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
        showPokemonArt(player->team[*activeIndex].pokemon);
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

        showPokemonArt(player->team[*activeIndex].pokemon);
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

    showPokemonArt(rival->team[*activeIndex].pokemon);
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
    showDefaultPokemonArt();
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
    clearScreen();
}

static int battleTrainer(Trainer *player, Trainer rival, int *turnsTakenOut)
{
    int playerActiveIndex = getFirstUsablePokemonIndex(*player);
    int enemyActiveIndex = getFirstUsablePokemonIndex(rival);
    int inputClosed = 0;
    int turnsTaken = 0;

    startBattleMusic();
    clearScreen();
    showPokemonArt(rival.team[enemyActiveIndex].pokemon);
    drawTextBox("                         VS", "");
    showPokemonArt(player->team[playerActiveIndex].pokemon);
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
            if (!canSwitchToPokemon(*player, playerActiveIndex, switchIndex)) {
                renderBattleScreen(*playerPokemon, *enemyPokemon);
                printf("\n그 포켓몬으로는 교체할 수 없습니다.\n");
                waitEnter();
                continue;
            }

            switchPokemon(player, &playerActiveIndex, switchIndex);
            playerPokemon = &player->team[playerActiveIndex];
            renderBattleScreen(*playerPokemon, *enemyPokemon);
            printf("\n%s는 %s을(를) 내보냈다!\n", player->name, playerPokemon->pokemon.name);
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
            renderBattleScreen(*playerPokemon, *enemyPokemon);
            printf("\n잘못된 입력입니다.\n");
            waitEnter();
            continue;
        }

        int moveIndex = selectMove(*playerPokemon);
        if (moveIndex == -2) { inputClosed = 1; break; }
        if (moveIndex < 0 || moveIndex >= playerPokemon->moveCount) {
            renderBattleScreen(*playerPokemon, *enemyPokemon);
            printf("\n잘못된 입력입니다.\n");
            waitEnter();
            continue;
        }

        Move playerMove = playerPokemon->moves[moveIndex];
        Move enemyMove = chooseAIMove(*enemyPokemon, *playerPokemon);

        renderBattleScreen(*playerPokemon, *enemyPokemon);
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
    showPokemonArt(player->team[playerActiveIndex].pokemon);
    printf("\n%s와의 승부에서 이겼다!\n", rival.name);
    waitEnter();
    return 1;
}

int main(void)
{
    setlocale(LC_ALL, "");
    int menu = 0;
    configureConsole();
    srand((unsigned int)time(NULL));
    atexit(stopBackgroundMusic);
    startTitleMusic();

    while (1) {
        showTitleScreen();
        if (!readIntAndClearScreen(&menu)) {
            continue;
        }
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
    showPokemonArt(player.team[0].pokemon);
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
            showDefaultPokemonArt();
            printf("\n최종 점수: %d점\n", totalScore);
            promptPlayerName(playerName, sizeof(playerName));
            showDefaultPokemonArt();
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
        showPokemonArt(player.team[0].pokemon);
        printf("\n%s를 쓰러뜨렸다!\n", opponents[i].name);
        printf("이번 점수: %d점 (턴: %d, 남은 포켓몬: %d)\n", battleScore, turns, remainingPokemon);
        stopBackgroundMusic();
        startVictoryMusic();
        waitEnter();
        stopBackgroundMusic();

        if (i + 1 < opponentCount) {
            healTrainer(&player);
            clearScreen();
            showDefaultPokemonArt();
            printf("\n승리 후 포켓몬들이 회복되었다!\n");
            printf("포켓몬 리그 다음 방으로 이동했다...\n");
            waitEnter();
        }
    }

    char playerName[32] = "";
    clearScreen();
    stopBackgroundMusic();
    startHallOfFameMusic();
    showPokemonArtByName("뮤");
    printf("\n모든 배틀 종료!\n최종 점수: %d점\n", totalScore);
    promptPlayerName(playerName, sizeof(playerName));
    showPokemonArtByName("뮤");
    addScoreToLeaderboard(playerName, totalScore);
    showLeaderboard();
    waitEnter();
    stopBackgroundMusic();
    showEndingScreen(1);
    return 0;
}

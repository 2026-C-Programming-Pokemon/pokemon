#ifndef POKEMON_H
#define POKEMON_H

#define POKEMON_COUNT 81
#define TEAM_SIZE 6
#define MOVE_SLOT_COUNT 4
#define BATTLE_LEVEL 60
#define DEFAULT_INDIVIDUAL_VALUE 31

/* 포켓몬 타입을 숫자 상수로 다루기 위한 열거형입니다. */
typedef enum {
    TYPE_NORMAL,
    TYPE_FIRE,
    TYPE_WATER,
    TYPE_ELECTRIC,
    TYPE_GRASS,
    TYPE_ICE,
    TYPE_FIGHTING,
    TYPE_POISON,
    TYPE_GROUND,
    TYPE_FLYING,
    TYPE_PSYCHIC,
    TYPE_BUG,
    TYPE_ROCK,
    TYPE_GHOST,
    TYPE_DRAGON,
    TYPE_DARK,
    TYPE_STEEL,
    TYPE_FAIRY,
    TYPE_COUNT,
    TYPE_NONE
} PokemonType;

/* 도감에 저장되는 기본 포켓몬 정보입니다. */
typedef struct {
    int id;
    char name[32];
    PokemonType type1;
    PokemonType type2;
    int hp;
    int atk;
    int def;
    int spa;
    int spd;
    int agi;
} Pokemon;

/* 물리 기술인지 특수 기술인지 구분합니다. */
typedef enum {
    MOVE_PHYSICAL,
    MOVE_SPECIAL
} MoveCategory;

/* 랭크업/디버프 기술이 바꿀 수 있는 능력치 종류입니다. */
typedef enum {
    STAT_NONE,
    STAT_ATK,
    STAT_DEF,
    STAT_SPA,
    STAT_SPD,
    STAT_AGI,
    STAT_ACC,
    STAT_EVA
} StatType;

/* 전투 중 포켓몬에게 걸릴 수 있는 상태이상입니다. */
typedef enum {
    STATUS_NONE,
    STATUS_SLEEP,
    STATUS_PARALYSIS,
    STATUS_POISON,
    STATUS_BURN,
    STATUS_FREEZE
} StatusCondition;

/* 기술 하나의 타입, 분류, 위력, 능력치 변화 정보를 저장합니다. */
typedef struct {
    char name[32];
    PokemonType type;
    MoveCategory category;
    int power;
    StatType stat;
    int statChange;
} Move;

/* 전투 중 변하는 현재 HP, 기술, 랭크, 상태이상을 포함한 포켓몬 정보입니다. */
typedef struct {
    Pokemon pokemon;
    int level;
    int currentHp;
    Move moves[MOVE_SLOT_COUNT];
    int moveCount;
    int atkStage;
    int defStage;
    int spaStage;
    int spdStage;
    int agiStage;
    int accStage;
    int evaStage;
    StatusCondition status;
    int sleepTurns;
    int transformed;
    int rechargeTurn;
    int chargingTurn;
    int semiInvulnerable;
    Move chargingMove;
    int substituteHp;
} BattlePokemon;

/* 플레이어, 사천왕, 챔피언처럼 포켓몬 팀을 가진 대상을 표현합니다. */
typedef struct {
    char name[32];
    BattlePokemon team[TEAM_SIZE];
    int teamSize;
} Trainer;

extern const char *typeNames[];
extern float typeChart[TYPE_COUNT][TYPE_COUNT];
extern Pokemon pokedex[POKEMON_COUNT];

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

void printPokemon(Pokemon pokemon);
float getTypeEffectiveness(PokemonType attackType, PokemonType defenderType1, PokemonType defenderType2);
int calculateFinalHp(int baseHp, int individualValue, int level);
int calculateFinalNonHpStat(int baseStat, int individualValue, int level);
Pokemon calculateFinalPokemonStats(Pokemon basePokemon, int level, int individualValue);
BattlePokemon createBattlePokemon(Pokemon pokemon, int level);

void clearMoves(BattlePokemon *pokemon);
void addMove(BattlePokemon *pokemon, Move move);
void shuffleIntArray(int array[], int size);
void setRandomLevelUpMoves(BattlePokemon *pokemon);
void setFixedMoves(BattlePokemon *pokemon, Move moves[], int moveCount);
Move getFallbackMove(void);
int getMoveAccuracy(Move move);

Trainer createRandomTrainer(char name[], int level);
BattlePokemon createBattlePokemonById(int pokemonId, int level);
Trainer createEmptyTrainer(char name[]);
void addPokemonToTrainer(Trainer *trainer, int pokemonId, int level);
void addPokemonToTrainerWithMoves(Trainer *trainer, int pokemonId, int level, Move moves[], int moveCount);
Trainer createLorelei(void);
Trainer createBruno(void);
Trainer createAgatha(void);
Trainer createLance(void);
Trainer createChampion(void);
void createEliteFourChallenge(Trainer opponents[], int *opponentCount);

int isFainted(BattlePokemon pokemon);
const char *getStatusName(StatusCondition status);
int hasUsablePokemon(Trainer trainer);
int getFirstUsablePokemonIndex(Trainer trainer);
void resetBattleStages(BattlePokemon *pokemon);
int canSwitchToPokemon(Trainer trainer, int activeIndex, int newIndex);
int switchPokemon(Trainer *trainer, int *activeIndex, int newIndex);
float getSameTypeAttackBonus(Pokemon attacker, PokemonType moveType);
float getStageMultiplier(int stage);
int getBattleStat(BattlePokemon pokemon, StatType stat);
void changeStatStage(BattlePokemon *pokemon, StatType stat, int change);
int calculateDamage(BattlePokemon attacker, BattlePokemon defender, Move move);
void printEffectiveness(float effectiveness);
void takeDamage(BattlePokemon *defender, int damage);
int checkMoveHit(BattlePokemon attacker, BattlePokemon defender, Move move);
void setStatusCondition(BattlePokemon *pokemon, StatusCondition status);
StatusCondition getMoveStatus(Move move);
int getMoveStatusChance(Move move);
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../pokemon.h"

int isFainted(BattlePokemon pokemon)
{
    return pokemon.currentHp <= 0;
}

/* 상태이상 enum 값을 출력용 한글 문자열로 바꿉니다. */
const char *getStatusName(StatusCondition status)
{
    if (status == STATUS_SLEEP) {
        return "수면";
    } else if (status == STATUS_PARALYSIS) {
        return "마비";
    } else if (status == STATUS_POISON) {
        return "독";
    } else if (status == STATUS_BURN) {
        return "화상";
    } else if (status == STATUS_FREEZE) {
        return "얼음";
    }

    return "정상";
}

/* 팀에 아직 싸울 수 있는 포켓몬이 남아 있는지 확인합니다. */
int hasUsablePokemon(Trainer trainer)
{
    for (int i = 0; i < trainer.teamSize; i++) {
        if (!isFainted(trainer.team[i])) {
            return 1;
        }
    }

    return 0;
}

/* 팀에서 처음으로 쓰러지지 않은 포켓몬의 인덱스를 찾습니다. */
int getFirstUsablePokemonIndex(Trainer trainer)
{
    for (int i = 0; i < trainer.teamSize; i++) {
        if (!isFainted(trainer.team[i])) {
            return i;
        }
    }

    return -1;
}

/* 교체 시 전투 중에만 유지되는 랭크/대기 상태를 초기화합니다. */
void resetBattleStages(BattlePokemon *pokemon)
{
    pokemon->atkStage = 0;
    pokemon->defStage = 0;
    pokemon->spaStage = 0;
    pokemon->spdStage = 0;
    pokemon->agiStage = 0;
    pokemon->accStage = 0;
    pokemon->evaStage = 0;
    pokemon->rechargeTurn = 0;
    pokemon->chargingTurn = 0;
    pokemon->semiInvulnerable = 0;
}

/* 선택한 포켓몬으로 교체할 수 있는지 확인합니다. */
int canSwitchToPokemon(Trainer trainer, int activeIndex, int newIndex)
{
    if (newIndex < 0 || newIndex >= trainer.teamSize) {
        return 0;
    }

    if (newIndex == activeIndex) {
        return 0;
    }

    if (isFainted(trainer.team[newIndex])) {
        return 0;
    }

    return 1;
}

/* 현재 포켓몬을 팀의 다른 포켓몬으로 교체합니다. */
int switchPokemon(Trainer *trainer, int *activeIndex, int newIndex)
{
    if (!canSwitchToPokemon(*trainer, *activeIndex, newIndex)) {
        return 0;
    }

    resetBattleStages(&trainer->team[*activeIndex]);
    resetBattleStages(&trainer->team[newIndex]);
    *activeIndex = newIndex;

    printf("%s는 %s을(를) 내보냈다!\n",
           trainer->name,
           trainer->team[*activeIndex].pokemon.name);

    return 1;
}

/* 기술 타입이 공격자 타입과 같으면 자속 보정 1.5배를 줍니다. */
float getSameTypeAttackBonus(Pokemon attacker, PokemonType moveType)
{
    if (attacker.type1 == moveType || attacker.type2 == moveType) {
        return 1.5;
    }

    return 1.0;
}

/* 랭크 변화값(-6~+6)을 실제 능력치 배율로 바꿉니다. */
float getStageMultiplier(int stage)
{
    if (stage > 6) {
        stage = 6;
    } else if (stage < -6) {
        stage = -6;
    }

    if (stage >= 0) {
        return (2.0 + stage) / 2.0;
    }

    return 2.0 / (2.0 - stage);
}

/* 랭크 변화와 상태이상까지 반영한 전투용 능력치를 계산합니다. */
int getBattleStat(BattlePokemon pokemon, StatType stat)
{
    if (stat == STAT_ATK) {
        int attack = (int)(pokemon.pokemon.atk * getStageMultiplier(pokemon.atkStage));
        /* 화상 상태에서는 물리 공격을 절반으로 낮춥니다. */
        if (pokemon.status == STATUS_BURN) {
            attack /= 2;
        }
        return attack;
    } else if (stat == STAT_DEF) {
        return (int)(pokemon.pokemon.def * getStageMultiplier(pokemon.defStage));
    } else if (stat == STAT_SPA) {
        return (int)(pokemon.pokemon.spa * getStageMultiplier(pokemon.spaStage));
    } else if (stat == STAT_SPD) {
        return (int)(pokemon.pokemon.spd * getStageMultiplier(pokemon.spdStage));
    } else if (stat == STAT_AGI) {
        int speed = (int)(pokemon.pokemon.agi * getStageMultiplier(pokemon.agiStage));
        /* 마비 상태에서는 스피드를 1/4로 낮춥니다. */
        if (pokemon.status == STATUS_PARALYSIS) {
            speed /= 4;
        }
        return speed;
    } else if (stat == STAT_ACC) {
        return (int)(100 * getStageMultiplier(pokemon.accStage));
    } else if (stat == STAT_EVA) {
        return (int)(100 * getStageMultiplier(pokemon.evaStage));
    }

    return 1;
}

/* 특정 능력치 랭크를 바꾸고 -6~+6 범위 안에 고정합니다. */
void changeStatStage(BattlePokemon *pokemon, StatType stat, int change)
{
    int *stage = NULL;

    if (stat == STAT_ATK) {
        stage = &pokemon->atkStage;
    } else if (stat == STAT_DEF) {
        stage = &pokemon->defStage;
    } else if (stat == STAT_SPA) {
        stage = &pokemon->spaStage;
    } else if (stat == STAT_SPD) {
        stage = &pokemon->spdStage;
    } else if (stat == STAT_AGI) {
        stage = &pokemon->agiStage;
    } else if (stat == STAT_ACC) {
        stage = &pokemon->accStage;
    } else if (stat == STAT_EVA) {
        stage = &pokemon->evaStage;
    }

    if (stage == NULL) {
        return;
    }

    *stage += change;

    if (*stage > 6) {
        *stage = 6;
    } else if (*stage < -6) {
        *stage = -6;
    }
}

/* 위력 수치가 없어도 고정 데미지나 일격기로 데미지를 주는 기술입니다. */
static int isFixedDamageMove(Move move)
{
    return strcmp(move.name, "용의분노") == 0 ||
           strcmp(move.name, "지구던지기") == 0 ||
           strcmp(move.name, "나이트헤드") == 0 ||
           strcmp(move.name, "소닉붐") == 0 ||
           strcmp(move.name, "뿔드릴") == 0 ||
           strcmp(move.name, "가위자르기") == 0 ||
           strcmp(move.name, "땅가르기") == 0;
}

/* 1세대 기준 주요 반동기의 반동 비율입니다. */
static int getRecoilDivisor(Move move)
{
    if (strcmp(move.name, "발버둥") == 0) {
        return 2;
    } else if (strcmp(move.name, "돌진") == 0 ||
               strcmp(move.name, "이판사판태클") == 0 ||
               strcmp(move.name, "지옥의바퀴") == 0) {
        return 4;
    }

    return 0;
}

static int isJumpKickMove(Move move)
{
    return strcmp(move.name, "점프킥") == 0 ||
           strcmp(move.name, "무릎차기") == 0;
}

static int needsRechargeAfterHit(Move move)
{
    return strcmp(move.name, "파괴광선") == 0;
}

static int isSelfDestructMove(Move move)
{
    return strcmp(move.name, "자폭") == 0 ||
           strcmp(move.name, "대폭발") == 0;
}

static int isTwoTurnAttackMove(Move move)
{
    return strcmp(move.name, "솔라빔") == 0 ||
           strcmp(move.name, "불새") == 0 ||
           strcmp(move.name, "구멍파기") == 0 ||
           strcmp(move.name, "공중날기") == 0 ||
           strcmp(move.name, "로켓박치기") == 0 ||
           strcmp(move.name, "로케트박치기") == 0;
}

static int isSemiInvulnerableMove(Move move)
{
    return strcmp(move.name, "구멍파기") == 0 ||
           strcmp(move.name, "공중날기") == 0;
}

static void printChargeMessage(BattlePokemon attacker, Move move)
{
    if (strcmp(move.name, "구멍파기") == 0) {
        printf("%s는 땅속으로 파고들었다!\n", attacker.pokemon.name);
    } else if (strcmp(move.name, "공중날기") == 0) {
        printf("%s는 하늘 높이 날아올랐다!\n", attacker.pokemon.name);
    } else if (strcmp(move.name, "솔라빔") == 0) {
        printf("%s는 빛을 모으고 있다!\n", attacker.pokemon.name);
    } else if (strcmp(move.name, "불새") == 0) {
        printf("%s는 불타는 기운을 모으고 있다!\n", attacker.pokemon.name);
    } else {
        printf("%s는 힘을 모으고 있다!\n", attacker.pokemon.name);
    }
}

/* 기술 위력, 공격/방어, 레벨, 자속, 타입 상성을 반영해 데미지를 계산합니다. */
int calculateDamage(BattlePokemon attacker, BattlePokemon defender, Move move)
{
    int attackStat;
    int defenseStat;
    float stab;
    float effectiveness;
    float damage;

    /* 일부 1세대 고정 데미지/일격기 기술은 일반 공식 대신 별도로 처리합니다. */
    if (strcmp(move.name, "용의분노") == 0) {
        return 40;
    } else if (strcmp(move.name, "지구던지기") == 0 || strcmp(move.name, "나이트헤드") == 0) {
        return attacker.level;
    } else if (strcmp(move.name, "소닉붐") == 0) {
        return 20;
    } else if (strcmp(move.name, "뿔드릴") == 0 ||
               strcmp(move.name, "가위자르기") == 0 ||
               strcmp(move.name, "땅가르기") == 0) {
        return defender.currentHp;
    }

    if (move.power <= 0) {
        return 0;
    }

    if (move.category == MOVE_PHYSICAL) {
        attackStat = getBattleStat(attacker, STAT_ATK);
        defenseStat = getBattleStat(defender, STAT_DEF);
    } else {
        attackStat = getBattleStat(attacker, STAT_SPA);
        defenseStat = getBattleStat(defender, STAT_SPD);
    }

    stab = getSameTypeAttackBonus(attacker.pokemon, move.type);
    effectiveness = getTypeEffectiveness(move.type, defender.pokemon.type1, defender.pokemon.type2);

    if (effectiveness == 0) {
        return 0;
    }

    damage = (((2 * attacker.level / 5.0 + 2) * move.power * attackStat / defenseStat) / 50 + 2);
    damage *= stab * effectiveness;

    if (damage < 1) {
        return 1;
    }

    return (int)damage;
}

/* 타입 상성 배율에 맞는 전투 메시지를 출력합니다. */
void printEffectiveness(float effectiveness)
{
    if (effectiveness == 0) {
        printf("효과가 없다!\n");
    } else if (effectiveness >= 2) {
        printf("효과가 굉장했다!\n");
    } else if (effectiveness < 1) {
        printf("효과가 별로인 듯하다...\n");
    }
}

/* 현재 HP에서 데미지를 빼고, 0 밑으로 내려가지 않게 합니다. */
void takeDamage(BattlePokemon *defender, int damage)
{
    defender->currentHp -= damage;

    if (defender->currentHp < 0) {
        defender->currentHp = 0;
    }
}

/* 명중률과 회피율 랭크를 반영해 기술이 맞는지 판정합니다. */
int checkMoveHit(BattlePokemon attacker, BattlePokemon defender, Move move)
{
    int moveAccuracy = getMoveAccuracy(move);
    int accuracy = getBattleStat(attacker, STAT_ACC);
    int evasion = getBattleStat(defender, STAT_EVA);
    int hitRate = moveAccuracy * accuracy / evasion;

    if (defender.semiInvulnerable) {
        return 0;
    }

    if (hitRate > 100) {
        hitRate = 100;
    } else if (hitRate < 30) {
        hitRate = 30;
    }

    return rand() % 100 < hitRate;
}

/* 이미 상태이상이 없을 때만 새 상태이상을 부여합니다. */
void setStatusCondition(BattlePokemon *pokemon, StatusCondition status)
{
    if (pokemon->status != STATUS_NONE || isFainted(*pokemon)) {
        return;
    }

    pokemon->status = status;

    if (status == STATUS_SLEEP) {
        pokemon->sleepTurns = rand() % 3 + 1;
    }

    printf("%s는 %s 상태가 되었다!\n", pokemon->pokemon.name, getStatusName(status));
}

/* 기술 이름을 보고 어떤 상태이상을 유발하는지 반환합니다. */
StatusCondition getMoveStatus(Move move)
{
    if (strcmp(move.name, "수면가루") == 0 || strcmp(move.name, "최면술") == 0 ||
        strcmp(move.name, "버섯포자") == 0 || strcmp(move.name, "노래하기") == 0) {
        return STATUS_SLEEP;
    } else if (strcmp(move.name, "저리가루") == 0 || strcmp(move.name, "전기자석파") == 0 ||
               strcmp(move.name, "뱀눈초리") == 0) {
        return STATUS_PARALYSIS;
    } else if (strcmp(move.name, "독가루") == 0 || strcmp(move.name, "독가스") == 0) {
        return STATUS_POISON;
    } else if (strcmp(move.name, "누르기") == 0 || strcmp(move.name, "번개") == 0 ||
               strcmp(move.name, "번개펀치") == 0 || strcmp(move.name, "핥기") == 0) {
        return STATUS_PARALYSIS;
    } else if (strcmp(move.name, "화염방사") == 0 || strcmp(move.name, "불꽃펀치") == 0) {
        return STATUS_BURN;
    } else if (strcmp(move.name, "냉동빔") == 0 || strcmp(move.name, "눈보라") == 0 ||
               strcmp(move.name, "냉동펀치") == 0) {
        return STATUS_FREEZE;
    }

    return STATUS_NONE;
}

/* 기술별 상태이상 발생 확률을 반환합니다. */
int getMoveStatusChance(Move move)
{
    if (strcmp(move.name, "수면가루") == 0 || strcmp(move.name, "최면술") == 0 ||
        strcmp(move.name, "버섯포자") == 0 || strcmp(move.name, "노래하기") == 0 ||
        strcmp(move.name, "저리가루") == 0 || strcmp(move.name, "전기자석파") == 0 ||
        strcmp(move.name, "뱀눈초리") == 0 || strcmp(move.name, "독가루") == 0 ||
        strcmp(move.name, "독가스") == 0) {
        return 100;
    } else if (strcmp(move.name, "누르기") == 0) {
        return 30;
    } else if (strcmp(move.name, "번개") == 0 || strcmp(move.name, "번개펀치") == 0 ||
               strcmp(move.name, "화염방사") == 0 || strcmp(move.name, "불꽃펀치") == 0 ||
               strcmp(move.name, "냉동빔") == 0 || strcmp(move.name, "눈보라") == 0 ||
               strcmp(move.name, "냉동펀치") == 0) {
        return 10;
    }

    return 0;
}

/* 기술의 부가 효과로 상태이상을 걸 수 있으면 확률 판정을 합니다. */
void tryApplyMoveStatus(BattlePokemon *defender, Move move)
{
    StatusCondition status = getMoveStatus(move);
    int chance = getMoveStatusChance(move);

    if (status == STATUS_NONE || chance <= 0 || defender->status != STATUS_NONE || isFainted(*defender)) {
        return;
    }

    if (rand() % 100 < chance) {
        setStatusCondition(defender, status);
    }
}

/* 수면/마비/얼음 때문에 이번 턴 행동 가능한지 확인합니다. */
int canPokemonMove(BattlePokemon *pokemon)
{
    if (pokemon->rechargeTurn) {
        pokemon->rechargeTurn = 0;
        printf("%s는 힘을 모으느라 움직일 수 없다!\n", pokemon->pokemon.name);
        return 0;
    }

    if (pokemon->status == STATUS_SLEEP) {
        if (pokemon->sleepTurns > 0) {
            pokemon->sleepTurns--;
            printf("%s는 잠들어 있다!\n", pokemon->pokemon.name);
            return 0;
        }

        pokemon->status = STATUS_NONE;
        printf("%s는 잠에서 깨어났다!\n", pokemon->pokemon.name);
    } else if (pokemon->status == STATUS_PARALYSIS) {
        if (rand() % 100 < 25) {
            printf("%s는 몸이 저려 움직일 수 없다!\n", pokemon->pokemon.name);
            return 0;
        }
    } else if (pokemon->status == STATUS_FREEZE) {
        if (rand() % 100 < 20) {
            pokemon->status = STATUS_NONE;
            printf("%s의 얼음이 녹았다!\n", pokemon->pokemon.name);
        } else {
            printf("%s는 얼어붙어 움직일 수 없다!\n", pokemon->pokemon.name);
            return 0;
        }
    }

    return 1;
}

/* 턴 종료 시 독/화상 데미지를 적용합니다. */
void applyEndTurnStatusDamage(BattlePokemon *pokemon)
{
    int damage;

    if (isFainted(*pokemon)) {
        return;
    }

    if (pokemon->status != STATUS_POISON && pokemon->status != STATUS_BURN) {
        return;
    }

    damage = pokemon->pokemon.hp / 8;
    if (damage < 1) {
        damage = 1;
    }

    takeDamage(pokemon, damage);
    printf("%s는 %s으로 %d의 데미지를 입었다!\n",
           pokemon->pokemon.name,
           getStatusName(pokemon->status),
           damage);

    if (isFainted(*pokemon)) {
        printf("%s는 쓰러졌다!\n", pokemon->pokemon.name);
    }
}

/* 변신은 현재 HP와 상태이상은 유지하고 상대의 전투 정보를 복사합니다. */
static void transformPokemon(BattlePokemon *attacker, BattlePokemon *defender)
{
    int originalHp = attacker->pokemon.hp;
    int currentHp = attacker->currentHp;
    StatusCondition status = attacker->status;
    int sleepTurns = attacker->sleepTurns;

    if (attacker->transformed) {
        printf("하지만 이미 변신해 있다!\n");
        return;
    }

    attacker->pokemon = defender->pokemon;
    attacker->pokemon.hp = originalHp;
    attacker->currentHp = currentHp;
    attacker->status = status;
    attacker->sleepTurns = sleepTurns;

    attacker->moveCount = defender->moveCount;
    for (int i = 0; i < defender->moveCount; i++) {
        attacker->moves[i] = defender->moves[i];
    }

    attacker->atkStage = defender->atkStage;
    attacker->defStage = defender->defStage;
    attacker->spaStage = defender->spaStage;
    attacker->spdStage = defender->spdStage;
    attacker->agiStage = defender->agiStage;
    attacker->accStage = defender->accStage;
    attacker->evaStage = defender->evaStage;
    attacker->rechargeTurn = 0;
    attacker->chargingTurn = 0;
    attacker->semiInvulnerable = 0;
    attacker->transformed = 1;

    printf("%s의 모습으로 변신했다!\n", attacker->pokemon.name);
}

/* 위력이 0인 회복, 상태이상, 랭크 변화 기술을 처리합니다. */
void useStatusMove(BattlePokemon *attacker, BattlePokemon *defender, Move move)
{
    BattlePokemon *target = attacker;
    StatusCondition status = getMoveStatus(move);
    int heal;

    /* 회복/특수 보조 기술은 이름으로 별도 처리합니다. */
    if (strcmp(move.name, "변신") == 0) {
        transformPokemon(attacker, defender);
        return;
    } else if (strcmp(move.name, "HP회복") == 0) {
        heal = attacker->pokemon.hp / 2;
        attacker->currentHp += heal;
        if (attacker->currentHp > attacker->pokemon.hp) {
            attacker->currentHp = attacker->pokemon.hp;
        }
        printf("%s는 체력을 회복했다!\n", attacker->pokemon.name);
        return;
    } else if (strcmp(move.name, "잠자기") == 0) {
        attacker->currentHp = attacker->pokemon.hp;
        attacker->status = STATUS_SLEEP;
        attacker->sleepTurns = 2;
        printf("%s는 잠들고 체력을 모두 회복했다!\n", attacker->pokemon.name);
        return;
    } else if (strcmp(move.name, "리플렉터") == 0) {
        changeStatStage(attacker, STAT_DEF, 2);
        printf("%s의 방어가 크게 올랐다!\n", attacker->pokemon.name);
        return;
    } else if (strcmp(move.name, "흰안개") == 0) {
        attacker->atkStage = attacker->defStage = attacker->spaStage = attacker->spdStage = 0;
        attacker->agiStage = attacker->accStage = attacker->evaStage = 0;
        defender->atkStage = defender->defStage = defender->spaStage = defender->spdStage = 0;
        defender->agiStage = defender->accStage = defender->evaStage = 0;
        printf("모든 능력 변화가 사라졌다!\n");
        return;
    }

    if (status != STATUS_NONE) {
        tryApplyMoveStatus(defender, move);
        return;
    }

    if (move.stat == STAT_NONE || move.statChange == 0) {
        printf("하지만 아무 일도 일어나지 않았다.\n");
        return;
    }

    if (move.statChange < 0) {
        /* 음수 변화량은 상대에게 거는 디버프입니다. */
        target = defender;
    }

    changeStatStage(target, move.stat, move.statChange);

    if (move.statChange > 0) {
        printf("%s의 능력치가 올랐다!\n", target->pokemon.name);
    } else {
        printf("%s의 능력치가 떨어졌다!\n", target->pokemon.name);
    }
}

/* 실제 공격 한 번을 처리합니다. 상태 체크, 명중, 데미지, 부가 효과 순서입니다. */
int attackPokemon(BattlePokemon *attacker, BattlePokemon *defender, Move move)
{
    int damage;
    int actualDamage;
    int defenderHpBefore;
    int recoilDivisor;
    int recoilDamage;
    int wasCharging = attacker->chargingTurn;
    float effectiveness;

    if (isFainted(*attacker)) {
        return 0;
    }

    /* 상태이상 때문에 움직이지 못하면 기술 사용 자체가 취소됩니다. */
    if (!canPokemonMove(attacker)) {
        if (attacker->chargingTurn) {
            attacker->chargingTurn = 0;
            attacker->semiInvulnerable = 0;
        }
        return 0;
    }

    if (wasCharging) {
        move = attacker->chargingMove;
        attacker->chargingTurn = 0;
        attacker->semiInvulnerable = 0;
    }

    printf("%s의 %s!\n", attacker->pokemon.name, move.name);

    if (isTwoTurnAttackMove(move) && !wasCharging) {
        attacker->chargingMove = move;
        attacker->chargingTurn = 1;
        attacker->semiInvulnerable = isSemiInvulnerableMove(move);
        printChargeMessage(*attacker, move);
        return 0;
    }

    if (!checkMoveHit(*attacker, *defender, move)) {
        printf("공격이 빗나갔다!\n");
        if (isJumpKickMove(move)) {
            recoilDamage = attacker->pokemon.hp / 8;
            if (recoilDamage < 1) {
                recoilDamage = 1;
            }
            takeDamage(attacker, recoilDamage);
            printf("%s는 균형을 잃고 %d의 데미지를 입었다!\n",
                   attacker->pokemon.name,
                   recoilDamage);
            if (isFainted(*attacker)) {
                printf("%s는 쓰러졌다!\n", attacker->pokemon.name);
            }
        }
        return 0;
    }

    /* 위력이 0인 기술은 데미지 계산 대신 보조 기술 처리로 넘깁니다. */
    if (move.power <= 0 && !isFixedDamageMove(move)) {
        useStatusMove(attacker, defender, move);
        return 0;
    }

    damage = calculateDamage(*attacker, *defender, move);
    effectiveness = getTypeEffectiveness(move.type, defender->pokemon.type1, defender->pokemon.type2);
    defenderHpBefore = defender->currentHp;

    takeDamage(defender, damage);
    actualDamage = defenderHpBefore - defender->currentHp;
    printf("%s에게 %d의 데미지!\n", defender->pokemon.name, damage);
    printEffectiveness(effectiveness);
    tryApplyMoveStatus(defender, move);
    printf("%s HP: %d/%d\n", defender->pokemon.name, defender->currentHp, defender->pokemon.hp);

    if (isFainted(*defender)) {
        printf("%s는 쓰러졌다!\n", defender->pokemon.name);
    }

    if (isSelfDestructMove(move) && !isFainted(*attacker)) {
        takeDamage(attacker, attacker->currentHp);
        printf("%s는 폭발의 반동으로 쓰러졌다!\n", attacker->pokemon.name);
    }

    recoilDivisor = getRecoilDivisor(move);
    if (recoilDivisor > 0 && actualDamage > 0 && !isFainted(*attacker)) {
        recoilDamage = actualDamage / recoilDivisor;
        if (recoilDamage < 1) {
            recoilDamage = 1;
        }
        takeDamage(attacker, recoilDamage);
        printf("%s도 반동으로 %d의 데미지를 입었다!\n",
               attacker->pokemon.name,
               recoilDamage);

        if (isFainted(*attacker)) {
            printf("%s는 쓰러졌다!\n", attacker->pokemon.name);
        }
    }

    if (needsRechargeAfterHit(move) && actualDamage > 0 && !isFainted(*attacker)) {
        attacker->rechargeTurn = 1;
    }

    return damage;
}


int scoreStatusMove(BattlePokemon attacker, BattlePokemon defender, Move move)
{
    int score = 0;
    StatusCondition status = getMoveStatus(move);

    if (strcmp(move.name, "변신") == 0) {
        return attacker.transformed ? 0 : 80;
    }

    /* 상대가 정상 상태면 수면/마비/독 같은 상태이상 기술을 높게 평가합니다. */
    if (status != STATUS_NONE) {
        if (defender.status == STATUS_NONE) {
            return 45;
        }

        return 0;
    }

    if (move.stat == STAT_NONE || move.statChange == 0) {
        return 0;
    }

    if (move.statChange > 0) {
        if (move.stat == STAT_ATK && attacker.atkStage < 3) {
            score = 30 + move.statChange * 10;
        } else if (move.stat == STAT_SPA && attacker.spaStage < 3) {
            score = 30 + move.statChange * 10;
        } else if (move.stat == STAT_DEF && attacker.defStage < 3) {
            score = 20 + move.statChange * 10;
        } else if (move.stat == STAT_SPD && attacker.spdStage < 3) {
            score = 20 + move.statChange * 10;
        } else if (move.stat == STAT_AGI && attacker.agiStage < 3) {
            score = 15 + move.statChange * 10;
        } else if (move.stat == STAT_EVA && attacker.evaStage < 3) {
            score = 15 + move.statChange * 10;
        }
    } else {
        if (move.stat == STAT_DEF && defender.defStage > -3) {
            score = 25 - move.statChange * 10;
        } else if (move.stat == STAT_ATK && defender.atkStage > -3) {
            score = 20 - move.statChange * 10;
        } else if (move.stat == STAT_SPD && defender.spdStage > -3) {
            score = 20 - move.statChange * 10;
        } else if (move.stat == STAT_AGI && defender.agiStage > -3) {
            score = 15 - move.statChange * 10;
        } else if (move.stat == STAT_ACC && defender.accStage > -3) {
            score = 15 - move.statChange * 10;
        }
    }

    return score;
}

/* AI는 가장 큰 데미지를 내거나 유리한 보조 효과를 주는 기술을 선택합니다. */
Move chooseAIMove(BattlePokemon attacker, BattlePokemon defender)
{
    Move bestMove;
    int bestScore = -1;

    if (attacker.moveCount == 0) {
        return getFallbackMove();
    }

    bestMove = attacker.moves[0];

    for (int i = 0; i < attacker.moveCount; i++) {
        Move move = attacker.moves[i];
        int score;

        if (move.power > 0) {
            score = calculateDamage(attacker, defender, move);
        } else {
            score = scoreStatusMove(attacker, defender, move);
        }

        if (score > bestScore) {
            bestScore = score;
            bestMove = move;
        }
    }

    return bestMove;
}

/* 1대1 전투 상태를 간단히 출력합니다. */
void printBattleStatus(BattlePokemon first, BattlePokemon second)
{
    printf("[%s] HP %d/%d\n", first.pokemon.name, first.currentHp, first.pokemon.hp);
    printf("[%s] HP %d/%d\n", second.pokemon.name, second.currentHp, second.pokemon.hp);
}

/* 한 턴을 진행합니다. 스피드가 높은 쪽이 먼저 움직입니다. */
void battleTurn(BattlePokemon *first, Move firstMove, BattlePokemon *second, Move secondMove)
{
    BattlePokemon *fastPokemon = first;
    BattlePokemon *slowPokemon = second;
    Move fastMove = firstMove;
    Move slowMove = secondMove;

    /* 마비로 낮아진 스피드도 getBattleStat에서 반영됩니다. */
    if (getBattleStat(*second, STAT_AGI) > getBattleStat(*first, STAT_AGI)) {
        fastPokemon = second;
        slowPokemon = first;
        fastMove = secondMove;
        slowMove = firstMove;
    }

    attackPokemon(fastPokemon, slowPokemon, fastMove);

    if (!isFainted(*slowPokemon) && !isFainted(*fastPokemon)) {
        attackPokemon(slowPokemon, fastPokemon, slowMove);
    }

    /* 양쪽 행동이 끝난 뒤 독/화상 데미지를 처리합니다. */
    applyEndTurnStatusDamage(first);
    applyEndTurnStatusDamage(second);
}

/* 두 포켓몬 중 하나가 쓰러질 때까지 1대1 배틀을 반복합니다. */
BattlePokemon *battleOneOnOne(BattlePokemon *first, Move firstMove, BattlePokemon *second, Move secondMove)
{
    int turn = 1;

    while (!isFainted(*first) && !isFainted(*second)) {
        printf("\n--- %d턴 ---\n", turn);
        battleTurn(first, firstMove, second, secondMove);
        printBattleStatus(*first, *second);
        turn++;
    }

    if (isFainted(*first)) {
        printf("\n%s의 승리!\n", second->pokemon.name);
        return second;
    }

    printf("\n%s의 승리!\n", first->pokemon.name);
    return first;
}


/* =========================
   추가 UI 함수들
   ========================= */

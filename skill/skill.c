#include <stdlib.h>
#include <string.h>

#include "../pokemon.h"

/* 각 포켓몬이 레드/블루 버전에서 레벨업으로 배울 수 있는 전체 기술 후보입니다. */
static Move moveCandidates1[] = {{"울음소리", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_ATK, -1}, {"씨뿌리기", TYPE_GRASS, MOVE_SPECIAL, 0, STAT_NONE, 0}, {"몸통박치기", TYPE_NORMAL, MOVE_PHYSICAL, 40, STAT_NONE, 0}, {"덩굴채찍", TYPE_GRASS, MOVE_SPECIAL, 45, STAT_NONE, 0}, {"독가루", TYPE_POISON, MOVE_PHYSICAL, 0, STAT_NONE, 0}, {"잎날가르기", TYPE_GRASS, MOVE_SPECIAL, 55, STAT_NONE, 0}, {"성장", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_ATK, 1}, {"수면가루", TYPE_GRASS, MOVE_SPECIAL, 0, STAT_NONE, 0}, {"솔라빔", TYPE_GRASS, MOVE_SPECIAL, 120, STAT_NONE, 0}};
static Move moveCandidates2[] = {{"불꽃세례", TYPE_FIRE, MOVE_SPECIAL, 40, STAT_NONE, 0}, {"울음소리", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_ATK, -1}, {"째려보기", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_DEF, -1}, {"할퀴기", TYPE_NORMAL, MOVE_PHYSICAL, 40, STAT_NONE, 0}, {"분노", TYPE_NORMAL, MOVE_PHYSICAL, 20, STAT_NONE, 0}, {"베어가르기", TYPE_NORMAL, MOVE_PHYSICAL, 70, STAT_NONE, 0}, {"화염방사", TYPE_FIRE, MOVE_SPECIAL, 90, STAT_NONE, 0}, {"회오리불꽃", TYPE_FIRE, MOVE_SPECIAL, 35, STAT_NONE, 0}};
static Move moveCandidates3[] = {{"거품", TYPE_WATER, MOVE_SPECIAL, 40, STAT_AGI, -1}, {"몸통박치기", TYPE_NORMAL, MOVE_PHYSICAL, 40, STAT_NONE, 0}, {"꼬리흔들기", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_DEF, -1}, {"물대포", TYPE_WATER, MOVE_SPECIAL, 40, STAT_NONE, 0}, {"물기", TYPE_NORMAL, MOVE_PHYSICAL, 60, STAT_NONE, 0}, {"껍질에숨기", TYPE_WATER, MOVE_SPECIAL, 0, STAT_DEF, 1}, {"로케트박치기", TYPE_NORMAL, MOVE_PHYSICAL, 130, STAT_NONE, 0}, {"하이드로펌프", TYPE_WATER, MOVE_SPECIAL, 110, STAT_NONE, 0}};
static Move moveCandidates4[] = {{"염동력", TYPE_PSYCHIC, MOVE_SPECIAL, 50, STAT_NONE, 0}, {"독가루", TYPE_POISON, MOVE_PHYSICAL, 0, STAT_NONE, 0}, {"저리가루", TYPE_GRASS, MOVE_SPECIAL, 0, STAT_NONE, 0}, {"수면가루", TYPE_GRASS, MOVE_SPECIAL, 0, STAT_NONE, 0}, {"초음파", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_NONE, 0}, {"날려버리기", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_NONE, 0}, {"환상빔", TYPE_PSYCHIC, MOVE_SPECIAL, 65, STAT_NONE, 0}};
static Move moveCandidates5[] = {{"마구찌르기", TYPE_NORMAL, MOVE_PHYSICAL, 15, STAT_NONE, 0}, {"기충전", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_NONE, 0}, {"더블니들", TYPE_BUG, MOVE_PHYSICAL, 25, STAT_NONE, 0}, {"분노", TYPE_NORMAL, MOVE_PHYSICAL, 20, STAT_NONE, 0}, {"바늘미사일", TYPE_BUG, MOVE_PHYSICAL, 25, STAT_NONE, 0}, {"고속이동", TYPE_PSYCHIC, MOVE_SPECIAL, 0, STAT_AGI, 2}};
static Move moveCandidates6[] = {{"바람일으키기", TYPE_NORMAL, MOVE_PHYSICAL, 40, STAT_NONE, 0}, {"전광석화", TYPE_NORMAL, MOVE_PHYSICAL, 40, STAT_NONE, 0}, {"모래뿌리기", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_ACC, -1}, {"날려버리기", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_NONE, 0}, {"날개치기", TYPE_FLYING, MOVE_PHYSICAL, 60, STAT_NONE, 0}, {"고속이동", TYPE_PSYCHIC, MOVE_SPECIAL, 0, STAT_AGI, 2}, {"따라하기", TYPE_FLYING, MOVE_PHYSICAL, 0, STAT_NONE, 0}};
static Move moveCandidates7[] = {{"전광석화", TYPE_NORMAL, MOVE_PHYSICAL, 40, STAT_NONE, 0}, {"몸통박치기", TYPE_NORMAL, MOVE_PHYSICAL, 40, STAT_NONE, 0}, {"꼬리흔들기", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_DEF, -1}, {"필살앞니", TYPE_NORMAL, MOVE_PHYSICAL, 80, STAT_NONE, 0}, {"기충전", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_NONE, 0}, {"분노의앞니", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_NONE, 0}};
static Move moveCandidates8[] = {{"울음소리", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_ATK, -1}, {"째려보기", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_DEF, -1}, {"쪼기", TYPE_FLYING, MOVE_PHYSICAL, 35, STAT_NONE, 0}, {"마구찌르기", TYPE_NORMAL, MOVE_PHYSICAL, 15, STAT_NONE, 0}, {"따라하기", TYPE_FLYING, MOVE_PHYSICAL, 0, STAT_NONE, 0}, {"회전부리", TYPE_FLYING, MOVE_PHYSICAL, 80, STAT_NONE, 0}, {"고속이동", TYPE_PSYCHIC, MOVE_SPECIAL, 0, STAT_AGI, 2}};
static Move moveCandidates9[] = {{"째려보기", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_DEF, -1}, {"독침", TYPE_POISON, MOVE_PHYSICAL, 15, STAT_NONE, 0}, {"김밥말이", TYPE_NORMAL, MOVE_PHYSICAL, 15, STAT_NONE, 0}, {"물기", TYPE_NORMAL, MOVE_PHYSICAL, 60, STAT_NONE, 0}, {"뱀눈초리", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_NONE, 0}, {"싫은소리", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_DEF, -2}, {"용해액", TYPE_POISON, MOVE_PHYSICAL, 40, STAT_SPD, -1}};
static Move moveCandidates10[] = {{"울음소리", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_ATK, -1}, {"전기쇼크", TYPE_ELECTRIC, MOVE_SPECIAL, 40, STAT_NONE, 0}, {"전기자석파", TYPE_ELECTRIC, MOVE_SPECIAL, 0, STAT_NONE, 0}};
static Move moveCandidates11[] = {{"모래뿌리기", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_ACC, -1}, {"할퀴기", TYPE_NORMAL, MOVE_PHYSICAL, 40, STAT_NONE, 0}, {"베어가르기", TYPE_NORMAL, MOVE_PHYSICAL, 70, STAT_NONE, 0}, {"독침", TYPE_POISON, MOVE_PHYSICAL, 15, STAT_NONE, 0}, {"스피드스타", TYPE_NORMAL, MOVE_PHYSICAL, 60, STAT_NONE, 0}, {"마구할퀴기", TYPE_NORMAL, MOVE_PHYSICAL, 18, STAT_NONE, 0}};
static Move moveCandidates12[] = {{"누르기", TYPE_NORMAL, MOVE_PHYSICAL, 85, STAT_NONE, 0}, {"할퀴기", TYPE_NORMAL, MOVE_PHYSICAL, 40, STAT_NONE, 0}, {"몸통박치기", TYPE_NORMAL, MOVE_PHYSICAL, 40, STAT_NONE, 0}, {"꼬리흔들기", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_DEF, -1}, {"독침", TYPE_POISON, MOVE_PHYSICAL, 15, STAT_NONE, 0}};
static Move moveCandidates13[] = {{"뿔찌르기", TYPE_NORMAL, MOVE_PHYSICAL, 65, STAT_NONE, 0}, {"독침", TYPE_POISON, MOVE_PHYSICAL, 15, STAT_NONE, 0}, {"몸통박치기", TYPE_NORMAL, MOVE_PHYSICAL, 40, STAT_NONE, 0}, {"난동부리기", TYPE_NORMAL, MOVE_PHYSICAL, 120, STAT_NONE, 0}};
static Move moveCandidates14[] = {{"연속뺨치기", TYPE_NORMAL, MOVE_PHYSICAL, 15, STAT_NONE, 0}, {"손가락흔들기", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_NONE, 0}, {"작아지기", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_EVA, 2}, {"노래하기", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_NONE, 0}};
static Move moveCandidates15[] = {{"불꽃세례", TYPE_FIRE, MOVE_SPECIAL, 40, STAT_NONE, 0}, {"전광석화", TYPE_NORMAL, MOVE_PHYSICAL, 40, STAT_NONE, 0}, {"울부짖기", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_NONE, 0}, {"꼬리흔들기", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_DEF, -1}};
static Move moveCandidates16[] = {{"웅크리기", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_DEF, 1}, {"사슬묶기", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_NONE, 0}, {"연속뺨치기", TYPE_NORMAL, MOVE_PHYSICAL, 15, STAT_NONE, 0}, {"노래하기", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_NONE, 0}};
static Move moveCandidates17[] = {{"물기", TYPE_NORMAL, MOVE_PHYSICAL, 60, STAT_NONE, 0}, {"흡혈", TYPE_BUG, MOVE_PHYSICAL, 80, STAT_NONE, 0}, {"싫은소리", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_DEF, -2}, {"초음파", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_NONE, 0}, {"이상한빛", TYPE_GHOST, MOVE_PHYSICAL, 0, STAT_NONE, 0}, {"날개치기", TYPE_FLYING, MOVE_PHYSICAL, 60, STAT_NONE, 0}, {"흑안개", TYPE_ICE, MOVE_SPECIAL, 0, STAT_NONE, 0}};
static Move moveCandidates18[] = {{"용해액", TYPE_POISON, MOVE_PHYSICAL, 40, STAT_SPD, -1}, {"꽃잎댄스", TYPE_GRASS, MOVE_SPECIAL, 120, STAT_NONE, 0}, {"수면가루", TYPE_GRASS, MOVE_SPECIAL, 0, STAT_NONE, 0}, {"저리가루", TYPE_GRASS, MOVE_SPECIAL, 0, STAT_NONE, 0}, {"독가루", TYPE_POISON, MOVE_PHYSICAL, 0, STAT_NONE, 0}};
static Move moveCandidates19[] = {{"흡혈", TYPE_BUG, MOVE_PHYSICAL, 80, STAT_NONE, 0}, {"할퀴기", TYPE_NORMAL, MOVE_PHYSICAL, 40, STAT_NONE, 0}, {"저리가루", TYPE_GRASS, MOVE_SPECIAL, 0, STAT_NONE, 0}, {"버섯포자", TYPE_GRASS, MOVE_SPECIAL, 0, STAT_NONE, 0}, {"베어가르기", TYPE_NORMAL, MOVE_PHYSICAL, 70, STAT_NONE, 0}, {"성장", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_ATK, 1}};
static Move moveCandidates20[] = {{"사슬묶기", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_NONE, 0}, {"흡혈", TYPE_BUG, MOVE_PHYSICAL, 80, STAT_NONE, 0}, {"독가루", TYPE_POISON, MOVE_PHYSICAL, 0, STAT_NONE, 0}, {"몸통박치기", TYPE_NORMAL, MOVE_PHYSICAL, 40, STAT_NONE, 0}, {"저리가루", TYPE_GRASS, MOVE_SPECIAL, 0, STAT_NONE, 0}, {"환상빔", TYPE_PSYCHIC, MOVE_SPECIAL, 65, STAT_NONE, 0}, {"수면가루", TYPE_GRASS, MOVE_SPECIAL, 0, STAT_NONE, 0}, {"사이코키네시스", TYPE_PSYCHIC, MOVE_SPECIAL, 90, STAT_SPD, -1}};
static Move moveCandidates21[] = {{"구멍파기", TYPE_GROUND, MOVE_PHYSICAL, 80, STAT_NONE, 0}, {"울음소리", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_ATK, -1}, {"할퀴기", TYPE_NORMAL, MOVE_PHYSICAL, 40, STAT_NONE, 0}, {"모래뿌리기", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_ACC, -1}, {"베어가르기", TYPE_NORMAL, MOVE_PHYSICAL, 70, STAT_NONE, 0}, {"지진", TYPE_GROUND, MOVE_PHYSICAL, 100, STAT_NONE, 0}};
static Move moveCandidates22[] = {{"물기", TYPE_NORMAL, MOVE_PHYSICAL, 60, STAT_NONE, 0}, {"울음소리", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_ATK, -1}, {"할퀴기", TYPE_NORMAL, MOVE_PHYSICAL, 40, STAT_NONE, 0}, {"싫은소리", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_DEF, -2}, {"고양이돈받기", TYPE_NORMAL, MOVE_PHYSICAL, 40, STAT_NONE, 0}, {"마구할퀴기", TYPE_NORMAL, MOVE_PHYSICAL, 18, STAT_NONE, 0}, {"베어가르기", TYPE_NORMAL, MOVE_PHYSICAL, 70, STAT_NONE, 0}};
static Move moveCandidates23[] = {{"사슬묶기", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_NONE, 0}, {"할퀴기", TYPE_NORMAL, MOVE_PHYSICAL, 40, STAT_NONE, 0}, {"꼬리흔들기", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_DEF, -1}, {"염동력", TYPE_PSYCHIC, MOVE_SPECIAL, 50, STAT_NONE, 0}, {"마구할퀴기", TYPE_NORMAL, MOVE_PHYSICAL, 18, STAT_NONE, 0}, {"하이드로펌프", TYPE_WATER, MOVE_SPECIAL, 110, STAT_NONE, 0}};
static Move moveCandidates24[] = {{"마구할퀴기", TYPE_NORMAL, MOVE_PHYSICAL, 18, STAT_NONE, 0}, {"태권당수", TYPE_NORMAL, MOVE_PHYSICAL, 50, STAT_NONE, 0}, {"째려보기", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_DEF, -1}, {"할퀴기", TYPE_NORMAL, MOVE_PHYSICAL, 40, STAT_NONE, 0}, {"기충전", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_NONE, 0}, {"지구던지기", TYPE_FIGHTING, MOVE_PHYSICAL, 0, STAT_NONE, 0}, {"난동부리기", TYPE_NORMAL, MOVE_PHYSICAL, 120, STAT_NONE, 0}};
static Move moveCandidates25[] = {{"불꽃세례", TYPE_FIRE, MOVE_SPECIAL, 40, STAT_NONE, 0}, {"째려보기", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_DEF, -1}, {"울부짖기", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_NONE, 0}, {"돌진", TYPE_NORMAL, MOVE_PHYSICAL, 90, STAT_NONE, 0}};
static Move moveCandidates26[] = {{"누르기", TYPE_NORMAL, MOVE_PHYSICAL, 85, STAT_NONE, 0}, {"연속뺨치기", TYPE_NORMAL, MOVE_PHYSICAL, 15, STAT_NONE, 0}, {"최면술", TYPE_PSYCHIC, MOVE_SPECIAL, 0, STAT_NONE, 0}, {"물대포", TYPE_WATER, MOVE_SPECIAL, 40, STAT_NONE, 0}};
static Move moveCandidates27[] = {{"염동력", TYPE_PSYCHIC, MOVE_SPECIAL, 50, STAT_NONE, 0}, {"사슬묶기", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_NONE, 0}, {"순간이동", TYPE_PSYCHIC, MOVE_SPECIAL, 0, STAT_NONE, 0}, {"환상빔", TYPE_PSYCHIC, MOVE_SPECIAL, 65, STAT_NONE, 0}, {"HP회복", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_NONE, 0}, {"사이코키네시스", TYPE_PSYCHIC, MOVE_SPECIAL, 90, STAT_SPD, -1}, {"리플렉터", TYPE_PSYCHIC, MOVE_SPECIAL, 0, STAT_NONE, 0}};
static Move moveCandidates28[] = {{"태권당수", TYPE_NORMAL, MOVE_PHYSICAL, 50, STAT_NONE, 0}, {"째려보기", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_DEF, -1}, {"안다리걸기", TYPE_FIGHTING, MOVE_PHYSICAL, 0, STAT_NONE, 0}, {"기충전", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_NONE, 0}, {"지구던지기", TYPE_FIGHTING, MOVE_PHYSICAL, 0, STAT_NONE, 0}, {"지옥의바퀴", TYPE_FIGHTING, MOVE_PHYSICAL, 80, STAT_NONE, 0}};
static Move moveCandidates29[] = {{"용해액", TYPE_POISON, MOVE_PHYSICAL, 40, STAT_SPD, -1}, {"잎날가르기", TYPE_GRASS, MOVE_SPECIAL, 55, STAT_NONE, 0}, {"수면가루", TYPE_GRASS, MOVE_SPECIAL, 0, STAT_NONE, 0}, {"저리가루", TYPE_GRASS, MOVE_SPECIAL, 0, STAT_NONE, 0}, {"김밥말이", TYPE_NORMAL, MOVE_PHYSICAL, 15, STAT_NONE, 0}, {"독가루", TYPE_POISON, MOVE_PHYSICAL, 0, STAT_NONE, 0}};
static Move moveCandidates30[] = {{"용해액", TYPE_POISON, MOVE_PHYSICAL, 40, STAT_SPD, -1}, {"초음파", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_NONE, 0}, {"김밥말이", TYPE_NORMAL, MOVE_PHYSICAL, 15, STAT_NONE, 0}, {"독침", TYPE_POISON, MOVE_PHYSICAL, 15, STAT_NONE, 0}, {"물대포", TYPE_WATER, MOVE_SPECIAL, 40, STAT_NONE, 0}, {"휘감기", TYPE_NORMAL, MOVE_PHYSICAL, 10, STAT_AGI, -1}, {"배리어", TYPE_PSYCHIC, MOVE_SPECIAL, 0, STAT_DEF, 2}, {"싫은소리", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_DEF, -2}, {"하이드로펌프", TYPE_WATER, MOVE_SPECIAL, 110, STAT_NONE, 0}};
static Move moveCandidates31[] = {{"웅크리기", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_DEF, 1}, {"몸통박치기", TYPE_NORMAL, MOVE_PHYSICAL, 40, STAT_NONE, 0}, {"돌떨구기", TYPE_ROCK, MOVE_PHYSICAL, 50, STAT_NONE, 0}, {"자폭", TYPE_NORMAL, MOVE_PHYSICAL, 200, STAT_NONE, 0}, {"단단해지기", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_DEF, 1}, {"지진", TYPE_GROUND, MOVE_PHYSICAL, 100, STAT_NONE, 0}, {"대폭발", TYPE_NORMAL, MOVE_PHYSICAL, 250, STAT_NONE, 0}};
static Move moveCandidates32[] = {{"불꽃세례", TYPE_FIRE, MOVE_SPECIAL, 40, STAT_NONE, 0}, {"울음소리", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_ATK, -1}, {"짓밟기", TYPE_NORMAL, MOVE_PHYSICAL, 65, STAT_NONE, 0}, {"꼬리흔들기", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_DEF, -1}, {"회오리불꽃", TYPE_FIRE, MOVE_SPECIAL, 35, STAT_NONE, 0}, {"돌진", TYPE_NORMAL, MOVE_PHYSICAL, 90, STAT_NONE, 0}, {"고속이동", TYPE_PSYCHIC, MOVE_SPECIAL, 0, STAT_AGI, 2}};
static Move moveCandidates33[] = {{"염동력", TYPE_PSYCHIC, MOVE_SPECIAL, 50, STAT_NONE, 0}, {"사슬묶기", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_NONE, 0}, {"박치기", TYPE_NORMAL, MOVE_PHYSICAL, 70, STAT_NONE, 0}, {"울음소리", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_ATK, -1}, {"물대포", TYPE_WATER, MOVE_SPECIAL, 40, STAT_NONE, 0}, {"껍질에숨기", TYPE_WATER, MOVE_SPECIAL, 0, STAT_DEF, 1}, {"망각술", TYPE_PSYCHIC, MOVE_SPECIAL, 0, STAT_SPD, 2}, {"사이코키네시스", TYPE_PSYCHIC, MOVE_SPECIAL, 90, STAT_SPD, -1}};
static Move moveCandidates34[] = {{"소닉붐", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_NONE, 0}, {"몸통박치기", TYPE_NORMAL, MOVE_PHYSICAL, 40, STAT_NONE, 0}, {"전기쇼크", TYPE_ELECTRIC, MOVE_SPECIAL, 40, STAT_NONE, 0}, {"초음파", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_NONE, 0}, {"전기자석파", TYPE_ELECTRIC, MOVE_SPECIAL, 0, STAT_NONE, 0}, {"스피드스타", TYPE_NORMAL, MOVE_PHYSICAL, 60, STAT_NONE, 0}, {"싫은소리", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_DEF, -2}};
static Move moveCandidates35[] = {{"쪼기", TYPE_FLYING, MOVE_PHYSICAL, 35, STAT_NONE, 0}, {"모래뿌리기", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_ACC, -1}, {"째려보기", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_DEF, -1}, {"마구찌르기", TYPE_NORMAL, MOVE_PHYSICAL, 15, STAT_NONE, 0}, {"칼춤", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_ATK, 2}, {"고속이동", TYPE_PSYCHIC, MOVE_SPECIAL, 0, STAT_AGI, 2}, {"베어가르기", TYPE_NORMAL, MOVE_PHYSICAL, 70, STAT_NONE, 0}};
static Move moveCandidates36[] = {{"마구찌르기", TYPE_NORMAL, MOVE_PHYSICAL, 15, STAT_NONE, 0}, {"울음소리", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_ATK, -1}, {"쪼기", TYPE_FLYING, MOVE_PHYSICAL, 35, STAT_NONE, 0}, {"회전부리", TYPE_FLYING, MOVE_PHYSICAL, 80, STAT_NONE, 0}, {"분노", TYPE_NORMAL, MOVE_PHYSICAL, 20, STAT_NONE, 0}, {"트라이어택", TYPE_NORMAL, MOVE_PHYSICAL, 80, STAT_NONE, 0}, {"고속이동", TYPE_PSYCHIC, MOVE_SPECIAL, 0, STAT_AGI, 2}};
static Move moveCandidates37[] = {{"오로라빔", TYPE_ICE, MOVE_SPECIAL, 65, STAT_ATK, -1}, {"울음소리", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_ATK, -1}, {"박치기", TYPE_NORMAL, MOVE_PHYSICAL, 70, STAT_NONE, 0}, {"잠자기", TYPE_PSYCHIC, MOVE_SPECIAL, 0, STAT_NONE, 0}, {"돌진", TYPE_NORMAL, MOVE_PHYSICAL, 90, STAT_NONE, 0}, {"냉동빔", TYPE_ICE, MOVE_SPECIAL, 90, STAT_NONE, 0}};
static Move moveCandidates38[] = {{"사슬묶기", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_NONE, 0}, {"독가스", TYPE_POISON, MOVE_PHYSICAL, 0, STAT_NONE, 0}, {"막치기", TYPE_NORMAL, MOVE_PHYSICAL, 40, STAT_NONE, 0}, {"작아지기", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_EVA, 2}, {"오물공격", TYPE_POISON, MOVE_PHYSICAL, 65, STAT_NONE, 0}, {"단단해지기", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_DEF, 1}, {"싫은소리", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_DEF, -2}, {"녹기", TYPE_POISON, MOVE_PHYSICAL, 0, STAT_DEF, 2}};
static Move moveCandidates39[] = {{"오로라빔", TYPE_ICE, MOVE_SPECIAL, 65, STAT_ATK, -1}, {"껍질끼우기", TYPE_WATER, MOVE_SPECIAL, 35, STAT_NONE, 0}, {"초음파", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_NONE, 0}, {"껍질에숨기", TYPE_WATER, MOVE_SPECIAL, 0, STAT_DEF, 1}, {"가시대포", TYPE_NORMAL, MOVE_PHYSICAL, 20, STAT_NONE, 0}};
static Move moveCandidates40[] = {{"이상한빛", TYPE_GHOST, MOVE_PHYSICAL, 0, STAT_NONE, 0}, {"핥기", TYPE_GHOST, MOVE_PHYSICAL, 30, STAT_NONE, 0}, {"나이트헤드", TYPE_GHOST, MOVE_PHYSICAL, 0, STAT_NONE, 0}, {"최면술", TYPE_PSYCHIC, MOVE_SPECIAL, 0, STAT_NONE, 0}, {"꿈먹기", TYPE_PSYCHIC, MOVE_SPECIAL, 100, STAT_NONE, 0}};
static Move moveCandidates41[] = {{"싫은소리", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_DEF, -2}, {"몸통박치기", TYPE_NORMAL, MOVE_PHYSICAL, 40, STAT_NONE, 0}, {"조이기", TYPE_NORMAL, MOVE_PHYSICAL, 15, STAT_NONE, 0}, {"돌떨구기", TYPE_ROCK, MOVE_PHYSICAL, 50, STAT_NONE, 0}, {"분노", TYPE_NORMAL, MOVE_PHYSICAL, 20, STAT_NONE, 0}, {"힘껏치기", TYPE_NORMAL, MOVE_PHYSICAL, 80, STAT_NONE, 0}, {"단단해지기", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_DEF, 1}};
static Move moveCandidates42[] = {{"염동력", TYPE_PSYCHIC, MOVE_SPECIAL, 50, STAT_NONE, 0}, {"사슬묶기", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_NONE, 0}, {"최면술", TYPE_PSYCHIC, MOVE_SPECIAL, 0, STAT_NONE, 0}, {"막치기", TYPE_NORMAL, MOVE_PHYSICAL, 40, STAT_NONE, 0}, {"박치기", TYPE_NORMAL, MOVE_PHYSICAL, 70, STAT_NONE, 0}, {"독가스", TYPE_POISON, MOVE_PHYSICAL, 0, STAT_NONE, 0}, {"사이코키네시스", TYPE_PSYCHIC, MOVE_SPECIAL, 90, STAT_SPD, -1}, {"요가포즈", TYPE_PSYCHIC, MOVE_SPECIAL, 0, STAT_ATK, 1}};
static Move moveCandidates43[] = {{"거품", TYPE_WATER, MOVE_SPECIAL, 40, STAT_AGI, -1}, {"째려보기", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_DEF, -1}, {"찝기", TYPE_NORMAL, MOVE_PHYSICAL, 55, STAT_NONE, 0}, {"가위자르기", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_NONE, 0}, {"짓밟기", TYPE_NORMAL, MOVE_PHYSICAL, 65, STAT_NONE, 0}, {"찝게햄머", TYPE_WATER, MOVE_SPECIAL, 100, STAT_NONE, 0}, {"단단해지기", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_DEF, 1}};
static Move moveCandidates44[] = {{"싫은소리", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_DEF, -2}, {"소닉붐", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_NONE, 0}, {"몸통박치기", TYPE_NORMAL, MOVE_PHYSICAL, 40, STAT_NONE, 0}, {"자폭", TYPE_NORMAL, MOVE_PHYSICAL, 200, STAT_NONE, 0}, {"빛의장막", TYPE_PSYCHIC, MOVE_SPECIAL, 0, STAT_NONE, 0}, {"스피드스타", TYPE_NORMAL, MOVE_PHYSICAL, 60, STAT_NONE, 0}, {"대폭발", TYPE_NORMAL, MOVE_PHYSICAL, 250, STAT_NONE, 0}};
static Move moveCandidates45[] = {{"구슬던지기", TYPE_NORMAL, MOVE_PHYSICAL, 15, STAT_NONE, 0}, {"최면술", TYPE_PSYCHIC, MOVE_SPECIAL, 0, STAT_NONE, 0}, {"짓밟기", TYPE_NORMAL, MOVE_PHYSICAL, 65, STAT_NONE, 0}};
static Move moveCandidates46[] = {{"뼈다귀치기", TYPE_GROUND, MOVE_PHYSICAL, 65, STAT_NONE, 0}, {"기충전", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_NONE, 0}, {"울음소리", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_ATK, -1}, {"째려보기", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_DEF, -1}, {"난동부리기", TYPE_NORMAL, MOVE_PHYSICAL, 120, STAT_NONE, 0}, {"뼈다귀부메랑", TYPE_GROUND, MOVE_PHYSICAL, 50, STAT_NONE, 0}, {"분노", TYPE_NORMAL, MOVE_PHYSICAL, 20, STAT_NONE, 0}};
static Move moveCandidates47[] = {{"두번치기", TYPE_FIGHTING, MOVE_PHYSICAL, 30, STAT_NONE, 0}, {"요가포즈", TYPE_PSYCHIC, MOVE_SPECIAL, 0, STAT_ATK, 1}, {"돌려차기", TYPE_FIGHTING, MOVE_PHYSICAL, 60, STAT_NONE, 0}, {"점프킥", TYPE_FIGHTING, MOVE_PHYSICAL, 100, STAT_NONE, 0}, {"기충전", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_NONE, 0}, {"무릎차기", TYPE_FIGHTING, MOVE_PHYSICAL, 130, STAT_NONE, 0}, {"메가톤킥", TYPE_NORMAL, MOVE_PHYSICAL, 120, STAT_NONE, 0}};
static Move moveCandidates48[] = {{"고속이동", TYPE_PSYCHIC, MOVE_SPECIAL, 0, STAT_AGI, 2}, {"연속펀치", TYPE_NORMAL, MOVE_PHYSICAL, 18, STAT_NONE, 0}, {"불꽃펀치", TYPE_FIRE, MOVE_SPECIAL, 75, STAT_NONE, 0}, {"냉동펀치", TYPE_ICE, MOVE_SPECIAL, 75, STAT_NONE, 0}, {"번개펀치", TYPE_ELECTRIC, MOVE_SPECIAL, 75, STAT_NONE, 0}, {"메가톤펀치", TYPE_NORMAL, MOVE_PHYSICAL, 80, STAT_NONE, 0}, {"카운터", TYPE_FIGHTING, MOVE_PHYSICAL, 0, STAT_NONE, 0}};
static Move moveCandidates49[] = {{"초음파", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_NONE, 0}, {"김밥말이", TYPE_NORMAL, MOVE_PHYSICAL, 15, STAT_NONE, 0}, {"짓밟기", TYPE_NORMAL, MOVE_PHYSICAL, 65, STAT_NONE, 0}, {"사슬묶기", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_NONE, 0}, {"웅크리기", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_DEF, 1}, {"힘껏치기", TYPE_NORMAL, MOVE_PHYSICAL, 80, STAT_NONE, 0}, {"싫은소리", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_DEF, -2}};
static Move moveCandidates50[] = {{"오물공격", TYPE_POISON, MOVE_PHYSICAL, 65, STAT_NONE, 0}, {"스모그", TYPE_POISON, MOVE_PHYSICAL, 30, STAT_NONE, 0}, {"몸통박치기", TYPE_NORMAL, MOVE_PHYSICAL, 40, STAT_NONE, 0}, {"연막", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_ACC, -1}, {"자폭", TYPE_NORMAL, MOVE_PHYSICAL, 200, STAT_NONE, 0}, {"흑안개", TYPE_ICE, MOVE_SPECIAL, 0, STAT_NONE, 0}, {"대폭발", TYPE_NORMAL, MOVE_PHYSICAL, 250, STAT_NONE, 0}};
static Move moveCandidates51[] = {{"마구찌르기", TYPE_NORMAL, MOVE_PHYSICAL, 15, STAT_NONE, 0}, {"뿔찌르기", TYPE_NORMAL, MOVE_PHYSICAL, 65, STAT_NONE, 0}, {"짓밟기", TYPE_NORMAL, MOVE_PHYSICAL, 65, STAT_NONE, 0}, {"꼬리흔들기", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_DEF, -1}, {"뿔드릴", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_NONE, 0}, {"째려보기", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_DEF, -1}, {"돌진", TYPE_NORMAL, MOVE_PHYSICAL, 90, STAT_NONE, 0}};
static Move moveCandidates52[] = {{"연속뺨치기", TYPE_NORMAL, MOVE_PHYSICAL, 15, STAT_NONE, 0}, {"막치기", TYPE_NORMAL, MOVE_PHYSICAL, 40, STAT_NONE, 0}, {"노래하기", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_NONE, 0}, {"울음소리", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_ATK, -1}, {"작아지기", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_EVA, 2}, {"웅크리기", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_DEF, 1}, {"빛의장막", TYPE_PSYCHIC, MOVE_SPECIAL, 0, STAT_NONE, 0}, {"이판사판태클", TYPE_NORMAL, MOVE_PHYSICAL, 120, STAT_NONE, 0}};
static Move moveCandidates53[] = {{"조이기", TYPE_NORMAL, MOVE_PHYSICAL, 15, STAT_NONE, 0}, {"휘감기", TYPE_NORMAL, MOVE_PHYSICAL, 10, STAT_AGI, -1}, {"흡수", TYPE_GRASS, MOVE_SPECIAL, 20, STAT_NONE, 0}, {"독가루", TYPE_POISON, MOVE_PHYSICAL, 0, STAT_NONE, 0}, {"저리가루", TYPE_GRASS, MOVE_SPECIAL, 0, STAT_NONE, 0}, {"수면가루", TYPE_GRASS, MOVE_SPECIAL, 0, STAT_NONE, 0}, {"힘껏치기", TYPE_NORMAL, MOVE_PHYSICAL, 80, STAT_NONE, 0}, {"성장", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_ATK, 1}};
static Move moveCandidates54[] = {{"연속펀치", TYPE_NORMAL, MOVE_PHYSICAL, 18, STAT_NONE, 0}, {"분노", TYPE_NORMAL, MOVE_PHYSICAL, 20, STAT_NONE, 0}, {"물기", TYPE_NORMAL, MOVE_PHYSICAL, 60, STAT_NONE, 0}, {"꼬리흔들기", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_DEF, -1}, {"메가톤펀치", TYPE_NORMAL, MOVE_PHYSICAL, 80, STAT_NONE, 0}, {"째려보기", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_DEF, -1}, {"잼잼펀치", TYPE_NORMAL, MOVE_PHYSICAL, 70, STAT_NONE, 0}};
static Move moveCandidates55[] = {{"거품", TYPE_WATER, MOVE_SPECIAL, 40, STAT_AGI, -1}, {"연막", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_ACC, -1}, {"째려보기", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_DEF, -1}, {"물대포", TYPE_WATER, MOVE_SPECIAL, 40, STAT_NONE, 0}, {"고속이동", TYPE_PSYCHIC, MOVE_SPECIAL, 0, STAT_AGI, 2}, {"하이드로펌프", TYPE_WATER, MOVE_SPECIAL, 110, STAT_NONE, 0}};
static Move moveCandidates56[] = {{"쪼기", TYPE_FLYING, MOVE_PHYSICAL, 35, STAT_NONE, 0}, {"초음파", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_NONE, 0}, {"꼬리흔들기", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_DEF, -1}, {"뿔찌르기", TYPE_NORMAL, MOVE_PHYSICAL, 65, STAT_NONE, 0}, {"마구찌르기", TYPE_NORMAL, MOVE_PHYSICAL, 15, STAT_NONE, 0}, {"폭포오르기", TYPE_WATER, MOVE_SPECIAL, 80, STAT_NONE, 0}, {"뿔드릴", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_NONE, 0}, {"고속이동", TYPE_PSYCHIC, MOVE_SPECIAL, 0, STAT_AGI, 2}};
static Move moveCandidates57[] = {{"단단해지기", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_DEF, 1}, {"몸통박치기", TYPE_NORMAL, MOVE_PHYSICAL, 40, STAT_NONE, 0}, {"물대포", TYPE_WATER, MOVE_SPECIAL, 40, STAT_NONE, 0}};
static Move moveCandidates58[] = {{"배리어", TYPE_PSYCHIC, MOVE_SPECIAL, 0, STAT_DEF, 2}, {"염동력", TYPE_PSYCHIC, MOVE_SPECIAL, 50, STAT_NONE, 0}, {"빛의장막", TYPE_PSYCHIC, MOVE_SPECIAL, 0, STAT_NONE, 0}, {"연속뺨치기", TYPE_NORMAL, MOVE_PHYSICAL, 15, STAT_NONE, 0}, {"요가포즈", TYPE_PSYCHIC, MOVE_SPECIAL, 0, STAT_ATK, 1}, {"대타출동", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_EVA, 2}};
static Move moveCandidates59[] = {{"전광석화", TYPE_NORMAL, MOVE_PHYSICAL, 40, STAT_NONE, 0}, {"째려보기", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_DEF, -1}, {"기충전", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_NONE, 0}, {"그림자분신", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_EVA, 1}, {"베어가르기", TYPE_NORMAL, MOVE_PHYSICAL, 70, STAT_NONE, 0}, {"칼춤", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_ATK, 2}, {"고속이동", TYPE_PSYCHIC, MOVE_SPECIAL, 0, STAT_AGI, 2}};
static Move moveCandidates60[] = {{"악마의키스", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_NONE, 0}, {"막치기", TYPE_NORMAL, MOVE_PHYSICAL, 40, STAT_NONE, 0}, {"핥기", TYPE_GHOST, MOVE_PHYSICAL, 30, STAT_NONE, 0}, {"연속뺨치기", TYPE_NORMAL, MOVE_PHYSICAL, 15, STAT_NONE, 0}, {"냉동펀치", TYPE_ICE, MOVE_SPECIAL, 75, STAT_NONE, 0}, {"누르기", TYPE_NORMAL, MOVE_PHYSICAL, 85, STAT_NONE, 0}, {"난동부리기", TYPE_NORMAL, MOVE_PHYSICAL, 120, STAT_NONE, 0}, {"눈보라", TYPE_ICE, MOVE_SPECIAL, 110, STAT_NONE, 0}};
static Move moveCandidates61[] = {{"째려보기", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_DEF, -1}, {"전광석화", TYPE_NORMAL, MOVE_PHYSICAL, 40, STAT_NONE, 0}, {"전기쇼크", TYPE_ELECTRIC, MOVE_SPECIAL, 40, STAT_NONE, 0}, {"싫은소리", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_DEF, -2}, {"번개펀치", TYPE_ELECTRIC, MOVE_SPECIAL, 75, STAT_NONE, 0}, {"빛의장막", TYPE_PSYCHIC, MOVE_SPECIAL, 0, STAT_NONE, 0}, {"번개", TYPE_ELECTRIC, MOVE_SPECIAL, 110, STAT_NONE, 0}};
static Move moveCandidates62[] = {{"불꽃세례", TYPE_FIRE, MOVE_SPECIAL, 40, STAT_NONE, 0}, {"째려보기", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_DEF, -1}, {"이상한빛", TYPE_GHOST, MOVE_PHYSICAL, 0, STAT_NONE, 0}, {"불꽃펀치", TYPE_FIRE, MOVE_SPECIAL, 75, STAT_NONE, 0}, {"연막", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_ACC, -1}, {"스모그", TYPE_POISON, MOVE_PHYSICAL, 30, STAT_NONE, 0}, {"화염방사", TYPE_FIRE, MOVE_SPECIAL, 90, STAT_NONE, 0}};
static Move moveCandidates63[] = {{"찝기", TYPE_NORMAL, MOVE_PHYSICAL, 55, STAT_NONE, 0}, {"지구던지기", TYPE_FIGHTING, MOVE_PHYSICAL, 0, STAT_NONE, 0}, {"가위자르기", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_NONE, 0}, {"기충전", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_NONE, 0}, {"단단해지기", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_DEF, 1}, {"베어가르기", TYPE_NORMAL, MOVE_PHYSICAL, 70, STAT_NONE, 0}, {"칼춤", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_ATK, 2}};
static Move moveCandidates64[] = {{"몸통박치기", TYPE_NORMAL, MOVE_PHYSICAL, 40, STAT_NONE, 0}, {"짓밟기", TYPE_NORMAL, MOVE_PHYSICAL, 65, STAT_NONE, 0}, {"꼬리흔들기", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_DEF, -1}, {"째려보기", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_DEF, -1}, {"분노", TYPE_NORMAL, MOVE_PHYSICAL, 20, STAT_NONE, 0}, {"돌진", TYPE_NORMAL, MOVE_PHYSICAL, 90, STAT_NONE, 0}};
static Move moveCandidates65[] = {{"물기", TYPE_NORMAL, MOVE_PHYSICAL, 60, STAT_NONE, 0}, {"용의분노", TYPE_DRAGON, MOVE_SPECIAL, 0, STAT_NONE, 0}, {"하이드로펌프", TYPE_WATER, MOVE_SPECIAL, 110, STAT_NONE, 0}, {"째려보기", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_DEF, -1}, {"파괴광선", TYPE_NORMAL, MOVE_PHYSICAL, 150, STAT_NONE, 0}};
static Move moveCandidates66[] = {{"울음소리", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_ATK, -1}, {"물대포", TYPE_WATER, MOVE_SPECIAL, 40, STAT_NONE, 0}, {"노래하기", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_NONE, 0}, {"흰안개", TYPE_ICE, MOVE_SPECIAL, 0, STAT_NONE, 0}, {"누르기", TYPE_NORMAL, MOVE_PHYSICAL, 85, STAT_NONE, 0}, {"이상한빛", TYPE_GHOST, MOVE_PHYSICAL, 0, STAT_NONE, 0}, {"냉동빔", TYPE_ICE, MOVE_SPECIAL, 90, STAT_NONE, 0}, {"하이드로펌프", TYPE_WATER, MOVE_SPECIAL, 110, STAT_NONE, 0}};
static Move moveCandidates67[] = {{"변신", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_NONE, 0}};
static Move moveCandidates68[] = {{"전광석화", TYPE_NORMAL, MOVE_PHYSICAL, 40, STAT_NONE, 0}, {"모래뿌리기", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_ACC, -1}, {"몸통박치기", TYPE_NORMAL, MOVE_PHYSICAL, 40, STAT_NONE, 0}, {"물대포", TYPE_WATER, MOVE_SPECIAL, 40, STAT_NONE, 0}, {"꼬리흔들기", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_DEF, -1}, {"물기", TYPE_NORMAL, MOVE_PHYSICAL, 60, STAT_NONE, 0}, {"녹기", TYPE_POISON, MOVE_PHYSICAL, 0, STAT_DEF, 2}, {"흑안개", TYPE_ICE, MOVE_SPECIAL, 0, STAT_NONE, 0}, {"흰안개", TYPE_ICE, MOVE_SPECIAL, 0, STAT_NONE, 0}, {"하이드로펌프", TYPE_WATER, MOVE_SPECIAL, 110, STAT_NONE, 0}};
static Move moveCandidates69[] = {{"전광석화", TYPE_NORMAL, MOVE_PHYSICAL, 40, STAT_NONE, 0}, {"모래뿌리기", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_ACC, -1}, {"몸통박치기", TYPE_NORMAL, MOVE_PHYSICAL, 40, STAT_NONE, 0}, {"전기쇼크", TYPE_ELECTRIC, MOVE_SPECIAL, 40, STAT_NONE, 0}, {"꼬리흔들기", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_DEF, -1}, {"전기자석파", TYPE_ELECTRIC, MOVE_SPECIAL, 0, STAT_NONE, 0}, {"두번치기", TYPE_FIGHTING, MOVE_PHYSICAL, 30, STAT_NONE, 0}, {"고속이동", TYPE_PSYCHIC, MOVE_SPECIAL, 0, STAT_AGI, 2}, {"바늘미사일", TYPE_BUG, MOVE_PHYSICAL, 25, STAT_NONE, 0}, {"번개", TYPE_ELECTRIC, MOVE_SPECIAL, 110, STAT_NONE, 0}};
static Move moveCandidates70[] = {{"불꽃세례", TYPE_FIRE, MOVE_SPECIAL, 40, STAT_NONE, 0}, {"전광석화", TYPE_NORMAL, MOVE_PHYSICAL, 40, STAT_NONE, 0}, {"모래뿌리기", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_ACC, -1}, {"몸통박치기", TYPE_NORMAL, MOVE_PHYSICAL, 40, STAT_NONE, 0}, {"꼬리흔들기", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_DEF, -1}, {"물기", TYPE_NORMAL, MOVE_PHYSICAL, 60, STAT_NONE, 0}, {"째려보기", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_DEF, -1}, {"회오리불꽃", TYPE_FIRE, MOVE_SPECIAL, 35, STAT_NONE, 0}, {"분노", TYPE_NORMAL, MOVE_PHYSICAL, 20, STAT_NONE, 0}, {"화염방사", TYPE_FIRE, MOVE_SPECIAL, 90, STAT_NONE, 0}};
static Move moveCandidates71[] = {{"텍스처", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_NONE, 0}, {"각지기", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_ATK, 1}, {"몸통박치기", TYPE_NORMAL, MOVE_PHYSICAL, 40, STAT_NONE, 0}, {"환상빔", TYPE_PSYCHIC, MOVE_SPECIAL, 65, STAT_NONE, 0}, {"HP회복", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_NONE, 0}, {"고속이동", TYPE_PSYCHIC, MOVE_SPECIAL, 0, STAT_AGI, 2}, {"트라이어택", TYPE_NORMAL, MOVE_PHYSICAL, 80, STAT_NONE, 0}};
static Move moveCandidates72[] = {{"뿔찌르기", TYPE_NORMAL, MOVE_PHYSICAL, 65, STAT_NONE, 0}, {"물대포", TYPE_WATER, MOVE_SPECIAL, 40, STAT_NONE, 0}, {"껍질에숨기", TYPE_WATER, MOVE_SPECIAL, 0, STAT_DEF, 1}, {"째려보기", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_DEF, -1}, {"가시대포", TYPE_NORMAL, MOVE_PHYSICAL, 20, STAT_NONE, 0}, {"하이드로펌프", TYPE_WATER, MOVE_SPECIAL, 110, STAT_NONE, 0}};
static Move moveCandidates73[] = {{"흡수", TYPE_GRASS, MOVE_SPECIAL, 20, STAT_NONE, 0}, {"단단해지기", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_DEF, 1}, {"할퀴기", TYPE_NORMAL, MOVE_PHYSICAL, 40, STAT_NONE, 0}, {"베어가르기", TYPE_NORMAL, MOVE_PHYSICAL, 70, STAT_NONE, 0}, {"째려보기", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_DEF, -1}, {"하이드로펌프", TYPE_WATER, MOVE_SPECIAL, 110, STAT_NONE, 0}};
static Move moveCandidates74[] = {{"고속이동", TYPE_PSYCHIC, MOVE_SPECIAL, 0, STAT_AGI, 2}, {"날개치기", TYPE_FLYING, MOVE_PHYSICAL, 60, STAT_NONE, 0}, {"초음파", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_NONE, 0}, {"물기", TYPE_NORMAL, MOVE_PHYSICAL, 60, STAT_NONE, 0}, {"돌진", TYPE_NORMAL, MOVE_PHYSICAL, 90, STAT_NONE, 0}, {"파괴광선", TYPE_NORMAL, MOVE_PHYSICAL, 150, STAT_NONE, 0}};
static Move moveCandidates75[] = {{"망각술", TYPE_PSYCHIC, MOVE_SPECIAL, 0, STAT_SPD, 2}, {"박치기", TYPE_NORMAL, MOVE_PHYSICAL, 70, STAT_NONE, 0}, {"잠자기", TYPE_PSYCHIC, MOVE_SPECIAL, 0, STAT_NONE, 0}, {"누르기", TYPE_NORMAL, MOVE_PHYSICAL, 85, STAT_NONE, 0}, {"단단해지기", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_DEF, 1}, {"이판사판태클", TYPE_NORMAL, MOVE_PHYSICAL, 120, STAT_NONE, 0}, {"파괴광선", TYPE_NORMAL, MOVE_PHYSICAL, 150, STAT_NONE, 0}};
static Move moveCandidates76[] = {{"냉동빔", TYPE_ICE, MOVE_SPECIAL, 90, STAT_NONE, 0}, {"쪼기", TYPE_FLYING, MOVE_PHYSICAL, 35, STAT_NONE, 0}, {"눈보라", TYPE_ICE, MOVE_SPECIAL, 110, STAT_NONE, 0}, {"고속이동", TYPE_PSYCHIC, MOVE_SPECIAL, 0, STAT_AGI, 2}, {"흰안개", TYPE_ICE, MOVE_SPECIAL, 0, STAT_NONE, 0}};
static Move moveCandidates77[] = {{"회전부리", TYPE_FLYING, MOVE_PHYSICAL, 80, STAT_NONE, 0}, {"전기쇼크", TYPE_ELECTRIC, MOVE_SPECIAL, 40, STAT_NONE, 0}, {"번개", TYPE_ELECTRIC, MOVE_SPECIAL, 110, STAT_NONE, 0}, {"고속이동", TYPE_PSYCHIC, MOVE_SPECIAL, 0, STAT_AGI, 2}, {"빛의장막", TYPE_PSYCHIC, MOVE_SPECIAL, 0, STAT_NONE, 0}};
static Move moveCandidates78[] = {{"회오리불꽃", TYPE_FIRE, MOVE_SPECIAL, 35, STAT_NONE, 0}, {"쪼기", TYPE_FLYING, MOVE_PHYSICAL, 35, STAT_NONE, 0}, {"째려보기", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_DEF, -1}, {"고속이동", TYPE_PSYCHIC, MOVE_SPECIAL, 0, STAT_AGI, 2}, {"불새", TYPE_FLYING, MOVE_PHYSICAL, 140, STAT_NONE, 0}};
static Move moveCandidates79[] = {{"고속이동", TYPE_PSYCHIC, MOVE_SPECIAL, 0, STAT_AGI, 2}, {"째려보기", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_DEF, -1}, {"전기자석파", TYPE_ELECTRIC, MOVE_SPECIAL, 0, STAT_NONE, 0}, {"김밥말이", TYPE_NORMAL, MOVE_PHYSICAL, 15, STAT_NONE, 0}, {"힘껏치기", TYPE_NORMAL, MOVE_PHYSICAL, 80, STAT_NONE, 0}, {"용의분노", TYPE_DRAGON, MOVE_SPECIAL, 0, STAT_NONE, 0}, {"파괴광선", TYPE_NORMAL, MOVE_PHYSICAL, 150, STAT_NONE, 0}};
static Move moveCandidates80[] = {{"염동력", TYPE_PSYCHIC, MOVE_SPECIAL, 50, STAT_NONE, 0}, {"사슬묶기", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_NONE, 0}, {"사이코키네시스", TYPE_PSYCHIC, MOVE_SPECIAL, 90, STAT_SPD, -1}, {"스피드스타", TYPE_NORMAL, MOVE_PHYSICAL, 60, STAT_NONE, 0}, {"배리어", TYPE_PSYCHIC, MOVE_SPECIAL, 0, STAT_DEF, 2}, {"HP회복", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_NONE, 0}, {"흰안개", TYPE_ICE, MOVE_SPECIAL, 0, STAT_NONE, 0}, {"망각술", TYPE_PSYCHIC, MOVE_SPECIAL, 0, STAT_SPD, 2}};
static Move moveCandidates81[] = {{"막치기", TYPE_NORMAL, MOVE_PHYSICAL, 40, STAT_NONE, 0}, {"변신", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_NONE, 0}, {"메가톤펀치", TYPE_NORMAL, MOVE_PHYSICAL, 80, STAT_NONE, 0}, {"손가락흔들기", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_NONE, 0}, {"사이코키네시스", TYPE_PSYCHIC, MOVE_SPECIAL, 90, STAT_SPD, -1}};

typedef struct {
    Move *moves;
    int count;
} MoveCandidateSet;

typedef struct {
    const char *name;
    int accuracy;
} MoveAccuracy;

static MoveCandidateSet levelUpMoveTable[POKEMON_COUNT] = {
    {moveCandidates1, (int)(sizeof(moveCandidates1) / sizeof(moveCandidates1[0]))},
    {moveCandidates2, (int)(sizeof(moveCandidates2) / sizeof(moveCandidates2[0]))},
    {moveCandidates3, (int)(sizeof(moveCandidates3) / sizeof(moveCandidates3[0]))},
    {moveCandidates4, (int)(sizeof(moveCandidates4) / sizeof(moveCandidates4[0]))},
    {moveCandidates5, (int)(sizeof(moveCandidates5) / sizeof(moveCandidates5[0]))},
    {moveCandidates6, (int)(sizeof(moveCandidates6) / sizeof(moveCandidates6[0]))},
    {moveCandidates7, (int)(sizeof(moveCandidates7) / sizeof(moveCandidates7[0]))},
    {moveCandidates8, (int)(sizeof(moveCandidates8) / sizeof(moveCandidates8[0]))},
    {moveCandidates9, (int)(sizeof(moveCandidates9) / sizeof(moveCandidates9[0]))},
    {moveCandidates10, (int)(sizeof(moveCandidates10) / sizeof(moveCandidates10[0]))},
    {moveCandidates11, (int)(sizeof(moveCandidates11) / sizeof(moveCandidates11[0]))},
    {moveCandidates12, (int)(sizeof(moveCandidates12) / sizeof(moveCandidates12[0]))},
    {moveCandidates13, (int)(sizeof(moveCandidates13) / sizeof(moveCandidates13[0]))},
    {moveCandidates14, (int)(sizeof(moveCandidates14) / sizeof(moveCandidates14[0]))},
    {moveCandidates15, (int)(sizeof(moveCandidates15) / sizeof(moveCandidates15[0]))},
    {moveCandidates16, (int)(sizeof(moveCandidates16) / sizeof(moveCandidates16[0]))},
    {moveCandidates17, (int)(sizeof(moveCandidates17) / sizeof(moveCandidates17[0]))},
    {moveCandidates18, (int)(sizeof(moveCandidates18) / sizeof(moveCandidates18[0]))},
    {moveCandidates19, (int)(sizeof(moveCandidates19) / sizeof(moveCandidates19[0]))},
    {moveCandidates20, (int)(sizeof(moveCandidates20) / sizeof(moveCandidates20[0]))},
    {moveCandidates21, (int)(sizeof(moveCandidates21) / sizeof(moveCandidates21[0]))},
    {moveCandidates22, (int)(sizeof(moveCandidates22) / sizeof(moveCandidates22[0]))},
    {moveCandidates23, (int)(sizeof(moveCandidates23) / sizeof(moveCandidates23[0]))},
    {moveCandidates24, (int)(sizeof(moveCandidates24) / sizeof(moveCandidates24[0]))},
    {moveCandidates25, (int)(sizeof(moveCandidates25) / sizeof(moveCandidates25[0]))},
    {moveCandidates26, (int)(sizeof(moveCandidates26) / sizeof(moveCandidates26[0]))},
    {moveCandidates27, (int)(sizeof(moveCandidates27) / sizeof(moveCandidates27[0]))},
    {moveCandidates28, (int)(sizeof(moveCandidates28) / sizeof(moveCandidates28[0]))},
    {moveCandidates29, (int)(sizeof(moveCandidates29) / sizeof(moveCandidates29[0]))},
    {moveCandidates30, (int)(sizeof(moveCandidates30) / sizeof(moveCandidates30[0]))},
    {moveCandidates31, (int)(sizeof(moveCandidates31) / sizeof(moveCandidates31[0]))},
    {moveCandidates32, (int)(sizeof(moveCandidates32) / sizeof(moveCandidates32[0]))},
    {moveCandidates33, (int)(sizeof(moveCandidates33) / sizeof(moveCandidates33[0]))},
    {moveCandidates34, (int)(sizeof(moveCandidates34) / sizeof(moveCandidates34[0]))},
    {moveCandidates35, (int)(sizeof(moveCandidates35) / sizeof(moveCandidates35[0]))},
    {moveCandidates36, (int)(sizeof(moveCandidates36) / sizeof(moveCandidates36[0]))},
    {moveCandidates37, (int)(sizeof(moveCandidates37) / sizeof(moveCandidates37[0]))},
    {moveCandidates38, (int)(sizeof(moveCandidates38) / sizeof(moveCandidates38[0]))},
    {moveCandidates39, (int)(sizeof(moveCandidates39) / sizeof(moveCandidates39[0]))},
    {moveCandidates40, (int)(sizeof(moveCandidates40) / sizeof(moveCandidates40[0]))},
    {moveCandidates41, (int)(sizeof(moveCandidates41) / sizeof(moveCandidates41[0]))},
    {moveCandidates42, (int)(sizeof(moveCandidates42) / sizeof(moveCandidates42[0]))},
    {moveCandidates43, (int)(sizeof(moveCandidates43) / sizeof(moveCandidates43[0]))},
    {moveCandidates44, (int)(sizeof(moveCandidates44) / sizeof(moveCandidates44[0]))},
    {moveCandidates45, (int)(sizeof(moveCandidates45) / sizeof(moveCandidates45[0]))},
    {moveCandidates46, (int)(sizeof(moveCandidates46) / sizeof(moveCandidates46[0]))},
    {moveCandidates47, (int)(sizeof(moveCandidates47) / sizeof(moveCandidates47[0]))},
    {moveCandidates48, (int)(sizeof(moveCandidates48) / sizeof(moveCandidates48[0]))},
    {moveCandidates49, (int)(sizeof(moveCandidates49) / sizeof(moveCandidates49[0]))},
    {moveCandidates50, (int)(sizeof(moveCandidates50) / sizeof(moveCandidates50[0]))},
    {moveCandidates51, (int)(sizeof(moveCandidates51) / sizeof(moveCandidates51[0]))},
    {moveCandidates52, (int)(sizeof(moveCandidates52) / sizeof(moveCandidates52[0]))},
    {moveCandidates53, (int)(sizeof(moveCandidates53) / sizeof(moveCandidates53[0]))},
    {moveCandidates54, (int)(sizeof(moveCandidates54) / sizeof(moveCandidates54[0]))},
    {moveCandidates55, (int)(sizeof(moveCandidates55) / sizeof(moveCandidates55[0]))},
    {moveCandidates56, (int)(sizeof(moveCandidates56) / sizeof(moveCandidates56[0]))},
    {moveCandidates57, (int)(sizeof(moveCandidates57) / sizeof(moveCandidates57[0]))},
    {moveCandidates58, (int)(sizeof(moveCandidates58) / sizeof(moveCandidates58[0]))},
    {moveCandidates59, (int)(sizeof(moveCandidates59) / sizeof(moveCandidates59[0]))},
    {moveCandidates60, (int)(sizeof(moveCandidates60) / sizeof(moveCandidates60[0]))},
    {moveCandidates61, (int)(sizeof(moveCandidates61) / sizeof(moveCandidates61[0]))},
    {moveCandidates62, (int)(sizeof(moveCandidates62) / sizeof(moveCandidates62[0]))},
    {moveCandidates63, (int)(sizeof(moveCandidates63) / sizeof(moveCandidates63[0]))},
    {moveCandidates64, (int)(sizeof(moveCandidates64) / sizeof(moveCandidates64[0]))},
    {moveCandidates65, (int)(sizeof(moveCandidates65) / sizeof(moveCandidates65[0]))},
    {moveCandidates66, (int)(sizeof(moveCandidates66) / sizeof(moveCandidates66[0]))},
    {moveCandidates67, (int)(sizeof(moveCandidates67) / sizeof(moveCandidates67[0]))},
    {moveCandidates68, (int)(sizeof(moveCandidates68) / sizeof(moveCandidates68[0]))},
    {moveCandidates69, (int)(sizeof(moveCandidates69) / sizeof(moveCandidates69[0]))},
    {moveCandidates70, (int)(sizeof(moveCandidates70) / sizeof(moveCandidates70[0]))},
    {moveCandidates71, (int)(sizeof(moveCandidates71) / sizeof(moveCandidates71[0]))},
    {moveCandidates72, (int)(sizeof(moveCandidates72) / sizeof(moveCandidates72[0]))},
    {moveCandidates73, (int)(sizeof(moveCandidates73) / sizeof(moveCandidates73[0]))},
    {moveCandidates74, (int)(sizeof(moveCandidates74) / sizeof(moveCandidates74[0]))},
    {moveCandidates75, (int)(sizeof(moveCandidates75) / sizeof(moveCandidates75[0]))},
    {moveCandidates76, (int)(sizeof(moveCandidates76) / sizeof(moveCandidates76[0]))},
    {moveCandidates77, (int)(sizeof(moveCandidates77) / sizeof(moveCandidates77[0]))},
    {moveCandidates78, (int)(sizeof(moveCandidates78) / sizeof(moveCandidates78[0]))},
    {moveCandidates79, (int)(sizeof(moveCandidates79) / sizeof(moveCandidates79[0]))},
    {moveCandidates80, (int)(sizeof(moveCandidates80) / sizeof(moveCandidates80[0]))},
    {moveCandidates81, (int)(sizeof(moveCandidates81) / sizeof(moveCandidates81[0]))},
};

/* 1세대 기준 기술별 기본 명중률입니다. 표에 없으면 100%로 취급합니다. */
static MoveAccuracy moveAccuracyTable[] = {
    {"악마의키스", 75},
    {"가위자르기", 30},
    {"구슬던지기", 85},
    {"김밥말이", 85},
    {"껍질끼우기", 75},
    {"나이트헤드", 100},
    {"난동부리기", 100},
    {"날려버리기", 85},
    {"노래하기", 55},
    {"눈보라", 90},
    {"대폭발", 100},
    {"독가루", 75},
    {"독가스", 55},
    {"돌려차기", 85},
    {"돌떨구기", 90},
    {"돌진", 85},
    {"땅가르기", 30},
    {"뼈다귀치기", 85},
    {"뼈다귀부메랑", 90},
    {"뿔드릴", 30},
    {"뱀눈초리", 75},
    {"불새", 90},
    {"사슬묶기", 55},
    {"수면가루", 75},
    {"스모그", 70},
    {"싫은소리", 85},
    {"안다리걸기", 90},
    {"연속뺨치기", 85},
    {"연속펀치", 85},
    {"용의분노", 100},
    {"울부짖기", 100},
    {"자폭", 100},
    {"점프킥", 95},
    {"조이기", 85},
    {"지구던지기", 100},
    {"지옥의바퀴", 85},
    {"찝게햄머", 85},
    {"최면술", 60},
    {"초음파", 55},
    {"파괴광선", 90},
    {"하이드로펌프", 80},
    {"회오리불꽃", 70},
    {"휘감기", 85},
    {"무릎차기", 90}
};

/* 사천왕과 챔피언은 원작 레드/블루 기준 기술을 고정으로 사용합니다. */
Move loreleiDewgongMoves[] = {{"울음소리", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_ATK, -1}, {"오로라빔", TYPE_ICE, MOVE_SPECIAL, 65, STAT_ATK, -1}, {"잠자기", TYPE_PSYCHIC, MOVE_SPECIAL, 0, STAT_NONE, 0}, {"돌진", TYPE_NORMAL, MOVE_PHYSICAL, 90, STAT_NONE, 0}};
Move loreleiCloysterMoves[] = {{"오로라빔", TYPE_ICE, MOVE_SPECIAL, 65, STAT_ATK, -1}, {"껍질에숨기", TYPE_WATER, MOVE_SPECIAL, 0, STAT_DEF, 1}, {"껍질끼우기", TYPE_WATER, MOVE_SPECIAL, 35, STAT_NONE, 0}, {"가시대포", TYPE_NORMAL, MOVE_PHYSICAL, 20, STAT_NONE, 0}};
Move loreleiSlowbroMoves[] = {{"울음소리", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_ATK, -1}, {"물대포", TYPE_WATER, MOVE_SPECIAL, 40, STAT_NONE, 0}, {"껍질에숨기", TYPE_WATER, MOVE_SPECIAL, 0, STAT_DEF, 1}, {"망각술", TYPE_PSYCHIC, MOVE_SPECIAL, 0, STAT_SPD, 2}};
Move loreleiJynxMoves[] = {{"연속뺨치기", TYPE_NORMAL, MOVE_PHYSICAL, 15, STAT_NONE, 0}, {"냉동펀치", TYPE_ICE, MOVE_SPECIAL, 75, STAT_NONE, 0}, {"누르기", TYPE_NORMAL, MOVE_PHYSICAL, 85, STAT_NONE, 0}, {"난동부리기", TYPE_NORMAL, MOVE_PHYSICAL, 120, STAT_NONE, 0}};
Move loreleiLaprasMoves[] = {{"누르기", TYPE_NORMAL, MOVE_PHYSICAL, 85, STAT_NONE, 0}, {"이상한빛", TYPE_GHOST, MOVE_PHYSICAL, 0, STAT_NONE, 0}, {"냉동빔", TYPE_ICE, MOVE_SPECIAL, 90, STAT_NONE, 0}, {"하이드로펌프", TYPE_WATER, MOVE_SPECIAL, 110, STAT_NONE, 0}};
Move brunoOnix53Moves[] = {{"돌떨구기", TYPE_ROCK, MOVE_PHYSICAL, 50, STAT_NONE, 0}, {"분노", TYPE_NORMAL, MOVE_PHYSICAL, 20, STAT_NONE, 0}, {"힘껏치기", TYPE_NORMAL, MOVE_PHYSICAL, 80, STAT_NONE, 0}, {"단단해지기", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_DEF, 1}};
Move brunoHitmonchanMoves[] = {{"냉동펀치", TYPE_ICE, MOVE_SPECIAL, 75, STAT_NONE, 0}, {"번개펀치", TYPE_ELECTRIC, MOVE_SPECIAL, 75, STAT_NONE, 0}, {"메가톤펀치", TYPE_NORMAL, MOVE_PHYSICAL, 80, STAT_NONE, 0}, {"카운터", TYPE_FIGHTING, MOVE_PHYSICAL, 0, STAT_NONE, 0}};
Move brunoHitmonleeMoves[] = {{"점프킥", TYPE_FIGHTING, MOVE_PHYSICAL, 70, STAT_NONE, 0}, {"기충전", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_NONE, 0}, {"무릎차기", TYPE_FIGHTING, MOVE_PHYSICAL, 85, STAT_NONE, 0}, {"메가톤킥", TYPE_NORMAL, MOVE_PHYSICAL, 120, STAT_NONE, 0}};
Move brunoOnix56Moves[] = {{"돌떨구기", TYPE_ROCK, MOVE_PHYSICAL, 50, STAT_NONE, 0}, {"분노", TYPE_NORMAL, MOVE_PHYSICAL, 20, STAT_NONE, 0}, {"힘껏치기", TYPE_NORMAL, MOVE_PHYSICAL, 80, STAT_NONE, 0}, {"단단해지기", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_DEF, 1}};
Move brunoMachampMoves[] = {{"안다리걸기", TYPE_FIGHTING, MOVE_PHYSICAL, 50, STAT_NONE, 0}, {"기충전", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_NONE, 0}, {"지구던지기", TYPE_FIGHTING, MOVE_PHYSICAL, 0, STAT_NONE, 0}, {"지옥의바퀴", TYPE_FIGHTING, MOVE_PHYSICAL, 80, STAT_NONE, 0}};
Move agathaGengar56Moves[] = {{"이상한빛", TYPE_GHOST, MOVE_PHYSICAL, 0, STAT_NONE, 0}, {"핥기", TYPE_GHOST, MOVE_PHYSICAL, 30, STAT_NONE, 0}, {"최면술", TYPE_PSYCHIC, MOVE_SPECIAL, 0, STAT_NONE, 0}, {"꿈먹기", TYPE_PSYCHIC, MOVE_SPECIAL, 100, STAT_NONE, 0}};
Move agathaGolbatMoves[] = {{"초음파", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_NONE, 0}, {"이상한빛", TYPE_GHOST, MOVE_PHYSICAL, 0, STAT_NONE, 0}, {"날개치기", TYPE_FLYING, MOVE_PHYSICAL, 60, STAT_NONE, 0}, {"흰안개", TYPE_ICE, MOVE_SPECIAL, 0, STAT_NONE, 0}};
Move agathaHaunterMoves[] = {{"이상한빛", TYPE_GHOST, MOVE_PHYSICAL, 0, STAT_NONE, 0}, {"핥기", TYPE_GHOST, MOVE_PHYSICAL, 30, STAT_NONE, 0}, {"최면술", TYPE_PSYCHIC, MOVE_SPECIAL, 0, STAT_NONE, 0}, {"꿈먹기", TYPE_PSYCHIC, MOVE_SPECIAL, 100, STAT_NONE, 0}};
Move agathaArbokMoves[] = {{"물기", TYPE_NORMAL, MOVE_SPECIAL, 60, STAT_NONE, 0}, {"뱀눈초리", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_NONE, 0}, {"싫은소리", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_DEF, -2}, {"용해액", TYPE_POISON, MOVE_PHYSICAL, 40, STAT_SPD, -1}};
Move agathaGengar60Moves[] = {{"이상한빛", TYPE_GHOST, MOVE_PHYSICAL, 0, STAT_NONE, 0}, {"핥기", TYPE_GHOST, MOVE_PHYSICAL, 30, STAT_NONE, 0}, {"최면술", TYPE_PSYCHIC, MOVE_SPECIAL, 0, STAT_NONE, 0}, {"꿈먹기", TYPE_PSYCHIC, MOVE_SPECIAL, 100, STAT_NONE, 0}};
Move lanceGyaradosMoves[] = {{"물기", TYPE_NORMAL, MOVE_SPECIAL, 60, STAT_NONE, 0}, {"하이드로펌프", TYPE_WATER, MOVE_SPECIAL, 110, STAT_NONE, 0}, {"용의분노", TYPE_DRAGON, MOVE_SPECIAL, 1, STAT_NONE, 0}, {"파괴광선", TYPE_NORMAL, MOVE_PHYSICAL, 150, STAT_NONE, 0}};
Move lanceDragonairMoves[] = {{"고속이동", TYPE_PSYCHIC, MOVE_SPECIAL, 0, STAT_AGI, 2}, {"힘껏치기", TYPE_NORMAL, MOVE_PHYSICAL, 80, STAT_NONE, 0}, {"용의분노", TYPE_DRAGON, MOVE_SPECIAL, 1, STAT_NONE, 0}, {"파괴광선", TYPE_NORMAL, MOVE_PHYSICAL, 150, STAT_NONE, 0}};
Move lanceAerodactylMoves[] = {{"초음파", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_NONE, 0}, {"물기", TYPE_NORMAL, MOVE_SPECIAL, 60, STAT_NONE, 0}, {"돌진", TYPE_NORMAL, MOVE_PHYSICAL, 90, STAT_NONE, 0}, {"파괴광선", TYPE_NORMAL, MOVE_PHYSICAL, 150, STAT_NONE, 0}};
Move lanceDragoniteMoves[] = {{"고속이동", TYPE_PSYCHIC, MOVE_SPECIAL, 0, STAT_AGI, 2}, {"힘껏치기", TYPE_NORMAL, MOVE_PHYSICAL, 80, STAT_NONE, 0}, {"용의분노", TYPE_DRAGON, MOVE_SPECIAL, 1, STAT_NONE, 0}, {"파괴광선", TYPE_NORMAL, MOVE_PHYSICAL, 150, STAT_NONE, 0}};
Move championPidgeotMoves[] = {{"날려버리기", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_NONE, 0}, {"날개치기", TYPE_FLYING, MOVE_PHYSICAL, 60, STAT_NONE, 0}, {"고속이동", TYPE_PSYCHIC, MOVE_SPECIAL, 0, STAT_AGI, 2}, {"따라하기", TYPE_FLYING, MOVE_PHYSICAL, 0, STAT_NONE, 0}};
Move championAlakazamMoves[] = {{"환상빔", TYPE_PSYCHIC, MOVE_SPECIAL, 65, STAT_NONE, 0}, {"HP회복", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_NONE, 0}, {"사이코키네시스", TYPE_PSYCHIC, MOVE_SPECIAL, 90, STAT_SPD, -1}, {"리플렉터", TYPE_PSYCHIC, MOVE_SPECIAL, 0, STAT_NONE, 0}};
Move championRhydonMoves[] = {{"마구찌르기", TYPE_NORMAL, MOVE_PHYSICAL, 15, STAT_NONE, 0}, {"꼬리흔들기", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_DEF, -1}, {"뿔드릴", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_NONE, 0}, {"째려보기", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_DEF, -1}};
Move championArcanineMoves[] = {{"돌진", TYPE_NORMAL, MOVE_PHYSICAL, 90, STAT_NONE, 0}, {"째려보기", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_DEF, -1}, {"울부짖기", TYPE_NORMAL, MOVE_PHYSICAL, 0, STAT_NONE, 0}, {"불꽃세례", TYPE_FIRE, MOVE_SPECIAL, 40, STAT_NONE, 0}};
Move championExeggutorMoves[] = {{"최면술", TYPE_PSYCHIC, MOVE_SPECIAL, 0, STAT_NONE, 0}, {"구슬던지기", TYPE_NORMAL, MOVE_PHYSICAL, 15, STAT_NONE, 0}, {"짓밟기", TYPE_NORMAL, MOVE_PHYSICAL, 65, STAT_NONE, 0}};
Move championBlastoiseMoves[] = {{"물기", TYPE_NORMAL, MOVE_SPECIAL, 60, STAT_NONE, 0}, {"껍질에숨기", TYPE_WATER, MOVE_SPECIAL, 0, STAT_DEF, 1}, {"로켓박치기", TYPE_NORMAL, MOVE_PHYSICAL, 130, STAT_NONE, 0}, {"하이드로펌프", TYPE_WATER, MOVE_SPECIAL, 110, STAT_NONE, 0}};

const char *getStatusName(StatusCondition status);

void clearMoves(BattlePokemon *pokemon)
{
    pokemon->moveCount = 0;
}

/* 기술칸이 남아 있으면 기술 하나를 추가합니다. */
void addMove(BattlePokemon *pokemon, Move move)
{
    if (pokemon->moveCount < MOVE_SLOT_COUNT) {
        pokemon->moves[pokemon->moveCount] = move;
        pokemon->moveCount++;
    }
}

/* Fisher-Yates 방식으로 정수 배열을 섞습니다. */
void shuffleIntArray(int array[], int size)
{
    for (int i = size - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        int temp = array[i];
        array[i] = array[j];
        array[j] = temp;
    }
}

/* 후보 기술 목록에서 중복 없이 최대 4개를 랜덤으로 골라 배치합니다. */
void setRandomMoves(BattlePokemon *pokemon, Move candidates[], int candidateCount)
{
    int indexes[64];
    int selectCount;

    clearMoves(pokemon);

    if (candidateCount > 64) {
        candidateCount = 64;
    }

    selectCount = candidateCount;
    if (selectCount > MOVE_SLOT_COUNT) {
        selectCount = MOVE_SLOT_COUNT;
    }

    for (int i = 0; i < candidateCount; i++) {
        indexes[i] = i;
    }

    shuffleIntArray(indexes, candidateCount);

    for (int i = 0; i < selectCount; i++) {
        addMove(pokemon, candidates[indexes[i]]);
    }
}

/* 포켓몬 id에 맞는 레벨업 기술 후보 배열을 찾아 랜덤 기술배치를 만듭니다. */
void setRandomLevelUpMoves(BattlePokemon *pokemon)
{
    int pokemonId = pokemon->pokemon.id;
    MoveCandidateSet candidateSet;

    if (pokemonId < 1 || pokemonId > POKEMON_COUNT) {
        clearMoves(pokemon);
        return;
    }

    candidateSet = levelUpMoveTable[pokemonId - 1];
    setRandomMoves(pokemon, candidateSet.moves, candidateSet.count);
}


void setFixedMoves(BattlePokemon *pokemon, Move moves[], int moveCount)
{
    clearMoves(pokemon);

    if (moveCount > MOVE_SLOT_COUNT) {
        moveCount = MOVE_SLOT_COUNT;
    }

    for (int i = 0; i < moveCount; i++) {
        addMove(pokemon, moves[i]);
    }
}


Move getFallbackMove(void)
{
    Move move = {"발버둥", TYPE_NORMAL, MOVE_PHYSICAL, 50, STAT_NONE, 0};
    return move;
}


int getMoveAccuracy(Move move)
{
    int count = (int)(sizeof(moveAccuracyTable) / sizeof(moveAccuracyTable[0]));

    for (int i = 0; i < count; i++) {
        if (strcmp(move.name, moveAccuracyTable[i].name) == 0) {
            return moveAccuracyTable[i].accuracy;
        }
    }

    return 100;
}

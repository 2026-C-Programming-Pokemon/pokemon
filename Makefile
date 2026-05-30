# 외부 라이브러리 의존성 없음. LLM 호출은 시스템에 깔린 curl 바이너리를 쓴다.
#
# .env 가 있으면 자동 로드 (KEY=VALUE 형식, # 로 시작하는 줄은 주석).
# `make run` 등 자식 프로세스에도 환경변수가 전달되도록 export 한다.
ifneq (,$(wildcard .env))
  include .env
  export $(shell sed -nE 's/^[[:space:]]*([A-Za-z_][A-Za-z0-9_]*)[[:space:]]*=.*/\1/p' .env)
endif

CC      = cc
CFLAGS  = -Wall -Wextra -std=c99 -O2 -I. -Illm

SRC     = pokemon.c dogam/dogam.c skill/skill.c battlelogic/battlelogic.c entry/entry.c llm/llm.c score/score.c
OBJ     = $(SRC:.c=.o)
BIN     = pokemon

.PHONY: all clean run

all: $(BIN)

$(BIN): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJ) $(BIN)

run: $(BIN)
	@echo "[LLM 로그는 llm.log 에 쌓입니다. 다른 터미널에서 'tail -f llm.log' 추천]"
	./$(BIN) 2>llm.log

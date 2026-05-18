# LLM=1 (기본): LLM 호출(POSIX 소켓 HTTP) 포함.
# LLM=0       : LLM 모듈을 스텁으로 컴파일.
# 어느 쪽이든 외부 라이브러리 의존성이 없어 추가 설치 없이 빌드됩니다.
LLM    ?= 1

# .env 가 있으면 자동 로드 (KEY=VALUE 형식, # 로 시작하는 줄은 주석).
# `make run` 등 자식 프로세스에도 환경변수가 전달되도록 export 합니다.
ifneq (,$(wildcard .env))
  include .env
  export $(shell sed -nE 's/^[[:space:]]*([A-Za-z_][A-Za-z0-9_]*)[[:space:]]*=.*/\1/p' .env)
endif

CC      = cc
CFLAGS  = -Wall -Wextra -std=c99 -O2 -I. -Illm
LDFLAGS =
LDLIBS  =

ifeq ($(LLM),0)
  CFLAGS += -DLLM_DISABLED
endif

SRC     = pokemon.c dogam/dogam.c skill/skill.c battlelogic/battlelogic.c entry/entry.c llm/llm.c llm/sha256.c llm/hmac_sha256.c score/score.c
OBJ     = $(SRC:.c=.o)
BIN     = pokemon

.PHONY: all clean run

all: $(BIN)

$(BIN): $(OBJ)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJ) $(BIN)

run: $(BIN)
	./$(BIN)

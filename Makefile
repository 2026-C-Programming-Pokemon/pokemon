CC := cc
CFLAGS := -Wall -Wextra -std=c11
CPPFLAGS := -I. -Idogam -Iskill -Ientry -Ibattlelogic
TARGET := pokemon
SRCS := pokemon.c dogam/dogam.c skill/skill.c entry/entry.c battlelogic/battlelogic.c

$(TARGET): $(SRCS) pokemon.h dogam/dogam.h skill/skill.h entry/entry.h battlelogic/battlelogic.h
	$(CC) $(CPPFLAGS) $(CFLAGS) $(SRCS) -o $(TARGET)

clean:
	rm -f $(TARGET)

.PHONY: clean

MKDIR   := mkdir
RMDIR   := rm -r

EXE     := nyansnake
DEV_EXE := devbuild
SRC     := src
INCLUDE := include
OBJ     := obj
DEV_OBJ := dev-obj

CC := cc
CFLAGS := -std=c99 -lc -lm -lcurses -pedantic-errors -Wall -I$(INCLUDE)
RELEASE_FLAGS := -O3
DEV_FLAGS := -Og -g

SRCS := $(wildcard $(SRC)/*.c)
OBJS := $(patsubst $(SRC)/%.c,$(OBJ)/%.o,$(SRCS))
DEV_OBJS := $(patsubst $(SRC)/%.c,$(DEV_OBJ)/%.o,$(SRCS))

.PHONY: all run devbuild clean

all: $(EXE)

$(EXE): $(OBJS) $(EXE).o
	$(CC) $(RELEASE_FLAGS) $^ $(CFLAGS) -o $@

$(DEV_EXE): $(DEV_OBJS) $(DEV_EXE).o
	$(CC) $(DEV_FLAGS) $^ $(CFLAGS) -o $@

$(OBJ)/%.o: $(SRC)/%.c | $(OBJ)
	$(CC) $(RELEASE_FLAGS) -c $< $(CFLAGS) -o $@

$(DEV_OBJ)/%.o: $(SRC)/%.c | $(DEV_OBJ)
	$(CC) $(DEV_FLAGS) -c $< $(CFLAGS) -o $@

$(EXE).o: $(EXE).c
	$(CC) $(RELEASE_FLAGS) -c $< $(CFLAGS) -o $@

$(DEV_EXE).o: $(EXE).c
	$(CC) $(DEV_FLAGS) -c $< $(CFLAGS) -o $@

$(OBJ):
	$(MKDIR) $@

$(DEV_OBJ):
	$(MKDIR) $@

clean:
	if [ -f $(EXE).o ]; then rm $(EXE).o; fi
	if [ -f $(EXE) ]; then rm $(EXE); fi
	if [ -f $(DEV_EXE).o ]; then rm $(DEV_EXE).o; fi
	if [ -f $(DEV_EXE) ]; then rm $(DEV_EXE); fi
	if [ -d $(OBJ) ]; then $(RMDIR) $(OBJ); fi
	if [ -d $(DEV_OBJ) ]; then $(RMDIR) $(DEV_OBJ); fi


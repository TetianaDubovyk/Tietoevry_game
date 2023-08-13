CC = gcc
CFLAGS = -Wall -lm

# Directories and source files for each project
MEDIATOR_DIR = mediator
PLAYER_DIR = player

MEDIATOR_SRCS = $(wildcard $(MEDIATOR_DIR)/*.c)
PLAYER_SRCS = $(wildcard $(PLAYER_DIR)/*.c)

# Output directory for the .exe files
OUTPUT_DIR = mediator

# Target names for the executables
MEDIATOR_EXE = $(OUTPUT_DIR)/mediator.exe
PLAYER_EXE = $(OUTPUT_DIR)/player.exe

all: $(MEDIATOR_EXE) $(PLAYER_EXE)

$(MEDIATOR_EXE): $(MEDIATOR_SRCS)
	$(CC) $(CFLAGS) -o $@ $(MEDIATOR_SRCS)

$(PLAYER_EXE): $(PLAYER_SRCS)
	$(CC) -o $@ $(PLAYER_SRCS) $(CFLAGS)

clean:
	rm -f $(MEDIATOR_EXE) $(PLAYER_EXE)
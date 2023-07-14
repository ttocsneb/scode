# CC := zig cc
# AR := zig ar
SRC := .
TST := test
OBJ := .out

TEST := $(OBJ)/test
LIB := $(OBJ)/scode.a

SRC_FILES = scode.c
OBJ_FILES = $(patsubst %.c,$(OBJ)/%.o,$(SRC_FILES))
HDR_FILES = scode.h
TST_FILES = $(wildcard $(TST)/*.c)

FLAGS = -I.
DEBUG_FLAGS = -Imunit

ifdef DEBUG
	CFLAGS = $(FLAGS) -g
	DEBUG_CFLAGS = $(DEBUG_FLAGS) 
else
	CFLAGS = $(FLAGS)
	DEBUG_CFLAGS = $(DEBUG_FLAGS)
endif

CXXFLAGS = -std=c++11

all: $(TEST) $(LIB) $(OBJ)/echo $(OBJ)/echocpp
lib: $(LIB)

test: $(TEST)
	$(TEST)

echo: $(OBJ)/echo
	$(OBJ)/echo

echocpp: $(OBJ)/echocpp
	$(OBJ)/echocpp

$(TEST): $(LIB) $(TST_FILES) munit/munit.c
	$(CC) $(CFLAGS) $(DEBUG_CFLAGS) -o $@ $^

$(OBJ)/echo: $(LIB) examples/echo.c
	$(CC) $(CFLAGS) -o $@ $^

$(OBJ)/echocpp: $(LIB) examples/echo.cpp
	$(CXX) $(CXXFLAGS) $(CFLAGS) -o $@ $^

$(LIB): $(OBJ_FILES)
	@mkdir -p $(OBJ)
	$(AR) -crs $@ $^

$(OBJ)/%.o: %.c $(HDR_FILES)
	@mkdir -p $(OBJ)
	$(CC) -c $(CFLAGS) -o $@ $<

compile_commands: 
	bear -- make clean all CC=cc AR=ar

clean:
	rm -rf $(OBJ) 

.PHONY: all lib test compile_commands clean echo

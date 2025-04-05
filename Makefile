CFLAGS = -std=c++20 -I../ba-graph/include
DBG_FLAGS = -g -O0 -pedantic -Wall -Wextra -DBA_GRAPH_DEBUG
COMPILE = $(CXX) $(CFLAGS) -O3
COMPILE_DBG = $(CXX) $(CFLAGS) $(DBG_FLAGS)
CMSAT_FLAGS = -DCOMPILE_WITH_CRYPTOMINISAT -lcryptominisat5 -lbreakid

TEST_VERSION ?= SAT

all: main

main: main.cpp
	$(COMPILE) main.cpp -o main.out $(CMSAT_FLAGS)
main_dbg: main.cpp
	$(COMPILE_DBG) main.cpp -o main.out $(CMSAT_FLAGS)

test_backtr:
	make test VERSION=BACKTR
test_sat:
	make test VERSION=SAT

test: test_ecd.cpp
	$(COMPILE_DBG) test_ecd.cpp -o test_ecd.out -D$(TEST_VERSION) $(CMSAT_FLAGS)

test_time: test_time.cpp
	$(COMPILE) test_time.cpp -o test_time.out $(CMSAT_FLAGS)

test_time_dbg: test_time.cpp
	$(COMPILE_DBG) test_time.cpp -o test_time.out $(CMSAT_FLAGS)

clean:
	rm -rf *.out

.PHONY: clean all

CFLAGS = -std=c++20 -I../ba-graph/include
DBGFLAGS = -g -O0 -pedantic -Wall -Wextra -DBA_GRAPH_DEBUG
COMPILE_DBG = $(CXX) $(CFLAGS) $(DBGFLAGS)

TEST_VERSION ?= SAT

all: main

main: main.cpp
	$(COMPILE_DBG) main.cpp -o main.out -DCOMPILE_WITH_CRYPTOMINISAT -lcryptominisat5

test_backtr:
	make test VERSION=BACKTR
test_sat:
	make test VERSION=SAT

test: test_ecd.cpp
	$(COMPILE_DBG) test_ecd.cpp -o test_ecd.out -D$(TEST_VERSION) -DCOMPILE_WITH_CRYPTOMINISAT -lcryptominisat5

test_time: test_time.cpp
	$(COMPILE_DBG) test_time.cpp -o test_time.out -D$(TEST_VERSION) -DCOMPILE_WITH_CRYPTOMINISAT -lcryptominisat5

clean:
	rm -rf *.out

.PHONY: clean all

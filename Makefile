CFLAGS = -std=c++20 -I../ba-graph/include
DBGFLAGS = -g -O0 -pedantic -Wall -Wextra -DBA_GRAPH_DEBUG
COMPILE_DBG = $(CXX) $(CFLAGS) $(DBGFLAGS)

TEST_VERSION ?= SAT

all: test_ecd

test_backtr:
	make test VERSION=BACKTR
test_sat:
	make test VERSION=SAT

test_ecd: test_ecd.cpp
	$(COMPILE_DBG) test_ecd.cpp -o test_ecd.out -D$(TEST_VERSION) -DCOMPILE_WITH_CRYPTOMINISAT -lcryptominisat5
	./test_ecd.out

clean:
	rm -rf *.out

.PHONY: clean all

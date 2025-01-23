CFLAGS = -std=c++20 -I../ba-graph/include
DBGFLAGS = -g -O0 -pedantic -Wall -Wextra -DBA_GRAPH_DEBUG -D_GLIBCXX_DEBUG
COMPILE_DBG = $(CXX) $(CFLAGS) $(DBGFLAGS)

all: test_ecd


test_ecd: test_ecd.cpp
	$(COMPILE_DBG) test_ecd.cpp -o test_ecd.out


clean:
	rm -rf test_ecd.out


.PHONY: clean all

CFLAGS = -std=c++20 -I../ba-graph/include
DBGFLAGS = -g -O0 -pedantic -Wall -Wextra -DBA_GRAPH_DEBUG
COMPILE_DBG = $(CXX) $(CFLAGS) $(DBGFLAGS)

all: test_time


test_time : test_time.cpp
	$(COMPILE_DBG) test_time.cpp -DCOMPILE_WITH_CRYPTOMINISAT -lcryptominisat5
	./a.out

test_ecd: test_ecd.cpp
	$(COMPILE_DBG) test_ecd.cpp

test_ecd_sat : test_ecd_sat.cpp
	$(COMPILE_DBG) test_ecd_sat.cpp -DCOMPILE_WITH_CRYPTOMINISAT -lcryptominisat5

clean:
	rm -rf test_ecd.out


.PHONY: clean all

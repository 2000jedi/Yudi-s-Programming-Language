CXX = c++
CXXFLAGS = -g -Wall -Wno-overloaded-virtual $(FLAGS) -fexceptions -std=c++1y

TARGET = auto
SRCS = src/err.cpp src/util.cpp src/lexical.cpp src/tree.cpp src/ast.cpp src/ast_gen.cpp src/scanner.cpp src/parser.cpp src/runtime.cpp
HEADERS = ${SRCS:.cpp=.hpp}
OBJS = ${SRCS:.cpp=.o}

auto: src/main.cpp $(OBJS) $(SRCS) $(HEADERS)
	$(CXX) $(CXXFLAGS) -o auto src/main.cpp $(OBJS)

clean:
	${RM} ${TARGET} ${OBJS}
	-rm -r *.dSYM

test: auto
	./auto sample/factorial.yc
	./auto sample/cast.yc

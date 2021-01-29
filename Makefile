CXX = c++
CXXFLAGS = -g -Wall $(FLAGS) -fexceptions -std=c++17

TARGET = auto
SRCS = src/err.cpp src/util.cpp src/ast.cpp src/scanner.cpp src/parser.cpp src/runtime.cpp
HEADERS = ${SRCS:.cpp=.hpp}
OBJS = ${SRCS:.cpp=.o}

OUT = ./auto

auto: src/main.cpp $(OBJS) $(SRCS) $(HEADERS)
	$(CXX) $(CXXFLAGS) -o $(OUT) src/main.cpp $(OBJS)

clean:
	${RM} ${OBJS} $(OUT)
	-rm -r *.dSYM

test: auto
	$(OUT) sample/factorial.yc
	$(OUT) sample/cast.yc
	$(OUT) sample/copy_move_deep.yc

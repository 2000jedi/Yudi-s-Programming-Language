CXX = c++
LLVMFLAGS = `llvm-config --cxxflags --libs core`
CXXFLAGS = -g -Wall -frtti $(FLAGS) $(LLVMFLAGS) -fexceptions -std=c++1y

TARGET = auto
SRCS = src/err.cpp src/util.cpp src/lexical.cpp src/tree.cpp src/ast.cpp src/scanner.cpp src/parser.cpp
OBJS = ${SRCS:.cpp=.o}

auto: src/main.cpp $(OBJS)
	$(CXX) $(CXXFLAGS) -o auto src/main.cpp $(OBJS)

clean:
	${RM} ${TARGET} ${OBJS}
	-rm -r *.dSYM

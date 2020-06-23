CXX = c++
LLVMFLAGS = `llvm-config --cxxflags --libs core`
CXXFLAGS = -g -Wall -frtti $(FLAGS) $(LLVMFLAGS)

TARGET = auto
SRCS = lexical.cpp tree.cpp ast.cpp scanner.cpp parser.cpp
OBJS = ${SRCS:.cpp=.o}

auto: main.cpp $(OBJS)
	$(CXX) $(CXXFLAGS) -o auto main.cpp $(OBJS)

clean:
	${RM} ${TARGET} ${OBJS}
	-rm -r *.dSYM

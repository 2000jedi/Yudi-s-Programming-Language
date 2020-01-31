CXX = g++
CXXFLAGS = -g -Wall -Werror -march=native -O3 -std=c++11 $(FLAGS)

TARGET = auto
SRCS = lexical.cpp scanner.cpp tree.cpp parser.cpp
OBJS = ${SRCS:.cpp=.o}

auto: main.cpp $(OBJS)
	$(CXX) $(CXXFLAGS) -o auto main.cpp $(OBJS)

clean:
	${RM} ${TARGET} ${OBJS}
	-rm -r *.dSYM

CXX=g++
CXXFLAGS=-std=c++11
SRCS=$(wildcard *.cpp)
OBJS=$(patsubst %.cpp, %.o, $(SRCS))
BOOST_ROOT := /usr/local/src/boost_1_81_0

main: $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJS) -L./lib -I./lib -Wl,-rpath=./lib -lllhttp -lpthread -lboost_thread

clean:
	rm -f *~ *.o *.so

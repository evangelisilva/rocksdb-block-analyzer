CXX = c++
CXXFLAGS = -std=c++17 -O2 -I/opt/homebrew/include -I./src
LDFLAGS = -L/opt/homebrew/lib -lrocksdb -lz -lbz2 -lsnappy -llz4 -lzstd

SRCS = src/main.cpp src/key_generator.cpp src/stats_reporter.cpp
OBJS = $(SRCS:.cpp=.o)
TARGET = rocksdb_block_size_demo

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(OBJS) -o $(TARGET) $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

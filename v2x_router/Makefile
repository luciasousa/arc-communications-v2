include /usr/local/etc/PcapPlusPlus.mk

CXXFLAGS = -std=c++11 -lm
LDFLAGS += $(shell pkg-config --libs json-c) 
LDFLAGS += $(shell pkg-config --libs libcurl)
CXXFLAGS += $(shell pkg-config --cflags libnl-genl-3.0)
CXXFLAGS += $(shell pkg-config --cflags json-c)

# All Target
all:
	g++ $(PCAPPP_BUILD_FLAGS) $(PCAPPP_INCLUDES) -c -o main.o main.cpp $(CXXFLAGS)
	g++ $(PCAPPP_LIBS_DIR) -static-libstdc++ -o main main.o $(PCAPPP_LIBS) $(CXXFLAGS) $(LDFLAGS)

# Clean Target
clean:
	rm main.o
	rm main


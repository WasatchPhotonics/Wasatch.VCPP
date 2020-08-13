TOP = ../..

INC_DIR = $(TOP)/include
LIB_DIR = $(TOP)/lib

LIB = $(LIB_DIR)/libwasatchcpp.a

SRCS = $(wildcard *.cpp)
OBJS = $(SRCS:.cpp=.o)

CXXFLAGS += --std=c++11     \
            -I$(INC_DIR)    \
            -I/usr/include/libusb-1.0

all: $(LIB)

clean:
	rm *.o $(LIB)

$(LIB): $(OBJS)
	mkdir -p $(TOP)/lib
	ar -r $@ $^

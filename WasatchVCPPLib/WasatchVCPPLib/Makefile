TOP = ../..

INC_DIR = $(TOP)/include
LIB_DIR = $(TOP)/lib

LIB = $(LIB_DIR)/libwasatchvcpp.a

SRCS = $(wildcard *.cpp)
OBJS = $(SRCS:.cpp=.o)

# /usr/local/Cellar is used on MacOS / Homebrew
CXXFLAGS += --std=c++11     \
            -I$(INC_DIR)    \
            -I/usr/include/libusb-1.0 \
            -I/usr/local/Cellar/libusb/1.0.24/include/libusb-1.0

all: $(LIB)

clean:
	rm -f *.o $(LIB)

new: clean all

$(LIB): $(OBJS)
	mkdir -p $(TOP)/lib
	ar -r $@ $^

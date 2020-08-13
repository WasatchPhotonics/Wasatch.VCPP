
CXXFLAGS += -I../include
LDFLAGS  += -L../lib        \
            -lwasatchvcpp   \
            -lusb-1.0
        
all: demo

new: clean all

clean:
	@rm -f *.o *.log demo

demo: demo.o
	g++ $(LDFLAGS) -o $@ $^ $(LDFLAGS)

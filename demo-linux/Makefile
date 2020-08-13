
CXXFLAGS += -I../include
LDFLAGS  += -L../lib        \
            -lwasatchvcpp   \
            -lusb-1.0
        
all: demo

clean:
	@rm -f *.o demo

demo: demo.o
	g++ $(LDFLAGS) -o $@ $^ $(LDFLAGS)

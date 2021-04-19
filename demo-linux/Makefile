
CXXFLAGS += --std=c++11     \
            -I../include
LDFLAGS  += -L../lib        \
            -lwasatchvcpp   \
            -lusb-1.0
        
all: demo

new: clean all

clean:
	@rm -f *.o *.log demo test-*

demo: demo.o
	g++ $(LDFLAGS) -o $@ $^ $(LDFLAGS)

##
# Run a simple command-line test which runs 100 iterations of the linux-demo
# with default arguments, checking the system exit code after each run. This
# was added when investigating an intermittent failure on subsequent runs.
test: demo
	COUNT=1 ;                            \
    SLEEP_SEC=0 ;                        \
    while [ $$COUNT -le 100 ] ;          \
    do                                   \
        ./demo > test-out.$$COUNT ;      \
        if [ $$? = 0 ] ;                 \
        then                             \
            echo "test $$COUNT ok" ;     \
        else                             \
            echo "test $$COUNT FAILED" ; \
        fi ;                             \
        mv demo.log test-log.$$COUNT ;   \
        sleep $$SLEEP_SEC ;              \
        COUNT=`expr $$COUNT + 1` ;       \
    done

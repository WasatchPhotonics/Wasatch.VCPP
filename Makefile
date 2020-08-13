.PHONY: doc docs

all: 
	@cd WasatchVCPPLib && $(MAKE) $@

new: clean all

clean: 
	@cd WasatchVCPPLib && $(MAKE) $@
	@rm -rf doxygen*                                            \
            WasatchVCPPLib/.vs                                  \
            WasatchVCPPLib/packages                             \
            WasatchVCPPLib/{Setup32,Setup64}/{Debug,Release}    \
            WasatchVCPPLib/{x64,Win32}                          \
            WasatchVCPPLib/{Debug,Release}                      \
            WasatchVCPPLib/WasatchVCPPLib/{x64,Win32}           \
            WasatchVCPPLib/WasatchVCPPLib/{Debug,Release}       \
            WasatchVCPPLib/WasatchVCPPDemo/{x64,Win32}          \
            WasatchVCPPLib/WasatchVCPPDemo/{Debug,Release}      \
            WasatchVCPPNet/{obj,bin}                            \
            bin/*.{exe,dll}                                     \
            lib/{x86,x64}/*.{lib,dll}                           \
            lib/*.{a,so}

doc docs:
	@(cat Doxyfile ; echo "PROJECT_NUMBER = $$VERSION") | doxygen - 1>doxygen.out 2>doxygen.err

# called by scripts/deploy to compare wp_foo() declarations in C vs C# APIs
check_headers:
	@TMPFILE="make-check-headers" ; \
    egrep -o 'wp_[a-z0-9_]+' include/WasatchVCPP.h | sort -u > $$TMPFILE.h ; \
    egrep -o 'wp_[a-z0-9_]+' WasatchVCPPNet/WasatchVCPP.cs | sort -u > $$TMPFILE.cs ; \
    diff $$TMPFILE.h $$TMPFILE.cs | egrep '^[<>]' | sort -u ; \
    rm -f $$TMPFILE.*

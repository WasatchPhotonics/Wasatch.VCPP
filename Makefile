.PHONY: doc docs

clean: 
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
            lib/{x86,x64}/*.{lib,dll}                           \
            bin/*.{exe,dll}

doc docs:
	@(cat Doxyfile ; echo "PROJECT_NUMBER = $$VERSION") | doxygen - 1>doxygen.out 2>doxygen.err

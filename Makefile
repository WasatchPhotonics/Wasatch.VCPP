.PHONY: doc docs

clean: 
	@rm -rf doxygen*                                            \
            WasatchVCPPLib/.vs                                  \
            WasatchVCPPLib/packages                             \
            WasatchVCPPLib/{Setup32,Setup64}/{Debug,Release}    \
            WasatchVCPPLib/{Debug,Release}                      \
            WasatchVCPPLib/WasatchVCPPLib/{Debug,Release}       \
            WasatchVCPPLib/WasatchVCPPDemo/{Debug,Release}      \
            bin/*.{exe,dll}                                     \
            lib/{x86,x64}/*.{lib,dll}

doc docs:
	@(cat Doxyfile ; echo "PROJECT_NUMBER = $$VERSION") | doxygen - 1>doxygen.out 2>doxygen.err

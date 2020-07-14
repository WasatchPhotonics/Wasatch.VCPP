.PHONY: doc docs

clean: 
	@rm -rf doxygen*                                    \
            WasatchVCPPLib/.vs                          \
            WasatchVCPPLib/packages                     \
            WasatchVCPPLib/{x64,Debug}                  \
            WasatchVCPPLib/WasatchVCPPLib/{x64,Debug}   \
            WasatchVCPPLib/WasatchVCPPDemo/{x64,Debug}  \
            bin/*.{exe,dll}                             \
            lib/{x86,x64}/*.{lib,dll}

doc docs:
	@(cat Doxyfile ; echo "PROJECT_NUMBER = $$VERSION") | doxygen - 1>doxygen.out 2>doxygen.err

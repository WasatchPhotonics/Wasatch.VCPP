.PHONY: doc docs

clean: 
	@rm -rf doxygen*                                    \
            WasatchVCPPLib/.vs                          \
            WasatchVCPPLib/packages                     \
            WasatchVCPPLib/{x64,Debug}                  \
            WasatchVCPPLib/WasatchVCPPLib/{x64,Debug}   \
            WasatchVCPPLib/WasatchVCPPDemo/{x64,Debug}

doc docs:
	@(cat Doxyfile ; echo "PROJECT_NUMBER = $$VERSION") | doxygen - 1>doxygen.out 2>doxygen.err

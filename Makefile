.PHONY: doc docs

doc docs:
	@(cat Doxyfile ; echo "PROJECT_NUMBER = $$VERSION") | doxygen - 1>doxygen.out 2>doxygen.err

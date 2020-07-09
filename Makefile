.PHONY: doc docs

doc docs:
	@doxygen 1>doxygen.out 2>doxygen.err

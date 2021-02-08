TEST_SOURCES := $(wildcard *.asm)
TEST_OUTPUT := $(TEST_SOURCES:.asm=.out)

%.out : %.asm
	mkdir -p test_output
	../src/crasm -o test_output/$@ -l $<
	@diff test_output/$@ reference_output/$@ || (echo "Output '$@' does not match expected!"; exit 1 )

test: ${TEST_OUTPUT}

clean:
	rm -rf test_output


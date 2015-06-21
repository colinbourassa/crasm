SHELL   =/bin/sh
CFLAGS  = -O
LDFLAGS	= 
LIBS    = -lm
RM	= rm

OBJ     =   crasm.o stdvocabulary.o pseudos.o macro.o label.o \
            parse.o filter.o operator.o output.o xref.o scode.o

CPU     =   cpulist.o cpu6800.o cpu6502.o cpuz80.o

all: crasm

crasm: $(OBJ) $(CPU)
	${CC} ${CFLAGS} ${LDFLAGS} -o crasm $(OBJ) $(CPU) ${LIBS}
	@echo "done"

clean: FORCE
	-${RM} 2>/dev/null *.o crasm

FORCE:

.PHONY: FORCE

$(OBJ):	dcl.h label.h
$(CPU):	cpu.h

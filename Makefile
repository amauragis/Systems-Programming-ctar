CC = gcc
EXEC = ctar
CCFLAGS = -Wall -g -ansi -pedantic
OBJS = main.o

${EXEC}: ${OBJS}
	${CC} ${CCFLAGS} -lm -o ${EXEC} ${OBJS}

default:
	${CC} ${CCFLAGS} -c $<

run: ${EXEC}
	./${EXEC}
    
clean:
	rm -f ${EXEC} ${OBJS}

test: ${EXEC}
	valgrind ./${EXEC}

#f.o: f.c f.h

CC=gcc
LDFLAGS=$(shell pkg-config --libs libnl-genl-3.0)
DEBUG_FLAGS=-DDEBUG=1 -std=gnu99 -Wall -Wextra -Wpedantic -g 
FLAGS=-std=gnu99 -Wall -Wpedantic -Wno-unused-parameter -Wunused-variable -lpthread -lm
LDFLAGS += $(shell pkg-config --libs json-c) 
LDFLAGS += $(shell pkg-config --libs libcurl)



ifeq (${DEBUG},1)
	FLAGS+=${DEBUG_FLAGS}
endif
FLAGS += $(shell pkg-config --cflags libnl-genl-3.0)
FLAGS += $(shell pkg-config --cflags json-c)


main: main.o 
	${CC} -o $@ main.o ${LDFLAGS} ${FLAGS}

accident: accident.o 
	${CC} -o $@ accident.o ${LDFLAGS} ${FLAGS}

main.o: main.c
	${CC} -o $@ -c $< ${FLAGS}

accident.o: accident.c
	${CC} -o $@ -c $< ${FLAGS}

client_functions: client_functions.o 
	${CC} -o $@ client_functions.o ${LDFLAGS} ${FLAGS}

client_functions.o: client_functions.c
	${CC} -o $@ -c $< ${FLAGS}

interact: interact.o 
	${CC} -o $@ interact.o ${LDFLAGS} ${FLAGS}

interact.o: interact.c
	${CC} -o $@ -c $< ${FLAGS}

clean:
	rm -rf *.o


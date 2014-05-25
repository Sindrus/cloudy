CC = gcc
TOP = .
INCLUDES = -I$(TOP)
CFLAGS = -g -std=c99
COMPILE = $(CC) $(INCLUDES) $(CFLAGS)

INCL=./jrb.h ./jval.h ./dllist.h ./fifo.h
COMMON=.
PRED=.
BOBJ=$(COMMON)/jval.o $(COMMON)/jrb.o $(COMMON)/dllist.o $(COMMON)/fifo.o
LIBS= -lm 

PROGRAMS = azm-search simple_taboo_search simple_taboo_search-6

all: $(PROGRAMS)


simple_taboo_search: simple_taboo_search.c $(INCL) ${BOBJ}
	$(COMPILE) ${INCLUDES} -o simple_taboo_search simple_taboo_search.c $(BOBJ) $(LIBS)

simple_taboo_search-6: simple_taboo_search-6.c $(INCL) ${BOBJ}
	$(COMPILE) ${INCLUDES} -o simple_taboo_search-6 simple_taboo_search-6.c $(BOBJ) $(LIBS)

azm-search: azm-search.c $(INCL) ${BOBJ}
	$(COMPILE) ${INCLUDES} -o azm-search azm-search.c c_api.c $(BOBJ) $(LIBS) -lcurl -lc -I jsmn/ -L jsmn/ -ljsmn

fifo.o: fifo.c fifo.h jrb.h jval.h dllist.h
	$(COMPILE) ${INCLUDES} -c fifo.c

jrb.o: jrb.c jrb.h jval.h
	$(COMPILE) ${INCLUDES} -c jrb.c

jval.o: jval.c jval.h
	$(COMPILE) ${INCLUDES} -c jval.c

dllist.o: dllist.c dllist.h jval.h
	$(COMPILE) ${INCLUDES} -c dllist.c

clean:
	rm -f *.o core

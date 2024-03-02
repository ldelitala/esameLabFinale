CC=gcc
CFLAGS=-Iinclude -Wall
LIBFLAGS_CLIENT=-lpthread
LIBFLAGS_SERVER=-lpthread

#NOMI ESEGUIBILI FINALI
CLIENT=bibclient
SERVER=bibserver
BIBACCESS=bibaccess

#DIRECTORIES

#main
DIR_BIN=bin
DIR_BUILD=build
DIR_SRC=src
DIR_LIB=lib
DIR_INCLUDE=include

#build
DIR_STR_DATI=$(DIR_BUILD)/struttura_dati
DIR_MY_LIB=$(DIR_BUILD)/my_lib
DIR_COMM=$(DIR_BUILD)/comunicazione
DIR_SOCKETS=sockets

#OBJECTS

#main
OBJ_CLIENT=$(DIR_BUILD)/client.o
OBJ_SERVER=$(DIR_BUILD)/server.o

#my_lib
OBJ_DIN_ARR=$(DIR_MY_LIB)/dynamic_array.o
OBJ_BINARY_TREE=$(DIR_MY_LIB)/binary_tree.o
OBJ_THREAD_SHARED_FIFOST=$(DIR_MY_LIB)/thread_shared_static_fifo.o
OBJ_FIFOST=$(DIR_MY_LIB)/static_fifo.o
OBJ_RW2=$(DIR_MY_LIB)/readers_writers2.o

#struttura_dati
OBJ_LIBRO=$(DIR_STR_DATI)/libro.o
OBJ_PERS_TIME=$(DIR_STR_DATI)/personal_time.o
OBJ_ARRAY_CAMPI=$(DIR_STR_DATI)/arrayCampi.o
OBJ_STR_DATI=$(DIR_STR_DATI)/struttura_dati.o

#comunicazione
OBJ_CODA_COND=$(DIR_COMM)/coda_condivisa.o
OBJ_SOCK_COM=$(DIR_COMM)/socket_comunication.o
OBJ_BIB_CONF=$(DIR_COMM)/bib_conf.o

# DIPENDENZE	

#main
DEP_CLIENT=$(OBJ_CLIENT) $(DEP_SOCKET_COMUNICATION) $(DEP_BIB_CONF)
DEP_SERVER=$(OBJ_SERVER) $(DEP_SOCKET_COMUNICATION) $(DEP_STRUTTURA_DATI) $(DEP_BIB_CONF)

#my_lib
DEP_FIFOST=$(OBJ_FIFOST) $(OBJ_DIN_ARR)
DEP_THREAD_SHARED_FIFOST=$(OBJ_THREAD_SHARED_FIFOST) $(DEP_FIFOST)
#struttura_dati
DEP_LIBRO=$(OBJ_LIBRO) $(OBJ_PERS_TIME)
DEP_ARRAYCAMPI=$(OBJ_ARRAY_CAMPI) $(DEP_LIBRO) $(OBJ_DIN_ARR) $(OBJ_BINARY_TREE) 
DEP_STRUTTURA_DATI=$(OBJ_STR_DATI) $(DEP_ARRAYCAMPI)

#comunicazione
DEP_CODA_CONDIVISA=$(OBJ_CODA_COND) $(DEP_THREAD_SHARED_FIFOST)
DEP_SOCKET_COMUNICATION=$(OBJ_SOCK_COM) $(DEP_CODA_CONDIVISA)
DEP_BIB_CONF=$(OBJ_BIB_CONF) $(OBJ_RW2)

#BASH PER TEST
TEST=$(DIR_SRC)/bash/lancia_test.sh
VALG_TEST=$(DIR_SRC)/bash/lancia_test_valgrind.sh

# OBBIETTIVI FINALI
all: crea_directories_mancanti $(DIR_BIN)/$(CLIENT) $(DIR_BIN)/$(SERVER) $(DIR_BIN)/$(BIBACCESS)

crea_directories_mancanti:
	if [ ! -d $(DIR_STR_DATI) ]; then \
    	mkdir -p $(DIR_STR_DATI); \
    	echo "Directory $(DIR_STR_DATI) creata."; \
    fi
	if [ ! -d $(DIR_MY_LIB) ]; then \
    	mkdir -p $(DIR_MY_LIB); \
    	echo "Directory $(DIR_MY_LIB) creata."; \
    fi
	if [ ! -d $(DIR_COMM) ]; then \
    	mkdir -p $(DIR_COMM); \
    	echo "Directory $(DIR_COMM) creata."; \
    fi
	if [ ! -d $(DIR_SOCKETS) ]; then \
    	mkdir -p $(DIR_SOCKETS); \
    	echo "Directory $(DIR_SOCKETS) creata."; \
    fi

$(DIR_BIN)/$(CLIENT): $(DEP_CLIENT)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBFLAGS_CLIENT)

$(DIR_BIN)/$(SERVER): $(DEP_SERVER)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBFLAGS_SERVER)

$(DIR_BIN)/$(BIBACCESS): $(DIR_SRC)/bash/bibaccess.sh
	cp $^ $@

# Regole di compilazione per le dipendenze
$(DIR_STR_DATI)/%.o: $(DIR_LIB)/struttura_dati/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(DIR_MY_LIB)/%.o: $(DIR_LIB)/my_lib/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(DIR_COMM)/%.o: $(DIR_LIB)/comunicazione/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(DIR_BUILD)/%.o: $(DIR_SRC)/client/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(DIR_BUILD)/%.o: $(DIR_SRC)/server/%.c
	$(CC) $(CFLAGS) -c $< -o $@



#TEST

test: all
	./$(TEST) $(DIR_BIN)/$(SERVER) $(DIR_BIN)/$(CLIENT) $(DIR_BIN)/$(BIBACCESS)

test_valgrind: all
	./$(VALG_TEST) $(DIR_BIN)/$(SERVER) $(DIR_BIN)/$(CLIENT) $(DIR_BIN)/$(BIBACCESS)

clean:
	rm -r $(DIR_BUILD) && mkdir $(DIR_BUILD)

clean_all: clean
	rm -r logs && mkdir logs && \
	rm -r sockets && mkdir sockets && \
	rm -r bin && mkdir bin && \
	rm data/file_records/bib1.txt && cp data/copia_originale/bib1.txt data/file_records/bib1.txt && \
	rm data/file_records/bib2.txt && cp data/copia_originale/bib2.txt data/file_records/bib2.txt && \
	rm data/file_records/bib3.txt && cp data/copia_originale/bib3.txt data/file_records/bib3.txt && \
	rm data/file_records/bib4.txt && cp data/copia_originale/bib4.txt data/file_records/bib4.txt && \
	rm data/file_records/bib5.txt && cp data/copia_originale/bib5.txt data/file_records/bib5.txt && \
	rm config/bib.conf && touch config/bib.conf
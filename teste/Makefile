#
# Makefile ESQUELETO
#
# DEVE ter uma regra "all" para geração da biblioteca
# regra "clean" para remover todos os objetos gerados.
#
# NECESSARIO adaptar este esqueleto de makefile para suas necessidades.
#

CC=gcc
LIB_DIR=../lib
INC_DIR=../include
BIN_DIR=../bin
SRC_DIR=../src

all: libt2fs.a copia teste1 teste2 teste3 teste4 teste5 teste6

libt2fs.a: 
	cd .. && make

copia: 
	cp ../t2fs_disk.dat . -i #Formatar disco?

shell: $(TST_DIR)/shell.c $(LIB_DIR)/libt2fs.a
	$(CC) -o shell $(TST_DIR)/shell.c -L$(LIB_DIR) -lt2fs -Wall -g

teste1: teste1.c $(LIB_DIR)/libt2fs.a
	$(CC) -o teste1 teste1.c -L$(LIB_DIR) -lt2fs -Wall

teste2: teste2.c $(LIB_DIR)/libt2fs.a
	$(CC) -o teste2 teste2.c -L$(LIB_DIR) -lt2fs -Wall

teste3: teste3.c $(LIB_DIR)/libt2fs.a
	$(CC) -o teste3 teste3.c -L$(LIB_DIR) -lt2fs -Wall

teste4: teste4.c $(LIB_DIR)/libt2fs.a
	$(CC) -o teste4 teste4.c -L$(LIB_DIR) -lt2fs -Wall

teste5: teste5.c $(LIB_DIR)/libt2fs.a
	$(CC) -o teste5 teste5.c -L$(LIB_DIR) -lt2fs -Wall

teste6: teste6.c $(LIB_DIR)/libt2fs.a
	$(CC) -o teste6 teste6.c -L$(LIB_DIR) -lt2fs -Wall

clean:
	rm -rf teste1 teste2 teste3 teste4 teste5 teste6 && cd .. && make clean
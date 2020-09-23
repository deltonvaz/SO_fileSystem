#
# Makefile ESQUELETO
#
# DEVE ter uma regra "all" para geração da biblioteca
# regra "clean" para remover todos os objetos gerados.
#
# NECESSARIO adaptar este esqueleto de makefile para suas necessidades.
#

CC=gcc
LIB_DIR=./lib
INC_DIR=./include
BIN_DIR=./bin
SRC_DIR=./src
TST_DIR=./teste

all: t2fs_auxiliar t2fs libt2fs shell copia

t2fs: $(SRC_DIR)/t2fs.c
	$(CC) -I$(INC_DIR) -c -o $(BIN_DIR)/t2fs.o $(SRC_DIR)/t2fs.c -Wall

t2fs_auxiliar: $(SRC_DIR)/t2fs_auxiliar.c
	$(CC) -I$(INC_DIR) -c -o $(BIN_DIR)/t2fs_auxiliar.o $(SRC_DIR)/t2fs_auxiliar.c -Wall -g
	
libt2fs: $(BIN_DIR)/t2fs.o $(BIN_DIR)/t2fs_auxiliar.o $(LIB_DIR)/apidisk.o $(LIB_DIR)/bitmap2.o
	ar crs $(LIB_DIR)/libt2fs.a $(BIN_DIR)/t2fs.o $(BIN_DIR)/t2fs_auxiliar.o $(LIB_DIR)/apidisk.o $(LIB_DIR)/bitmap2.o
	
testes: $(TST_DIR)/teste.c $(LIB_DIR)/libt2fs.a
	$(CC) -o tst $(TST_DIR)/teste.c -L$(LIB_DIR) -lt2fs -Wall -g

shell: $(TST_DIR)/shell.c $(LIB_DIR)/libt2fs.a
	$(CC) -o shell $(TST_DIR)/shell.c -L$(LIB_DIR) -lt2fs -Wall -g

#regra para testes de gravação de arquivos
copia: 
	cp teste/t2fs_disk.dat . -i
	
clean:
	rm -rf $(LIB_DIR)/*.a $(BIN_DIR)/*.o $(SRC_DIR)/*~ $(INC_DIR)/*~ *~ $(TST_DIR)/*.o tst shell
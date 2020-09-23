/* 
 * Protótipos das funções auxiliares
 * Delton Vaz
 * Flavio Keglevich 
 * */
#ifndef _T2FSAUXILIAR_
#define _T2FSAUXILIAR_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define NOMES "Delton Vaz - 00229779\nFlavio Keglevich - 00229724\n"
#define TRUE 1
#define FALSE 0
#define ERROR -1
#define MAXTUPLA 32
#define MFT_SIZE 512
#define TUPLA_SIZE 16
#define MAXARQUIVOS 20
#define REG_SIZE 64
#define RECORDSPORSETOR 4
#define NOTALLOCATED 0

typedef struct t2fs_4tupla TUPLA;
typedef struct t2fs_bootBlock BOOTBLOCK;
typedef struct t2fs_record REGISTRO;

/*
	Struct para lidar com os MFT
*/
typedef struct mft{
	TUPLA tupla[MAXTUPLA];
}MFT;

/*
	Struct que define um arquivo 
	que está sendo lendo no momento
*/
typedef struct arquivo{
	REGISTRO regDir;
	int current_Pointer; //Para uso de truncate
	int MFTNumber; //Para caso necessite salvar novamente
}ARQUIVO;

//Assinatura das funções extras
int inicializarSistema();
void imprimirDadosBootBlock();
void imprimirDadosRoot();

MFT leMFT(int MFTNumber);

int searchMFT();

DWORD stodw(unsigned char *string, int inicio);
TUPLA uctoTupla(unsigned char *string, int inicio);
int inteiroParaString(unsigned char *string, int inicio, int inteiro);
void inicializaArquivosAbertos();
int readRegister(MFT mft, REGISTRO *registro);
int IsDirRaiz2(char *pathname); //Usada no opendir
int IsDirRaiz(char *pathname);
int abreArquivo(REGISTRO regDir, int MFTNumber);
int isFileOpen(char *filename);
void extractFileName(char *pathname, char *filename);
int handleValido(int handle);
char* DivideString(char* pathname);

int getNumberofRegisters(int bloco);

int procuraDiretorio(MFT mft, char *diretorio, REGISTRO *regDir, REGISTRO *registro);
int procuraArquivo(MFT mft, char *nomeArquivo, REGISTRO *regDir, REGISTRO *registro);
int procuraPath(char *pathname, REGISTRO *regDir, REGISTRO *registro);

int alocarNovoBloco(MFT mft, int MFTNumber);

int criarRegistro(MFT novoMFT, REGISTRO novoRegistro, int MFTNumberPai);

//Funções responsáveis pela escrita dos registros de memória no disco
int escreveRegistroDisco(REGISTRO *registros, int bloco);
int escreveMFTDisco(MFT *mft, int mftNumber);

//Funções utilizadas para exclusão
int deletarArquivo(MFT mft, char *filename);
int deletarArquivoDisco(REGISTRO *registro, int MFTNumber, MFT MFTPai);
int deletarDiretorio(MFT mft, char *filename);

//Funções utilizadas para leitura, escrita e atualização de registros
int lerDadosMFT(MFT mft, int ptrIndex, ARQUIVO *arquivo, int *size, char *buffer, int*bufferStart);
int escreverRegistro(MFT mft, int ponteiroIndex, ARQUIVO *arquivo, int *tamanho, char *buffer, int*bufferComeco);
int atualizarRegistro(int MFTNumberPai, REGISTRO registro);

#endif

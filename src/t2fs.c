#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/t2fs.h"
#include "../include/apidisk.h"
#include "../include/bitmap2.h"
#include "../include/t2fs_auxiliar.h"

/* Variáveis globais */
 ARQUIVO arquivosAbertos[MAXARQUIVOS];

int debug = 0;

char nomeSubDir[MAX_FILE_NAME_SIZE];

//Variável que indica o tamanho de um bloco (4 setores)
int blockSize = 0;
//Variável que indica quantos blocos existem na área de MFTBlockSize
int MFTBlocksSize = 0;

//Flag que indica se o sistema já foi inicializado ou não
int started = 0;

//Variável que indica quantos arquivos estão abertos no sistema
int numArquivos = 0;

//Variável que contém os dados do bootBlock
BOOTBLOCK bootBlock;

//MFT descritor do bloco raiz
MFT descritorRaiz;

//Variável que informa qual é o diretório aberto atualmente
REGISTRO *DirAtual;

//Variável que indica qual é o bloco que está sendo usado atualmente
int currentBlock;


int identify2 (char *name, int size){
	if(size < sizeof(NOMES)) return -1;
	strcpy(name, NOMES);
	return 0;
}

FILE2 create2 (char *filename){
	int dirRaiz = 0, blocoNmr, mftNmr, k;
	REGISTRO *regDir, *registro, novoRegistro;
	char name[MAX_FILE_NAME_SIZE];
	MFT mft, novoMFT;

	//Registro unico para funções
	registro = (REGISTRO *) malloc(sizeof(REGISTRO));

	//Lista de registros para funções
	regDir = (REGISTRO *) malloc(SECTOR_SIZE*blockSize*8);

	// Inicializa a estrutura com as infos do bloco de boot
	if(inicializarSistema() == ERROR){
		return ERROR;
	}

	//Verifica se o registro deve ser gravado no dir raiz
	if (IsDirRaiz(filename)){
		dirRaiz = 1;
		mft = leMFT(1);
	}else{
		dirRaiz = 0;
	}

	//Verifica se os paths necessários para acessar o arquivo existem
	if (!procuraPath(filename, regDir, registro)) 
	{
		return -1;
	}

	if(!dirRaiz){
		mft = leMFT(registro->MFTNumber);
	}

	// Se o(s) path(s) ja existem criar nova entrada no último path
	extractFileName(filename, name);

	//Verifica se a entrada ja existe no diretorio
	if (procuraArquivo(mft, name, regDir, registro)){
		return -1;
	}

	//Cria o registro em memória
	strcpy(novoRegistro.name, name);
	novoRegistro.TypeVal = TYPEVAL_REGULAR;
	novoRegistro.blocksFileSize = 1;
	novoRegistro.bytesFileSize = 0;
	blocoNmr = searchBitmap2(NOTALLOCATED);
	if (blocoNmr < 0) return ERROR; // Não tem mais blocos livres
	mftNmr = searchMFT();
	if (mftNmr < 0) return ERROR; //Não tem mais MFTS livres
	novoRegistro.MFTNumber = mftNmr;

	//Cria o novo MFT
	novoMFT.tupla[0].atributeType = 1;
	novoMFT.tupla[0].virtualBlockNumber = 0;
	novoMFT.tupla[0].logicalBlockNumber = blocoNmr; //O bloco livre
	novoMFT.tupla[0].numberOfContiguosBlocks = 1; //Normalmente começa com um bloco só

	novoMFT.tupla[1].atributeType = 0; //Marca o fim do encadeamento das tuplas
	novoMFT.tupla[1].virtualBlockNumber = -1; 
	novoMFT.tupla[1].logicalBlockNumber = -1; 
	novoMFT.tupla[1].numberOfContiguosBlocks = -1; 
	

	//Zerar as outras tuplas
	for (k = 2; k < MAXTUPLA; k++){
		novoMFT.tupla[k].atributeType = -1; 
		novoMFT.tupla[k].virtualBlockNumber = -1; 
		novoMFT.tupla[k].logicalBlockNumber = -1; 
		novoMFT.tupla[k].numberOfContiguosBlocks = -1; 
	}

	//Criar registro no disco
	if (!dirRaiz){
		k = criarRegistro(novoMFT, novoRegistro, registro->MFTNumber);
	}
	else {
		k = criarRegistro(novoMFT, novoRegistro, 1); //O 1 indica do diretório raiz
	}

	free(registro);
	free(regDir);
	return k;

}

int delete2(char *filename)
{
	REGISTRO *regDir, *registro;
	char name[MAX_FILE_NAME_SIZE];
	int retorno, dirRaiz = 0;
	MFT mft;
	if(inicializarSistema() == ERROR){
		return ERROR;
	}
	//Registro unico para funções
	registro = (REGISTRO *) malloc(sizeof(REGISTRO));
	//Lista de registros para funções
	regDir = (REGISTRO *) malloc(SECTOR_SIZE*blockSize*8);

	if (IsDirRaiz(filename))
		dirRaiz = 1;
	else if (!procuraPath(filename, regDir, registro))
		return -1;

	if (dirRaiz)
		mft = leMFT(1);
	else 
		mft = leMFT(regDir->MFTNumber);
	
	extractFileName(filename, name);
	
	retorno = deletarArquivo(mft, name);

	free(registro);
	free(regDir);
	return retorno;

}

FILE2 open2(char *filename){
	char name[MAX_FILE_NAME_SIZE];
	REGISTRO *regDir, *registro;
	MFT mft;
	int dirRaiz = 0, mftNumber, retorno = 0;
	if(inicializarSistema() == ERROR){
		return ERROR;
	}
	//Registro unico para funções
	registro = (REGISTRO *) malloc(sizeof(REGISTRO));
	//Lista de registros para funções
	regDir = (REGISTRO *) malloc(SECTOR_SIZE*blockSize*8);

	if (IsDirRaiz(filename))
		dirRaiz = 1;
	else if (!procuraPath(filename, regDir, registro))
		return -1;

	extractFileName(filename, name);

	// Verificar se o arquivo já está aberto
	if (isFileOpen(name)){
		return -1;
	}
	
	if (dirRaiz){
		mft = leMFT(1);
		mftNumber = 1;
	}
	else{
		mft = leMFT(registro->MFTNumber);
		mftNumber = registro->MFTNumber;
	}

	if(procuraArquivo(mft, name, regDir, registro)){
		retorno = abreArquivo(*registro, mftNumber);
		free(registro);
		free(regDir);
		return retorno;
	}

	return -1;
}

int close2(FILE2 handle)
{
	ARQUIVO arquivo;
	if(inicializarSistema() == ERROR){
		return ERROR;	
	}
	if (handleValido(handle))
	{
		arquivo = arquivosAbertos[handle];

		if(arquivo.regDir.TypeVal == TYPEVAL_REGULAR)
		{
			if (atualizarRegistro(arquivo.MFTNumber, arquivo.regDir))
			{
			 	arquivosAbertos[handle].regDir.TypeVal = TYPEVAL_INVALIDO;
			 	return 0;
			 }
			else
				return -1;
		}
		else 
			return -1;
	}

	else
		return -1;
}

int read2(FILE2 handle, char *buffer, int size)
{
	int bufferStart = 0;
	ARQUIVO arquivo;
	MFT mft;
	if(inicializarSistema() == ERROR){
		return ERROR;	
	}
	//Se é um handle valido
	if ((handleValido(handle)) && (arquivosAbertos[handle].regDir.TypeVal == TYPEVAL_REGULAR))
	{
		arquivo = arquivosAbertos[handle];
		//bufferStart = arquivo.bytesFileSize;
		mft = leMFT(arquivo.regDir.MFTNumber);
		//Se o arquivo for menor que um bloco e maior que 0 bytes
		if ((arquivo.current_Pointer < (blockSize * SECTOR_SIZE)) && (arquivo.regDir.blocksFileSize >= 0))
		{
			//Le os dados do bloco indicado pelo MFT (iniciando pelo bloco 0)		
			lerDadosMFT(mft, 0, &arquivo, &size, buffer, &bufferStart);		
		}
		arquivosAbertos[handle] = arquivo;

		return bufferStart;
	}else {
		//Em caso de erro retorna um buffer vazio
		memset(buffer,0,sizeof(buffer));
		return -1;
	}
}

int write2(FILE2 handle, char *buffer, int size)
{
	int comecoBuffer = 0, retorno = 0;
	MFT mft;
	ARQUIVO arquivo;
	if(inicializarSistema() == ERROR){
		return ERROR;
	}
	if(handleValido(handle))
	{
		arquivo = arquivosAbertos[handle];
		mft = leMFT(arquivo.regDir.MFTNumber);
		if ((arquivo.current_Pointer < (blockSize * SECTOR_SIZE)) && (arquivo.regDir.blocksFileSize >= 0))
		{
			retorno = escreverRegistro(mft, 0, &arquivo, &size, buffer, &comecoBuffer);		
		}
		else
			retorno = -1;

		arquivosAbertos[handle] = arquivo;

		return retorno;
	}
	else
		return -1;
}

int truncate2 (FILE2 handle){
	if(inicializarSistema() == ERROR){
		return ERROR;
	}
	if(handleValido(handle))
	{
		if(arquivosAbertos[handle].current_Pointer < SECTOR_SIZE*blockSize)
		{
			if(arquivosAbertos[handle].regDir.blocksFileSize > 1)
			{
				arquivosAbertos[handle].regDir.bytesFileSize = arquivosAbertos[handle].regDir.bytesFileSize - SECTOR_SIZE*blockSize; 
			}

			arquivosAbertos[handle].regDir.bytesFileSize = arquivosAbertos[handle].regDir.bytesFileSize - arquivosAbertos[handle].current_Pointer;
			return 0;
		}
		else if(arquivosAbertos[handle].current_Pointer < SECTOR_SIZE*blockSize*2)
		{
			arquivosAbertos[handle].regDir.bytesFileSize = arquivosAbertos[handle].regDir.bytesFileSize - arquivosAbertos[handle].current_Pointer;
			return 0;
		}
	}
	return -1;
}

int seek2 (FILE2 handle, DWORD offset){
	if(inicializarSistema() == ERROR){
		return ERROR;
	}
	if (handleValido(handle))
	{
		if (offset > 0)
		{
			arquivosAbertos[handle].current_Pointer = 0;
			arquivosAbertos[handle].current_Pointer =+ offset;
			return 0;
		}
		else if (offset == -1)
		{
			arquivosAbertos[handle].current_Pointer = arquivosAbertos[handle].regDir.bytesFileSize + 1;
			return 0;
		}
		else{
			return -1;
		}
	}
	else{
		return -1;
	}
}

DIR2 opendir2(char *pathname){
	if (debug) printf("Inicio da opendir2\n");
	MFT mft;
	REGISTRO *regDir, *registro;
	//Quantidade de arquivos que já estão abertos
	if (!(numArquivos < MAXARQUIVOS)) return ERROR;	
	int dirRaiz = 0, MFTNum, i, retorno;
	char nomeSubDir[MAX_FILE_NAME_SIZE];

	if (!inicializarSistema()) return ERROR;

	//Registro unico para funções
	registro = (REGISTRO *) malloc(sizeof(REGISTRO));
	//Lista de registros para funções
	regDir = (REGISTRO *) malloc(SECTOR_SIZE*blockSize*8);

	//Verificação para caso de nome de arquivo errado
	i = strlen(pathname);
	// Se o último caractere for uma barra (e nao for raiz) retorna erro
	if ((i > 2) && (pathname[i-1] == '/')) return -END_OF_DIR;

	//Busca o bloco do diretório
	if (IsDirRaiz(pathname)){
		dirRaiz = 1;
	}
	
	if (!procuraPath(pathname, regDir, registro)){
		return -1;
	}
			
	extractFileName(pathname, nomeSubDir);
	//dirRaiz? printf("Raiz \n") : printf("Nome sub dir: %s\n", nomeSubDir);
	// Verificar se o arquivo já está aberto
	if (isFileOpen(dirRaiz? pathname : nomeSubDir)){
		return -1;
	}

	if (dirRaiz)
		MFTNum = 1;
	else //Ele enctrou no caso de procuraPath e o MFT number está no regDir
		MFTNum  = registro->MFTNumber;

	mft = leMFT(MFTNum);

	if (procuraDiretorio(mft, nomeSubDir, regDir, registro)){
		retorno = abreArquivo(*registro, registro->MFTNumber);
		free(registro);
		free(regDir);
		return retorno;
	}else{
		registro->TypeVal = TYPEVAL_DIRETORIO;
		strcpy(registro->name,"/");
		registro->blocksFileSize = 1;
		registro->bytesFileSize = 1;
		registro->MFTNumber = MFTNum;		
		retorno = abreArquivo(*registro, registro->MFTNumber);
		free(registro);
		free(regDir);
		return retorno;
	}
	return 0;	
}

int readdir2(DIR2 handle, DIRENT2 *dentry){
	if (!handleValido(handle)) return -1;
	if (!inicializarSistema())
		return -1;
	ARQUIVO arquivo;
	REGISTRO *registro = (REGISTRO *) malloc(SECTOR_SIZE*blockSize*8);
	MFT mft;
	arquivo = arquivosAbertos[handle];
	
	if (arquivosAbertos[handle].regDir.TypeVal != TYPEVAL_INVALIDO)
	{
		mft = leMFT(arquivo.regDir.MFTNumber);
		if (readRegister(mft, registro)){
			if (arquivosAbertos[handle].current_Pointer < 64){
				if (registro[arquivosAbertos[handle].current_Pointer].TypeVal != TYPEVAL_INVALIDO){
					strcpy(dentry->name, registro[arquivosAbertos[handle].current_Pointer].name);
					dentry->fileType = registro[arquivosAbertos[handle].current_Pointer].TypeVal;
					// if (dentry->fileType == 1){
					// 	printf("Arquivo ");
					// }else
					// 	printf("Diretorio ");
					
					dentry->fileSize = registro[arquivosAbertos[handle].current_Pointer].bytesFileSize;
				}else{
					dentry->fileType = TYPEVAL_INVALIDO;
					return -1;
				}
			}
			arquivosAbertos[handle].current_Pointer++;
			return 0;
		}else{
			return -1;
		}
	}else{
		return -1;		
	}
}

int closedir2(DIR2 handle){
	ARQUIVO arquivo;
	if (!inicializarSistema())
		return -1;
	if (handleValido(handle))
	{
		arquivo = arquivosAbertos[handle];
		if(arquivo.regDir.TypeVal == TYPEVAL_DIRETORIO)
		{
				arquivosAbertos[handle].regDir.TypeVal = TYPEVAL_INVALIDO;
				return 0;
		}
		else 
			return -1;
	}
	else
		return -1;
	
}

int mkdir2(char *pathname){
	REGISTRO *regDir, *registro, novoRegistro;
	MFT novoMFT, mft;
	char name[MAX_FILE_NAME_SIZE];
	int dirRaiz = 0, blocoNmr, mftNmr, k, i;
	if (!inicializarSistema()) return -1;

	//Registro unico para funções
	registro = (REGISTRO *) malloc(sizeof(REGISTRO));
	//Lista de registros para funções
	regDir = (REGISTRO *) malloc(SECTOR_SIZE*blockSize*8);

	i = strlen(pathname);
	// Se o último caractere for uma barra (e nao for raiz) retorna erro
	if ((i > 2) && (pathname[i-1] == '/')) return ERROR;

	//Verifica se a pasta será criada na raiz ou se os subDiretórios já existem
	if (IsDirRaiz(pathname)){
		dirRaiz = 1;
		mft = leMFT(1);
	}

	if (!procuraPath(pathname, regDir, registro)){
		return -1;
	}
		
	if(!dirRaiz){
		mft = leMFT(registro->MFTNumber);
	}
	// Verificar se já não existe
	extractFileName(pathname, name);
	if (procuraDiretorio(mft, name, regDir, registro)){
        return -1;
    }

	//Cria o registro em memória
	strcpy(novoRegistro.name, name);
	novoRegistro.TypeVal = TYPEVAL_DIRETORIO;
	novoRegistro.blocksFileSize = 1;
	novoRegistro.bytesFileSize = 0;
	blocoNmr = searchBitmap2(NOTALLOCATED);
	if (blocoNmr < 0) return ERROR; // Não tem mais blocos livres
	mftNmr = searchMFT();
	if (mftNmr < 0) return ERROR; //Não tem mais MFTS livres
	novoRegistro.MFTNumber = mftNmr;
	//Cria o novo MFT
	novoMFT.tupla[0].atributeType = 1;
	novoMFT.tupla[0].virtualBlockNumber = 0;
	novoMFT.tupla[0].logicalBlockNumber = blocoNmr; //O bloco livre
	novoMFT.tupla[0].numberOfContiguosBlocks = 1; //Normalmente começa com um bloco só

	novoMFT.tupla[1].atributeType = 0; //Marca o fim do encadeamento das tuplas
	novoMFT.tupla[1].virtualBlockNumber = -1; 
	novoMFT.tupla[1].logicalBlockNumber = -1; 
	novoMFT.tupla[1].numberOfContiguosBlocks = -1; 
	//Zerar as outras tuplas
	for (k = 2; k < MAXTUPLA; k++){
		novoMFT.tupla[k].atributeType = -1; 
		novoMFT.tupla[k].virtualBlockNumber = -1; 
		novoMFT.tupla[k].logicalBlockNumber = -1; 
		novoMFT.tupla[k].numberOfContiguosBlocks = -1; 
	}

	//Criar registro no disco
	if (!dirRaiz){
		k = criarRegistro(novoMFT, novoRegistro, registro->MFTNumber);
	}
	else {
		k = criarRegistro(novoMFT, novoRegistro, 1); //O 1 indica do diretório raiz
	}
	free(registro);
	free(regDir);
	return k;
}

int rmdir2 (char *pathname){
	char nomeSubDir[MAX_FILE_NAME_SIZE];
	int dirRaiz = 0, retorno;
	MFT mft;
	REGISTRO *regDir, *registro;
	//Registro unico para funções
	registro = (REGISTRO *) malloc(sizeof(REGISTRO));
	//Lista de registros para funções
	regDir = (REGISTRO *) malloc(SECTOR_SIZE*blockSize*8);
	if (!inicializarSistema()) return -1;

	if (IsDirRaiz(pathname))
		dirRaiz = 1;
	if (!procuraPath(pathname, regDir, registro))
		return -1;
	
	if (dirRaiz) 
		mft = leMFT(1);
	else 
		mft = leMFT(registro->MFTNumber);

	extractFileName(pathname, nomeSubDir);
	retorno = deletarDiretorio(mft, nomeSubDir);
	free(registro);
	free(regDir);
	return retorno;
}
#include "../include/t2fs.h"
#include "../include/apidisk.h"
#include "../include/bitmap2.h"
#include "../include/t2fs_auxiliar.h"


/* Externs */
extern ARQUIVO arquivosAbertos[MAXARQUIVOS];

/*
	Variáveis globais do sistema
*/
//Variável de debug
extern int debug;

extern char nomeSubDir[MAX_FILE_NAME_SIZE];

//Variável que indica o tamanho de um bloco (4 setores)
extern int blockSize;
//Variável que indica quantos blocos existem na área de MFTBlockSize
extern int MFTBlocksSize;

//Flag que indica se o sistema já foi inicializado ou não
extern int started;

//Variável que indica quantos arquivos estão abertos no sistema
extern int numArquivos;

//Variável que contém os dados do bootBlock
extern BOOTBLOCK bootBlock;

//MFT descritor do bloco raiz
extern MFT descritorRaiz;

//Variável que informa qual é o diretório aberto atualmente
extern REGISTRO *DirAtual;

//Variável que indica qual é o bloco que está sendo usado atualmente
extern int currentBlock;

/*
	Função que atualiza o registro quando ele é fechado
*/
int atualizarRegistro(int MFTNumberPai, REGISTRO registro)
{
	int i = 0, k = 0, blocoPai;
	MFT mftPai, mftFilho;
	REGISTRO *regDirAux;
	regDirAux = (REGISTRO *) malloc(SECTOR_SIZE*blockSize*8);
	mftPai = leMFT(MFTNumberPai);
	mftFilho = leMFT(registro.MFTNumber);
	readRegister(mftPai, regDirAux);
	while (1){
		if((regDirAux[i].TypeVal == registro.TypeVal) && (!strcmp(regDirAux[i].name, registro.name)))
	 		{
 				while(1){
					if (mftPai.tupla[k].atributeType == 1) break;
					k++;
				}
				blocoPai = mftPai.tupla[k].logicalBlockNumber;
				regDirAux[i] = registro;
				escreveMFTDisco(&mftFilho, registro.MFTNumber);
	 			escreveRegistroDisco(regDirAux, blocoPai);
	 			return 1;
	 			break;
	 		}	 		
		i++;
	}
	//YOU SHALL NOT PASS
	return 0;
}

/*
	Função que escreve nos blocos dos registros
	usada em write2
*/
int escreverRegistro(MFT mft, int ponteiroIndex, ARQUIVO *arquivo, int *tamanho, char *buffer, int*bufferComeco)
{
	int i = 0, j, limite, inicio, retorno = -1, k = 0, sectorLimit, bloco;
	unsigned int setor;
	unsigned char bufferLeitura[SECTOR_SIZE];
	for (i = 0; i < SECTOR_SIZE; i++)
		bufferLeitura[i] = 0;
	while (1){
		if (mft.tupla[k].atributeType == 0 || mft.tupla[k].atributeType == -1) break;
		bloco = mft.tupla[k].logicalBlockNumber;
		sectorLimit = mft.tupla[k].numberOfContiguosBlocks;
		k++;
	}
	//Calculo do setor dos dados do MFT
	setor = bloco*blockSize + (arquivo->current_Pointer) / SECTOR_SIZE; //Selecionar o setor que começa o bloco
	//Ultimo setor com os dados do determinado MFT
	limite = bloco*blockSize + (4*sectorLimit) + (arquivo->current_Pointer) / SECTOR_SIZE;
	while ((*tamanho) && (setor <= limite)) 
	{
		read_sector(setor, bufferLeitura);
		inicio = arquivo->current_Pointer % (SECTOR_SIZE - 1);
		j = *bufferComeco;

		for (i = inicio; i < SECTOR_SIZE; i++)
		{
			bufferLeitura[i] = buffer[j]; 
			j++;
			*tamanho = *tamanho - 1;
			*bufferComeco = *bufferComeco + 1;
			retorno = *bufferComeco;
			arquivo->current_Pointer++;
			if (arquivo->current_Pointer >= arquivo->regDir.bytesFileSize)
				arquivo->regDir.bytesFileSize++;
			if (!*tamanho)
			{
				break;
			}
		}
		write_sector(setor, bufferLeitura);
		setor++;
	}

	arquivo->current_Pointer++;
	return retorno;
}


/*
	Função responsável pela leitura dos blocos do MFT
	usado na função read2
*/
int lerDadosMFT(MFT mft, int ptrIndex, ARQUIVO *arquivo, int *size, char *buffer, int*bufferStart)
{
	int i, j, sectorLimit, start, retorno = -1, k = 0, bloco, limite;
	unsigned int setor;
	unsigned char readBuffer[SECTOR_SIZE];
	while (1){
		if (mft.tupla[k].atributeType == 0 || mft.tupla[k].atributeType == -1) break;
		bloco = mft.tupla[k].logicalBlockNumber;
		limite = mft.tupla[k].numberOfContiguosBlocks;
		k++;
	}
	//Calculo do setor dos dados do MFT
	setor = bloco*blockSize + (arquivo->current_Pointer) / SECTOR_SIZE; //Selecionar o setor que começa o bloco
	
	//Ultimo setor com os dados do determinado MFT
	sectorLimit = bloco*blockSize + (4*limite) + (arquivo->current_Pointer) / SECTOR_SIZE;
	while ((*size) && (setor <= sectorLimit)) 
	{
		retorno = read_sector(setor, readBuffer) + retorno;
		
		start = arquivo->current_Pointer % (SECTOR_SIZE - 1);
		j = *bufferStart;
		for (i = start; i < SECTOR_SIZE; i++)
		{
			buffer[j] = readBuffer[i]; 
			if (buffer[j] == '\0'){
				arquivo->current_Pointer = arquivo->current_Pointer-1; //Remover o \0 e o counter a mais
				return *bufferStart-2;		
			}
			j++;
			*size = *size - 1;
			*bufferStart = i-1;
			arquivo->current_Pointer++;
			if (!*size)
			{
				break;
			}
		}
		retorno++;
		setor++;
	}
	return retorno;
}

/*
	Função responsável pela exclusão de pastas em disco
	(não foi aprimorada pra limpar os blocos que os arquivos dentro dela utilizam)
*/
int deletarDiretorio(MFT mft, char *filename)
{
	int i = 0;
	REGISTRO *regDirAux;
	regDirAux = (REGISTRO *) malloc(SECTOR_SIZE*blockSize*8);
	//Lê os dados do MFT por tuplas
	if (readRegister(mft, regDirAux)){
		while (1){
			if (regDirAux[i].TypeVal == TYPEVAL_INVALIDO && regDirAux[i+1].TypeVal == TYPEVAL_INVALIDO) break;
			if ((regDirAux[i].TypeVal == TYPEVAL_DIRETORIO) && (!strcmp(filename, regDirAux[i].name)))			
			{	
				regDirAux[i].TypeVal = TYPEVAL_INVALIDO;
				strcpy(regDirAux[i].name, "esssteNomejamaisexitstira");
				deletarArquivoDisco(regDirAux, regDirAux[i].MFTNumber, mft);
				return 0;
				break;
			}
			i++;
		}
	}
	free(regDirAux);
	return 1;
}

/*
	Função responsável pela exclusão de arquivos em disco
*/
int deletarArquivo(MFT mft, char *filename)
{
	int i = 0;
	REGISTRO *regDirAux;
	regDirAux = (REGISTRO *) malloc(SECTOR_SIZE*blockSize*8);
	//Lê os dados do MFT por tuplas
	if (readRegister(mft, regDirAux)){
		while (1){
			if (regDirAux[i].TypeVal == TYPEVAL_INVALIDO && regDirAux[i+1].TypeVal == TYPEVAL_INVALIDO) break;
			if ((regDirAux[i].TypeVal == TYPEVAL_REGULAR) && (!strcmp(filename, regDirAux[i].name)))			
			{	
				regDirAux[i].TypeVal = TYPEVAL_INVALIDO;
				strcpy(regDirAux[i].name, "esssteNomejamaisexitstira");
				deletarArquivoDisco(regDirAux, regDirAux[i].MFTNumber, mft);
				return 0;
				break;
			}		
			i++;
		}
	}
	free(regDirAux);
	return 0;
}

/*
	Função que faz uma exclusão lógico do arquivo arquivo no disco
	@params
		int mftNumber
	@return int
*/
int deletarArquivoDisco(REGISTRO *registro, int MFTNumberFilho, MFT MFTPai)
{
	MFT mft;
	int k = 0, i, bloco, indiceTUPLA, blocoPai;
	mft = leMFT(MFTNumberFilho);
	while (1){
		//Procura pela tupla que está sendo liberada para encontrar o bloco a ser liberado
		if (mft.tupla[k].atributeType == 1) break;
		k++;
	}
	indiceTUPLA = k;
	while(1){
		if (MFTPai.tupla[k].atributeType == 1) break;
		k++;
	}
	blocoPai = MFTPai.tupla[k].logicalBlockNumber;
	bloco =  mft.tupla[indiceTUPLA].logicalBlockNumber;
	//Libera o bitmap dos blocos (sem precisar limpar o bloco de fato)
	//ocupados pelos blocos do MFT
	for (i = 0; i < mft.tupla[indiceTUPLA].numberOfContiguosBlocks; i++){
		setBitmap2(bloco+i, 0);		
	};
	//Marca como um MFT livre
	mft.tupla[indiceTUPLA].atributeType = -1;

	//Grava MFT e registro no disco
	escreveMFTDisco(&mft, MFTNumberFilho);

	//Atualiza os registros do pai
	escreveRegistroDisco(registro, blocoPai);
	return 1;
}


/*
	Função que grava um novo registro no disco
	@params
	NovoMFT: novo MFT a ser registrado na área de MFT
	NovoRegistro: novo registro a ser registrado
	MFTNumber: nmr do MFT ao qual está recebendo o novo registro (pai)
*/
int criarRegistro(MFT novoMFT, REGISTRO novoRegistro, int MFTNumberPai)
{
	MFT mft;
	REGISTRO *regDirAux = (REGISTRO *) malloc(SECTOR_SIZE*blockSize); //Tamanho de um bloco
	int i = 0, j = 0, k = 0, leitura, bloco = 0, novoBloco, indiceMFT;
	unsigned int setor;
	unsigned char *buffer = (unsigned char *) malloc(SECTOR_SIZE);

	mft = leMFT(MFTNumberPai);	
	//Descobre qual é o último bloco do MFT
	while (1){
		if (mft.tupla[k].atributeType == 0 || mft.tupla[k].atributeType == -1) break;
		bloco = mft.tupla[k].logicalBlockNumber;
		k++;
	}

	indiceMFT = k-1;

	//Verifica quantos registros existem no bloco, se for mais de 16
	//devemos alocar um bloco novo e atualizar o MFT
	if (getNumberofRegisters(bloco) == 16){
		bloco = alocarNovoBloco(mft, MFTNumberPai);
		indiceMFT = k;
	};

	novoBloco = novoMFT.tupla[indiceMFT].logicalBlockNumber;

	//Copia os dados para um buffer
	for (i=0; i < blockSize; i++) //BlockSize = 4 setores
	{	
		setor = bloco*blockSize + i; //Selecionar o setor que começa o bloco
		leitura = read_sector(setor, buffer); // Realiza leitura por setor
		if (!leitura){
			memcpy((regDirAux+j), buffer, 256);
			j = j+4;
		}
		else return ERROR;
	}

	//Concatena o novo registro no bloco (cada setor pode ter 4 registros)
	//ou cada bloco pode ter 16 registros
	i = 0;
	while (1){
		if (regDirAux[i].TypeVal == TYPEVAL_INVALIDO) break;
		i++;
	}
	regDirAux[i] = novoRegistro;
	
	//Como na criaçao de um novo registro ele ocupa somente um bloco ou 1 novo bloco
	//escreveRegistroDisco(registro, bloco, novoMFT.tupla[indiceMFT].logicalBlockNumber);
	escreveRegistroDisco(regDirAux, bloco);
	escreveMFTDisco(&novoMFT, novoRegistro.MFTNumber);
	//Escreve no mapa de bitmaps dos dados que o bloco está ocupado
	setBitmap2(novoBloco, 1);

	free(buffer);
	free(regDirAux);

	return 0;
}

/*
	Função que aloca novo bloco atualizando o MFT
	@params MFT mft
	@return int bloco
*/
int alocarNovoBloco(MFT mft, int MFTNumber){
	int k = 0, blocoNmr;
	//Encontra uma tupla livre para ser registrada
	while (1){
		if (mft.tupla[k].atributeType == 0 || mft.tupla[k].atributeType == -1) break;
		k++;
	}

	//Inicializa os valores em memória
	blocoNmr = searchBitmap2(NOTALLOCATED);
	mft.tupla[k].atributeType = 1;
	mft.tupla[k].virtualBlockNumber = 0;
	mft.tupla[k].logicalBlockNumber = blocoNmr; //O bloco livre
	mft.tupla[k].numberOfContiguosBlocks = 1; //Normalmente começa com um bloco só

	mft.tupla[k+1].atributeType = 0; //Marca o fim do encadeamento da tupla posterior
	mft.tupla[k+1].virtualBlockNumber = -1; 
	mft.tupla[k+1].logicalBlockNumber = -1; 
	mft.tupla[k+1].numberOfContiguosBlocks = -1;

	//Grava valores em disco
	escreveMFTDisco(&mft, MFTNumber);
	return blocoNmr;
}

/*
	Função que escreve um bloco (com t2fs_records) no disco
	@params
	bloco
	registros
	blocoBitMap
*/
int escreveRegistroDisco(REGISTRO *registros, int bloco)
{
	int i, j = 0;
	unsigned int setor;
	unsigned char *buffer = (unsigned char *) malloc(SECTOR_SIZE);
	//Realiza a escrita dos registros setor por setor
	for (i = 0; i < blockSize; i++){
		setor = bloco*blockSize + i; //Selecionar o setor que começa o bloco
		//Passar para o buffer a quantidade de dados de um setor
		memcpy(buffer, &registros[j], 256);
		j = j+4;
		if (write_sector(setor, buffer)) return ERROR;
	}
	return 1;
}

/*
	Função que escreve um MFT novo no disco
	@params
	MFT mft
	int MftNumber
*/
int escreveMFTDisco(MFT *mft, int mftNumber)
{
	int i, j = 0;
	unsigned int setor;
	unsigned char *buffer = (unsigned char *) malloc(2*SECTOR_SIZE); //São 2 setores pra cada MFT
	unsigned char *buffer2 = (unsigned char *) malloc(SECTOR_SIZE); //Tamanho de 1 setor
	//Copia todos os MFT para um buffer auxiliar
	memcpy(buffer, &mft[0], 512);
	//Realiza a escrita dos registros setor por setor
	//Cada 2 setores formam um MFT
	for (i = 0; i < 2; i++){
		setor = 4+(mftNumber*2)+i; //Selecionar o setor que começa o bloco
		//Passar para o buffer a quantidade de dados de um setor
		memcpy(buffer2, buffer+j, 256);
		j = j+256;
		if (write_sector(setor, buffer2)) return ERROR;
	}
	free(buffer);
	free(buffer2);
	return 1;
}


/* 
	Função que procura um arquivo dado MFT 
*/
int procuraArquivo(MFT mft, char *nomeArquivo, REGISTRO *regDir, REGISTRO *registro){
	int i = 0;
	REGISTRO *regDirAux;
	regDirAux = (REGISTRO *) malloc(SECTOR_SIZE*blockSize*8);
	//Lê os dados do MFT por tuplas
	if (readRegister(mft, regDirAux)){
		while (1){
			if (regDirAux[i].TypeVal == TYPEVAL_INVALIDO && regDirAux[i+1].TypeVal == TYPEVAL_INVALIDO) break;
			if ((regDirAux[i].TypeVal == TYPEVAL_REGULAR) && (!strcmp(nomeArquivo, regDirAux[i].name)))			
			{	
				*registro = regDirAux[i];
				return 1;
				break;
			}
			i++;
		}
	}
	free(regDirAux);
	//Não encontrou
	return 0;
}

/* 
	Função que procura um diretorio dado MFT
*/
int procuraDiretorio(MFT mft, char *diretorio, REGISTRO *regDir, REGISTRO *registro)
{
	int i = 0;
	REGISTRO *regDirAux;
	regDirAux = (REGISTRO *) malloc(SECTOR_SIZE*blockSize*8);
	//Lê os dados do MFT por tuplas
	if (readRegister(mft, regDirAux)){
		while (1){
			if (regDirAux[i].TypeVal == TYPEVAL_INVALIDO && regDirAux[i+1].TypeVal == TYPEVAL_INVALIDO) break;
			if ((regDirAux[i].TypeVal == TYPEVAL_DIRETORIO) && (!strcmp(diretorio, regDirAux[i].name)))			
			{	
				*registro = regDirAux[i];
				return 1;
				break;
			}		
			i++;
		}
	}
	free(regDirAux);
	return 0;
}


/*
	Função que busca se o pathname já existe e retorna no record passado como parametro
*/
int procuraPath(char *pathname, REGISTRO *regDir, REGISTRO *registro)
{	
	char *diretorio;	
	char *diretorioAux;
	MFT mft;
	diretorioAux = malloc(strlen(pathname) + 1); 
    strcpy(diretorioAux, pathname);

	// se o primeiro caractere é igual ao diretório raíz
	if (pathname[0] == '/')	
		{
			// o MFT recebe o MFT do dirRaiz
			// Vejo quais registros existem no raiz pra ver se a pasta existe
			mft = leMFT(1);
			diretorioAux = &diretorioAux[1];
			diretorioAux = strtok(diretorioAux, "/");
		}
	else 
		return 0;
	
	
	diretorio = diretorioAux; //guarda o diretorio pai
	//procurar pelo diretorio que foi passado
	while(diretorio != NULL)
	{
		diretorioAux = strtok(NULL, "/"); //Diretorio pra ver se existe
		if (diretorioAux != NULL) //Se não for o raiz
		{
			if (procuraDiretorio(mft, diretorio, regDir, registro)) //Se encontrou o diretório, vai para o próximo
				mft = leMFT(registro->MFTNumber);
			else
				return 0;	
		}
		diretorio = diretorioAux;
	}
	free(diretorioAux);
	return 1;
}


/*
	Função que divide o caminho do arquivo em partes (a cada barra encontrada)
	Salva a primeira parte em "nomeSubDir" e retorna o resto do caminho
*/
char* DivideString(char* pathname){
	char *s;
	char *d;
	s = pathname;
	if(*s=='/')	s++;
	d = s;
	while(*d != '/' && *d != '\0') {
		++d;
   }
    if(*d != '\0')
	{
		*d = '\0';
		d++;
	}
	strcpy(nomeSubDir,s);
	return d;
}


/*
	Função que verifica se o handle informado é válido
*/
int handleValido(int handle)
{
	if ((handle >= 0) && (handle < 20))
		return 1;
	else 
		return 0;

}

/* 
	Abre um arquivo e retorna o handle
*/
int abreArquivo(REGISTRO regDir, int MFTNumber)
{
	int i;
	for (i = 0; i<MAXARQUIVOS; i++)
	{
		if (arquivosAbertos[i].regDir.TypeVal == TYPEVAL_INVALIDO && arquivosAbertos[i+1].regDir.TypeVal == TYPEVAL_INVALIDO)	
		{
			arquivosAbertos[i].regDir = regDir;				
			arquivosAbertos[i].current_Pointer = 0;
			arquivosAbertos[i].MFTNumber = MFTNumber; 
			return i;
		}
	}
	return -1;
}

/*
	Função que verifica se o diretório que está sendo aberto é o diretório raiz
*/
int IsDirRaiz2(char *pathname)
{
	int aux;
	//Para ser raiz ele tem size=1 pois "\\0"
	if (strlen(pathname) == 1){
		(pathname[0] == '/') ? (aux = 1) : (aux = 0);
	}else{
		aux = 0;
	}
	return aux;
}

int IsDirRaiz(char *pathname)
{
	char *pathname2;
	char *aux;
	// Se inicializa com / é o diretório raiz
	if (pathname[0] == '/')
	{
		pathname2 = malloc(strlen(pathname) + 1); 
	    
		// Passa pathname para pathname2 (que será alterada)
		strcpy(pathname2, pathname);

		//início da string
		aux = pathname2;

		pathname2 = strtok(pathname2, "/"); 
		//Pegar o ultimo nome
		pathname2 = strtok(NULL, "/"); // Remove a primeira barra
		
		if (pathname2 == NULL) //não é pra gravar em raiz
		{
			free(aux);
			return 1;
		}
		else // Grava no dir raiz
		{ 
			free(aux);
			return 0;
		}
	}
	else 
		return 0;
}

/*
	Função que verifica se o arquivo já está aberto
*/
int isFileOpen(char *filename)
{
	int i;
	
	for (i = 0; i<MAXARQUIVOS; i++)
	{		
		if (!(strcmp(filename, arquivosAbertos[i].regDir.name)) && (arquivosAbertos[i].regDir.TypeVal != TYPEVAL_INVALIDO))
		{
			return 1;
		}
	}
	return 0;

}

/* 
	Função que extrai o nome do arquivo a partir de um pathname
*/
void extractFileName(char *pathname, char *filename)
{
	char *filenameAux = malloc(strlen(pathname) + 1); 
	strcpy(filenameAux, pathname);
	if (pathname[0] == '/')	
		{
			filenameAux = &filenameAux[1];
			filenameAux = strtok(filenameAux, "/");
			
		}
	while(filenameAux != NULL)
	{
		strcpy(filename, filenameAux);
	
		filenameAux = strtok(NULL, "/");
	}
}

/*	Função que lê os registros do param bloco
	Retorna um array com os todos records do MFT apontado
 */
int readRegister(MFT mft, REGISTRO *registro)
{
	
	int i, j = 0, k = 0, leitura, bloco = 0;
	unsigned int setor;
	unsigned char *buffer = (unsigned char *) malloc(SECTOR_SIZE);
	REGISTRO *regRetorno = (REGISTRO *) malloc(SECTOR_SIZE*blockSize*8); //Tamanho arbitrário que acredito que nunca va passar
	while (1){
		if (mft.tupla[k].atributeType == 0 || mft.tupla[k].atributeType == -1) break;
		bloco = mft.tupla[k].logicalBlockNumber;
		for (i=0; i < blockSize; i++) //BlockSize = 4 setores
		{	
			setor = bloco*blockSize + i; //Selecionar o setor que começa o bloco
			leitura = read_sector(setor, buffer); // Realiza leitura por setor
			if (!leitura){
				memcpy((regRetorno+j), buffer, 256);
				j = j+4;		
			}
			else return ERROR;
		}
		k++;
	}	
	
	memcpy(registro, regRetorno, SECTOR_SIZE*blockSize*8);
	free(buffer);
	free(regRetorno);
	return 1;
}

/*Função que inicializa o sistema, ou seja:
	-> lê os dados do boot block
	-> lê os MFTs base:
		Registro 0 (zero): descritor do arquivo que fornece o bitmap de blocos de dados livres e ocupados;
		Registro 1 (um): descritor do arquivo associado ao diretório raiz;
		Registros 2 e 3: descritores reservados para uso futuro
*/
int inicializarSistema(){
	if (!started){
		started = TRUE;
		
		//Lê o setor 0 para a estrtura do bootBlock
		if(read_sector(0, (unsigned char*)&bootBlock) != 0) return ERROR;
		
		//Inicialização das variáveis globais
		blockSize = bootBlock.blockSize;
		MFTBlocksSize = bootBlock.MFTBlocksSize;		
		
		//Lê o descritor do MFT raiz
		descritorRaiz = leMFT(1);
		
		//Inicializa currentBlock
		currentBlock = descritorRaiz.tupla[0].logicalBlockNumber;
	
		//Inicializa os arquivos abertos
		inicializaArquivosAbertos();
	}
	if (debug) imprimirDadosRoot();
	if (debug) imprimirDadosBootBlock();
	return 1;
}

//Função que lê o MFT indicado por MFTNumber e retorna o MFT
MFT leMFT(int MFTNumber){
	int i, j = 0, k;
	unsigned char *buffer = (unsigned char *) malloc(SECTOR_SIZE);
	TUPLA TuplaRetorno;
	MFT MFTRetorno;
	//O for começa em 4 pois no bloco 0 está o boot block (cada bloco possui 4 setores)
	//A cada 2 setores há um MFT, portanto: 4+(MFTNumber*2)+k
	for (k = 0; k < 2; k++){
		if(read_sector(4+(MFTNumber*2)+k, (unsigned char*) buffer) == ERROR){
			return MFTRetorno;
		}
		for (i = 0; i<SECTOR_SIZE; i = i+TUPLA_SIZE){
			TuplaRetorno = uctoTupla(buffer, i);
			MFTRetorno.tupla[j] = TuplaRetorno;
			j++;
		}
	}
		
	free(buffer);
	return MFTRetorno;
}


//Função que procura o primeiro MFT livre e retorna seu número
int searchMFT(){
	int i;
	unsigned char *buffer = (unsigned char *) malloc(SECTOR_SIZE);
	TUPLA TuplaRetorno;
	//O for começa em 4 pois no bloco 0 está o boot block (cada bloco possui 4 setores)
	//A cada 2 setores há um MFT, portanto: 4+(i*2)
	//Começando pelo MFT de nmr i = 4 pois os anteriores são reservados
	i = 4;
	while(1){
		if(read_sector(4+(i*2), (unsigned char*) buffer) == ERROR){
			return ERROR;
		}
		TuplaRetorno = uctoTupla(buffer, 0);
		if (TuplaRetorno.atributeType == -1){
				return i;
				break;
			}
		i++;
	}	
	free(buffer);
	return i;
}

/*
	Função que retorna quando registros já existem em um bloco
*/
int getNumberofRegisters(int bloco){
	REGISTRO *regDirAux = (REGISTRO *) malloc(SECTOR_SIZE*blockSize); //Tamanho de um bloco
	int i = 0, j = 0, leitura;
	unsigned int setor;
	unsigned char *buffer = (unsigned char *) malloc(SECTOR_SIZE);
	//Copia os dados para um buffer
	for (i=0; i < blockSize; i++) //BlockSize = 4 setores
	{	
		setor = bloco*blockSize + i; //Selecionar o setor que começa o bloco
		leitura = read_sector(setor, buffer); // Realiza leitura por setor
		if (!leitura){
			memcpy((regDirAux+j), buffer, 256);
			j = j+4;
		}
		else return ERROR;
	}

	while (1){
		if (regDirAux[i].TypeVal == TYPEVAL_INVALIDO && regDirAux[i+1].TypeVal == TYPEVAL_INVALIDO) break;
		i++;
	}
	free(regDirAux);
	return i;
}


/*
	String to DWORD
	params
	@unsigned char *string
		Variável unsingned char para transformar em DWORD
	@int inicio
		Variável para indicar onde começa a leitura do string
*/

DWORD stodw(unsigned char *string, int inicio)
{
	unsigned char retorno[4];
	int i;
	for (i = 0; i < 4; i++){
		retorno[i] = string[inicio+i];
	}
	return (*((DWORD *)retorno));
}


/*
	Unsigned char to TUPLA
	params
	@unsigned char *string
		Variável unsingned char para transformar em DWORD
	@int inicio
		Variável para indicar onde começa a leitura do string
*/
TUPLA uctoTupla(unsigned char *string, int inicio)
{
	unsigned char retorno[TUPLA_SIZE];
	int i;
	for (i = 0; i < TUPLA_SIZE; i++){
		retorno[i] = string[inicio+i];
	}
	return (*((TUPLA *)retorno));
}

/* 
	Função que converte inteiro para string
*/
int inteiroParaString(unsigned char *string, int inicio, int inteiro)
{	
	unsigned char *bufferAux;
	bufferAux = (unsigned char*)&inteiro;
	int i;
	for (i = 0; i < 4; i++){
		string[inicio+i] = bufferAux[i];
	}
	return 0;
}

/*
	Função que inicializa os arquivos abertos em memória
*/
void inicializaArquivosAbertos()
{
	int i;
	for (i=0; i<MAXARQUIVOS; i++)
	{		
		arquivosAbertos[i].regDir.TypeVal = TYPEVAL_INVALIDO;
	}
}


/*
	Função utilizada apenas para debug
*/
void imprimirDadosBootBlock(){
	printf("---- Bloco de boot: ----\n");
	printf("%c%c%c%c\n", bootBlock.id[0], bootBlock.id[1], bootBlock.id[2], bootBlock.id[3]);
	printf("Versao: %d \n", bootBlock.version);
	printf("BlockSize: %u \n", bootBlock.blockSize);
	printf("MFTBlocksSize: %u \n", bootBlock.MFTBlocksSize);
	printf("diskSectorSize: %u \n", bootBlock.diskSectorSize);
}

void imprimirDadosRoot(){
	int i = 0;
	printf("---- Descritor do MFT raiz: ----\n");
	while (1){
		if (descritorRaiz.tupla[i].atributeType == 0 || descritorRaiz.tupla[i].atributeType == -1) break;
		printf("atributeType: %d\n", descritorRaiz.tupla[i].atributeType);
		printf("virtualBlockNumber: %d\n", descritorRaiz.tupla[i].virtualBlockNumber);
		printf("logicalBlockNumber: %d\n", descritorRaiz.tupla[i].logicalBlockNumber);
		printf("numberOfContiguosBlocks: %d\n", descritorRaiz.tupla[i].numberOfContiguosBlocks);
		i++;
	}
}


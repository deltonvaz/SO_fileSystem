#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/t2fs.h"
/*
	NESSE TESTE SÃO REALIZADAS VERIFICAÇÕES NA CRIAÇÃO DE DIRETORIO DENTRO DE DIRETORIO
	E CRIAÇÃO DE ARQUIVOS DENTRO DE DIRETORIO
	APOS ISSO É TESTADA A EXCLUSÃO DO ARQUIVO
	E A EXCLUSÃO DO DIRETORIO dir2 IMPOSSIBILITANDO A CRIAÇÃO DE UM NOVO ARQUIVO DENTRO DELE
	
	FUNÇÕES TESTADAS:
		mkdir2()
		rmdir()
		create2()
		delete2()

*/
int main(){
	char string1[] = "/dir1";
	char string2[] = "/dir1/dir2";
	char string3[] = "/dir1/dir2/arquivo1";
	char string4[] = "/dir1/dir2/arquivo2";

	if(!mkdir2(string1))
		printf("[D]iretorio de path %s criado.\n", string1);
	
	if(!mkdir2(string2))
		printf("[D]iretorio de path %s criado.\n", string2);
	
	if(!create2(string3))
		printf("[A]rquivo %s criado.\n", string3);
	
	if(!delete2(string3))
		printf("[A]rquivo %s deletado.\n", string3);
	
	if(!rmdir2(string2))
		printf("[D]iretorio de path %s deletado.\n", string2);
	
	//Deve ocorrer um erro aqui pois a pasta nao existe mais
	if(create2(string4) == 0)
		printf("[A]rquivo %s criado.\n", string4);
	else
		printf("[E]rro ao criar o arquivo %s.\n", string4);
	
	return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/t2fs.h"

int main(){
	//Arquivo com nome errado
	char string1[] = "arq1";
	//Esse nome está correto pois começa com o / de dirRaiz
	char string2[] = "/arq2";
	//Esse arquivo vai retornar erro pois o arq1 ainda não existe
	char string3[] = "/arq1/arqui2";
	
	if(create2(string1) == 0)
		printf("Arquivo de nome %s criado.\n", string1);
	else
		printf("[E] Erro ao criar arquivo de nome %s.\n", string1);
	
	
	if(create2(string2) == 0)
		printf("[S] Arquivo de nome %s criado com sucesso.\n", string2);
	else
		printf("[E] Erro ao criar arquivo de nome %s.\n", string2);
	
	
	if(delete2(string2) == 0)
		printf("[S] Arquivo de nome %s deletado.\n", string2);
	else
		printf("[E] Erro ao excluir arquivo de nome %s.\n", string2);
	
	
	if(create2(string3) == 0)
		printf("[S] Arquivo de nome %s criado.\n", string3);
	else
		printf("[E] Erro ao criar arquivo de nome %s.\n", string3);
	return 0;
}

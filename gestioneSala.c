#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>

#include "cinema.h"
#include "gestioneSala.h"
#define FILE_S "sala.txt"
/*
 * Metodo di supporto all'inizializzazione dello stadio: alloca lo spazio nello heap
 * per le file del settore i-esimo e per i posti di tali file.
 * Inizializza file e posti del settore i-esimo.
 */

char** split_line(char* s){ //side effect su fila e posto
	char* p=malloc(sizeof(char)*1024);
	static char** ret;
  ret = malloc(sizeof(char*)*2);
  ret[0]=malloc(sizeof(char)*16);
  ret[1]=malloc(sizeof(char)*1024);
	strcpy(p,s);
	int i =0;
	p = strtok(s, " ");
	printf("[INPUT_POSTO] %s\n", p);
	while(p != NULL) {
   printf("%s\n", p);
		if(i==0) strcpy(ret[0],p);
		else if(i==1) strcpy(ret[1],p);
   p = strtok(NULL, " ");
		i++;
	}
	return ret;
}

void creaFilePosti(int i, int nfile, int npostifila){

  char line[1024];
  FILE *f = fopen(FILE_S, "r");
  if(f!=NULL){
    //esiste sala!!
    if(fgets(line,sizeof(line),f)==NULL){
      //eof allora esite file, ma vuoto
    }
    else{
      //esiste sala salvata
      //prima riga nFile, primo el riga nPosti
      int n_file = atoi(line);
      printf("-%s\n", line);
      printf("%d\n", n_file);
      int i,j;
      sala.nFile = n_file;
    	sala.file = malloc(sizeof(struct fila)*n_file);
      for(j=0; j<n_file; j++){
        fgets(line,sizeof(line),f);
        int n_posti;
        char filara[1024];
        char** ret=malloc(sizeof(char*)*2);
        ret[0] = malloc(sizeof(char)*16);
        ret[1] = malloc(sizeof(char)*1014);
        ret = split_line(line);
        printf("%s - %s\n",ret[0],ret[1]);
        //n_posti = atoi(ret[0]);
        n_posti=20;

        sala.file[j].nPosti = n_posti;
    		sala.file[j].posti = malloc(sizeof(struct posto)*n_posti);
        printf("OK-4\n");
        strcpy(filara, ret[1]);
        printf("OK-5\n");
        for(i=0; i<n_posti;i++){
          printf("OK-6\n");
          sala.file[j].posti[i].codice = i;
          printf("OK-7\n");
          sala.file[j].posti[i].codice_fila = j;
          printf("OK-8\n");
          int c;
          if(filara[i]=='0') c=0;
          else c=1;
          sala.file[j].posti[i].prenotato = c;
          printf("OK-9\n");
        }
        sala.file[j].codice = j;
    		sala.file[j].nPostiLiberi = npostifila;
      }
      fclose(f);
    }
  }else{
  int j,k;
	sala.nFile = nfile;
	sala.file = malloc(sizeof(struct fila)*nfile);
	for (j = 0; j < nfile; j++){
		sala.file[j].nPosti = npostifila;
		sala.file[j].posti = malloc(sizeof(struct posto)*npostifila);
		for (k = 0; k < npostifila; k++){
			sala.file[j].posti[k].codice = k;
			sala.file[j].posti[k].codice_fila = j;
			sala.file[j].posti[k].prenotato = 0;
		}
		sala.file[j].codice = j;
		sala.file[j].nPostiLiberi = npostifila;
		//pthread_mutex_init(&sala.file[j].mx_fila,NULL);
	}
  }
}
void inizializzaSala(){
	int i;
	int nfile=9;
	int npostifila=20;
	int costoIntero=0;
	pthread_mutexattr_t attr;

	//inizializzo sala
		creaFilePosti(i, nfile, npostifila);

		sala.postiDisponibili = nfile * npostifila;

		pthread_mutexattr_init(&attr);
		pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ERRORCHECK_NP);
		pthread_mutex_init(&sala.mx_aggiornamentoSala,&attr);
		pthread_mutexattr_destroy(&attr);

    pthread_mutexattr_init(&attr);
		pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ERRORCHECK_NP);
		pthread_mutex_init(&sala.mx_aggiornamentoFilePrenotazioni,&attr);
		pthread_mutexattr_destroy(&attr);

    char line[CODLEN];
    //cerco se esiste già un cod da cui partire
    FILE *f = fopen("codice.txt","r");
    if(f!=NULL){
      if(fgets(line,sizeof(line),f)!=NULL){
        strcpy(COD_PREN,line);
        fclose(f);
      }
    }
    else{
      strcpy(COD_PREN,"0");
    }
		printf("<SERVER-INFO> cinema Inizializzato\n");
}

//stampa sala
void stampaSala(struct sala_cinema s){
	int j,k;
  pthread_mutex_lock(&(sala.mx_aggiornamentoSala));
	for (j = 0; j < s.nFile; j++){
		for (k = 0; k < s.file[j].nPosti; k++){
			printf("%d  ", s.file[j].posti[k].prenotato);
		}
		printf("\n");
	}
  pthread_mutex_unlock(&(sala.mx_aggiornamentoSala));
}

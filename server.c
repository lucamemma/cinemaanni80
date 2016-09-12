
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>  /* per usare socket */
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <pthread.h>
#include <errno.h>        /* gestione degli errori */
#include <signal.h>

#include "cinema.h"
#include "gestioneSala.h"
#define FILE_F "prenotazioni.txt"
#define FILE_S "sala.txt"

char COD_PREN[CODLEN]="0";

int * input_posto_blank(char* s){
	char* p=malloc(sizeof(char)*16);
	static int ret[2];
	strcpy(p,s);
	int i =0;
	p = strtok(s, " ");
	//printf("[INPUT_POSTO] %s\n", p);
	while(p != NULL) {
    printf("%s\n", p);
		if(i==0) ret[0] = atoi(p);
		else if(i==1) ret[1]=atoi(p);
    p = strtok(NULL, " ");
		i++;
	}
	return ret;
}
void sig_handler(int signo){
		if (signo == SIGINT){
				salva_sala();
				salva_cod();
		}
		else if (signo == SIGINT){
				salva_sala();
				salva_cod();
		}
}

//invia info sala
int info(int socket) {
	int i, j;
	//manda numero file
	pthread_mutex_lock(&(sala.mx_aggiornamentoSala));
	if (send(socket, &(sala.nFile), sizeof(int),0)< 0) {
		printf("[SERV-ERR] Errore nella write dalla socket (-1) %s",strerror(errno));
		exit(1);
	}
	for(i=0; i<sala.nFile; i++){
		//per ogni fila manda il numero posti
		if (send(socket, &(sala.file[i].nPosti), sizeof(int),0)< 0) {
			printf("[SERV-ERR] Errore nella write dalla socket (-1) %s",strerror(errno));
			exit(1);
		}
		for(j=0; j<sala.file[i].nPosti; j++){
			//INT codice, INT codice_fila,INT prenotato. manda i dati struttura posto
			if (send(socket, &(sala.file[i].posti[j].codice), sizeof(int),0)< 0) {
				printf("[SERV-ERR] Errore nella write dalla socket (-1) %s",strerror(errno));
				exit(1);
			}
			if (send(socket, &(sala.file[i].posti[j].codice_fila), sizeof(int),0)< 0) {
				printf("[SERV-ERR] Errore nella write dalla socket (-1) %s",strerror(errno));
				exit(1);
			}
			if (send(socket, &(sala.file[i].posti[j].prenotato), sizeof(int),0)< 0) {
				printf("[SERV-ERR] Errore nella write dalla socket (-1) %s",strerror(errno));
				exit(1);
			}
		}
	}
	pthread_mutex_unlock(&(sala.mx_aggiornamentoSala));
	return socket;
}

char* get_codice_univoco(){  //numerico 0-9 ma string (più facile utilizzo)
	//prendi cod pren
	static char ret[CODLEN];
	strcpy(ret, COD_PREN);
	int n = atoi(COD_PREN);
	//incrementa cod pren
	n++;
	sprintf(COD_PREN, "%d", n);
	return ret;
}

int controlla_validita_posti(struct posto* pren, int n){ //TODO mutex
	int i, valido=1;
	pthread_mutex_lock(&(sala.mx_aggiornamentoSala));
	for(i=0; i<n; i++){
		//controllo indici validi
		if((pren[i].codice_fila>=sala.nFile) || (pren[i].codice>=sala.file[i].nPosti)){
			valido = 0;
			return valido;
		}
		//controllo posto libero
		if(sala.file[pren[i].codice_fila].posti[pren[i].codice].prenotato==1){
			valido =  0;
			return valido;
		}
	}
	pthread_mutex_unlock(&(sala.mx_aggiornamentoSala));
	return valido;
}

void prenota_posti(struct posto* pren, int n){
	int i;
	for(i=0; i<n; i++){
		//setto posti
		sala.file[pren[i].codice_fila].posti[pren[i].codice].prenotato=1;
		}
}
void sprenota_posti(struct posto* pren, int n){
	int i;
	for(i=0; i<n; i++){
		//setto posti
		sala.file[pren[i].codice_fila].posti[pren[i].codice].prenotato=0;
		}
}
void salva_prenotazione(struct posto* posti, int n, char *c){

	FILE *fd=fopen(FILE_F, "a");
	if(fd==NULL) {
		perror("Errore in apertura del file");
		exit(1);
	}
		/* scrive il numero */
		pthread_mutex_lock(&(sala.mx_aggiornamentoFilePrenotazioni));
		int i =0;
		fprintf(fd, "\n%s\n%d\n", c, n);
		for (i=0; i<n;i++){
				fprintf(fd, "%d %d\n",posti[i].codice_fila, posti[i].codice);//, posti[i].prenotato);
		}
		pthread_mutex_unlock(&(sala.mx_aggiornamentoFilePrenotazioni));
	fclose(fd);
}

void salva_sala(){
	FILE *fd=fopen(FILE_S, "w");
	if(fd==NULL) {
		perror("Errore in apertura del file");
		exit(1);
	}
		int i,j;
		pthread_mutex_lock(&(sala.mx_aggiornamentoSala));
		fprintf(fd, "%d\n",sala.nFile);
		for (i=0; i<sala.nFile;i++){
			fprintf(fd, "%d ",sala.file[i].nPosti);
			for(j=0;j<sala.file[i].nPosti;j++){
				fprintf(fd, "%d",sala.file[i].posti[j].prenotato);//, posti[i].prenotato);
			}
			fprintf(fd, "\n");
		}
		pthread_mutex_unlock(&(sala.mx_aggiornamentoSala));
	fclose(fd);
}

void accettaPrenotazioneEffettuata(int socket){
	printf("ACCETTA prenotazione effettuata\n");
	//riceve len di prenotazione
	int n_posti,i;
	if (recv(socket, &(n_posti), sizeof(int),0)< 0) {
		printf("[SERV-ERR] Errore nella write dalla socket (-1) %s",strerror(errno));
		exit(1);
	}
	//riceve i posti da prenotare
	struct posto* prenotazione=malloc(sizeof(struct posto)*n_posti);

	for(i=0; i<n_posti; i++){
		if (recv(socket, &(prenotazione[i].codice), sizeof(int),0)< 0) {
			printf("[SERV-ERR] Errore nella write dalla socket (-1) %s",strerror(errno));
			exit(1);
		}
		if (recv(socket, &(prenotazione[i].codice_fila), sizeof(int),0)< 0) {
			printf("[SERV-ERR] Errore nella write dalla socket (-1) %s",strerror(errno));
			exit(1);
		}
		if (recv(socket, &(prenotazione[i].prenotato), sizeof(int),0)< 0) {
			printf("[SERV-ERR] Errore nella write dalla socket (-1) %s",strerror(errno));
			exit(1);
		}
		//printf("%d",pren);
	}
	/*for(i=0; i<n_posti;i++){
		printf("[DEBUG] FILA:%d POSTO:%d\n",prenotazione[i].codice_fila,prenotazione[i].codice );
	}*/
	//controlla validità posti
	//int ok = controlla_validita_posti(prenotazione);
	char conferma[CODLEN];
	if(controlla_validita_posti(prenotazione, n_posti)){
		pthread_mutex_lock(&(sala.mx_aggiornamentoSala));
			prenota_posti(prenotazione, n_posti);
		pthread_mutex_unlock(&(sala.mx_aggiornamentoSala));
		strcpy(conferma, get_codice_univoco());
		// MEMORIZZA PREN E COD salva prenotazione
		salva_prenotazione(prenotazione, n_posti, conferma);
	}
	else{
		strcpy(conferma, NONDIS);
	}

	if (send(socket, conferma, sizeof(char)*CODLEN,0)< 0){
		printf("[SERV-ERR] Errore nella write dalla socket (-1) %s",strerror(errno));
		exit(1);
	}

}

void annullaPrenotazioneEffettuata(int socket){
	printf("ANNULLA prenotazione effettuata\n");
	/* -dato codice prenotazione
		-cerco nel file prenotazione
		-se la trovo, la leggo
				- pongo il codice di quella prenotazione a -1
				- per ogni posto della prenotazione, porto a 0 il "prenotato" di quel posto nella sala
	*/
	//ricevo il codice prenotazione
	char cod_pre[CODLEN];
	if (recv(socket, cod_pre, sizeof(char)*CODLEN,0)< 0) {
		printf("[SERV-ERR] Errore nella write dalla socket (-1) %s",strerror(errno));
		exit(1);
	}
	// leggo il file
	FILE* fd=fopen(FILE_F, "r");

	if(fd==NULL) exit(1);
	struct posto* posti_file = NULL;
	char cod[CODLEN];
	int flag = 0, i, n_posti, num_posti;

			char line[128];
			char* r = malloc(sizeof(char)*128);
			//r = fgets(line,sizeof(line),fd);
			pthread_mutex_lock(&(sala.mx_aggiornamentoFilePrenotazioni));
      while (r!=NULL){
					r = fgets(line,sizeof(line),fd);
					fputs (line, stdout);

					if(strcmp(line,"\n")==0){  //allora prossima riga codice
						r = fgets(line,sizeof(line),fd);
						fputs (line, stdout);
						char c[4];
						sprintf(c, "%d", atoi(line)); //i to S
						if(strcmp(cod_pre, c)==0){ //codice
							flag=1;

						}
						fgets(line,sizeof(line),fd); //numero posti
						fputs (line, stdout);
						n_posti = atoi(line); //quante righe devo leggere per il prossimo record
						//printf("NPOSTI FILE%d\n",n_posti);
						if(flag) {posti_file = malloc(sizeof(struct posto)*n_posti);num_posti=n_posti;}
						int *p = malloc(sizeof(int)*2);
						for(i=0; i<n_posti;i++){
								r = fgets(line,sizeof(line),fd);
								fputs (line, stdout);
								if(flag){
									p = input_posto_blank(line);
									printf("- P0: %d - P1: %d \n",p[0],p[1]);
									posti_file[i].codice_fila = p[0];
									posti_file[i].codice = p[1];
								}
						}
						//if(flag) break;
					}
         //fputs (line, stdout);
      }
			pthread_mutex_unlock(&(sala.mx_aggiornamentoFilePrenotazioni));

			sprenota_posti(posti_file,num_posti);
			fclose (fd);
			fd = fopen("temp.txt","w");
			FILE *fs = fopen(FILE_F,"r");
			if(fd == NULL) exit(1);
			char line_c[128];
			char line_n[128];
			char line_e[128];
			r = malloc(sizeof(char)*128);
			flag=0;
			pthread_mutex_lock(&(sala.mx_aggiornamentoFilePrenotazioni));
      while (r!=NULL){
					printf("---- in -----\n");
					r = fgets(line,sizeof(line),fs);
					fputs (line, stdout);

					if(strcmp(line,"\n")==0){  //allora prossima riga codice
						r = fgets(line,sizeof(line),fs);
						fputs (line, stdout);
						strcpy(line_c,line);
						char c[4];
						sprintf(c, "%d", atoi(line)); //i to S
						if(strcmp(cod_pre, c)==0){ //codice
							flag=1;
						}
						else{
							char line_e[128]="\n";
							fputs(line_e,fd); //metto a capo
							fputs (line, fd); //metto codice non prenotazione annullata
						}
						fgets(line,sizeof(line),fs); //numero posti
						fputs (line, stdout);
						strcpy(line_n,line);
						if(!flag){fputs(line_n, fd);} //metto lunghezza }
						n_posti = atoi(line); //quante righe devo leggere per il prossimo record

						printf("SEC NPOSTI FILE: %d\n",n_posti);
						for(i=0; i<n_posti;i++){
								r = fgets(line,sizeof(line),fs);
								if(!flag){
									fputs (line, fd);
								}
						}
				}
				pthread_mutex_unlock(&(sala.mx_aggiornamentoFilePrenotazioni));
         //fputs (line, stdout);
      }

			fclose(fs);
			fclose(fd);
			pthread_mutex_lock(&(sala.mx_aggiornamentoFilePrenotazioni));
			int ret = remove(FILE_F);
			if(ret) printf("OK! delete\n");
			ret = rename("temp.txt", FILE_F);
			if (ret) {printf("OK! rename\n");
			pthread_mutex_unlock(&(sala.mx_aggiornamentoFilePrenotazioni));
		}
}
void salva_cod(){
	FILE *f = fopen("codice.txt","w");
	if(f!=NULL){
		fputs(COD_PREN,f);
		fclose(f);
	}
	printf("CODICE NON SALVATO!! \n");
}
//funzione esuguita dai thread che gestiscono i client
void* threadPrincipale(void* args){
	//torna l'id del thread chiamante
	int nClient = pthread_self();
	//socket di dialogo
	int sockmsg = (int)args;
	//messaggio benvenuto OK
	//int operazione;
	char op[16];
	//while(1) {
		while (1)	{
			//leggo operazione da effettuare
			printf("[SERV-INFO] Connesso al client con thread numero %d\n",nClient);

			if (recv(sockmsg, op, sizeof(op),0)<0) {
				printf("[SERV-ERR] Errore nella read dalla socket (1) %s\n", strerror(errno));
				exit(1);
			}
			int operazione = atoi(op);
			switch(operazione){
				case 1:
					info(sockmsg);
					break;
				case 2:
					info(sockmsg);
					accettaPrenotazioneEffettuata(sockmsg);
					break;
				case 3:
					annullaPrenotazioneEffettuata(sockmsg);
					break;
				default:
					//salva_sala();
					close(sockmsg);
					printf("[SERV-INFO] Server %d: ho chiuso la socket di dialogo con il client %d\n",getpid(),nClient);
					void* ret = malloc(sizeof(void));
					pthread_exit(ret);
					break;
			}
		}
}

/*
 * Crea una socket di ascolto e accetta connessioni da parte di più
 * cliente, delegando per ciascun client la gestione ad uno specifico
 * thread detached
 */
int main(int argc, char** argv) {
	int sock; /* socket di ascolto */
	int sockmsg; /* socket di dialogo */
	struct sockaddr_in server;
	//per i thread
	pthread_t thread;
	pthread_attr_t attr;

	//correttezza parametri
	if ( argc != 2 ) {
		printf("[SERV-ERR] Devi specificare <numero-della-porta>\n");
		exit(EXIT_FAILURE);
	}

	//creao la socket TCP per la famiglia di processi su Internet
	sock = socket(AF_INET, SOCK_STREAM, 0);
	if( sock < 0 ) {
		printf("[SERV-ERR] server %d: errore %s nel creare la socket\n", getpid(), strerror(errno));
		exit(EXIT_FAILURE);
	}

	//inizializzo la struttura del server
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;	//indirizzo della macchina
	server.sin_port = htons(atoi(argv[1]));

	//assegno alla socket "sock" una porta contenuta nella struttura "server"
	if( bind(sock, (struct sockaddr *)&server, sizeof(server))<0 ) {
		printf("[SERV-ERR] server %d: bind fallita\n", getpid());
		exit(EXIT_FAILURE);
	}
	//inizializzo il cinmea
	inizializzaSala();
	stampaSala(sala);
	printf("[SERV-INFO] server %d: pronto sulla porta %d\n", getpid(), ntohs(server.sin_port));

	//etichetto la socket come "di ascolto" TODO connessioni max
	if( listen(sock, 8) <0 ) {
		printf("[SERV-ERR] server %d: errore %s nella listen\n",getpid(), strerror(errno));
		exit(EXIT_FAILURE);
	}

	//il server cicla
	while(1) {
		//estraggo una connessione dalla coda, se non c'è rimango in attesa
		if( (sockmsg = accept(sock, 0, 0)) <0 )	{
			printf("[SERV-ERR] server %d: errore %s nella accept\n",getpid(), strerror(errno));
			exit(1);
		}
		printf("[SERV-INFO] numero socket %d\n",sockmsg);
		printf("[SERV-INFO] server %d: accettata una nuova connessione\n",getpid());

		signal(SIGINT, sig_handler);
		signal(SIGQUIT, sig_handler);
		signal(SIGSTOP, sig_handler);

		//delego ad un thread detached e vado a gestire la connessione seguente
		pthread_attr_init(&attr);
		pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED); //detached
		pthread_create(&thread,&attr,threadPrincipale,(void* )sockmsg);
		pthread_attr_destroy(&attr);
	}
	close(sock);
	printf("[SERV-INFO] server %d: ho chiuso la socket di ascolto\n",getpid());
}

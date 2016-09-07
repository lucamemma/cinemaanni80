#include<stdlib.h>
#include<stdio.h>
#define FILE_F "output.txt"
struct posto{
	int codice;
	int codice_fila;
	int prenotato;
};
struct posto* posti;

int main() {
  posti = malloc(sizeof(struct posto)*2);
  posti[0].codice = 1;
  posti[0].codice_fila = 1;
  posti[0].prenotato = 1;

  posti[1].codice = 2;
  posti[1].codice_fila = 2;
  posti[1].prenotato = 2;
	FILE *fd=fopen(FILE_F, "a");
  if(fd==NULL) {
    perror("Errore in apertura del file");
    exit(1);
  }
		int i;
		for (i=0; i<2;i++){
  			fprintf(fd, "%d %d %d\n\n", posti[i].codice, posti[i].codice_fila, posti[i].prenotato);
		}
  fclose(fd);
	printf("provo lettura\n");
	fd=fopen(FILE_F, "r");
	struct posto posti_f[2];
	for (i=0; i<2;i++){
			fscanf(fd, "%d %d %d\n\n", &posti_f[i].codice, &posti_f[i].codice_fila, &posti_f[i].prenotato);
	}

	for (i=0; i<2;i++){
			printf("- %d %d %d\n", posti_f[i].codice, posti_f[i].codice_fila, posti_f[i].prenotato);
	}

  fclose(fd);
}

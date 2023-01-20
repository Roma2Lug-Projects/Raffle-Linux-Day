#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>

#define DISP_PATH "./config/dispFile"
#define TIPI_PATH "./config/premiFile"
#define MAX_LEN_PREMIO 1024
#define abort(s, s2)                                \
        do {                                            \
                fprintf(stderr,"Errore %s: %s\n", s,s2);    \
                exit(EXIT_FAILURE);                         \
        } while(0)                                      \

#define flush while(getc(stdin) != '\n') {}

size_t disp_n = 0;
size_t premi_n = 0;
off_t disp_len;
off_t premi_len;
size_t *disp_arr;
char **premi_arr;
int disp_fd;
int premi_fd;

void stampaPremio(size_t premio)
{
        printf("Il premio estratto e': %s!!!\n", premi_arr[premio]);
}

int aggiornaDisp(size_t premio)
{
        disp_arr[premio]--;
        if ((pwrite(disp_fd,&disp_arr[premio], sizeof(*disp_arr), (off_t)(premio * sizeof(*disp_arr))) < 0))
                return -1;

        return 0;
}

size_t convertiInPremio(size_t estratto)
{
        size_t cnt = 0;
        size_t tot = 0;
    
        while((cnt < premi_n) && (estratto > tot)){
                tot += disp_arr[cnt];
                cnt++;
        }

        return cnt - 1;
}

size_t estrai(unsigned int seed)
{
        srand(seed);
        return rand()%disp_n;
}

void stampaDisp(void)
{
        if ((premi_n > 0) && (disp_n > 0)) {
                printf("I premi attualmente disponibili sono:\n");
                printf("[Classi di premio totali: %zu][Premi disponibili: %zu]", premi_n,disp_n);
                puts("");
                size_t cnt = 0;

                while(cnt < premi_n) {
                        printf("[Premio %zu: %s][Disponibili: %zu]\n", cnt, premi_arr[cnt], disp_arr[cnt]);
                        cnt++;
                }
        } else {
                printf("Non ci sono premi in memoria!\n");
        }
}

ssize_t rdline(int fd, void *vptr, size_t maxlen)
{
        //  Read a line from a file
        ssize_t n, rc;
        char c, *buffer;

        buffer = vptr;

        for (n = 1; n < maxlen; n++) {
                rc = read(fd, &c, 1);
                if (rc == 1)
                {
                        *buffer++ = c;
                        if (c == '\n')
                                break;
                } else if (rc == 0) {
                        if (n == 1)
                                return 0;
                        else
                                break;
                } else {
                        if (errno == EINTR)
                                continue;

                        return -1;
                }
        }

        *(buffer-1) = 0;
        return n;
}       

void carica(void)
{
        //apertura file disponibilita'
        errno = 0;
        if (!(disp_fd = open(DISP_PATH, O_CREAT | O_RDWR, 0777)))
                abort("disp_fd open", strerror(errno));
        //apertura file premi
        errno = 0;
        if(!(premi_fd = open(TIPI_PATH, O_CREAT | O_RDWR, 0777)))
                abort("premi_fd open", strerror(errno));
        //conta dei bytes in dispFile e calcolo del numero di tipi di premio presenti
        errno = 0;
        if((disp_len = lseek(disp_fd, 0L, SEEK_END)) < 0)
                abort("disp_fd SEEK_END", strerror(errno));

        if(lseek(disp_fd, 0L, SEEK_SET) < 0)
                abort("disp_fd SEEK_SET", strerror(errno));

        premi_n = (size_t)(disp_len / sizeof(*disp_arr));
        //allocazione del vettore con le disponibilita' dei vari premi
        if(!(disp_arr = malloc(premi_n*sizeof(*disp_arr))))
                abort("malloc","disp_arr");

        errno = 0;
        if((premi_len = lseek(premi_fd, 0L, SEEK_END)) < 0)
                abort("premi_fd SEEK_END", strerror(errno));

        if(lseek(premi_fd, 0L, SEEK_SET) < 0)
                abort("disp_fd SEEK_SET", strerror(errno));

        //lettura delle disponibilita' dal file, scrittura nel vettore e conta dei singoli premi totali
        size_t cnt = 0;
        disp_n = 0;
        while(cnt < premi_n){
                errno = 0;
                if ((read(disp_fd, &disp_arr[cnt], sizeof(*disp_arr))) < 0)
                        abort("disp_arr read", strerror(errno));

                disp_n += disp_arr[cnt++];
        }
        //allocazione dei puntatori a carattere per i nomi dei premi
        if (!(premi_arr = malloc(premi_n * sizeof(*premi_arr))))
                abort("malloc","premi_arr");

        //lettura da file dei nomi dei premi e scrittura nel vettore
        cnt = 0;
        char buf[MAX_LEN_PREMIO];
        
        while(cnt < premi_n) {
                errno = 0;
                if ((rdline(premi_fd, buf,MAX_LEN_PREMIO)) < 0)
                        abort("premi_arr read", strerror(errno));

                if (!(premi_arr[cnt] = malloc(strlen(buf) + 1)))
                        abort("malloc", "premi_arr[cnt]");

                strcpy(premi_arr[cnt], buf);
                cnt++;
        }

        printf("Dati caricati!\n");
        close(disp_fd);
        close(premi_fd);
}

int main(int argc, char **argv)
{
        printf("Sistema di estrazione dei premi per il Linux Day 2022\n");
        printf("Caricamento dei dati...");
    
        unsigned int seed;
        size_t premio;
        //gioco
        while (1) {
                carica();
                stampaDisp();

                if(!disp_n)
                        exit(EXIT_SUCCESS);

                //apertura file disponibilita'
                errno = 0;
                if(!(disp_fd = open(DISP_PATH, O_CREAT | O_RDWR, 0777)))
                        abort("disp_fd open", strerror(errno));

                //apertura file premi
                errno = 0;
                if(!(premi_fd = open(TIPI_PATH, O_CREAT | O_RDWR, 0777)))
                        abort("premi_fd open", strerror(errno));

                printf("\nPremere invio per cominciare...\n");
                flush
                printf("Inserisci un numero compreso tra 0 e %u e premi invio:\n", UINT_MAX);
                scanf("%u",&seed);
                flush
                printf("Il numero inserito e': %u. Premere invio per continuare con l'estrazione del premio!\n",seed);
                flush
                size_t estratto = estrai(seed);
                printf("Il numero estratto e':%zu\n", estratto);
                premio = convertiInPremio(estratto);
                stampaPremio(premio);
                errno = 0;
                if (aggiornaDisp(premio))
                        abort("aggiornaDisp, impossibile aggiornare il file della disponibilita'", strerror(errno));

                close(disp_fd);
                close(premi_fd);
                printf("\nPremere invio per continuare...\n");
                flush
        }
}
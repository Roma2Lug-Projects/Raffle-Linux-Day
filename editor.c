#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>

#define DISP_PATH "./config/dispFile"
#define TIPI_PATH "./config/premiFile"
#define MAX_LEN_PREMIO 1024
#define abort(s,s2) {fprintf(stderr,"Errore %s: %s\n", s,s2); exit(EXIT_FAILURE);}
#define flush while(getc(stdin)!='\n'){}

size_t disp_n=0;
size_t premi_n=0;
off_t disp_len;
off_t premi_len;
size_t *disp_arr;
char **premi_arr;
int disp_fd;
int premi_fd;

int aggiornaDisp(size_t premio)
{
    disp_arr[premio]--;
    if((pwrite(disp_fd,&disp_arr[premio],sizeof(*disp_arr),(premio*sizeof(*disp_arr)))<0)){
        return -1;
    }
    return 0;
}

size_t estrai(unsigned int seed)
{
    srand(seed);
    return rand()%disp_n;
}

void stampaDisp(void)
{
    if((premi_n>0)&&(disp_n>0)){
        printf("I premi attualmente disponibili sono:\n");
        printf("[Classi di premio totali: %zu][Premi disponibili: %zu]",premi_n,disp_n);
        puts("");
        size_t cnt=0;
        while(cnt<premi_n){
            printf("[Premio %zu: %s][Disponibili: %zu]\n",cnt,premi_arr[cnt],disp_arr[cnt]);
            cnt++;
        }
    }
    else
        printf("Non ci sono premi in memoria!\n");
}

ssize_t rdline(int fd, void *vptr, size_t maxlen)
{
    //  Read a line from a file
    ssize_t n, rc;
    char    c, *buffer;

    buffer = vptr;

    for ( n = 1; n < maxlen; n++ )
    {
        rc = read(fd, &c, 1);
        if ( rc == 1 )
        {
            *buffer++ = c;
            if ( c == '\n' ) break;
        }
        else
        if ( rc == 0 )
        {
            if ( n == 1 ) return 0;
            else break;
        }
        else
        {
            if ( errno == EINTR ) continue;
            return -1;
        }
    }
    *(buffer-1) = 0;
    return n;
}

ssize_t wrline(int fd, const void *vptr, size_t n)
{
    //  Write a line to a file
    size_t      nleft;
    ssize_t     nwritten;
    const char *buffer;

    buffer = vptr;
    nleft  = n;

    while ( nleft > 0 )
    {
        if ((nwritten = write(fd, buffer, nleft)) <= 0 )
        {
            if ( errno == EINTR ) nwritten = 0;
            else return -1;
        }
        nleft  -= nwritten;
        buffer += nwritten;
    }
    return n;
}

void carica(void)
{
    //apertura file disponibilita'
    errno=0;
    if(!(disp_fd=open(DISP_PATH, O_CREAT | O_RDWR, 0777))){
        abort("disp_fd open",strerror(errno))
    }
    //apertura file premi
    errno=0;
    if(!(premi_fd=open(TIPI_PATH, O_CREAT | O_RDWR, 0777))){
        abort("premi_fd open",strerror(errno))
    }
    //conta dei bytes in dispFile e calcolo del numero di tipi di premio presenti
    errno=0;
    if((disp_len=lseek(disp_fd, 0L, SEEK_END)) < 0){
        abort("disp_fd SEEK_END",strerror(errno))
    }
    if(lseek(disp_fd, 0L, SEEK_SET) < 0){
        abort("disp_fd SEEK_SET",strerror(errno))
    }
    premi_n=(size_t)(disp_len / sizeof(*disp_arr));
    //allocazione del vettore con le disponibilita' dei vari premi
    if(!(disp_arr=malloc(premi_n*sizeof(*disp_arr)))){
        abort("malloc","disp_arr")
    }
    errno=0;
    if((premi_len=lseek(premi_fd, 0L, SEEK_END)) < 0){
        abort("premi_fd SEEK_END",strerror(errno))
    }
    if(lseek(premi_fd, 0L, SEEK_SET) < 0){
        abort("disp_fd SEEK_SET",strerror(errno))
    }
    //lettura delle disponibilita' dal file, scrittura nel vettore e conta dei singoli premi totali
    size_t cnt=0;
    disp_n=0;
    while(cnt<premi_n){
        errno=0;
        if((read(disp_fd, &disp_arr[cnt], sizeof(*disp_arr))) < 0){
            abort("disp_arr read",strerror(errno))
        }
        disp_n+=disp_arr[cnt++];
    }
    //allocazione dei puntatori a carattere per i nomi dei premi
    if(!(premi_arr=malloc(premi_n*sizeof(*premi_arr)))){
        abort("malloc","premi_arr")
    }
    //lettura da file dei nomi dei premi e scrittura nel vettore
    cnt=0;
    char buf[MAX_LEN_PREMIO];
    while(cnt<premi_n){
        errno=0;
        if((rdline(premi_fd, buf,MAX_LEN_PREMIO)) < 0){
            abort("premi_arr read",strerror(errno))
        }
        if(!(premi_arr[cnt]=malloc(strlen(buf)+1))){
            abort("malloc","premi_arr[cnt]")
        }
        strcpy(premi_arr[cnt],buf);
        cnt++;
    }
    printf("Dati caricati!\n");
    close(disp_fd);
    close(premi_fd);
}

int main(int argc, char **argv) {
    printf("Editor dei file per il Linux Day 2022\n");
    printf("Caricamento dei dati...");
    carica();

    char c;
    //applicazione
    while(1){
        carica();
        stampaDisp();
        printf("Editor dei premi e delle disponibilita':\n");
        printf("Premere A per aggiornare la disponibilita' di un premio\n");
        printf("Premere C per creare un nuovo premio\n");
        printf("Premere R per rimuovere un premio dalla lista\n");
        printf("Premere X per uscire\n");
        printf("Premere invio per confermare\n");
        scanf("%c",&c);
        flush
        c=toupper(c);
        printf("Hai selezionato: %c\n",c);
        if(!(disp_fd=open(DISP_PATH, O_CREAT | O_RDWR, 0777))){
            abort("disp_fd open while",strerror(errno))
        }
        //apertura file premi
        errno=0;
        if(!(premi_fd=open(TIPI_PATH, O_CREAT | O_RDWR, 0777))){
            abort("premi_fd open while",strerror(errno))
        }
        if(c=='A'){ //aggiorna
            size_t premio;
            ssize_t delta;
            stampaDisp();
            printf("Seleziona il premio che vuoi aggiornare:\n");
            scanf("%zu",&premio);
            flush
            printf("Seleziona il valore da sommare alla disponibilita':\n");
            scanf("%zd",&delta);
            flush
            printf("Hai selezionato il premio: %s\n Hai chiesto di sommare: %zd\n", premi_arr[premio], delta);
            printf("Premi invio per confermare\n");
            flush
            disp_arr[premio]+=delta;
            disp_n+=delta;
            if((pwrite(disp_fd,&disp_arr[premio],sizeof(*disp_arr),(off_t)(premio*sizeof(*disp_arr)))<0)){
                abort("aggiorna pwrite",strerror(errno))
            }
            printf("Aggiornamento confermato!\n");
            close(disp_fd);
            close(premi_fd);
            continue;
        }
        else if (c=='C'){ //crea
            char buf[MAX_LEN_PREMIO];
            char buf2[MAX_LEN_PREMIO];
            size_t disp;
            printf("Scrivi il nome del premio che vuoi aggiungere e premi invio:\n");
            scanf("%[^\n]",buf);
            flush
            size_t buflen=strlen(buf)+1;
            printf("Seleziona il valore da assegnare alla disponibilita':\n");
            scanf("%s",buf2);
            disp=(size_t)strtol(buf2,NULL,10);
            flush
            printf("Stai aggiungendo il premio: %s\n Con disponibilita': %zu\n",buf,disp);
            printf("Premi invio per confermare\n");
            flush
            buf[buflen-1]='\n';
            //disp_arr realloc + inserimento premio da implementare
            if((pwrite(disp_fd,(char *)(&disp),sizeof(disp),(off_t)(premi_n*sizeof(disp)))<0)){
                abort("disp_fd crea pwrite",strerror(errno))
            }
            printf("OK\n");
            if((pwrite(premi_fd,buf,buflen,premi_len)<0)){
                abort("premi_fd crea pwrite",strerror(errno))
            }
            close(disp_fd);
            close(premi_fd);
            premi_len+=buflen;
            premi_n++;
            printf("Creazione del premio confermata!\n");
            continue;
        }
        else if (c=='R'){ //rimuovi
            close(disp_fd);
            close(premi_fd);
            continue;

        }
        else if(c=='X'){ //uscita
            close(disp_fd);
            close(premi_fd);
            break;
        }
        else {
            printf("inserire un carattere valido!\n");
            close(disp_fd);
            close(premi_fd);
            continue;
        }
    }
    return 0;
}
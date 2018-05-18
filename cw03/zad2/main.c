#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/resource.h>

#define _POSIX_C_SOURCE 200809L

int split_line( char* source, char** buff ){
    int i = 0;
    source[ strlen(source)-1 ]='\0';
    char* t = source;
    while( ( t = strtok(t, " ") ) ){
        
	if( t[0] == '#'  ) 
            if ((t = getenv(t + 1)) == NULL)
                t = "err";

        buff[i] = malloc( strlen(t)+1 );
        strcpy( buff[i], t );
        t = NULL;
        i++;
    }
    return (i-1);
}

int main( int argc, char** argv) {
    //extern char** environ;

    if(argc < 3){
	printf("Brak wystarczającej ilości argumentów\n");
	exit(-1);
    }

    FILE* f;
    f = fopen(argv[1], "r");

    if( !f ) {
        printf("Otwarcie %s nie powiodło się. \n", argv[1]);
        exit(-1);
    }

    char* b = NULL;
    size_t len=0;
    char* key = NULL; char* val = NULL;
    char** args = (char **)malloc((5*sizeof(char*)) + 1);

    while ( getline( &b, &len, f ) != -1 ) {
        if (b[0] == '#') {
            key = strtok(b, " \n") + 1;//+1 żeby pominąć '#' oraz zignorować '\n'
            val = strtok(NULL, "\n");
            
            if (val == NULL) unsetenv(key);
            else {
                val[strlen(val)] = '\0'; //strip \n
                setenv(key, val, 1);
                printf("%s = %s\n", key, getenv(key));
            }
        } else {

            printf("POLECENIE: %s\t", b);

            int i = split_line(b, args);

            if (i > -1 ) {
                pid_t p = fork();
                args[i+1] = NULL;
                if (p == 0) {

                    //ustwaienie limitów
                    struct rlimit* lim = malloc(sizeof(struct rlimit));
                    lim->rlim_cur = atoi(argv[2]);  //jeśli przekroczy wysyła SIGXCPU - można złapać wtedy nie zakończy
                    lim->rlim_max = atoi(argv[2]); //maksymalnie 10s CPU na proces

                    int succ = setrlimit( RLIMIT_CPU, lim );
                    if( succ == -1 ){
                        free( lim );
                        exit( -1 );
                    }

                    free(lim);
                    lim = malloc(sizeof(struct rlimit));
                    lim->rlim_cur = atoi(argv[3]);
                    lim->rlim_max = atoi(argv[3]);

                    succ = setrlimit( RLIMIT_DATA, lim);
                    if( succ == -1 ){
                        printf("err %i\n", errno);
                        free( lim );
                        exit( -1 );
                    }
                    //getrlimit( RLIMIT_AS, lim );
                    //printf( "cur: %li, \t hard %li\n", lim->rlim_cur, lim->rlim_max);

                    free( lim );

                    printf( "Wywołanie potomnego\n\t\t"  );
                    execvp(args[0], args);
                    exit(-1);
                } else {
                    wait(&p);
                    struct rusage k;
                    getrusage( RUSAGE_CHILDREN, &k);
                    printf( " mem: %li, cpu_time_user: %li, cpu_time_system: %li \n", k.ru_maxrss, k.ru_utime.tv_sec, k.ru_stime.tv_sec );
                    if( p == 0 )
                        printf("Wykonano pomyślnie (%i)\n",p);
                    else{
                        printf("wykonanie '%s' nie powiodło się (%i) %i\n", b, p, errno);
                        exit(p);
                    }
                }

            }
            for(int j=0; j < i+1; j++) free(args[j]);
        }
    }

    free(args);
    free(b);
    fclose(f);
    return 0;
}

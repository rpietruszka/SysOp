#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/file.h>

int check_acces(int file_desc, struct flock *l)
{
    fcntl(file_desc, F_GETLK, l);
    if (l->l_type != F_UNLCK)
        return l->l_pid;
    return 0;
}

void print_locks(int fd)
{
    int max = lseek(fd, 0, SEEK_END);
    struct flock tmp;
    for (int i = 0; i <= max; i++)
    {
        tmp.l_type = F_WRLCK;
        tmp.l_whence = SEEK_SET;
        tmp.l_len = 1;
        tmp.l_start = i;
        fcntl(fd, F_GETLK, &tmp);
        if (tmp.l_type != F_UNLCK)
        {
            printf(" PID %i zablokowa bajt %i do %s\n", tmp.l_pid, i, (tmp.l_type == F_RDLCK) ? "F_RFLCK" : "F_RWLCK");
        }
    }
}

int unlock(int file_desc, int byte_nr)
{

    struct flock *lock = malloc(sizeof(struct flock));
    lock->l_len = 1;
    lock->l_type = F_UNLCK;
    lock->l_start = byte_nr;
    lock->l_whence = SEEK_SET;

    int code = fcntl(file_desc, F_SETLKW, lock);

    free(lock);
    return code;
}

int set_lock(int fd)
{
    int byte_nr;
    char type;
    int blokujacy;

    printf("\t PODAJ: <POZYCJE BAJTU ( 1... )> <znak w(zapis) lub r(odczyt)> < 1 = blokujący / 0 = nieblokujacy>\nparam>\t");
    scanf("%i %c %i", &byte_nr, &type, &blokujacy);
    getc(stdin);
    struct flock *lock = malloc(sizeof(struct flock));
    //struct flock *lock2 = malloc(sizeof(struct flock));

    int cmd = (blokujacy == 1) ? F_SETLKW : F_SETLK;
    lock->l_type = (type == 'w') ? F_WRLCK : F_RDLCK;

    lock->l_len = 1;
    lock->l_start = byte_nr;

    lock->l_whence = SEEK_SET;
    int code = -1;
    //if (check_acces( fd, lock ) == 0 )
    code = fcntl(fd, cmd, lock);
    //else
    //    printf( "Inny plik odczytuje bajt nie można go teraz zapisać\n" );
    free(lock);
    return code;
}

int read_form(int fd, int byte)
{
    struct flock lock;
    lock.l_type = F_RDLCK;
    lock.l_start = byte;
    lock.l_len = 1;
    lock.l_whence = SEEK_SET;
    char b = 0;
    if (check_acces(fd, &lock) == 0)
    {
        lseek(fd, byte, SEEK_SET);

        int res = read(fd, &b, 1);

        if (res)
            printf("\nOdczytany bajt ma wartość ASCII ( %c )\n", b);
        else
            fprintf(stderr, "\nOdczyt nie powiódł się - brak dostępu do podanego bajtu\n");
        return res;
    }
    else
    {
        printf("Nie można odczytac bajtu %i, inny proces o PID %i zablował go do zapisu\n", byte, lock.l_pid);
        return -1;
    }
}

int write_to(int fd, int byte, char b)
{
    struct flock lock;
    lock.l_type = F_WRLCK;
    lock.l_start = byte;
    lock.l_len = 1;
    lock.l_whence = SEEK_SET;
    if (check_acces(fd, &lock) == 0)
    {
        lseek(fd, byte, SEEK_SET);
        int res = write(fd, &b, 1);
        if (res > 0)
            printf("\nZapisany bajt ma wartość ASCII ( %i )\n", b);
        else
            //fprintf(stderr, "\nZapis nie powiódł się - brak dostępu do podanego bajtu\n");
            fprintf(stdout, "\nZapis nie powiódł się - brak dostępu do podanego bajtu\n");
        return res;
    }
    else
    {
        printf("Zapis nie powiódł się bajt jest już zablokoway przez inny proces");
        return -1;
    }
}

int main(int argc, char **argv)
{

    if (argc < 2)
    {
        fprintf(stderr, "Nie podano pliku!\n");
        exit(1);
    }

    int file_desc = open(argv[1], O_RDWR);

    if (file_desc == -1)
    {
        fprintf(stderr, "Nie można uzyskać dostępu do %s\n", argv[1]);
        exit(1);
    }

    printf("Program pozwala na zakładnie oraz zdejmowanie założonych locków\n");
    printf("Akceptuje opcje: ( w przypadku potrzeby zadaje pytanie o dodatkowe dane )\n");
    printf("\t\t\t s -> ustaw blokadę\n");
    printf("\t\t\t u -> zdjęcie blokay\n");
    printf("\t\t\t r -> odczyt bajtu\n");
    printf("\t\t\t w -> zapis bajtu\n");
    printf("\t\t\t l -> listowanie locków (innych procesów)\n\n");

    char ans = ' ';
    int byte = 0;
    int loop = 1;
    while (loop == 1)
    {
        printf("prompt> ");
        scanf("%c", &ans);
        getc(stdin);
        switch (ans)
        {
        case 'q':
            loop = 0;
            break;

        case 'u':
            printf("PODAJ BAJT DO ODBLOKOWANIA: ");
            scanf("%i", &byte);
            getc(stdin);
            if (unlock(file_desc, byte) < 0)
                fprintf(stderr, "\n!ZWALNIANIE BLOKADY NIE POWIODŁO SIE!\n");
            break;
        case 'r':
            printf("Podaj pozycję bajtu do odczytania ");
            scanf("%i", &byte);
            getc(stdin);
            byte = read_form(file_desc, byte);
            break;
        case 'w':
            printf("Podaj pozycję bajtu oraz znak do zapisania ");
            scanf("%i %c", &byte, &ans);
            getc(stdin);
            byte = write_to(file_desc, byte, ans);
            break;

        case 's': //setlock
            set_lock(file_desc);

        case 'l':
            print_locks(file_desc);

        default:
            break;
        }
        printf("\n");
    }

    if (close(file_desc) != 0)
        printf("Zamknięcie pliku nie powiodło się\n");

    return 0;
}

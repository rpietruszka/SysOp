#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#include <fcntl.h>
#include <time.h>
#include <sys/times.h>
#include <unistd.h> // _SC_CLK_TCK

#define CLK_PS sysconf(_SC_CLK_TCK)

/*
 * args:
 * 1 -> "sys"/"lib"  using system or standard c lib functions
 * 2 -> "shuffle"/"sort"/"generate"
 * 3 -> file or path to file
 * 4 -> number of records
 * 5 -> size of single record [ Bytes ]
 */

void print_times(struct tms *begin, struct tms *end, char *msg);

//use /dev/random to create file passed as 2nd argument for program
void generate(char *filename, int records, int size_of_record)
{
    FILE *source_of_data;
    FILE *destination;
    int alloc_size = (records * size_of_record);
    char *buffer = (char *)malloc(alloc_size + 1); //z uwzglednienim '\0'

    //otwarcie /dev/random do pobrania danych
    source_of_data = fopen("/dev/random", "r");
    if (source_of_data == NULL)
    {
        printf("Cannot acces /dev/random\n");
        return;
    }
    //otwarcie pliku docelowego do zapisu
    destination = fopen(filename, "w");

    for (int i = 0; i < records; i++)
    {
        //wymagane jest pobranie całego bufora = długość pojedynczego rekordu
        //fscanf zwaraca ilośc ocztanych
        if (fread(buffer, sizeof(char), size_of_record, source_of_data) == (size_t)size_of_record)
        {
#ifdef _DEBUG
            for (int k = 0; k < alloc_size; k++)
                buffer[k] = rand() % ('Z' - 'A') + 'A';
#endif
            if (fwrite(buffer, sizeof(char), size_of_record, destination) == (size_t)size_of_record)
                fflush(destination); //żeby synchronizacza bufora była widoczna
            else
            {
                printf("zapis nie powiódł się\n");
                i--; //powtorz losowanie
            }
        }
    }
    free(buffer);
    //zamknięcie plików
    fclose(destination);
    fclose(source_of_data);
}

void sort_sys(char *file_name, int records, int size_of_record)
{
    //pozyskanie deskryptora pliku ( w razie braku pliku jest tworzony, operacje synchronizowane)
    int file_desc = open(file_name, O_RDWR);

    size_t buffer_size = size_of_record * sizeof(char);
    //buffers keeps records to swap -> size_of_record+1 ( '\n' )
    char *buffer1 = (char *)malloc(buffer_size + 1);
    char *buffer2 = (char *)malloc(buffer_size + 1);

    if (file_desc == -1)
    {
        fprintf(stderr, "Nie można pozyskać deskryptora dla: %s\n", file_name);
        return;
    }
    //bubblesort
    for (int i = 0; i < records; i++)
    {
        lseek(file_desc, 0, SEEK_SET);

        for (int j = 0; j < records - i; j++)
        {
            if (read(file_desc, buffer1, buffer_size) == size_of_record)
            {
                if (read(file_desc, buffer2, buffer_size) == size_of_record)
                {
                    if ((unsigned char)buffer1[0] > (unsigned char)buffer2[0])
                    {
                        lseek(file_desc, -2 * buffer_size, SEEK_CUR);
                        write(file_desc, buffer2, buffer_size);
                        write(file_desc, buffer1, buffer_size);
                    }
                }
            }
            lseek(file_desc, -buffer_size, SEEK_CUR);
        }
    }
    free(buffer1);
    free(buffer2);
    if (close(file_desc) != 0)
        fprintf(stderr, "ZAMYKANIE PLIKU NIE POWIODŁO SIE");
}

void sort_lib(char *file_name, int records, int size_of_record)
{
    //pozyskanie deskryptora pliku ( w razie braku pliku jest tworzony, operacje synchronizowane)
    FILE *f;
    f = fopen(file_name, "rw+");
    size_t buffer_size = size_of_record * sizeof(char);
    char *buffer1 = (char *)malloc(buffer_size + 1);
    char *buffer2 = (char *)malloc(buffer_size + 1);

    if (!f)
    {
        fprintf(stderr, "Nie można pozyskać uchwytu dla: %s\n", file_name);
        exit(-1);
    }
    //bubblesort
    for (int i = 0; i < records; i++)
    {
        fseek(f, 0, SEEK_SET);

        for (int j = 0; j < records - i; j++)
        {
            if (fread(buffer1, sizeof(char), size_of_record, f) == (size_t)size_of_record)
            {
                if (fread(buffer2, sizeof(char), size_of_record, f) == (size_t)size_of_record)
                {
                    if ((unsigned char)buffer1[0] > (unsigned char)buffer2[0])
                    {
                        fseek(f, -2 * buffer_size, SEEK_CUR);
                        fwrite(buffer2, sizeof(char), size_of_record, f);
                        fwrite(buffer1, sizeof(char), size_of_record, f);
                    }
                }
            }
            fseek(f, -buffer_size, SEEK_CUR);
        }
    }
    free(buffer1);
    free(buffer2);
    if (fclose(f) != 0)
        fprintf(stderr, "ZAMYKANIE PLIKU NIE POWIODŁO SIE");
}

void shuffle_sys(char *file_name, int records, int size_of_record)
{
    int file_desc = open(file_name, O_RDWR);

    size_t buffer_size = size_of_record * sizeof(char);
    //buffers keeps records to swap -> size_of_record+1 ( '\n' )
    char *buffer1 = (char *)malloc(buffer_size + 1);
    char *buffer2 = (char *)malloc(buffer_size + 1);

    if (file_desc == -1)
    {
        fprintf(stderr, "Nie można pozyskać deskryptora dla: %s\n", file_name);
        return;
    }
    int j = 0;
    int i_poz = 0;
    for (int i = 0; i < records - 1; i++)
    {
        i_poz = lseek(file_desc, i * size_of_record, SEEK_SET);
        read(file_desc, buffer1, buffer_size);
        j = rand() % (records - i) + i;
        lseek(file_desc, j * size_of_record, SEEK_SET);
        read(file_desc, buffer2, buffer_size);

        lseek(file_desc, -buffer_size, SEEK_CUR);
        write(file_desc, buffer1, buffer_size);
        lseek(file_desc, i_poz, SEEK_SET);
        write(file_desc, buffer2, buffer_size);
    }
    free(buffer1);
    free(buffer2);
    close(file_desc);
}

void shuffle_lib(char *file_name, int records, int size_of_record)
{
    FILE *f;
    f = fopen(file_name, "rw+");

    size_t buffer_size = size_of_record * sizeof(char);
    //buffers keeps records to swap -> size_of_record+1 ( '\n' )
    char *buffer1 = (char *)malloc(buffer_size + 1);
    char *buffer2 = (char *)malloc(buffer_size + 1);

    if (!f)
    {
        fprintf(stderr, "Nie można pozyskać deskryptora dla: %s\n", file_name);
        return;
    }
    int j = 0;
    for (int i = 0; i < records - 1; i++)
    {
        fseek(f, i * size_of_record, SEEK_SET);
        fread(buffer1, sizeof(char), size_of_record, f);
        j = rand() % (records - i) + i;
        fseek(f, j * size_of_record, SEEK_SET);
        fread(buffer2, sizeof(char), size_of_record, f);

        fseek(f, -buffer_size, SEEK_CUR);
        fwrite(buffer1, sizeof(char), size_of_record, f);
        fseek(f, i * size_of_record, SEEK_SET);
        fwrite(buffer2, sizeof(char), size_of_record, f);
    }
    free(buffer1);
    free(buffer2);
    fclose(f);
}

int main(int argc, char **argv)
{
    srand(time(NULL));

    if (argc < 5)
    {
        printf("Wyamgane jest 5 arg: <sys/lib> <shuffle/sort/generate> <filename> <liczba_rekordów> <rozmiar_rekordu>\n");
        exit(-1);
    }

    int records = atoi(argv[4]);
    int size_of_record = atoi(argv[5]);

    struct tms *begin = malloc(sizeof(struct tms)), *end = malloc(sizeof(struct tms));
    times(begin);

    if (argv[2][0] == 'g') //genenrate nie jest zależne od sys/lib
        generate(argv[3], records, size_of_record);

    else if (argv[1][0] == 's') //wywołania sysemowe
        switch (argv[2][1])
        { //s*
        case 'o':
            sort_sys(argv[3], records, size_of_record);
            break;
        case 'h':
            shuffle_sys(argv[3], records, size_of_record);
            break;
        }
    else if (argv[1][0] == 'l')
        switch (argv[2][1])
        { //s*
        case 'o':
            sort_lib(argv[3], records, size_of_record);
            break;
        case 'h':
            shuffle_lib(argv[3], records, size_of_record);
            break;
        }
    else
        return -1;

    times(end);
    printf(" TYP: %-5s  OPERACJA: %-10s  ILOSC: %i  ROZMIAR: %i  ", argv[1], argv[2], records, size_of_record);
    print_times(begin, end, "");
    free(begin);
    free(end);
    return 0;
}
//wymaga wszczsniejszego zainicjowania tms* begin
void print_times(struct tms *begin, struct tms *end, char *msg)
{
    times(end);
    double sys = (double)(end->tms_stime - begin->tms_stime) / CLK_PS;
    double user = (double)(end->tms_utime - begin->tms_utime) / CLK_PS;
    printf("%s\t User: %.4f \t System: %.4f\n", msg, user, sys);
}

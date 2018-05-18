#include "lib.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/times.h>
#include <unistd.h> // _SC_CLK_TCK

#define CLK_PS sysconf(_SC_CLK_TCK) //ilosc taktów na sek
#define none

void print_times( struct tms*, struct tms*, char* );

int main(int argc, char **argv){
    //drzewo
    printf("\n\nWYNIKI DLA DRZEWA\n");
    int i = 0;
    struct tms *begin = malloc(sizeof(struct tms)), *end = malloc(sizeof(struct tms));
    times( begin);
	Adress_book_BST* book = create_book_tree();
    print_times(begin, end, "Tworzenie książki");

    char* name = (char *) malloc( 50*sizeof(char) );
    char* surname = (char *) malloc( 50*sizeof(char) );
    char* address = (char *) malloc( 50*sizeof(char) );
    char* email = (char *) malloc( 50*sizeof(char) );
    char* date = (char *) malloc( 50*sizeof(char) );
    char* phone = (char *) malloc( 50*sizeof(char) );

    times( begin);
    while( scanf( "%[^\t]\t%[^\t]\t%[^\t]\t%[^\t]\t%[^\t]\t%[^\n]\n",name,surname,address,email,date,phone ) != EOF && ++i < 1000 ){
        if( add_contact_tree( book, create_entry_node( name, surname, address, email, date, phone ) ) == -1)
            printf("NIEPOWODZENIE\n");
    }
    print_times(begin, end, "Wstawienie 1000 rekortów");
    //zwolnienie buforów
    //free( name ); free( surname ); free( address ); free( email ); free( date ); free( phone );

    times( begin);
    sort_book_tree_by(book,'e');
    sort_book_tree_by(book,'n');
    sort_book_tree_by(book,'b');
    sort_book_tree_by(book,'n');
    sort_book_tree_by(book,'s');
    print_times(begin, end, "Sortiwanie 1000 x5");

    //przeszykiwanie optymistycznie korzeń
    times( begin );
    find_by_name_tree(book, book->first->name, book->first->surname);
    print_times(begin, end, "Optymistyczne Przeszukiwanie");
    //pesymistyczne - najgłębiej położony node Tedd Young
    times( begin );
    find_by_name_tree(book, "Tedd", "Young");
    print_times(begin, end, "Pesymistyczne przeszukiwanie");

    //jak dla wyszukiwania
    times( begin );
    delete_contatct_tree(book, book->first->name, book->first->surname);
    print_times(begin, end, "Optymistyczne usuwanie");
    times( begin );
    delete_contatct_tree(book, "Tedd", "Young");
    print_times(begin, end, "Pesymistyczne usuwanie");


    times( begin);
    delete_book_tree(book);
    
    
    //error
        free(book);
    
    print_times(begin, end, "Usuwanie książki 998 wpisów");

    /////////

    printf("\n\nWYNIKI DLA LISTY\n");
    times(begin);
    Adress_book* book_list = create_book();
    print_times(begin, end, "Tworzenie książki");

    times( begin);
    while( scanf( "%[^\t]\t%[^\t]\t%[^\t]\t%[^\t]\t%[^\t]\t%[^\n]\n",name,surname,address,email,date,phone ) != EOF && i++ < 2000 ){
        if( add_contact( book_list, create_entry( name, surname, address, email, date, phone ) ) == -1)
            printf("NIEPOWODZENIE\n");
    }
    print_times(begin, end, "Wstawienie 1000 rekortów");

    //zwolnienie buforów
    free( name ); free( surname ); free( address ); free( email ); free( date ); free( phone );

    times( begin);
    sort_book_by(book_list,'p');
    sort_book_by(book_list,'n');
    sort_book_by(book_list,'e');
    sort_book_by(book_list,'d');
    sort_book_by(book_list,'s');
    print_times(begin, end, "Sortiwanie 1000 x5");

    //przeszykiwanie optymistycznie 1 elementr
    times( begin );
    find_by_name(book_list, book_list->first->name, book_list->first->surname);
    print_times(begin, end, "Optymistyczne Przeszukiwanie");
    //pesymistyczne - najgłębiej położony node Tedd Young
    times( begin );
    find_by_name(book_list, "Tedd", "Young");
    print_times(begin, end, "Pesymistyczne przeszukiwanie");

    //Jak dla wyszukiwania
    times(begin);
    delete_contatct(book_list, book_list->first->name, book_list->first->surname);
    print_times(begin,end,"Optymistyczne usuwanie");
    //pesymistyczny ostatni Todd Young
    times( begin );
    delete_contatct(book_list, "Todd", "Young");
    print_times(begin, end, "Pesymistyczne usuwanie");

    times( begin);
    delete_book( book_list );
    book_list=NULL;
    print_times(begin, end, "Usuwanie książki 998 wpisów");
    printf("\n\n");
    free(begin);
    free(end);

	return 0;
}

//wymaga wszczsniejszego zainicjowania tms* begin
void print_times ( struct tms* begin, struct tms* end, char* msg ){
    times( end );
    double sys = (double) ( end->tms_stime - begin->tms_stime )/CLK_PS;
    double user = (double) ( end->tms_utime - begin->tms_utime )/CLK_PS;
    printf( "%s\n RealTime: %.4f \t|\t User: %.4f \t|\t System: %.4f\n", msg, sys+user, user, sys );

}

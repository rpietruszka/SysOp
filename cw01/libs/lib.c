//zapobieda wielkorotnemu dołączniu biblioteki przez zdefiniowanie nazwy modułu
#ifndef _BOOK
#define _BOOK

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/*
 *  Bibloteka zawiera implementacje książki adresowej, wykorzystując listę dwukierunkwą do przechowywania informacji
 */

 // Contact jest pojendyczym wpisen zawierajcym wymagane informacje
struct Contact {
    char *name;
    char *surname;
    char *birth_date;
    char *email;
    char *phone_number;
	char *address;
	struct Contact* next;
	struct Contact* prev;
} typedef book_entry; //custom alias for type

struct Adress_book {
    int entry_count;
    book_entry* first;
}typedef Adress_book;


Adress_book* create_book(void){
    Adress_book* b = ( Adress_book* )malloc(sizeof( struct Adress_book));
    b->first=0;
    b->entry_count=0;
    return b;
}

struct Contact* create_entry (char* name, char* surname, char* address, char* email, char* date, char* phone){

	struct Contact* entry = ( book_entry* )malloc( sizeof( struct Contact ) );

	if (entry != NULL){ //Zallokowano pamięc na nowy wpis
        entry->name = strcpy( (char*) malloc( strlen(name)*sizeof(char)+1 ), name );
        entry->surname = strcpy( (char*) malloc( strlen(surname)*sizeof(char)+1 ), surname );
        entry->address = strcpy( (char*) malloc( strlen(address)*sizeof(char)+1 ), address );
        entry->email = strcpy( (char*) malloc( strlen(email)*sizeof(char)+1 ), email );
        entry->birth_date = strcpy( (char*) malloc( strlen(date)*sizeof(char)+1 ), date );
        entry->phone_number = strcpy( (char*) malloc( strlen(phone)*sizeof(char)+1 ), phone );
        entry->next=NULL;
        entry->prev=NULL;
	} else
        printf("Tworzenie nowego wpisu nie powiodło się - brak wystarczającej ilości pamięci");

	return entry;
}

int add_contact( Adress_book* book, book_entry* entry ){

    if ( book == NULL ){
        return -1;
    } else {

        entry->next = book->first;

        if ( book->first != NULL ){
            entry->prev = book->first->prev;
            book->first->prev = entry;
        }else entry->next = NULL;

        book->first = entry;
        book->entry_count++;
        return 0; // 0 = no error
    }

}


struct Contact* find_by_name ( Adress_book* book, char* name, char* surname ){
    book_entry* i;
    for( i = book->first; i != NULL; i = i->next)
        if( strcmp( i->name, name ) == 0 && strcmp( i->surname, surname ) == 0 )
            return i;

    return NULL;
}

void delete_entry ( book_entry* e ){

    free( e->name );
    free( e->surname );
    free( e->address );
    free( e->email );
    free( e->birth_date );
    free( e->phone_number );
    free( e );

}

void delete_contatct( Adress_book* book,char* name, char* surname ){
    book_entry* e = find_by_name( book, name, surname );
    if( e != NULL ){

        if( e->prev != NULL )
            e->prev->next = e->next;
        else{ //jeśli null to e jest 1 elementem na liście
            book->first = e->next;
            e->next->prev = NULL;
            delete_entry( e );
            return;
        }

        if( e->next != NULL)
            e->next->prev = e->prev;
        delete_entry( e );
        book->entry_count--;
    }

}

void delete_book( Adress_book* book ){
    if( book == NULL ) return;
    book_entry* e = book->first;

    if( e != NULL ){
        while( e->next != NULL ){
            e=e->next;
            delete_entry(e->prev);
            e->prev=NULL;
        }
        delete_entry(e);
    }
    free(book);
}

void print_contact( book_entry* e ){
    printf( "| Name: | %-20s\t| Surname: | %-35s\t| Address: | %-30s\t| Mail: | %-40s\t| Phone: | %-20s\t| Birth date: | %-10s\t|\n",
             e->name, e->surname, e->address, e->email, e->phone_number, e->birth_date );
}

void print_book( Adress_book* book ){
    if( book == NULL ){
    }else{
        book_entry* i = book->first;
        while( i != NULL ){
            print_contact(i);
            i=i->next;
        }
    }
}


/*
 *  funkcja sortująca ciągi znaków - stworzona przez niedowskonałość strcmp() z string.h
 *  jest wykorzystowana przy wszystkich sortowaniach, zwaraca wartości:
 *   -1   gdy s1 jest leksykograficznie po s2
 *   0    gdy s1 = s2;
 *   1    gdy s1 poprzedza s2 leksykograficznie
 */
int string_sort( char* str1, char* str2){

    char* s1 = (char*) malloc( (strlen(str1)+1) * sizeof(char) );
    strcpy(s1, str1);
    char* s2 = (char*) malloc( (strlen(str2)+1) * sizeof(char) );
    strcpy(s2, str2);
    int order = 0;

    int i;
    for( i = 0; order == 0 && i < strlen( s1  ); i++ ){

            //dla wyrównania ACII małych i dużych liter
        if ( s1[i] >= 97 ) s1[i] -= 32;
        if ( s2[i] >= 97 ) s2[i] -= 32;

        if( s1[i] > s2[i] )
            order = -1;
        else if ( s1[i] < s2[i] )
            order = 1 ;
    }

    free( s1 );
    free( s2 );
    return order;
}


//funkcja dla debugowania qs wypisuje na konsolę podany łańcuch book_entry*
void ls (book_entry* l){
    book_entry* tmp=l;
    while ( tmp ){
        print_contact(tmp);
        tmp=tmp->next;
    }
}

/* qsort = quick sort
 * @param entry_list -> wskaźnik listę wpisów,
 * @param sort_fnc  -> wskaźnik na funkcję sortującą
 * funkcja sortująca jest jednocześnie predykatem wsyzstnie funkcje wykorzysują
 * pomocniczą fnc string_sort
 */
book_entry* qsort1 ( book_entry* entry_list, int ( *sort_fnc )( book_entry*, book_entry* ) ){
    // p = predecessor jest listą poprzedników, elementów leksykograficznie poprzedzającyh pivot
    book_entry *p = NULL;
    // s = successor lista następników dla pivota
    book_entry *s = NULL;

    book_entry *pivot,      //pivot dla quicksorta
               *iterator,   //wskaźnik wykorzystwany do iteracji po przekazanej do posotwowania liście
               *tmp,        //wskaźnik wykorzystywany do przypiszań tymczasowych i iteracji
               *result = NULL;  //wynik sortowania zwracany przez funcję

    //Lista musi zawierać conajmniej 2 elementy, aby ją delej sortiwać
    //w przeciwnym przypadku zwracany pojedynczy element
    if( entry_list == NULL || entry_list->next == NULL ) return entry_list;


    pivot = entry_list;
    iterator =  pivot->next;    //poprzedni warunek gwarantuje istnienie 2 elementów (!= NULL)
    pivot->next = NULL;         // wypinam pivot, aby nie doszło do zapętlenia

    //pętla wykomuje podział na listy elementów 'mniejszych' i 'większych od pivota'
    while( iterator != NULL ){
        //przechowuje wartośc kolejnej iteracji
        tmp = iterator->next;

        // sortwanie odywa się za pomocą wskazanej fnc.
        if( sort_fnc( pivot , iterator ) != -1 ){
            // 1 => pivot < iterator, elent trafia do listy następników pivota
            iterator->next = s;
            s = iterator;
        }
        else{   //przypadek przciwny
            iterator->next = p;
            p = iterator;
        }

        iterator = tmp;
    }
    //result  = predecessor : pivot : successor

    //sortowanie listy elementów mniejszych od pivota
    result = qsort1( p, sort_fnc );
    if( result != NULL ){
        //wyszukanie ostatniego elementu listy i dodanie pivota
        for( tmp = result; tmp->next != NULL; tmp = tmp->next );
        tmp->next = pivot;
    }
    else    //nie ma elementów mniejszych od pivota
        result = pivot;

    //sortiwanie więszych od pivota
    tmp = qsort1( s, sort_fnc );
    pivot->next = tmp;

    //Lista isnieje ale należy naprawić wskazania poprzedników
    //przenieść za funkcje -> zmnieszy złożność
    tmp = result;
    if ( tmp != NULL ){
        tmp->prev = NULL;
        while ( tmp->next != NULL ){
            tmp->next->prev = tmp;
            tmp = tmp->next;
        }
    }

    //zwraca posortowaną listę
    return result;
}



//warper dla sortowania po imionach
int compare_name( book_entry* e1, book_entry* e2 ){
   return string_sort( e1->name, e2->name);
}
//po nazwisku
int compare_surname( book_entry* e1, book_entry* e2 ){
   return string_sort( e1->surname, e2->surname);
}

int compare_date( book_entry* e1, book_entry* e2 ){
   return string_sort( e1->birth_date, e2->birth_date);
}

int compare_phone( book_entry* e1, book_entry* e2 ){
   return string_sort( e1->phone_number, e2->phone_number);
}

int compare_email( book_entry* e1, book_entry* e2 ){
   return string_sort( e1->email, e2->email);
}
/*
 *  sort_book reorganizuje książkę wykonując jej sortowanie po wsazanym polu,
 *  @param book -> wskaźnik na książkę,
 *  @param option -> znak określający po jakim polu zosanie wykonane sortowanie,
 *       n -> name
 */
void sort_book_by( Adress_book* book, char option ){
    if( book == NULL || book->first == NULL ) return;

    switch( option ){
        case 'n':   //name
            book->first = qsort1(book->first, &compare_name);
            break;
        case 's':   //surname
            book->first = qsort1(book->first, &compare_surname);
            break;
        case 'd':   //address
            book->first = qsort1(book->first, &compare_date);
            break;
        case 'e':   //email
            book->first = qsort1(book->first, &compare_email);
            break;
        case 'p':   //phone_number
            book->first = qsort1(book->first, &compare_phone);
            break;
        default:
            return;

    }

}


//////////
struct Contact_node {
    char *name;
    char *surname;
    char *birth_date;
    char *email;
    char *phone_number;
	char *address;
    struct Contact_node* parent;
	struct Contact_node* left;
	struct Contact_node* right;
} typedef book_entry_node; //custom alias for type

struct Adress_book_BST {
    int entry_count;
    char order_by;   //może być n- name, s-surname... wskazuje po jakim atrybicie jest posortowane drzewo
    book_entry_node* first;
}typedef Adress_book_BST;

Adress_book_BST* create_book_tree( void ){
    Adress_book_BST* book = ( Adress_book_BST* )malloc(sizeof( struct Adress_book_BST));
    book->order_by = 's';    //domyślna metoda sortowania po nazwisku
    book->first = NULL;
    return book;
}

void delete_entry_node ( book_entry_node* e ){
    free( e->name );
    free( e->surname );
    free( e->address );
    free( e->email );
    free( e->birth_date );
    free( e->phone_number );
    free( e );
}

void delete_book_node( book_entry_node* root ){
    if( root == NULL ) return;
    delete_book_node( root->left );
    delete_book_node( root->right );
    delete_entry_node( root );
}

void delete_book_tree( Adress_book_BST* book ){
    if( book == NULL ) return;
    if ( book->first != NULL ) delete_book_node( book->first );
    free(book);
}

struct Contact_node* create_entry_node (char* name, char* surname, char* address, char* email, char* date, char* phone){

	struct Contact_node* entry = ( book_entry_node* )malloc( sizeof( struct Contact_node ) );

	if (entry != NULL){ //Zallokowano pamięc na nowy wpis
        entry->name = strcpy( (char*) malloc( strlen(name)*sizeof(char)+1 ), name );
        entry->surname = strcpy( (char*) malloc( strlen(surname)*sizeof(char)+1 ), surname );
        entry->address = strcpy( (char*) malloc( strlen(address)*sizeof(char)+1 ), address );
        entry->email = strcpy( (char*) malloc( strlen(email)*sizeof(char)+1 ), email );
        entry->birth_date = strcpy( (char*) malloc( strlen(date)*sizeof(char)+1 ), date );
        entry->phone_number = strcpy( (char*) malloc( strlen(phone)*sizeof(char)+1 ), phone );
        entry->left = entry->right = entry->parent  = NULL;
	} else
        printf("Tworzenie nowego wpisu nie powiodło się - brak wystarczającej ilości pamięci");

	return entry;
}

void print_contact_node( book_entry_node* e ){
    if( e == NULL ) printf("Brak_Konstaktu\n");
    else
        printf( "| Name: | %-20s\t| Surname: | %-35s\t| Address: | %-30s\t| Mail: | %-40s\t| Phone: | %-20s\t| Birth date: | %-10s\t|\n",
                e->name, e->surname, e->address, e->email, e->phone_number, e->birth_date );
}


void print_book_node( book_entry_node* entry ){
    if( entry == NULL ) return;

    print_book_node( entry->left );
    print_contact_node( entry );
    print_book_node( entry->right );
}

void print_book_tree( Adress_book_BST* book ){
    if( book == NULL ) return;
    print_book_node( book->first );
}

//warper dla sortowania po imionach
int compare_name_node( book_entry_node* e1, book_entry_node* e2 ){
   return string_sort( e1->name, e2->name);
}
//po nazwisku
int compare_surname_node( book_entry_node* e1, book_entry_node* e2 ){
   return string_sort( e1->surname, e2->surname);
}

int compare_date_node( book_entry_node* e1, book_entry_node* e2 ){
   return string_sort( e1->birth_date, e2->birth_date);
}

int compare_email_node( book_entry_node* e1, book_entry_node* e2 ){
   return string_sort( e1->email, e2->email);
}

int compare_phone_node( book_entry_node* e1, book_entry_node* e2 ){
   return string_sort( e1->phone_number, e2->phone_number);
}

typedef int (*sort_fn)(book_entry_node*, book_entry_node*);

sort_fn select_sort ( char option ){
    switch( option ){
        case 's':
            return &compare_surname_node;
        case 'n':
            return &compare_name_node;
        case 'e':
            return &compare_email_node;
        case 'p':
            return &compare_phone_node;
        case 'd':
            return &compare_date_node;
        default:
            return &compare_surname_node;
    }

}

book_entry_node* add_node( book_entry_node* root, book_entry_node* entry, int ( *sort_fnc )( book_entry_node*, book_entry_node* ) ){
    if( entry == NULL ) return root;
    entry->left = entry->right = NULL;
    int side = 0;
    if( root == NULL ){
        entry->parent = NULL;
        root = entry;
    }
    else{
        book_entry_node *iterator = root, *prev=NULL;
        while( iterator != NULL ){
            prev = iterator;
            side = sort_fnc( entry, iterator );

            if ( side == -1 ) iterator = iterator->right;
            else iterator = iterator->left;

        }

        entry->parent = prev;
        if( side == -1 ) prev->right = entry;
        else prev->left = entry;
    }
    return root;
}

int add_contact_tree( Adress_book_BST* book, book_entry_node* entry ){
    if ( book == NULL ){
        return -1;
    } else {
        book->first = add_node( book->first, entry, select_sort( book->order_by ) );
        return 0; // 0 = no error
    }
}

//tworzy nowe drzewo ( sortowanie przez tworznie nowego  )
book_entry_node* pre_ord ( book_entry_node* root, book_entry_node* target, int ( *sort_fnc )( book_entry_node*, book_entry_node* ) ){
    if( root == NULL ) return target;
    target = pre_ord( root->left, target, sort_fnc );
    target = pre_ord( root->right, target, sort_fnc );
    return add_node( target, root, sort_fnc );
}

void sort_book_tree_by( Adress_book_BST* book, char option ){
    if( book == NULL || book->order_by == option || book->first == NULL ) return;
    book_entry_node* target = NULL;
    book->first = pre_ord( book->first, target, select_sort(option) );
    book->order_by = option;
}

struct Contact_node* find_by_name_tree ( Adress_book_BST* book, char* name, char* surname ){
    if ( book == NULL || book->first == NULL ) return NULL;
    if ( book->order_by != 's' ) sort_book_tree_by(  book, 's' );
    int cmp_res = 0;
    book_entry_node* iterator = book->first;
    while ( iterator != NULL ){

        cmp_res = string_sort( surname, iterator->surname );
        if( cmp_res == -1 )
            iterator = iterator->right;

        else{
			if ( strcmp ( name, iterator->name ) == 0 )
				return iterator;
            iterator = iterator->left;
		}

    }
    return NULL;
}

book_entry_node* get_pred( book_entry_node* e ){
    if( e == NULL ) return NULL;
    book_entry_node* iterator;
    if( e->left != NULL ){
        iterator = e->left;
        while( iterator->right != NULL ) iterator = iterator->right;
        return iterator;
    } else {
        iterator = e->parent;
        while ( iterator != NULL && e == iterator->left ){
            e = iterator;
            iterator = iterator->parent;
        }
        return iterator;
    }
}

void delete_contatct_tree( Adress_book_BST* book,char* name, char* surname ){
    book_entry_node* e = find_by_name_tree( book, name, surname );
    book_entry_node *child_to_move = NULL;
    if( e != NULL ){ //musi istenieć przynajmniej korzeń

		book_entry_node* p = e->parent;
        if( e->left == NULL || e->right == NULL ){


            if (  e->left == NULL && e->right == NULL )
                ;

            else if ( e->left != NULL && e->right == NULL )
                child_to_move = e->left;

            else if ( e->right != NULL )
                child_to_move = e->right;



            if (  p != NULL ){
                if( child_to_move != NULL )
                    child_to_move->parent = p;
                if ( p->left == e )
                    p->left = child_to_move;
                if ( p->right == e )
                    p->right = child_to_move;

            }else{
                if( child_to_move != NULL ) child_to_move->parent = NULL;
                book->first = child_to_move;
            }

            delete_entry_node( e );
            return;
        }
        book_entry_node* pred = get_pred( e );
        //Posiada obu synów

            if ( pred->right != NULL )	//posiada jedynie lewego syna
                child_to_move = pred->right;
            else if (pred->left!=NULL )
                child_to_move = pred->left;

            if( child_to_move != NULL ) child_to_move->parent = pred->parent;

            if ( pred->parent->left == pred ) pred->parent->left = child_to_move;
            else pred->parent->right = child_to_move;

            if( e->parent != NULL ){
                if ( e == e->parent->left ) e->parent->left = pred;
                else e->parent->right = pred;
            }
            pred->parent = e->parent;
            pred->left = e->left;
            pred->right = e->right;

        if( pred->parent == NULL ) book->first = pred;
        delete_entry_node( e );
        book->entry_count--;
    }
}


#endif

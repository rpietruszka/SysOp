#ifndef _BOOK1
#define _BOOK1
//list
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


Adress_book* create_book(void);
book_entry* create_entry (char*, char*, char*, char*, char* , char*);
int add_contact( Adress_book*, book_entry* );
struct Contact* find_by_name ( Adress_book* , char* , char* );
void delete_entry( book_entry* );
void delete_contatct( Adress_book* ,char* , char*  );
void delete_book( Adress_book* );
void print_contact( book_entry* );
void print_book( Adress_book* );
int string_sort( char*, char*);
void ls (book_entry* );
book_entry* qsort1 ( book_entry*, int ( *sort_fnc )( book_entry*, book_entry* ) );
int compare_name( book_entry* , book_entry* );
int compare_surname( book_entry*, book_entry* );
int compare_address( book_entry*, book_entry* );
int compare_phone( book_entry*, book_entry* );
int compare_email( book_entry*, book_entry* );
void sort_book_by( Adress_book* , char  );


//tree
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
} typedef book_entry_node;

struct Adress_book_BST {
    int entry_count;
    char order_by;   
    book_entry_node* first;
}typedef Adress_book_BST;

Adress_book_BST* create_book_tree( void );
void delete_entry_node ( book_entry_node*  );
void delete_book_node( book_entry_node* );
void delete_book_tree( Adress_book_BST*);
struct Contact_node* create_entry_node (char* , char* , char* , char* , char* , char* );
void print_contact_node( book_entry_node*);
void print_book_node( book_entry_node* );
void print_book_tree( Adress_book_BST* );
int compare_name_node( book_entry_node*, book_entry_node* );
int compare_surname_node( book_entry_node*, book_entry_node*);
int compare_date_node( book_entry_node*, book_entry_node* );
int compare_email_node( book_entry_node*, book_entry_node*);
int compare_phone_node( book_entry_node*, book_entry_node*);
typedef int (*sort_fn)(book_entry_node*, book_entry_node*);
sort_fn select_sort ( char );
book_entry_node* add_node( book_entry_node*, book_entry_node*, int ( *sort_fnc )( book_entry_node*, book_entry_node* ) );
int add_contact_tree( Adress_book_BST* , book_entry_node* );
book_entry_node* pre_ord ( book_entry_node*, book_entry_node*, int ( *sort_fnc )( book_entry_node*, book_entry_node* ) );
void sort_book_tree_by( Adress_book_BST* , char );
struct Contact_node* find_by_name_tree ( Adress_book_BST* , char* , char* );
book_entry_node* get_pred( book_entry_node*  );
void delete_contatct_tree( Adress_book_BST* ,char* , char* );


#endif
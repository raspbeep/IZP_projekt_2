/**
 * @name IZP Projekt 2 - Praca s datovymi strukturami
 * @author Pavel Kratochvíl
 * login: xkrato61
 * version: V1.0
 */


#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#define DEF_DELIM ' '
#define MAX_ARGUMENT_LENGTH 1000
#define MAX_DELIM_SIZE 101

// TODO dorobit file input ako posledny argument zo vstupu
// TODO mallocovat agrument string

typedef enum {SELECTION, ARGUMENT} ParseMode;

// struct premennej, vie v sebe ulozit cislo alebo string
typedef struct variable {
    int typ_premennej;      // -1 = neurcena, 0 = float, 1 = string
    int selection[4];       // miesto na docasne ulozenie selekcie
    double cislo_f;         // miesto na ulozenie floatu
    char *string;           // miesto na ulozenie stringu
} Variable;

typedef struct selection_of_cells {
    int selection[4];       // vyber na konkretne okno buniek
    int selection_type;     // 0 = selekcia s dvomi parametrami [1,1], 1 = selekcia rozsahu [1,1,1,2]
    bool max;               // indikuje ci je zvoleny aj max parameter
    bool min;               // min parameter
    bool find;              // ci bol zvoleny aj find
    char *find_str;         // ak bol zvoleny find tak potrebuje aj str
    int set_selection[4];   // selekcia buniek pre [set]
    double selected_value;     // hodnota vybrana blizsou selekciou s [max], [min]
} Selection;

typedef struct cell {
    int dlzka_obsahu;
    int *obsah;
} Cell;

typedef struct row {
    int pocet_buniek;
    Cell *zoznam_buniek;
} Row;

typedef struct table {
    int pocet_riadkov;
    Row *zoznam_riadkov;
} Table;

typedef enum {SCAN_DELIM, AWAIT_DELIM, DONE} DelimMode;
typedef enum {SCAN_DELIM_AND_ARGS, EXIT}RunMode;//, RUN, ALL_DONE, RANGE_ERROR, EMPTY_FILE} RunMode;
int is_delim (const char *delim, char *delim_string[], const bool *multi_character_delim, int znak);
char * save_delim_and_args(int argc, char **argv, char *delim, char delim_string[], bool *multi_character_delim);
Table * load_table_from_file(char *delim, char *delim_string, bool multi_character_delim);
void print_table (Table *tabulka, char delim);
void dealloc_table (Table *tabulka);
void level_table(Table *tabulka);
void dealloc_variables(Variable *list);
Variable *init_list_of_variables();
bool parse_arguments (const char *string_of_all_params, Table *tabulka, Variable *list_of_variables);
void init_selection(Selection *aktualna_selekcia);
void zero_selection(Selection *aktualna_selekcia);
bool refill_table (Table *tabulka, Selection *aktualna_selekcia);
bool set_selection (Table *tabulka, Selection *aktualna_selekcia, int *position, int *position_of_next_bracket, const char *string);
bool extract_params(const char *string, int *position, int pos_of_first_n, int *param1, int *param2);
bool swap_cells (Table *tabulka, int row1, int col1, int row2, int col2);

// FUNKCIE NA UPRAVU STRUKTURY TABULKY
bool irow(Table *tabulka, Selection *aktualna_selekcia);
bool arow(Table *tabulka, Selection *aktualna_selekcia);
bool drow(Table *tabulka, Selection *aktualna_selekcia);
bool icol(Table *tabulka, Selection *aktualna_selekcia);
bool acol(Table *tabulka, Selection *aktualna_selekcia);
bool dcol(Table *tabulka, Selection *aktualna_selekcia);

// FUNKCIE NA UPRAVU OBSAHU TABULKY
bool set_str (Table *tabulka, Selection *aktualna_selekcia, char string_param[1000]);
bool clear(Table *tabulka, Selection *aktualna_selekcia);
bool swap(Table *tabulka, Selection *aktualna_selekcia, int row, int col);
bool sum(Table *tabulka, Selection *aktualna_selekcia, int row, int col);
bool avg(Table *tabulka, Selection *aktualna_selekcia, int row, int col);
bool count(Table *tabulka, Selection *aktualna_selekcia, int row, int col);

// FUNKCIE NA PREMENNE
bool def_var (Table *tabulka, Selection *aktualna_selekcia, Variable *list_of_variables, int *position, const char *string);
bool use_var(Table *tabulka, Selection *aktualna_selekcia, Variable *list_of_variables, int *position, const char *string);
bool inc_var(Selection *aktualna_selekcia, Variable *list_of_variables, int *position, const char *string);
bool set_var(Selection *aktualna_selekcia, Variable *list_of_variables, int *position);
bool set_var_use(Selection *aktualna_selekcia, Variable *list_of_variables, int *position);

int main(int argc, char *argv[]) {

    /*
    // TODO NA KONCI ODKOMENTOVAT

    if (argc != 5 && argc != 3) return -1;

    if (argc == 3) {
        freopen(argv[2], "r", stdin);
    } else {
        freopen(argv[4], "r", stdin);
    }

    */

    // jednoznakovy delim
    char delim;

    // buffer na viacznakovy delim
    char delim_string[MAX_DELIM_SIZE] = "";
    // bool ci som nasiel zadany delim

    // bool ked mam viacznakovy
    bool multi_character_delim = false;

    // hlavny stavovy automat
    RunMode run_mode = SCAN_DELIM_AND_ARGS;

    // string vsetkych parametrov z inputu
    char *string_of_all_params;

    // vytvorenie pointra na tabulku
    Table *tabulka;

    // vytvorenie listu pre premenne def _X
    Variable *list_of_variables;
    // inicializacia listu premennych, naplnia sa nulami, "", a -1
    list_of_variables = init_list_of_variables();

    //Arguments *list_of_arguments;

    int exit_code = 0;

    while (run_mode != EXIT) {
        switch (run_mode) {
            case SCAN_DELIM_AND_ARGS:

                // ulozenie delimov a string_of_all_params
                string_of_all_params = save_delim_and_args(argc, argv, &delim, delim_string, &multi_character_delim);

                // nacitanie tabulky do trojrozmerneho structov
                tabulka = load_table_from_file(&delim, delim_string, multi_character_delim);

                // dorovnanie tabulky aby bol vsade rovnaky pocet stlpcov
                level_table(tabulka);

                if (parse_arguments(string_of_all_params, tabulka, list_of_variables) == false) exit_code = -1;

                // basic vytlacenie tabulky
                print_table(tabulka, delim);

                run_mode = EXIT;
                break;

            case EXIT :
                break;
        }
    }
    // DEALLOC

    // string input stringu s argumentmi
    free (string_of_all_params);

    // dealokacia celej tabulky
    dealloc_table(tabulka);

    // dealokacia structu na vsetky premenne
    //free(all_args);

    // dealokacia listu premennych
    dealloc_variables(list_of_variables);

    return exit_code;
}

Table *load_table_from_file(char *delim, char *delim_string, bool multi_character_delim) {
    // nahra tabulku zo suboru

    int znak;

    // alokacia pamate pre struct bunka
    Table *tabulka;
    tabulka = malloc(sizeof(Table));
    if (tabulka == NULL) return NULL;

    // dlzka aktualnej bunky
    int dlzka_obsahu = 1;

    // prvotna inicializacia structov
    //vytvori sa zoznam riadkov, prvy riadok, zoznam buniek, bunka a obsah
    tabulka->pocet_riadkov = 1;

    Row *new_row;
    new_row = malloc(sizeof(Row) * 1);
    if (new_row == NULL) return NULL;
    tabulka->zoznam_riadkov = new_row;

    Cell *new_cell;
    new_cell = malloc(sizeof(Cell) * 1);
    if (new_cell == NULL) return NULL;
    tabulka->zoznam_riadkov->zoznam_buniek = new_cell;

    tabulka->zoznam_riadkov->pocet_buniek = 1;
    tabulka->zoznam_riadkov->zoznam_buniek->dlzka_obsahu = dlzka_obsahu;

    int *new_content;
    new_content = malloc(sizeof(int));
    if (new_content == NULL) return NULL;
    tabulka->zoznam_riadkov->zoznam_buniek->obsah = new_content;

    // pocet buniek ktore som uz presiel v aktualnom riadku
    int aktualny_pocet_riadkov = 1;

    // pocet buniek ktore som uz presiel v aktualnom riadku
    int aktualny_pocet_buniek = 1;

    // nacitanie prveho znaku
    znak = fgetc(stdin);

    // premenna na quotes mode
    bool in_quotes = false;

    // bool na escape char
    bool escape_char = false;

    // kym neskapem
    while (znak != EOF) {

        // ak mam delim uzaviem aktualnu bunku a ulozim do aktualneho zoznamu buniek
        if (is_delim(delim, &delim_string, &multi_character_delim, znak) && !in_quotes && !escape_char) {

            // nacitanie noveho znaku
            znak = fgetc(stdin);

            // ak nie je dalsi znak EOF tak si chcem zvacsit miesto na dalsie bunky
            if (znak != '\n' && znak != EOF) {

                // inkrementacia poctu buniek v aktualnom riadku
                aktualny_pocet_buniek++;

                //Cell *init_ptr = NULL;
                //tabulka->zoznam_riadkov[aktualny_pocet_riadkov-1].zoznam_buniek = init_ptr;

                // zvacsenie zoznamu aktualnych buniek
                Cell *newptr;
                newptr = realloc(tabulka->zoznam_riadkov[aktualny_pocet_riadkov-1].zoznam_buniek, sizeof(Cell) * aktualny_pocet_buniek);

                // overenie pointra
                if (newptr == NULL) return NULL;

                // priradenie nenuloveho pointra
                tabulka->zoznam_riadkov[aktualny_pocet_riadkov-1].zoznam_buniek = newptr;

                // intrementracia poctu buniek v aktualnom riadku
                tabulka->zoznam_riadkov[aktualny_pocet_riadkov - 1].pocet_buniek++;

                //ak je znak doublequote tak sa zapne mod in_quotes
                if (znak == 34) {
                    in_quotes = true;

                    // alokacia novej pamate na obsah bunky
                    int *new_ptr = malloc(sizeof(int) * 2);
                    if (new_ptr == NULL) return NULL;
                    tabulka->zoznam_riadkov[aktualny_pocet_riadkov-1].zoznam_buniek[aktualny_pocet_buniek - 1].obsah = new_ptr;

                    // uvodzovky sa pridaju na zaciatok obsahu bunky
                    tabulka->zoznam_riadkov[aktualny_pocet_riadkov-1].zoznam_buniek[aktualny_pocet_buniek-1].obsah[0] = 34;

                    //resetovanie dlzky obsahu novej bunky
                    tabulka->zoznam_riadkov[aktualny_pocet_riadkov-1].zoznam_buniek[aktualny_pocet_buniek - 1].dlzka_obsahu = 2;

                    dlzka_obsahu = 2;

                }else {

                    // alokacia novej pamate na obsah bunky
                    int *new_ptr = malloc(sizeof(int));
                    if (new_ptr == NULL) return NULL;
                    tabulka->zoznam_riadkov[aktualny_pocet_riadkov-1].zoznam_buniek[aktualny_pocet_buniek - 1].obsah = new_ptr;

                    //resetovanie dlzky obsahu novej bunky
                    tabulka->zoznam_riadkov[aktualny_pocet_riadkov-1].zoznam_buniek[aktualny_pocet_buniek - 1].dlzka_obsahu = 1;

                    // resetovanie dlzky obsahu
                    dlzka_obsahu = 1;
                }

            } else {
                if (znak == '\n' ) {

                    // inkrementacia poctu buniek v aktualnom riadku
                    aktualny_pocet_buniek++;

                    // alokacia pamate na novy pocet buniek
                    Cell *newptr;
                    newptr = realloc(tabulka->zoznam_riadkov[aktualny_pocet_riadkov-1].zoznam_buniek, sizeof(Cell) * aktualny_pocet_buniek);
                    if (newptr == NULL) return NULL;
                    tabulka->zoznam_riadkov[aktualny_pocet_riadkov - 1].zoznam_buniek = newptr;
                    tabulka->zoznam_riadkov[aktualny_pocet_riadkov - 1].pocet_buniek++;

                    // alokacia pamate na novy obsah bunky
                    int *novy_obsah;
                    novy_obsah = malloc(sizeof(int));
                    if (novy_obsah == NULL) return NULL;

                    tabulka->zoznam_riadkov[aktualny_pocet_riadkov-1].zoznam_buniek[aktualny_pocet_buniek - 1].obsah = novy_obsah;
                    tabulka->zoznam_riadkov[aktualny_pocet_riadkov-1].zoznam_buniek[aktualny_pocet_buniek - 1].dlzka_obsahu = 1;

                    // vytvorenie prazdnej bunky na konci riadku
                    dlzka_obsahu = 1;

                }
            }

        // iny znak ako delim
        } else if (znak != '\n'){


            if (znak == 92) {
                // zapne sa mode escape_char takze dalsi znak sa ulozi aj keby to bol delim
                escape_char = true;

            // ak je znak double quote tak sa vypne z modu in quotes
            }
            if (znak == 34 && in_quotes) {

                //nacitanie dalsieho znaku lebo quote mozem preskocit
                znak = fgetc(stdin);


                if (znak == 92) {
                    // zapne sa mode escape_char takze dalsi znak sa ulozi aj keby to bol delim
                    escape_char = true;
                }

                // ak najdem uvodzovky a dalsi znak je neescapnuty delim tak chcem vypnut in quotes
                if (is_delim(delim, &delim_string, &multi_character_delim, znak)) {
                    // vypne sa in quotes mode
                    in_quotes = false;

                    //uvodzovky sa pridaju do obsahu bunky
                    tabulka->zoznam_riadkov[aktualny_pocet_riadkov-1].zoznam_buniek[aktualny_pocet_buniek-1].obsah[dlzka_obsahu-1] = 34;

                    // zvacsi sa dlzka obsahu v aktualnej bunke
                    tabulka->zoznam_riadkov[aktualny_pocet_riadkov-1].zoznam_buniek[aktualny_pocet_buniek-1].dlzka_obsahu = dlzka_obsahu;
                } else {

                    //znak sa prida do obsahu
                    tabulka->zoznam_riadkov[aktualny_pocet_riadkov - 1].zoznam_buniek[aktualny_pocet_buniek - 1].obsah[dlzka_obsahu - 1] = znak;

                    // zvacsi sa dlzka obsahu v aktualnej bunke
                    tabulka->zoznam_riadkov[aktualny_pocet_riadkov - 1].zoznam_buniek[aktualny_pocet_buniek - 1].dlzka_obsahu = dlzka_obsahu;

                    if (znak != 92 && escape_char) {
                        // vypnutie escape charu po pridani escapenuteho znaku
                        escape_char = false;
                    }


                    // nacitanie dalsieho znaku
                    znak = fgetc(stdin);
                }
            } else {

                //znak sa prida do obsahu
                tabulka->zoznam_riadkov[aktualny_pocet_riadkov - 1].zoznam_buniek[aktualny_pocet_buniek - 1].obsah[dlzka_obsahu - 1] = znak;

                // zvacsi sa dlzka obsahu v aktualnej bunke
                tabulka->zoznam_riadkov[aktualny_pocet_riadkov - 1].zoznam_buniek[aktualny_pocet_buniek - 1].dlzka_obsahu = dlzka_obsahu;

                if (znak != 92 && escape_char) {
                    // vypnutie escape charu po pridani escapenuteho znaku
                    escape_char = false;
                }

                // nacitanie dalsieho znaku
                znak = fgetc(stdin);
            }

            // ak nie je dalsi znak EOF tak vytvorim nove miesto na dalsi znak
            if (znak != EOF && znak != '\n' && !escape_char && !is_delim(delim, &delim_string, &multi_character_delim, znak)) {

                // inkrementacia dlzky obsahu lebo dalsi znak bude tiez patrit do aktualnej bunky
                dlzka_obsahu++;

                // alokacia pamate na dalsi znak obsahu
                int *new_obsah;
                new_obsah = realloc(tabulka->zoznam_riadkov[aktualny_pocet_riadkov-1].zoznam_buniek[aktualny_pocet_buniek-1].obsah, sizeof(int) * dlzka_obsahu);

                // konla nuloveho pointra
                if (new_obsah == NULL) return NULL;

                // priradenie nenuloveho pointra
                tabulka->zoznam_riadkov[aktualny_pocet_riadkov-1].zoznam_buniek[aktualny_pocet_buniek-1].obsah = new_obsah;

            } else if (escape_char) {

                // inkrementacia dlzky obsahu lebo dalsi znak bude tiez patrit do aktualnej bunky
                dlzka_obsahu++;

                // alokacia pamate na dalsi znak obsahu
                int *new_obsah;
                new_obsah = realloc(tabulka->zoznam_riadkov[aktualny_pocet_riadkov-1].zoznam_buniek[aktualny_pocet_buniek-1].obsah, sizeof(int) * dlzka_obsahu);

                // konla nuloveho pointra
                if (new_obsah == NULL) return NULL;

                // priradenie nenuloveho pointra
                tabulka->zoznam_riadkov[aktualny_pocet_riadkov-1].zoznam_buniek[aktualny_pocet_buniek-1].obsah = new_obsah;

            }


        } else {
            // narazil som na koniec riadku, ulozim obsah bunky do Cell

            // nacitanie noveho znaku
            znak = fgetc(stdin);

            if (znak != EOF) {

                // inkrementacia aktualneho poctu riadkov
                aktualny_pocet_riadkov++;

                // zvacsenie aktualneho zoznamu riadkov
                Row *newptr;
                newptr = realloc(tabulka->zoznam_riadkov, sizeof(Row) * aktualny_pocet_riadkov);

                // kontrola nuloveho pointra
                if (newptr == NULL) return NULL;

                // priradenie nenuloveho pointra
                tabulka->zoznam_riadkov = newptr;

                // inkrementacia poctu riadkov
                tabulka->pocet_riadkov++;

                // resetovanie dlzky obsahu
                dlzka_obsahu = 1;

                // resetovanie poctu buniek v aktualnom riakdu
                aktualny_pocet_buniek = 1;

                // alokacia pamate na bunku v novom riadku
                Cell *newcell;
                newcell = malloc(sizeof(Cell) * aktualny_pocet_buniek);

                // kontrola nuloveho pointra
                if (newcell == NULL) return NULL;

                // priradenie nenuloveho pointra
                tabulka->zoznam_riadkov[aktualny_pocet_riadkov-1].zoznam_buniek = newcell;

                // alokacia pamate na obsahu bunky
                tabulka->zoznam_riadkov[aktualny_pocet_riadkov - 1].zoznam_buniek[aktualny_pocet_buniek - 1].obsah = malloc(sizeof(int));

                // resetovanie dlzky obsahu bunky
                tabulka->zoznam_riadkov[aktualny_pocet_riadkov - 1].zoznam_buniek[aktualny_pocet_buniek - 1].dlzka_obsahu = 1;

                // resetovanie poctu buniek v riadku
                tabulka->zoznam_riadkov[aktualny_pocet_riadkov - 1].pocet_buniek = 1;
            }
        }
    }

    return tabulka;
}

void print_table (Table *tabulka, char delim) {
    // vyprintuje celu tabulku, nahradi delimi, kazdy riadok okrem posledneho zalomi

    for (int riadok = 0; riadok < tabulka->pocet_riadkov; riadok++) {
        for (int bunka = 0; bunka < tabulka->zoznam_riadkov[riadok].pocet_buniek; bunka++) {
            if (tabulka->zoznam_riadkov[riadok].zoznam_buniek[bunka].dlzka_obsahu == 1) {
                if (tabulka->zoznam_riadkov[riadok].zoznam_buniek[bunka].obsah[0] != '\0') {
                    for (int znak = 0; znak < tabulka->zoznam_riadkov[riadok].zoznam_buniek[bunka].dlzka_obsahu;znak++) {
                        printf("%c", tabulka->zoznam_riadkov[riadok].zoznam_buniek[bunka].obsah[znak]);
                    }
                }
            }else {
                for (int znak = 0; znak < tabulka->zoznam_riadkov[riadok].zoznam_buniek[bunka].dlzka_obsahu;znak++) {
                    printf("%c", tabulka->zoznam_riadkov[riadok].zoznam_buniek[bunka].obsah[znak]);
                }
            }

            if (bunka+1 < tabulka->zoznam_riadkov[riadok].pocet_buniek) printf("%c", delim);
        }
        if (riadok + 1 < tabulka->pocet_riadkov) printf("\n");
    }
}

void dealloc_table (Table *tabulka) {
    // dealokuje celu tabulku vratane structu tabulka, neostane nic

    for (int riadok = 0; riadok < tabulka->pocet_riadkov; riadok++) {
        for (int bunka = 0; bunka < tabulka->zoznam_riadkov[riadok].pocet_buniek; bunka ++) {
            free(tabulka->zoznam_riadkov[riadok].zoznam_buniek[bunka].obsah);
        }
        free(tabulka->zoznam_riadkov[riadok].zoznam_buniek);
    }
    free(tabulka->zoznam_riadkov);
    free(tabulka);
}

void zero_selection(Selection *aktualna_selekcia) {
    for (int i = 0; i < 4; i++) {
        aktualna_selekcia->selection[i] = 0;
    }
}

char *save_delim_and_args(int argc, char **argv, char *delim, char delim_string[MAX_DELIM_SIZE], bool *multi_character_delim) {
    // zisti ci bol zadany delim, ulozi ho, prekopiruje string argumentov string_of_all_params
    if (!strcmp(argv[1], "-d")) {

        if (strlen(argv[2]) > 1) {
            *multi_character_delim = true;
            strcpy(delim_string, argv[2]);
            *delim = argv[2][0];
        } else {
            *delim = *argv[2];
        }

        char *string_of_all_params = malloc(sizeof(char) * (strlen(argv[3]) + 1));

        strcpy(string_of_all_params, argv[3]);
        string_of_all_params[strlen(argv[3])] = '\0';
        return string_of_all_params;

    } else {
        *delim = DEF_DELIM;
        *multi_character_delim = false;

        char *string_of_all_params = malloc(sizeof(char) * (strlen(argv[1]) + 1));
        strcpy(string_of_all_params, argv[1]);
        string_of_all_params[strlen(argv[1])] = '\0';
        return string_of_all_params;

    }
}

int is_delim (const char *delim, char *delim_string[], const bool *multi_character_delim, int znak) {
    // porovna ci sa zadany znak nachadza v retazci delim
    //  ak je znak delim, vrati 1, inak 0

    if (*multi_character_delim) {
        if (strchr(*delim_string, (char)znak) != NULL ) {
            return 1;
        }
        return 0;
    }else{
        if(*delim == znak){
            return 1;
        }
        return 0;
    }
}

void level_table (Table *tabulka) {
    // najde maximalny pocet stlpcov v celej tabulke a dorovna tabulku na tento pocet v kazdom riadku

    int max_pocet_stlpcov = 1;

    for (int riadok = 0; riadok < tabulka->pocet_riadkov; riadok++) {

        if (tabulka->zoznam_riadkov[riadok].pocet_buniek > max_pocet_stlpcov) {
            max_pocet_stlpcov = tabulka->zoznam_riadkov[riadok].pocet_buniek;
        }
    }

    for (int riadok = 0; riadok < tabulka->pocet_riadkov; riadok++) {
        if (tabulka->zoznam_riadkov[riadok].pocet_buniek != max_pocet_stlpcov) {

            int pocet_buniek_v_stlpci = tabulka->zoznam_riadkov[riadok].pocet_buniek;

            for (int aktualny_pocet_buniek = pocet_buniek_v_stlpci + 1; aktualny_pocet_buniek <= max_pocet_stlpcov; aktualny_pocet_buniek++) {

                Cell *newcell = realloc(tabulka->zoznam_riadkov[riadok].zoznam_buniek, sizeof(Cell) * aktualny_pocet_buniek);

                // priradenie nenuloveho pointra
                tabulka->zoznam_riadkov[riadok].zoznam_buniek = newcell;

                // alokacia pamate na obsahu bunky
                tabulka->zoznam_riadkov[riadok].zoznam_buniek[aktualny_pocet_buniek - 1].obsah = malloc(sizeof(int));

                // resetovanie dlzky obsahu bunky
                tabulka->zoznam_riadkov[riadok].zoznam_buniek[aktualny_pocet_buniek - 1].dlzka_obsahu = 1;

                tabulka->zoznam_riadkov[riadok].pocet_buniek = aktualny_pocet_buniek;
            }

        }
    }
}

Variable *init_list_of_variables (){
    Variable *list;
    list = malloc(sizeof(Variable) * 11);

    for (int i = 0; i < 11; i++) {
        for (int pos = 0; pos < 4; pos++) list[i].selection[pos] = 0;
        list[i].typ_premennej = -1;
        list[i].cislo_f = 0.0;
    }

    return list;
}

void dealloc_variables (Variable *list) {
    free(list);
}

int verified_int (const char string[]) {
    // overenie spravnosti zadaneho argumentu
    // ak je vsetko dobre vrati 1

    // kontrola minusu v int
    if (string[0] == '-'){
        fprintf(stderr, "[ERROR] Parameter nesmie byt zaporne cislo.\n");
        return 0;
    }

    // kontrola ci su vsetky znaky int
    for (int position_in_string = 0; string[position_in_string] != '\0'; position_in_string++){
        if(!isdigit((char)string[position_in_string])){
            fprintf(stderr,"[ERROR] Nespravne argumenty (objavil sa necislovy znak.)\n");
            return 0;
        }
    }

    // kontrola ci je ine od nuly
    if((int)strtol(string, NULL, 10) == 0){ // NOLINT(cert-err34-c)
        fprintf(stderr, "[ERROR] Parameter 1 musi byt vacsi ako nula.\n");
        return 0;
    }
    return 1;
}

void init_selection (Selection *aktualna_selekcia) {

    for (int i = 0; i < 4; i++) aktualna_selekcia->selection[i] = 0;
    aktualna_selekcia->selection_type = 0;
    aktualna_selekcia->max = false;
    aktualna_selekcia->min = false;
    aktualna_selekcia->find = false;
    aktualna_selekcia->find_str = NULL;
    for (int i = 0; i < 4; i++) aktualna_selekcia->set_selection[i] = 0;

}

void dealloc_selection (Selection *aktualna_selekcia) {
    free(aktualna_selekcia->find_str);
}

bool refill_table (Table *tabulka, Selection *aktualna_selekcia) {

    int rows_over = aktualna_selekcia->selection[2] - tabulka->pocet_riadkov;
    int cols_over = aktualna_selekcia->selection[3] - tabulka->zoznam_riadkov->pocet_buniek;

    if (rows_over > 0) {

        // alokacia pamate na nove riadky
        Row *new_rows = realloc(tabulka->zoznam_riadkov, sizeof(Row) * aktualna_selekcia->selection[2]);

        if (new_rows == NULL) return false;

        tabulka->zoznam_riadkov = new_rows;

        for (int riadok = tabulka->pocet_riadkov; riadok < aktualna_selekcia->selection[2]; riadok++) {

            Cell *new_cells;
            new_cells = malloc(sizeof(Cell) * tabulka->zoznam_riadkov->pocet_buniek);

            if (new_cells == NULL) return false;

            tabulka->zoznam_riadkov[riadok].zoznam_buniek = new_cells;

            for (int bunka = 0; bunka < tabulka->zoznam_riadkov->pocet_buniek; bunka++) {

                int *new_content = malloc(sizeof(int));

                if (new_content == NULL) return false;

                *new_content = '\0';

                tabulka->zoznam_riadkov[riadok].zoznam_buniek[bunka].obsah = new_content;
                tabulka->zoznam_riadkov[riadok].zoznam_buniek[bunka].dlzka_obsahu = 1;

            }
        }
        tabulka->pocet_riadkov =  aktualna_selekcia->selection[2];
    }

    if (cols_over > 0) {
        for (int riadok = 0; riadok < aktualna_selekcia->selection[2]; riadok++) {

            Cell *new_row;

            new_row = realloc(tabulka->zoznam_riadkov[riadok].zoznam_buniek, sizeof(Cell) * aktualna_selekcia->selection[3]);

            if(new_row == NULL) return false;

            tabulka->zoznam_riadkov[riadok].zoznam_buniek = new_row;

            for (int bunka = tabulka->zoznam_riadkov[riadok].pocet_buniek; bunka < aktualna_selekcia->selection[3]; bunka++) {

                int *new_content = malloc(sizeof(int));

                if (new_content == NULL) return false;

                *new_content = '\0';

                tabulka->zoznam_riadkov[riadok].zoznam_buniek[bunka].obsah = new_content;
                tabulka->zoznam_riadkov[riadok].zoznam_buniek[bunka].dlzka_obsahu = 1;

            }

            tabulka->zoznam_riadkov[riadok].pocet_buniek = aktualna_selekcia->selection[3];

        }
    }

    return true;
}

bool verify_command (const char *string_of_all_params, int *position, char verif_command[4]) {
    int position_of_next_delim = *position;
    char command[100] = "";
    int position_in_command = 0;
    while (string_of_all_params[position_of_next_delim] != ';' && string_of_all_params[position_of_next_delim] != '\0') {
        command[position_in_command] = string_of_all_params[position_of_next_delim];
        position_in_command++;
        position_of_next_delim++;
    }
    command[position_of_next_delim] = '\0';

    *position = position_of_next_delim;

    // overenie ci to je skutocne irow
    if (strstr(command, verif_command) != NULL) {
        return true;
    }else {
        return false;
    }

}

bool set_selection (Table *tabulka, Selection *aktualna_selekcia, int *position, int *position_of_next_bracket, const char *string) {
    zero_selection(aktualna_selekcia);

    // chcem hladat ']' hned od dalsej pozicie
    *position_of_next_bracket = *position + 1;
    int pocet_ciarok = 0;
    int pozicia_ciarok[3] = {0};

    // najdenie pozicie uzatvarajucej ']'
    while(string[*position_of_next_bracket] != ']') {

        // ak su po ceste nejakej ',' tak si ulozim ich pocet a poziciu
        if (string[*position_of_next_bracket] == ',') {

            // ulozenie pozicie
            pozicia_ciarok[pocet_ciarok] = *position_of_next_bracket;

            //ulozenie poctu ciarok
            pocet_ciarok++;
        }
        *position_of_next_bracket = *position_of_next_bracket + 1;
    };

    if (pocet_ciarok != 1 && pocet_ciarok != 3) return false;

    // tmp postition zacina na position
    int tmp_position = *position;

    // string do ktoreho ulozim priestor medzi kazdymi dvomi ciarkami
    // stanovena max dlzka zo zadania
    char tmp_string[MAX_ARGUMENT_LENGTH] = "";
    int cele_cislo = 0;

    //cele_cislo = (int)strtol(tmp_string, NULL, 10);

    // kolko cifier ma aktualne cislo
    int dlzka_cisla = 0;

    // ulozenie kazdeho cisla do parametra structu aktualna_selekcia
    for (int i = 0; i < pocet_ciarok + 1; i++) {

        // zistenie v akom rozsahu sa nachadza dalsie cislo
        if (pocet_ciarok == 1) {

            if (i==0) {
                dlzka_cisla = pozicia_ciarok[i] - tmp_position - 1;
            } else {
                dlzka_cisla = *position_of_next_bracket - tmp_position - 1;
            }
            // ak mam 3 ciarky a teda 4 parametre
        } else {

            if (i < 3) {

                // od poslednej ciarky po dalsiu ciarku
                dlzka_cisla = pozicia_ciarok[i] - tmp_position - 1;
            } else {

                // od poslednej ciarky po dalsiu zatvorku ']'
                dlzka_cisla = *position_of_next_bracket - tmp_position - 1;
            };
        }

        // skopirovanie jedneho cisla do tmp_string
        for (int x = 0; x < dlzka_cisla; x++) {
            tmp_string[x] = string[tmp_position + 1 + x];
        }

        // uzavretie tmp_string na poslendom mieste
        tmp_string[dlzka_cisla] = '\0';

        bool pomlcka = false;

        if (pocet_ciarok == 3 && tmp_string[0] == '-' && tmp_string[1] == '\0') pomlcka = true;


        // ak narazim na parameter bez cisla [_,1] alebo [1,_]
        if (tmp_string[0] == '_' && tmp_string[1] == '\0') {

            // ak je to selekcia s dvomi parametrami
            if (pocet_ciarok == 1) {

                // ak je _ na nultej pozicii [_,1]
                if (i == 0) {

                    // idem od nulteho riadka
                    aktualna_selekcia->selection[0] = 1;

                    // az po posledny riadok
                    aktualna_selekcia->selection[2] = tabulka->pocet_riadkov;

                    // vyber typu selekcie
                    aktualna_selekcia->selection_type = 1;

                    // ak je na prvej pozicii [1,_]
                } else {

                    // idem od prveho stlpca
                    aktualna_selekcia->selection[1] = 1;

                    // az po posledny stlpec (v celej tabulke by mal byt rovnaky pocet buniek v kazdom riadku)
                    aktualna_selekcia->selection[3] = tabulka->zoznam_riadkov->pocet_buniek;
                }
            }
        } else



        if (pocet_ciarok == 1) {

            // ak som na prvom cisle z dvoch
            if (i == 0) {
                cele_cislo = (int)strtol(tmp_string, NULL, 10);
                // ulozenie prveho cisla
                aktualna_selekcia->selection[0] = cele_cislo;

                // ulozenie prveho cisla
                aktualna_selekcia->selection[2] = cele_cislo;

                // nastavenie typu selekcie
                aktualna_selekcia->selection_type = 0;

                // som na druhom cisle z dvoch
            } else {
                cele_cislo = (int)strtol(tmp_string, NULL, 10);
                // nastavenie druheho cisla
                aktualna_selekcia->selection[1] = cele_cislo;

                // nastavenie druheho cisla
                aktualna_selekcia->selection[3] = cele_cislo;
            }

            // ak mam 3 ciarky, teda 4 parametre
        } else {
            cele_cislo = (int)strtol(tmp_string, NULL, 10);
            if (pomlcka) {
                if (i == 2) {
                    aktualna_selekcia->selection[i] = tabulka->pocet_riadkov;
                }else if (i == 3){
                    aktualna_selekcia->selection[i] = tabulka->zoznam_riadkov->pocet_buniek;
                }
            } else {
                // nastavenie i-teho cisla na aktualny parameter
                aktualna_selekcia->selection[i] = cele_cislo;
            }


        }
        // posun na poziciu dlasej ciarky
        tmp_position = pozicia_ciarok[i];

    }
    // posuvam sa v hlavnom stringu o 1 lebo chcem znak za najblizsou dvojbodkou
    *position = *position_of_next_bracket + 1;
    return true;
}

bool extract_params(const char *string, int *position, int pos_of_first_n, int *param1, int *param2) {
    // index na ktorom je prve cislo
    int tmp_position = *position + pos_of_first_n;

    char tmp_string[1000] = "";

    while (string[tmp_position] != ',' && string[tmp_position] != '\0') {
        tmp_position++;
    }

    for (int znak = *position + pos_of_first_n, position_in_string = 0; znak < tmp_position;znak++, position_in_string++) {
        tmp_string[position_in_string] = string[znak];
    }

    *position = tmp_position + 1;

    // vytvorenie miesta na string ak by v bunke nebolo iba cislo napr. '3.14abc'
    char endptr1[MAX_ARGUMENT_LENGTH] = "";

    // pointer na prvy znak v endptr kvoli funkcii strtod
    char *p1_ptr = endptr1;

    // inicializacia aktualneho cisla
    double curr_number1 = 0.0;

    curr_number1 = strtod(tmp_string, &p1_ptr);

    if (p1_ptr == NULL) return false;

    *param1 = (int)curr_number1;

    tmp_position++;

    while (string[tmp_position] != ']' && string[tmp_position] != '\0') {
        tmp_position++;
    }

    int last_index = 0;
    for (int znak = *position, position_in_string = 0; znak < tmp_position;znak++, position_in_string++) {
        tmp_string[position_in_string] = string[znak];
        last_index++;
    }

    tmp_string[last_index] = '\0';

    // pointer na prvy znak v endptr kvoli funkcii strtod
    char *p2_ptr = endptr1;

    // inicializacia aktualneho cisla
    double curr_number2 = 0.0;

    curr_number2 = strtod(tmp_string, &p2_ptr);

    if (p2_ptr == NULL) return false;

    *param2 = (int)curr_number2;
    *position = tmp_position;
    return true;
}

bool parse_arguments(const char *string, Table *tabulka, Variable *list_of_variables) {

    // vytvorenie pointra na aktualnu selekciu
    Selection *aktualna_selekcia;

    // alokacia pamate na aktualnu selekciu
    aktualna_selekcia = malloc(sizeof(Selection));

    // vynulovanie vsetkych parametrov v selection
    init_selection(aktualna_selekcia);

    // aktualna pozicia v stringu
    int position = 0;

    int position_of_next_bracket;

    // prehladavanie vsetkych argumentov a spustanie funkcii
    while (string[position] != '\0') {

        // ak narazim na hranate zatvorky a za nimi nie je "m"([max],[min]) ani "f"([find])
        // tak to znamena ze po najblizsiu ']' budem mat 2-4 parametre
        if (string[position] == '[' && string[position + 1] != 'm' && string[position + 1] != 'f') {

            if (!set_selection(tabulka, aktualna_selekcia, &position, &position_of_next_bracket, string)) return false;
        }

        // ak narazim na [max] alebo [min]
        else if (string[position] == '[' && string[position + 1] == 'm') {

            // zistim kolko riadkov a stlpcov mam prehladat, + 1 pretoze ak mam od prveho po treti riadok tak je to 3 ale 3-1=2
            int pocet_riadkov = aktualna_selekcia->selection[2] - aktualna_selekcia->selection[0] + 1;

            //  zistim kolko stlpcov mam prehladat
            int pocet_stlpcov = aktualna_selekcia->selection[3] - aktualna_selekcia->selection[1] + 1;

            // ak chcem zisti max/min z vyberu kde je jedna bunka, selekcia ostane na nej, nic sa nestane

            // ak potrebujem najst min/max z vyberu kde je viac buniek
            if (pocet_riadkov != 1 && pocet_stlpcov != 1) {

                int min_riadok = aktualna_selekcia->selection[0] - 1;
                int min_bunka = aktualna_selekcia->selection[1] - 1;

                int max_riadok = aktualna_selekcia->selection[2] - 1;
                int max_bunka = aktualna_selekcia->selection[3] - 1;


                double min_number;
                double max_number;
                bool found_max = false;
                bool found_min = false;
                bool found_number = false;

                // ak mam funkciu max tak hladam najvacsiu hodnotu v selekcii buniek

                if (string[position + 2] == 'a') {

                    for (int row = min_riadok; row <= max_riadok; row++){
                        for (int col = min_bunka; col <= max_bunka; col++) {

                            // reset found_max aby nezapisal mensie cislo ako je max_number
                            found_max = false;

                            char tmp_string[MAX_ARGUMENT_LENGTH] = "";

                            for (int znak = 0; znak < tabulka->zoznam_riadkov[row].zoznam_buniek[col].dlzka_obsahu; znak++) {
                                tmp_string[znak] = (char)tabulka->zoznam_riadkov[row].zoznam_buniek[col].obsah[znak];
                            }

                            // uzavretie stringu
                            tmp_string[tabulka->zoznam_riadkov[row].zoznam_buniek[col].dlzka_obsahu + 1] = '\0';

                            // vytvorenie miesta na string ak by v bunke nebolo iba cislo napr. '3.14abc'
                            char endptr[MAX_ARGUMENT_LENGTH] = "";

                            // pointer na prvy znak v endptr kvoli funkcii strtod
                            char *p = endptr;

                            // inicializacia aktualneho cisla
                            double curr_number = 0.0;

                            curr_number = strtod(tmp_string, &p);

                            // ak bolo v bunke iba platne cislo
                            if (*p == '\0') {

                                if (!found_number) {
                                    max_number = curr_number;
                                    found_number = true;
                                    found_max = true;

                                // kontrola ci je najdene cislo vacsie ako doterajsie max_number
                                }
                                if (curr_number > max_number) {

                                    found_max = true;

                                    // zmena max_number na current_number
                                    max_number = curr_number;
                                }
                            }

                            // ak nasiel aspon jedno cislo tak ulozi
                            // ak uz predtym nasiel cislo ale teraz nasiel vacsie, ulozi vacsie
                            if (found_max) {

                                aktualna_selekcia->max = true;

                                // + 1 kvoli indexovaniu
                                aktualna_selekcia->selection[0] = row + 1;
                                aktualna_selekcia->selection[2] = row + 1;
                                aktualna_selekcia->selection[1] = col + 1;
                                aktualna_selekcia->selection[3] = col + 1;
                                aktualna_selekcia->selected_value = curr_number;

                            }

                        }
                    }

                }

                if (string[position + 2] == 'i') {
                    for (int row = min_riadok; row <= max_riadok; row++){
                        for (int col = min_bunka; col <= max_bunka; col++) {
                            char tmp_string[MAX_ARGUMENT_LENGTH] = "";

                            for (int znak = 0; znak < tabulka->zoznam_riadkov[row].zoznam_buniek[col].dlzka_obsahu; znak++) {
                                tmp_string[znak] = (char)tabulka->zoznam_riadkov[row].zoznam_buniek[col].obsah[znak];
                            }

                            // uzavretie stringu
                            tmp_string[tabulka->zoznam_riadkov[row].zoznam_buniek[col].dlzka_obsahu + 1] = '\0';

                            // vytvorenie miesta na string ak by v bunke nebolo iba cislo napr. '3.14abc'
                            char endptr[MAX_ARGUMENT_LENGTH] = "";

                            char *p = endptr;

                            double curr_number = 0.0;

                            curr_number = strtod(tmp_string, &p);

                            // ak bolo v bunke iba platne cislo
                            if (*p == '\0') {

                                if (!found_number) {
                                    min_number = curr_number;
                                    found_number = true;
                                    found_min = true;

                                    // kontrola ci je najdene cislo vacsie ako doterajsie max_number
                                }

                                if (curr_number < min_number) {

                                    found_min = true;

                                    // zmena max_number na current_number
                                    min_number = curr_number;
                                }
                            }

                            // ak nasiel aspon jedno cislo tak ulozi

                            if (found_min) {
                                aktualna_selekcia->min = true;
                                aktualna_selekcia->selection[0] = row;
                                aktualna_selekcia->selection[2] = row;
                                aktualna_selekcia->selection[1] = col;
                                aktualna_selekcia->selection[3] = col;
                                aktualna_selekcia->selected_value = curr_number;
                            }
                        }
                    }
                }
            } else {
                position++;
            }
        }

        // [find STR]
        else if (string[position] == '[' && string[position + 1] == 'f') {


            //na tejto pozicii zacina retazec STR
            position_of_next_bracket = position + 6;

            // najdem prvu neescapenutu ']'
            while (string[position_of_next_bracket] != ']' || string[position_of_next_bracket - 1] == 92) {
                position_of_next_bracket++;
                if (string[position_of_next_bracket] == ']' && string[position_of_next_bracket - 1] !=  '\\') {
                    break;
                }
            }

            char tmp_string[MAX_ARGUMENT_LENGTH] = "";
            for (int i = position + 6, j = 0; i < position_of_next_bracket; i++, j++) {
                tmp_string[j] = string[i];
            }
            tmp_string[position_of_next_bracket] = '\0';position++;


            // zistim kolko riadkov a stlpcov mam prehladat, + 1 pretoze ak mam od prveho po treti riadok tak je to 3 ale 3-1=2
            int pocet_riadkov = aktualna_selekcia->selection[2] - aktualna_selekcia->selection[0] + 1;

            //  zistim kolko stlpcov mam prehladat
            int pocet_stlpcov = aktualna_selekcia->selection[3] - aktualna_selekcia->selection[1] + 1;

            // ak mam v selekcii iba jednu bunku
            if (pocet_riadkov == 1 && pocet_stlpcov == 1) {
                char string_v_bunke[MAX_ARGUMENT_LENGTH] = "";

                // prekopirovanie bunky do prechodneho stringu na porovnanie
                for (int x = 0; x < tabulka->zoznam_riadkov[aktualna_selekcia->selection[0] - 1].zoznam_buniek[aktualna_selekcia->selection[1] - 1].dlzka_obsahu; x++) {
                    string_v_bunke[x] = (char)tabulka->zoznam_riadkov[aktualna_selekcia->selection[0] - 1].zoznam_buniek[aktualna_selekcia->selection[1] - 1].obsah[x];
                }

                // uzavretie stringu
                string_v_bunke[tabulka->zoznam_riadkov[aktualna_selekcia->selection[0] - 1].zoznam_buniek[aktualna_selekcia->selection[1] - 1].dlzka_obsahu] = '\0';

                // ak je string v argumente [find STR] zhodny so stringom v bunke
                if (strstr(tmp_string, string_v_bunke)) {
                    aktualna_selekcia->find = true;
                    char *p;
                    p = malloc((strlen(string_v_bunke)+1) * sizeof(char));
                    if (p != NULL) {
                        aktualna_selekcia->find_str = p;

                        for (int x = 0; x < tabulka->zoznam_riadkov[aktualna_selekcia->selection[0] - 1].zoznam_buniek[aktualna_selekcia->selection[1] - 1].dlzka_obsahu; x++) {
                            aktualna_selekcia->find_str[x] = (char)tabulka->zoznam_riadkov[aktualna_selekcia->selection[0] - 1].zoznam_buniek[aktualna_selekcia->selection[1] - 1].obsah[x];
                        }
                    }
                }

            } else {

                // bool aby som ulozil iba prvu bunku s vyskytom stringu vo [find STR]
                bool found_string = false;

                // ulozenie rozsahu ktory budem prehladavat
                int min_riadok = aktualna_selekcia->selection[0] - 1;
                int min_bunka = aktualna_selekcia->selection[1] - 1;

                int max_riadok = aktualna_selekcia->selection[2] - 1;
                int max_bunka = aktualna_selekcia->selection[3] - 1;

                for (int row = min_riadok; row <= max_riadok; row++){
                    for (int col = min_bunka; col <= max_bunka; col++) {
                        if (!found_string) {

                            // vytvorenie prechodneho stringu
                            char string_v_bunke[MAX_ARGUMENT_LENGTH] = "";

                            for (int znak = 0; znak < tabulka->zoznam_riadkov[row].zoznam_buniek[col].dlzka_obsahu; znak++) {
                                string_v_bunke[znak] = (char)tabulka->zoznam_riadkov[row].zoznam_buniek[col].obsah[znak];
                            }

                            // hlada substringy, vrati NULL ak ho v danom stringu nenaslo
                            if(strstr(string_v_bunke, tmp_string) != NULL) {
                                found_string = true;

                                aktualna_selekcia->find = true;
                                char *p;
                                p = malloc((strlen(string_v_bunke) + 1) * sizeof(char));
                                if (p != NULL) {
                                    aktualna_selekcia->find_str = p;

                                    // skopirovanie stringu to mallocovaneho miesta v aktualnej selekcii
                                    for (int x = 0; x < tabulka->zoznam_riadkov[aktualna_selekcia->selection[row]].zoznam_buniek[aktualna_selekcia->selection[col]].dlzka_obsahu; x++) {
                                        aktualna_selekcia->find_str[x] = (char) tabulka->zoznam_riadkov[aktualna_selekcia->selection[row]].zoznam_buniek[aktualna_selekcia->selection[col]].obsah[x];
                                    }
                                    // uzavretie stringu v aktualnej selekcii
                                    aktualna_selekcia->find_str[tabulka->zoznam_riadkov[aktualna_selekcia->selection[row]].zoznam_buniek[aktualna_selekcia->selection[col]].dlzka_obsahu] = '\0';
                                }
                            }
                        }
                    }
                }
            }
            // nastavenie pozicie na najblizsi dalsi oddelovac
            position = position_of_next_bracket+1;

        }

        // irow
        else if (string[position] == 'i' && string[position + 1] == 'r') {
            if (verify_command(string, &position, "irow")) {
                if(!irow(tabulka, aktualna_selekcia)) return false;
            }
        }

        // arow
        else if (string[position] == 'a' && string[position + 1] == 'r') {
            if (verify_command(string, &position, "arow")) {
                if(!arow(tabulka, aktualna_selekcia)) return false;
            }
        }

        // drow
        else if (string[position] == 'd' && string[position + 1] == 'r') {
            if (verify_command(string, &position, "drow")) {
                if(!drow(tabulka, aktualna_selekcia)) return false;
            }
        }

        // icol
        else if (string[position] == 'i' && string[position + 1] == 'c') {
            if (verify_command(string, &position, "icol")) {
                if(!icol(tabulka, aktualna_selekcia)) return false;
            }
        }

        // acol
        else if (string[position] == 'a' && string[position + 1] == 'c') {
            if (verify_command(string, &position, "acol")) {
                if(!acol(tabulka, aktualna_selekcia)) return false;
            }
        }

        // dcol
        else if (string[position] == 'd' && string[position + 1] == 'c') {
            if (verify_command(string, &position, "dcol")) {
                if(!dcol(tabulka, aktualna_selekcia)) return false;
            }
        }

        // set STR
        else if (string[position] == 's' && string[position + 1] == 'e' && string[position + 2] == 't') {

            int position_of_string_end = position + 4;

            char string_param[1000] = "";

            int pos_in_str_param = 0;

            // najdem prvu neescapenutu ';'

            while(string[position_of_string_end]) {

                if (string[position_of_string_end] == '\0') {
                    break;
                } else

                if (string[position_of_string_end] == ';' && string[position_of_string_end - 1] != '\\') {
                    break;
                } else {
                    string_param[pos_in_str_param] = string[position_of_string_end];
                    pos_in_str_param++;
                    position_of_string_end++;
                }

            }
            string_param[pos_in_str_param + 1] = '\0';
            set_str(tabulka, aktualna_selekcia, string_param);

            position = position_of_string_end + 1;
        }

        // clear
        else if (string[position] == 'c' && string[position + 1] == 'l' && string[position + 2] == 'e' &&
                string[position + 3] == 'a' && string[position + 4] == 'r') {

            if (!clear(tabulka, aktualna_selekcia)) return false;

            position = position + 5;
        }

        //swap
        else if (string[position] == 's' && string[position + 1] == 'w' && string[position + 2] == 'a' &&
                string[position + 3] == 'p') {
            int param1, param2;

            if (!extract_params(string, &position, 6, &param1, &param2)) return false;
            if (!swap(tabulka, aktualna_selekcia, param1, param2)) return false;
        }

        else if (string[position] == 's' && string[position + 1] == 'u' && string[position + 2] == 'm') {
            int param1, param2;

            if (!extract_params(string, &position, 5, &param1, &param2)) return false;
            if (!sum(tabulka, aktualna_selekcia, param1, param2)) return false;

        }

        else if (string[position] == 'a' && string[position + 1] == 'v' && string[position + 2] == 'g') {
            int param1, param2;

            if (!extract_params(string, &position, 5, &param1, &param2)) return false;
            if (!avg(tabulka, aktualna_selekcia, param1, param2)) return false;
        }

        else if (string[position] == 'c' && string[position + 1] == 'o' && string[position + 2] == 'u' &&
        string[position + 3] == 'n' && string[position + 4] == 't') {
            int param1, param2;

            if (!extract_params(string, &position, 7, &param1, &param2)) return false;

            if (!count(tabulka, aktualna_selekcia, param1, param2)) return false;
        }

        else if (string[position] == 'd' && string[position + 1] == 'e' && string[position + 2] == 'f') {
            if (!def_var(tabulka, aktualna_selekcia, list_of_variables, &position, string)) return false;
        }

        else if (string[position] == 'u' && string[position + 1] == 's' && string[position + 2] == 'e') {
            if (!use_var(tabulka, aktualna_selekcia, list_of_variables, &position, string)) return false;
        }

        else if (string[position] == 'i' && string[position + 1] == 'n' && string[position + 2] == 'c') {
            if (!inc_var(aktualna_selekcia, list_of_variables, &position, string)) return false;
        }

        else if (string[position] == '[' && string[position + 1] == 's' && string[position + 2] == 'e' && string[position + 3] == 't'
                && string[position + 4] == 't' && string[position + 5] == ']') {
            if (!set_var(aktualna_selekcia, list_of_variables, &position)) return false;
        }

        else if (string[position] == '[' && string[position + 1] == '_' && string[position + 2] == ']') {
            if (!set_var_use(aktualna_selekcia, list_of_variables, &position)) return false;
        }

        else {
            position++;
        }
    }
    dealloc_selection(aktualna_selekcia);
    free(aktualna_selekcia);
    return true;

}

bool irow(Table *tabulka, Selection *aktualna_selekcia) {
    // funkcia ktora vlozi jeden prazdny riadok pred selekciu buniek

    // ak je selekcia vacsia ako rozmery tabulky, doplni sa
    refill_table(tabulka, aktualna_selekcia);

    // row pred ktory budem davat prazdny riadok
    // je to vzdy pred nultym indexm v selection
    int row = aktualna_selekcia->selection[0];

    // pocet buniek ktore musim vlozit do prazdneho stlpca
    int pocet_buniek_v_stlpcoch = tabulka->zoznam_riadkov[0].pocet_buniek;

    // pocet riadkov v tabulke
    int pocet_riadkov_v_tabulke = tabulka->pocet_riadkov;

    // inkrementacia poctu riadkov v celej tabulke
    tabulka->pocet_riadkov++;

    // zvacsenie aktualneho zoznamu riadkov na novy pocet riadkov
    Row *newptr = realloc(tabulka->zoznam_riadkov, sizeof(Row) * tabulka->pocet_riadkov);

    // kontrola nuloveho pointra
    if (newptr == NULL) return false;

    // priradenie nenuloveho pointra
    tabulka->zoznam_riadkov = newptr;

    // poposuvanie pointrov +1 od riadka pred ktory vkladam prazdny riadok
    //
    for (int riadok = pocet_riadkov_v_tabulke - 1;riadok > row - 2; riadok--) {
        tabulka->zoznam_riadkov[riadok + 1] = tabulka->zoznam_riadkov[riadok];
    }

    // alokacia pamate na pocet buniek ako v ostatnych riadkoch vo zvysku tabulky
    Cell *new_cells = malloc(sizeof(Cell) * pocet_buniek_v_stlpcoch);

    // kontrola nuloveho pointra
    if (new_cells == NULL) return false;

    // priradenie nenuloveho pointra
    tabulka->zoznam_riadkov[row - 1].zoznam_buniek = new_cells;

    // alokacia pamate na obsah buniek v pridanom riadku
    // nastavenie dlzky ich obsahu na 1 (\0)
    for (int bunka = 0;bunka < pocet_buniek_v_stlpcoch; bunka++) {

        int *new_obsah = malloc(sizeof(int));

        if (new_obsah == NULL) return false;
        *new_obsah='\0';
        tabulka->zoznam_riadkov[row-1].zoznam_buniek[bunka].obsah = new_obsah;
        tabulka->zoznam_riadkov[row-1].zoznam_buniek[bunka].dlzka_obsahu = 1;
    }

    // true, vsetko sa podarilo
    return true;
}

bool arow(Table *tabulka, Selection *aktualna_selekcia) {
    // funkcia ktora vlozi jeden prazdny riadok za selekciu buniek

    // ak je selekcia vacsia ako rozmery tabulky, doplni sa
    refill_table(tabulka, aktualna_selekcia);

    // row za ktory budem davat prazdny riadok
    // je to vzdy pred nultym indexm v selection
    int row = aktualna_selekcia->selection[2];

    // pocet buniek ktore musim vlozit do prazdneho stlpca
    int pocet_buniek_v_stlpcoch = tabulka->zoznam_riadkov[0].pocet_buniek;

    // pocet buniek v kazdom stlpci
    int pocet_riadkov_v_tabulke = tabulka->pocet_riadkov;

    // zvacsenie aktualneho zoznamu riadkov, +1 lebo jeden pridavam
    Row *newptr = realloc(tabulka->zoznam_riadkov, sizeof(Row) * (pocet_riadkov_v_tabulke + 1));

    // inkrementacia poctu riadkov v celej tabulke
    tabulka->pocet_riadkov++;

    // kontrola nuloveho pointra
    if (newptr == NULL) return false;

    // priradenie nenuloveho pointra
    tabulka->zoznam_riadkov = newptr;

    // poposuvanie pointrov +1 od riadka pred ktory vkladam prazdny riadok
    for (int riadok = pocet_riadkov_v_tabulke - 1; riadok > row - 1 ; riadok--) {
        tabulka->zoznam_riadkov[riadok + 1] = tabulka->zoznam_riadkov[riadok];
    }

    // alokacia pamate na pocet buniek ako v ostatnych riadkoch vo zvysku tabulky
    Cell *new_cells = malloc(sizeof(Cell) * pocet_buniek_v_stlpcoch);

    // kontrola nuloveho pointra
    if (new_cells == NULL) return false;

    // priradenie nenuloveho pointra
    tabulka->zoznam_riadkov[row].zoznam_buniek = new_cells;

    // alokacia pamate na obsah buniek v pridanom riadku
    // nastavenie dlzky ich obsahu na 1 (\0)
    for (int bunka = 0 ; bunka < pocet_buniek_v_stlpcoch ; bunka++) {

        int *new_obsah = malloc(sizeof(int));

        if (new_obsah == NULL) return false;

        tabulka->zoznam_riadkov[row].zoznam_buniek[bunka].obsah = new_obsah;
        tabulka->zoznam_riadkov[row].zoznam_buniek[bunka].dlzka_obsahu = 1;
    }

    // true, vsetko sa podarilo
    return true;
}

bool drow(Table *tabulka, Selection *aktualna_selekcia) {
    // funkcia ktora odstrani riadky vo vybranej selekcii

    // ak je selekcia vacsia ako rozmery tabulky, doplni sa
    refill_table(tabulka, aktualna_selekcia);

    // prvy riadok na zmazanie
    int row_from = aktualna_selekcia->selection[0];

    // posledny riadok na zmazanie
    int row_to = aktualna_selekcia->selection[2];

    // pocet buniek v kazdom riadku
    int pocet_buniek_v_riadku = tabulka->zoznam_riadkov->pocet_buniek;

    // pocet riadkov na zmazanie
    int n_of_row_to_delete = row_to - row_from + 1;

    // freeovanie obsahov buniek,  v riadkoch na zmazanie
    for (int riadok = row_from - 1 ; riadok < row_to ; riadok++) {
        for (int bunka = 0; bunka < pocet_buniek_v_riadku; bunka++) {
            free(tabulka->zoznam_riadkov[riadok].zoznam_buniek[bunka].obsah);
        }
        free(tabulka->zoznam_riadkov[riadok].zoznam_buniek);
    }


    for (int nova_pozicia = row_from - 1, stara_pozicia = row_to; stara_pozicia < tabulka->pocet_riadkov; nova_pozicia++, stara_pozicia++) {
        tabulka->zoznam_riadkov[nova_pozicia] = tabulka->zoznam_riadkov[stara_pozicia];
    }

    // dekrementacia na novy pocet riadkov
    tabulka->pocet_riadkov = tabulka->pocet_riadkov - n_of_row_to_delete;

    // nadze sigabrt pri inpute [2,1];irow;[1,1,1,1];drow
    Row *nove_riadky = realloc(tabulka->zoznam_riadkov, sizeof(Row) * tabulka->pocet_riadkov - 1);

    if (nove_riadky == NULL) return false;

    tabulka->zoznam_riadkov = nove_riadky;

    // true, vsetko sa podarilo
    return true;
}

bool icol(Table *tabulka, Selection *aktualna_selekcia) {
    // funkcia vlozi stlpec pred selekciu

    // ak je selekcia vacsia ako rozmery tabulky, doplni sa
    refill_table(tabulka, aktualna_selekcia);

    int insert_before = aktualna_selekcia->selection[1];

    // new number of cells in each row
    int new_n_of_cols = tabulka->zoznam_riadkov->pocet_buniek + 1;


    for (int riadok = 0; riadok < tabulka->pocet_riadkov; riadok++) {

        // inkrementacia poctu buniek v riadku
        tabulka->zoznam_riadkov[riadok].pocet_buniek = new_n_of_cols;

        // alokacia pamate na novu bunku
        Cell *new_cells = realloc(tabulka->zoznam_riadkov[riadok].zoznam_buniek, sizeof(Cell) * new_n_of_cols);

        // overenie nenuloveho pointra
        if (new_cells == NULL) return NULL;

        // priradenie novej pamate na dany riaodk
        tabulka->zoznam_riadkov[riadok].zoznam_buniek = new_cells;

        // presun buniek na nove pozicie
        for (int nova_pozicia = new_n_of_cols - 1, stara_pozicia = new_n_of_cols - 2; stara_pozicia >= insert_before - 1; nova_pozicia--, stara_pozicia--) {
            tabulka->zoznam_riadkov[riadok].zoznam_buniek[nova_pozicia] = tabulka->zoznam_riadkov[riadok].zoznam_buniek[stara_pozicia];
        }

        // alokacia pamate na novu bunku
        int *new_content = malloc(sizeof(int));

        // overenie nenuloveho pointra novej bunky
        if (new_content == NULL) return NULL;

        *new_content = '\0';

        // priradenie obsahu novej bunky
        tabulka->zoznam_riadkov[riadok].zoznam_buniek[insert_before - 1].obsah = new_content;
        tabulka->zoznam_riadkov[riadok].zoznam_buniek[insert_before - 1].dlzka_obsahu = 1;
    }

    //true, vsetko sa podarilo
    return true;
}

bool acol(Table *tabulka, Selection *aktualna_selekcia) {
    // function appends an empty row after selection

    // ak je selekcia vacsia ako rozmery tabulky, doplni sa
    refill_table(tabulka, aktualna_selekcia);

    int insert_after = aktualna_selekcia->selection[3];

    // new number of cells in each row
    int new_n_of_cols = tabulka->zoznam_riadkov->pocet_buniek + 1;


    for (int riadok = 0; riadok < tabulka->pocet_riadkov; riadok++) {

        // inkrementacia poctu buniek v riadku
        tabulka->zoznam_riadkov[riadok].pocet_buniek = new_n_of_cols;

        // alokacia pamate na novu bunku
        Cell *new_cells = realloc(tabulka->zoznam_riadkov[riadok].zoznam_buniek, sizeof(Cell) * new_n_of_cols);

        // overenie nenuloveho pointra
        if (new_cells == NULL) return NULL;

        // priradenie novej pamate na dany riaodk
        tabulka->zoznam_riadkov[riadok].zoznam_buniek = new_cells;

        // presun buniek na nove pozicie
        for (int nova_pozicia = new_n_of_cols - 1, stara_pozicia = new_n_of_cols - 2; stara_pozicia >= insert_after; nova_pozicia--, stara_pozicia--) {
            tabulka->zoznam_riadkov[riadok].zoznam_buniek[nova_pozicia] = tabulka->zoznam_riadkov[riadok].zoznam_buniek[stara_pozicia];
        }

        // alokacia pamate na novu bunku
        int *new_content = malloc(sizeof(int));

        // overenie nenuloveho pointra novej bunky
        if (new_content == NULL) return NULL;

        *new_content = '\0';

        // priradenie obsahu novej bunky
        tabulka->zoznam_riadkov[riadok].zoznam_buniek[insert_after].obsah = new_content;
        tabulka->zoznam_riadkov[riadok].zoznam_buniek[insert_after].dlzka_obsahu = 1;
    }

    //true, vsetko sa podarilo
    return true;
}

bool dcol(Table *tabulka, Selection *aktualna_selekcia) {
    // function that deletes columns in selection

    // ak je selekcia vacsia ako rozmery tabulky, doplni sa
    refill_table(tabulka, aktualna_selekcia);

    int delete_from = aktualna_selekcia->selection[1];

    int delete_to = aktualna_selekcia->selection[3];

    int n_of_cols_to_delete = delete_to - delete_from + 1;

    // new number of cells in each row
    int new_n_of_cols = tabulka->zoznam_riadkov->pocet_buniek - n_of_cols_to_delete;


    for (int riadok = 0; riadok < tabulka->pocet_riadkov; riadok++) {

        // inkrementacia poctu buniek v riadku
        tabulka->zoznam_riadkov[riadok].pocet_buniek = new_n_of_cols;

        for (int bunka = delete_from - 1; bunka < delete_to; bunka++) {
            free(tabulka->zoznam_riadkov[riadok].zoznam_buniek[bunka].obsah);
        }


        // presun buniek na nove pozicie
        for (int nova_pozicia = delete_from - 1, stara_pozicia = delete_to; stara_pozicia <= new_n_of_cols + 1; nova_pozicia++, stara_pozicia++) {
            tabulka->zoznam_riadkov[riadok].zoznam_buniek[nova_pozicia] = tabulka->zoznam_riadkov[riadok].zoznam_buniek[stara_pozicia];
        }

        // zmensenie pamate na novy pocet buniek
        Cell *new_row = realloc(tabulka->zoznam_riadkov[riadok].zoznam_buniek, sizeof(Cell) * new_n_of_cols);

        // might delete later
        tabulka->zoznam_riadkov[riadok].zoznam_buniek = new_row;

    }

    //true, vsetko sa podarilo
    return true;
}

bool set_str (Table *tabulka, Selection *aktualna_selekcia, char string_param[1000]) {
    // function that copies STR parameter into every cell in selection

    // ak je selekcia vacsia ako rozmery tabulky, doplni sa
    refill_table(tabulka, aktualna_selekcia);

    // ulozenie selekcie pre lepsiu pracu
    int row_from = aktualna_selekcia->selection[0];
    int row_to = aktualna_selekcia->selection[2];

    int col_from = aktualna_selekcia->selection[1];
    int col_to = aktualna_selekcia->selection[3];

    // dlzka zadaneho string parametra
    int length_of_str_param = (int)strlen(string_param);

    // pre danu selekciu
    for (int riadok = row_from - 1; riadok < row_to; riadok++) {
        for (int bunka = col_from - 1; bunka < col_to; bunka++) {

            // ak nema string rovnaku dlzku ako obsah tabulky
            if (tabulka->zoznam_riadkov[riadok].zoznam_buniek[bunka].dlzka_obsahu != length_of_str_param) {

                // realokacia novej pamate
                int *new_content;
                new_content = realloc(tabulka->zoznam_riadkov[riadok].zoznam_buniek[bunka].obsah, sizeof(int) * length_of_str_param);

                // overenie nenuloveho pointra
                if (new_content == NULL) return false;

                tabulka->zoznam_riadkov[riadok].zoznam_buniek[bunka].obsah = new_content;

                // ulozenie novej dlzky premennej
                tabulka->zoznam_riadkov[riadok].zoznam_buniek[bunka].dlzka_obsahu = length_of_str_param;
            }
            // prekopirovanie string parametra do bunky
            for (int i = 0; i < length_of_str_param; i++) {
                tabulka->zoznam_riadkov[riadok].zoznam_buniek[bunka].obsah[i] = (int)string_param[i];
            }
        }
    }
    return true;
}

bool clear(Table *tabulka, Selection *aktualna_selekcia) {
    // function that clear the contents of cells in selection

    // ak je selekcia vacsia ako rozmery tabulky, doplni sa
    refill_table(tabulka, aktualna_selekcia);

    // ulozenie selekcie pre lepsiu pracu
    int row_from = aktualna_selekcia->selection[0];
    int row_to = aktualna_selekcia->selection[2];

    int col_from = aktualna_selekcia->selection[1];
    int col_to = aktualna_selekcia->selection[3];


    // pre danu selekciu
    for (int riadok = row_from - 1; riadok < row_to; riadok++) {
        for (int bunka = col_from - 1; bunka < col_to; bunka++) {
            tabulka->zoznam_riadkov[riadok].zoznam_buniek[bunka].dlzka_obsahu = 1;

            int *new_content;
            new_content = realloc(tabulka->zoznam_riadkov[riadok].zoznam_buniek[bunka].obsah, sizeof(int));
            if (new_content == NULL) return false;
            *new_content = '\0';
            tabulka->zoznam_riadkov[riadok].zoznam_buniek[bunka].obsah = new_content;
        }
    }
    return true;
}

bool swap(Table *tabulka, Selection *aktualna_selekcia, int row, int col) {
    // function that swaps a SINGLE cell in selection with the one set byt parameters swap [R,C]

    // ak je selekcia vacsia ako rozmery tabulky, doplni sa
    refill_table(tabulka, aktualna_selekcia);

    int row_from = aktualna_selekcia->selection[0];
    int row_to = aktualna_selekcia->selection[2];

    int col_from = aktualna_selekcia->selection[1];
    int col_to = aktualna_selekcia->selection[3];


    for (int riadok = row_from - 1; riadok < row_to; riadok++) {
        for (int bunka = col_from - 1; bunka < col_to; bunka++) {
            swap_cells(tabulka, riadok, bunka, row-1, col-1);
        }
    }

    return true;
}

bool sum(Table *tabulka, Selection *aktualna_selekcia, int row, int col) {
    // function that save the sum of valid number cells in selection into cell provided by parameters sum [R,C]

    // if the selection is bigger that the dimensions of table
    refill_table(tabulka, aktualna_selekcia);

    int row_from = aktualna_selekcia->selection[0];
    int row_to = aktualna_selekcia->selection[2];

    int col_from = aktualna_selekcia->selection[1];
    int col_to = aktualna_selekcia->selection[3];

    double total_sum = 0.0;
    char tmp_string[1000] = "";

    for (int riadok = row_from - 1; riadok < row_to; riadok++) {
        for (int bunka = col_from - 1; bunka < col_to; bunka++) {

            char endptr[1000] = "";

            char *p = endptr;

            double curr_number = 0.0;

            int position_in_str = 0;

            for (int znak = 0; znak < tabulka->zoznam_riadkov[riadok].zoznam_buniek[bunka].dlzka_obsahu; znak++) {
                tmp_string[znak] = (char)tabulka->zoznam_riadkov[riadok].zoznam_buniek[bunka].obsah[znak];
                position_in_str++;
            }

            tmp_string[position_in_str] = '\0';

            curr_number = (double)strtold(tmp_string, &p);

            if (*p == '\0') {
                total_sum = total_sum + curr_number;
            }

        }
    }

    char buffer[MAX_ARGUMENT_LENGTH];

    int output_int;

    output_int = snprintf(buffer, MAX_ARGUMENT_LENGTH, "%g", total_sum);

    if (output_int >= 0 && output_int < MAX_ARGUMENT_LENGTH) {

        int *new_content;
        new_content = realloc(tabulka->zoznam_riadkov[row - 1].zoznam_buniek[col - 1].obsah, sizeof(int) * strlen(buffer));
        if (new_content == NULL) return false;

        tabulka->zoznam_riadkov[row - 1].zoznam_buniek[col - 1].obsah = new_content;


        for (int i = 0, length = (int)strlen(buffer); i < length; i++) {
            tabulka->zoznam_riadkov[row - 1].zoznam_buniek[col - 1].obsah[i] = (int)buffer[i];
        }

        tabulka->zoznam_riadkov[row - 1].zoznam_buniek[col - 1].dlzka_obsahu = (int)strlen(buffer);

    }
    return true;
}

bool avg(Table *tabulka, Selection *aktualna_selekcia, int row, int col) {
    // function that save the sum of valid number cells in selection into cell provided by parameters sum [R,C]

    // if the selection is bigger that the dimensions of table
    refill_table(tabulka, aktualna_selekcia);

    int row_from = aktualna_selekcia->selection[0];
    int row_to = aktualna_selekcia->selection[2];

    int col_from = aktualna_selekcia->selection[1];
    int col_to = aktualna_selekcia->selection[3];

    double total_sum = 0.0;
    char tmp_string[1000] = "";
    int n_of_valid_cells = 0;

    for (int riadok = row_from - 1; riadok < row_to; riadok++) {
        for (int bunka = col_from - 1; bunka < col_to; bunka++) {

            char endptr[1000] = "";

            char *p = endptr;

            double curr_number = 0.0;

            int position_in_str = 0;

            for (int znak = 0; znak < tabulka->zoznam_riadkov[riadok].zoznam_buniek[bunka].dlzka_obsahu; znak++) {
                tmp_string[znak] = (char)tabulka->zoznam_riadkov[riadok].zoznam_buniek[bunka].obsah[znak];
                position_in_str++;
            }

            tmp_string[position_in_str] = '\0';

            curr_number = (double)strtold(tmp_string, &p);

            if (*p == '\0') {
                total_sum = total_sum + curr_number;
                n_of_valid_cells++;
            }

        }
    }

    char buffer[MAX_ARGUMENT_LENGTH];

    int output_int;

    output_int = snprintf(buffer, MAX_ARGUMENT_LENGTH, "%g", total_sum/n_of_valid_cells);

    if (output_int >= 0 && output_int < 100) {

        int *new_content;
        new_content = realloc(tabulka->zoznam_riadkov[row - 1].zoznam_buniek[col - 1].obsah, sizeof(int) * strlen(buffer));
        if (new_content == NULL) return false;

        tabulka->zoznam_riadkov[row - 1].zoznam_buniek[col - 1].obsah = new_content;


        for (int i = 0, length = (int)strlen(buffer); i < length; i++) {
            tabulka->zoznam_riadkov[row - 1].zoznam_buniek[col - 1].obsah[i] = (int)buffer[i];
        }

        tabulka->zoznam_riadkov[row - 1].zoznam_buniek[col - 1].dlzka_obsahu = (int)strlen(buffer);

    }
    return true;
}

bool count(Table *tabulka, Selection *aktualna_selekcia, int row, int col) {
    // function that save the sum of valid number cells in selection into cell provided by parameters sum [R,C]

    // if the selection is bigger that the dimensions of table
    refill_table(tabulka, aktualna_selekcia);

    int row_from = aktualna_selekcia->selection[0];
    int row_to = aktualna_selekcia->selection[2];

    int col_from = aktualna_selekcia->selection[1];
    int col_to = aktualna_selekcia->selection[3];

    int n_of_valid_cells = 0;

    for (int riadok = row_from - 1; riadok < row_to; riadok++) {
        for (int bunka = col_from - 1; bunka < col_to; bunka++) {

            if (tabulka->zoznam_riadkov[riadok].zoznam_buniek[bunka].dlzka_obsahu == 1) {
                if (tabulka->zoznam_riadkov[riadok].zoznam_buniek[bunka].obsah[0] != '\0') {
                    n_of_valid_cells++;
                }
            } else {
                n_of_valid_cells++;
            }
        }
    }

    char buffer[MAX_ARGUMENT_LENGTH] = "";

    int new_length_of_content = snprintf(buffer, MAX_ARGUMENT_LENGTH, "%d", n_of_valid_cells);

    int *new_content;

    new_content = malloc(sizeof(int) * new_length_of_content);
    if (new_content == NULL) return false;

    free(tabulka->zoznam_riadkov[row - 1].zoznam_buniek[col - 1].obsah);
    tabulka->zoznam_riadkov[row - 1].zoznam_buniek[col - 1].obsah = new_content;
    tabulka->zoznam_riadkov[row - 1].zoznam_buniek[col - 1].dlzka_obsahu = new_length_of_content;

    for (int znak = 0; znak < new_length_of_content; znak++) {
        tabulka->zoznam_riadkov[row - 1].zoznam_buniek[col - 1].obsah[znak] = (int)buffer[znak];
    }



    return true;
}

bool def_var(Table *tabulka, Selection *aktualna_selekcia, Variable *list_of_variables, int *position, const char *string) {
    // funkcia ktora ulozi hodnotu aktualnej jednej bunky v selekcii a ulozi ju do premennej _X

    // ak je selekcia vacsia ako rozmery tabulky, doplni sa
    refill_table(tabulka, aktualna_selekcia);

    // kontrola ci mam v selekcii iba jednu bunku
    if (aktualna_selekcia->selection[0] != aktualna_selekcia->selection[2] || aktualna_selekcia->selection[1] != aktualna_selekcia->selection[3]) return false;

    // premenna ktora bude na indexe za koncom cisla
    int position_of_number = *position + 5;

    char cislo[2] = "";

    cislo[0] = string[position_of_number];
    cislo[1] = '\0';
    char *p;

    long int ret;
    ret = strtol(cislo, &p, 10);

    if (p == NULL) return false;
    if (ret < -1 || ret > 9) return false;

    char tmp_string[100] = "";

    double curr_number = 0.0;

    int length_of_content = tabulka->zoznam_riadkov[aktualna_selekcia->selection[0] - 1].zoznam_buniek[aktualna_selekcia->selection[1] - 1].dlzka_obsahu;

    for (int znak = 0; znak < length_of_content;znak++) {
        tmp_string[znak] = (char)tabulka->zoznam_riadkov[aktualna_selekcia->selection[0] - 1].zoznam_buniek[aktualna_selekcia->selection[1] - 1].obsah[znak];
    }

    curr_number = (double)strtold(tmp_string, &p);

    // obsah je cislo
    if (*p == '\0') {

        // ulozenie cisla do premennej
        list_of_variables[ret - 1].cislo_f = curr_number;

        // nastavenie typu premennej na cislo
        list_of_variables[ret - 1].typ_premennej = 0;
    } else {

        // obsah je string
        char *new_string = malloc(sizeof(char) * (length_of_content + 1));
        if (new_string == NULL) return false;

        // prekopirovanie obsahu bunky v selekcii do variable
        for (int znak = 0; znak < length_of_content;znak++) {
            new_string[znak] = (char)tabulka->zoznam_riadkov[aktualna_selekcia->selection[0] - 1].zoznam_buniek[aktualna_selekcia->selection[1] - 1].obsah[znak];
        }

        new_string[length_of_content] = '\0';

        // ulozenie stringu do premennej
        list_of_variables[ret - 1].string = new_string;

        // nastavenie typu premennej na string
        list_of_variables[ret - 1].typ_premennej = 1;

    }
    *position = *position + 6;
    return true;
}

bool use_var(Table *tabulka, Selection *aktualna_selekcia, Variable *list_of_variables, int *position, const char *string) {
    // funkcia ktora prepise hodnotu jednej bunky v aktualnej

    // ak je selekcia vacsia ako rozmery tabulky, doplni sa
    refill_table(tabulka, aktualna_selekcia);

    int row_from = aktualna_selekcia->selection[0];
    int row_to = aktualna_selekcia->selection[2];

    int col_from = aktualna_selekcia->selection[1];
    int col_to = aktualna_selekcia->selection[3];

    // premenna ktora bude na indexe za koncom cisla
    int position_of_number = *position + 5;

    char cislo[2] = "";

    cislo[0] = string[position_of_number];
    cislo[1] = '\0';
    char *p;

    long int ret;
    ret = strtol(cislo, &p, 10);

    if (p == NULL) return false;
    if (ret < -1 || ret > 9) return false;

    char new_cell_content[MAX_ARGUMENT_LENGTH] = "";
    int dlzka_obsahu = 0;

    if (list_of_variables[ret - 1].typ_premennej == 0) {
        dlzka_obsahu = snprintf(new_cell_content, MAX_ARGUMENT_LENGTH, "%g", (float)list_of_variables[ret - 1].cislo_f);
    } else {
        dlzka_obsahu = (int)strlen(list_of_variables[ret - 1].string);
        strcpy(new_cell_content, list_of_variables[ret - 1].string);
    }
    if (dlzka_obsahu >= 0 && dlzka_obsahu < MAX_ARGUMENT_LENGTH) {
        for (int riadok = row_from - 1; riadok < row_to; riadok++) {
            for (int bunka = col_from - 1; bunka < col_to; bunka++) {

                // ak je dlzka obsahu bunky ina ako dlzka noveho obsahu, reallokujem na potrebnu dlzku
                if (tabulka->zoznam_riadkov[riadok].zoznam_buniek[bunka].dlzka_obsahu != dlzka_obsahu - 1) {

                    int *new_content = realloc(tabulka->zoznam_riadkov[riadok].zoznam_buniek[bunka].obsah, sizeof(int) * (dlzka_obsahu));

                    if (new_content == NULL) return false;

                    tabulka->zoznam_riadkov[riadok].zoznam_buniek[bunka].obsah = new_content;
                }

                for (int znak = 0; znak < dlzka_obsahu; znak++) {
                    tabulka->zoznam_riadkov[riadok].zoznam_buniek[bunka].obsah[znak] = (int)new_cell_content[znak];
                }
                tabulka->zoznam_riadkov[riadok].zoznam_buniek[bunka].dlzka_obsahu = dlzka_obsahu;

            }
        }

    }
    *position = *position + 6;
    return true;
}

bool inc_var(Selection *aktualna_selekcia, Variable *list_of_variables, int *position, const char *string) {
    // funkcia ktora inkrementuje hodnotu v premennej o 1, ak tam nie je ulozene cislo,

    // premenna ktora bude na indexe za koncom cisla
    int position_of_number = *position + 5;

    char cislo[2] = "";

    cislo[0] = string[position_of_number];
    cislo[1] = '\0';
    char *p;

    long int ret;
    ret = strtol(cislo, &p, 10);

    if (p == NULL) return false;
    if (ret < -1 || ret > 9) return false;

    // ak je ulozena premenna cislo
    if (list_of_variables[ret - 1].typ_premennej == 0) {
        list_of_variables[ret - 1].cislo_f = list_of_variables[ret - 1].cislo_f + 1;
    } else if (list_of_variables[ret - 1].typ_premennej == 1){
        free(list_of_variables[ret - 1].string);
        list_of_variables[ret - 1].typ_premennej = 0;
        list_of_variables[ret - 1].cislo_f = 1;
    }else {
        list_of_variables[ret - 1].typ_premennej = 0;
        list_of_variables[ret - 1].cislo_f = 1;
    }

    *position = *position + 6;
    return true;
}

bool set_var(Selection *aktualna_selekcia, Variable *list_of_variables, int *position) {
    // funkcia ktora

    // posledna, 10ta, je urcena iba na ulozenie selekcie
    for (int i = 0; i < 3; i++) {
        list_of_variables[10].selection[i] = aktualna_selekcia->selection[i];
    }

    *position = *position + 5;
    return true;
}

bool set_var_use(Selection *aktualna_selekcia, Variable *list_of_variables, int *position) {
    // funkcia ktora ulozi zvolenu selekciu do premennej

    // posledna, 10ta, je urcena iba na ulozenie selekcie
    for (int i = 0; i < 3; i++) {
        aktualna_selekcia->selection[i] = list_of_variables[10].selection[i];
    }

    *position = *position + 5;
    return true;
}

bool swap_cells (Table *tabulka, int row1, int col1, int row2, int col2) {
    // helper function for swapping of two cells

    // inicializacia pomocnych premennych na swap
    int *tmp_cell_content = tabulka->zoznam_riadkov[row1].zoznam_buniek[col1].obsah;
    int tmp_length_of_content = tabulka->zoznam_riadkov[row1].zoznam_buniek[col1].dlzka_obsahu;

    tabulka->zoznam_riadkov[row1].zoznam_buniek[col1].obsah = tabulka->zoznam_riadkov[row2].zoznam_buniek[col2].obsah;
    tabulka->zoznam_riadkov[row1].zoznam_buniek[col1].dlzka_obsahu = tabulka->zoznam_riadkov[row2].zoznam_buniek[col2].dlzka_obsahu;

    tabulka->zoznam_riadkov[row2].zoznam_buniek[col2].obsah = tmp_cell_content;
    tabulka->zoznam_riadkov[row2].zoznam_buniek[col2].dlzka_obsahu = tmp_length_of_content;

    return true;
}
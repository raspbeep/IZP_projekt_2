#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
//#include <ctype.h>
#define DEF_DELIM ' '
//#define MAX_PARAM_LENGTH 1000
#define MAX_CELL_SIZE 101

//#define INIT_CELL_SIZE 1
//#define INIT_ROW_COUNT 1
//#define END_OF_LINE '\0'

typedef struct bunka {
    int dlzka_obsahu;
    int *obsah;
} Cell;

typedef struct riadok {
    int pocet_buniek;
    Cell *zoznam_buniek;
} Row;

typedef struct tabulka {
    int pocet_riadkov;
    Row *zoznam_riadkov;
} Table;



typedef enum {SCAN_DELIM, AWAIT_DELIM, DONE} DelimMode;
typedef enum {SCAN_DELIM_AND_ARGS, EXIT}RunMode;//, RUN, ALL_DONE, RANGE_ERROR, EMPTY_FILE} RunMode;
//void process_args (int argc, char **argv, const bool *found_delim, int *param1, int *param2, char *string_param, ArgsMode *args_mode, ParamMode *param_mode);

int is_delim (const char *delim, char *delim_string[], const bool *multi_character_delim, int znak);
//void find_delim(int argc, char **argv, char *delim, bool *found_delim, char delim_string[]);
char * save_delim_and_args(int argc, char **argv, char *delim, char delim_string[], RunMode *run_mode, bool *multi_character_delim);
Table * load_table_from_file(char *delim, char *delim_string, bool multi_character_delim);
void print_table (Table *tabulka, char delim);
void dealloc_table (Table *tabulka);
void level_table(Table *tabulka);



int main(int argc, char *argv[]) {

    // neboli zadane ziadne argumenty
    if (argc == 1) return 2;

    // presmerovanie stdin pretoze CLion to nepodporuje ako stdin < na vstupe
    freopen("a.txt","r",stdin);

    // jednoznakovy delim
    char delim;

    // buffer na viacznakovy delim
    char delim_string[MAX_CELL_SIZE] = "";
    // bool ci som nasiel zadany delim


    // bool ked mam viacznakovy
    bool multi_character_delim = false;

    // hlavny stavovy automat
    RunMode run_mode = SCAN_DELIM_AND_ARGS;
    char *string_of_all_params;
    Table *tabulka;

    while (run_mode != EXIT) {
        switch (run_mode) {
            case SCAN_DELIM_AND_ARGS:

                string_of_all_params = save_delim_and_args(argc, argv, &delim, delim_string, &run_mode, &multi_character_delim);

                tabulka = load_table_from_file(&delim, delim_string, multi_character_delim);
                print_table(tabulka, delim);

                run_mode = EXIT;
                free (string_of_all_params);
                break;

            case EXIT :
                break;
        }
    }

    // DEALLOC
    dealloc_table(tabulka);

    return 0;
}

Table *load_table_from_file(char *delim, char *delim_string, bool multi_character_delim) {
    // nahra tabulku zo suboru
    // obsah bunky do Cell, skupinu buniek Cell do Row a vsetky Row do Table

    //Cell bunka = {1, obsah};
    int znak;

    // alokacia pamate pre struct bunka
    Table *tabulka;
    tabulka = malloc(sizeof(Table));

    // dlzka aktualnej bunky
    int dlzka_obsahu = 1;

    // prvotna inicializacia structov
    //vytvori sa zoznam riadkov, prvy riadok, zoznam buniek, bunka a obsah
    tabulka->pocet_riadkov = 1;
    tabulka->zoznam_riadkov = malloc(sizeof(Row) * 1);
    tabulka->zoznam_riadkov->zoznam_buniek = malloc(sizeof(Cell) * 1);
    tabulka->zoznam_riadkov->pocet_buniek = 1;
    tabulka->zoznam_riadkov->zoznam_buniek->dlzka_obsahu = dlzka_obsahu;
    tabulka->zoznam_riadkov->zoznam_buniek->obsah = malloc(sizeof(int));

    // pocet buniek ktore som uz presiel v aktualnom riadku
    int aktualny_pocet_riadkov = 1;

    // pocet buniek ktore som uz presiel v aktualnom riadku
    int aktualny_pocet_buniek = 1;

    // nacitanie prveho znaku
    znak = fgetc(stdin);

    // premenna na quotes mode
    bool in_quotes = false;

    // kym neskapem
    while (znak != EOF) {

        // ak mam delim uzaviem aktualnu bunku a ulozim do aktualneho zoznamu buniek
        if (is_delim(delim, &delim_string, &multi_character_delim, znak) && !in_quotes) {

            // nacitanie noveho znaku
            znak = fgetc(stdin);

            // ak nie je dalsi znak EOF tak si chcem zvacsit miesto na dalsie bunky
            if (znak != '\n' && znak != EOF) {

                // inkrementacia poctu buniek v aktualnom riadku
                aktualny_pocet_buniek++;

                // zvacsenie zoznamu aktualnych buniek
                Cell *newptr = realloc(tabulka->zoznam_riadkov[aktualny_pocet_riadkov-1].zoznam_buniek, sizeof(Cell) * aktualny_pocet_buniek);

                // overenie pointra
                if (newptr == NULL) return NULL;

                // priradenie nenuloveho pointra
                tabulka->zoznam_riadkov[aktualny_pocet_riadkov-1].zoznam_buniek = newptr;

                // intrementracia poctu buniek v aktualnom riadku
                tabulka->zoznam_riadkov[aktualny_pocet_riadkov - 1].pocet_buniek++;

                // alokacia novej pamate na obsah bunky
                tabulka->zoznam_riadkov[aktualny_pocet_riadkov-1].zoznam_buniek[aktualny_pocet_buniek - 1].obsah = malloc(sizeof(int));

                //resetovanie dlzky obsahu novej bunky
                tabulka->zoznam_riadkov[aktualny_pocet_riadkov-1].zoznam_buniek[aktualny_pocet_buniek - 1].dlzka_obsahu = 1;

                //ak je znak doublequote tak sa zapne mod in_quotes a preskoci ich
                if (znak == 34) {
                    in_quotes = true;
                    znak = fgetc(stdin);
                }
                dlzka_obsahu = 1;

            } else {
                if (znak == '\n' ) {

                    // inkrementacia poctu buniek v aktualnom riadku
                    aktualny_pocet_buniek++;

                    Cell *newptr = realloc(tabulka->zoznam_riadkov[aktualny_pocet_riadkov-1].zoznam_buniek, sizeof(Cell) * aktualny_pocet_buniek);
                    if (newptr == NULL) return NULL;
                    tabulka->zoznam_riadkov[aktualny_pocet_riadkov - 1].zoznam_buniek = newptr;
                    tabulka->zoznam_riadkov[aktualny_pocet_riadkov - 1].pocet_buniek++;

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

            // ak je znak backslash tak ho preskoci
            if (znak == 92) {
                znak = fgetc(stdin);

            // ak je znak double quote tak sa vypne z modu in quotes
            } else if (znak == 34) {

                // vypne sa in quotes mode
                in_quotes = false;

                //nacitanie dalsieho znaku lebo quote mozem preskocit
                znak = fgetc(stdin);

                // -1 pretoze v predoslom cykle sa zvacsil ale
                dlzka_obsahu--;
            } else {

                //znak sa prida do obsahu
                tabulka->zoznam_riadkov[aktualny_pocet_riadkov-1].zoznam_buniek[aktualny_pocet_buniek-1].obsah[dlzka_obsahu-1] = znak;

                // zvacsi sa dlzka obsahu v aktualnej bunke
                tabulka->zoznam_riadkov[aktualny_pocet_riadkov-1].zoznam_buniek[aktualny_pocet_buniek-1].dlzka_obsahu = dlzka_obsahu;


                // nacitanie dalsieho znaku
                znak = fgetc(stdin);

                // ak nie je dalsi znak EOF tak vytvorim nove miesto na dalsi znak
                if (znak != EOF && znak != '\n' && !is_delim(delim, &delim_string, &multi_character_delim, znak)) {

                    // inkrementacia dlzky obsahu lebo dalsi znak bude tiez patrit do aktualnej bunky
                    dlzka_obsahu++;

                    // alokacia pamate na dalsi znak obsahu
                    int *new_obsah = realloc(tabulka->zoznam_riadkov[aktualny_pocet_riadkov-1].zoznam_buniek[aktualny_pocet_buniek-1].obsah, sizeof(int) * dlzka_obsahu);

                    // konla nuloveho pointra
                    if (new_obsah == NULL) return NULL;

                    // priradenie nenuloveho pointra
                    tabulka->zoznam_riadkov[aktualny_pocet_riadkov-1].zoznam_buniek[aktualny_pocet_buniek-1].obsah = new_obsah;
                }

            }
        } else {
            // narazil som na koniec riadku, ulozim obsah bunky do Cell

            // nacitanie noveho znaku
            znak = fgetc(stdin);

            if (znak != EOF) {

                // inkrementacia aktualneho poctu riadkov
                aktualny_pocet_riadkov++;

                // zvacsenie aktualneho zoznamu riadkov
                Row *newptr = realloc(tabulka->zoznam_riadkov, sizeof(Row) * aktualny_pocet_riadkov);

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
                Cell *newcell = realloc(tabulka->zoznam_riadkov[aktualny_pocet_riadkov-1].zoznam_buniek, sizeof(Cell) * aktualny_pocet_buniek);

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
            for (int znak = 0; znak < tabulka->zoznam_riadkov[riadok].zoznam_buniek[bunka].dlzka_obsahu;znak++) {
                printf("%c", tabulka->zoznam_riadkov[riadok].zoznam_buniek[bunka].obsah[znak]);
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

char *save_delim_and_args(int argc, char **argv, char *delim, char delim_string[MAX_CELL_SIZE], RunMode *run_mode, bool *multi_character_delim) {
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
        return string_of_all_params;

    } else {
        *delim = DEF_DELIM;
        strcpy(delim_string, argv[1]);

        char *string_of_all_params = malloc(sizeof(char) * (strlen(argv[3]) + 1));
        strcpy(string_of_all_params, argv[3]);
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

void level_table(Table *tabulka) {
    // najde maximalny pocet stlpcov v celej tabulke
    int max_pocet_stlpcov = 1;
    for (int riadok = 0; riadok < tabulka->pocet_riadkov; riadok++) {
        if (tabu) {

        }
    }
}
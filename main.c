#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#define DEF_DELIM ' '
#define MAX_ARGUMENT_LENGTH 1000
#define MAX_DELIM_SIZE 101

// TODO dorobit file input ako posledny argument zo vstupu

typedef enum {IROW, AROW, DROW, ICOL, ACOL, DCOL, SET, CLEAR, SWAP, SUM, AVG, COUNTER, LEN, DEF, USER, } Commands;

typedef enum {SELECTION, ARGUMENT} ParseMode;

// struct premennej, vie v sebe ulozit cislo alebo string
typedef struct premenna {
    int typ_premennej;      // -1 = neurcena, 0 = cislo, 1 = float, 2 = string
    int cislo;              // miesto na ulozenie cisla
    double cislo_f;         // miesto na ulozenie floatu
    char *string;           // miesto na ulozenie stringu
} Variable;

typedef struct vyber_buniek {
    int selection[4];       // vyber na konkretne okno buniek
    int selection_type;     // 0 = selekcia s dvomi parametrami [1,1], 1 = selekcia rozsahu [1,1,1,2]
    bool max;               // indikuje ci je zvoleny aj max parameter
    bool min;               // min parameter
    bool find;              // ci bol zvoleny aj find
    char *find_str;         // ak bol zvoleny find tak potrebuje aj str
    int set_selection[4];   // selekcia buniek pre [set]
    double selected_value;     // hodnota vybrana blizsou selekciou s [max], [min]
} Selection;

typedef struct prikaz {
    Selection vyber_buniek;
    Commands prikaz;        // konkretny prikaz z enumu
    char *set_str;          // strin parameter na argument set STR
    int param1;             // parameter 1
    int param2;             // parameter 2

} Arg;

typedef struct prikazy {
    int pocet;
    Arg *list_of_arguments;
} Arguments;

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
int is_delim (const char *delim, char *delim_string[], const bool *multi_character_delim, int znak);
char * save_delim_and_args(int argc, char **argv, char *delim, char delim_string[], RunMode *run_mode, bool *multi_character_delim);
Table * load_table_from_file(char *delim, char *delim_string, bool multi_character_delim);
void print_table (Table *tabulka, char delim);
void dealloc_table (Table *tabulka);
void level_table(Table *tabulka);
void dealloc_variables(Variable *list);
Variable *init_list_of_variables();
Arguments *parse_arguments (const char *string_of_all_params, Table *tabulka);
int verified_int (const char string[]);
void init_selection(Selection *aktualna_selekcia);

int main(int argc, char *argv[]) {

    // neboli zadane ziadne argumenty
    if (argc == 1) return 2;

    // presmerovanie stdin pretoze CLion to nepodporuje ako stdin < na vstupe
    freopen("a.txt","r",stdin);

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

    Arguments *list_of_arguments;

    while (run_mode != EXIT) {
        switch (run_mode) {
            case SCAN_DELIM_AND_ARGS:

                // ulozenie delimov a string_of_all_params
                string_of_all_params = save_delim_and_args(argc, argv, &delim, delim_string, &run_mode, &multi_character_delim);

                // nacitanie tabulky do trojrozmerneho structov
                tabulka = load_table_from_file(&delim, delim_string, multi_character_delim);

                // dorovnanie tabulky aby bol vsade rovnaky pocet stlpcov
                level_table(tabulka);

                // basic vytlacenie tabulky
                print_table(tabulka, delim);

                parse_arguments(string_of_all_params, tabulka);

                //save_args(argv);

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

    return 0;
}

Table *load_table_from_file(char *delim, char *delim_string, bool multi_character_delim) {
    // nahra tabulku zo suboru

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

                // zvacsenie zoznamu aktualnych buniek
                Cell *newptr = realloc(tabulka->zoznam_riadkov[aktualny_pocet_riadkov-1].zoznam_buniek, sizeof(Cell) * aktualny_pocet_buniek);

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
                    tabulka->zoznam_riadkov[aktualny_pocet_riadkov-1].zoznam_buniek[aktualny_pocet_buniek - 1].obsah = malloc(sizeof(int) * 2);

                    // uvodzovky sa pridaju na zaciatok obsahu bunky
                    tabulka->zoznam_riadkov[aktualny_pocet_riadkov-1].zoznam_buniek[aktualny_pocet_buniek-1].obsah[0] = 34;

                    //resetovanie dlzky obsahu novej bunky
                    tabulka->zoznam_riadkov[aktualny_pocet_riadkov-1].zoznam_buniek[aktualny_pocet_buniek - 1].dlzka_obsahu = 2;

                    dlzka_obsahu = 2;

                }else {

                    // alokacia novej pamate na obsah bunky
                    tabulka->zoznam_riadkov[aktualny_pocet_riadkov-1].zoznam_buniek[aktualny_pocet_buniek - 1].obsah = malloc(sizeof(int));

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
                    Cell *newptr = realloc(tabulka->zoznam_riadkov[aktualny_pocet_riadkov-1].zoznam_buniek, sizeof(Cell) * aktualny_pocet_buniek);
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
                int *new_obsah = realloc(tabulka->zoznam_riadkov[aktualny_pocet_riadkov-1].zoznam_buniek[aktualny_pocet_buniek-1].obsah, sizeof(int) * dlzka_obsahu);

                // konla nuloveho pointra
                if (new_obsah == NULL) return NULL;

                // priradenie nenuloveho pointra
                tabulka->zoznam_riadkov[aktualny_pocet_riadkov-1].zoznam_buniek[aktualny_pocet_buniek-1].obsah = new_obsah;

            } else if (escape_char) {

                // inkrementacia dlzky obsahu lebo dalsi znak bude tiez patrit do aktualnej bunky
                dlzka_obsahu++;

                // alokacia pamate na dalsi znak obsahu
                int *new_obsah = realloc(tabulka->zoznam_riadkov[aktualny_pocet_riadkov-1].zoznam_buniek[aktualny_pocet_buniek-1].obsah, sizeof(int) * dlzka_obsahu);

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

char *save_delim_and_args(int argc, char **argv, char *delim, char delim_string[MAX_DELIM_SIZE], RunMode *run_mode, bool *multi_character_delim) {
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
        strcpy(delim_string, argv[1]);

        char *string_of_all_params = malloc(sizeof(char) * (strlen(argv[3]) + 1));
        strcpy(string_of_all_params, argv[3]);
        string_of_all_params[strlen(argv[3])] = '\0';
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

Variable *init_list_of_variables(){
    Variable *list;
    list = malloc(sizeof(Variable) * 10);

    for (int i = 0; i < 10; i++) {
        list[i].typ_premennej = -1;
        list[i].cislo = 0;
        list[i].cislo_f = 0.0;
        list[i].string = malloc(sizeof(char));
    }

    return list;
}

void dealloc_variables(Variable *list) {
    for (int i = 0; i < 10; i++) {
        free(list[i].string);
    }
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

void init_selection(Selection *aktualna_selekcia) {

    for (int i = 0; i < 4; i++) aktualna_selekcia->selection[i] = 0;
    aktualna_selekcia->selection_type = 0;
    aktualna_selekcia->max = false;
    aktualna_selekcia->min = false;
    aktualna_selekcia->find = false;
    aktualna_selekcia->find_str = NULL;
    for (int i = 0; i < 4; i++) aktualna_selekcia->set_selection[i] = 0;

}

void dealloc_selection(Selection *aktualna_selekcia) {
    free(aktualna_selekcia->find_str);
}

Arguments *parse_arguments(const char *string, Table *tabulka) {
    ParseMode parse_mode;

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


            // chcem hladat ']' hned od dalsej pozicie
            position_of_next_bracket = position + 1;
            int pocet_ciarok = 0;
            int pozicia_ciarok[3] = {0};

            // najdenie pozicie uzatvarajucej ']'
            while(string[position_of_next_bracket] != ']') {

                // ak su po ceste nejakej ',' tak si ulozim ich pocet a poziciu
                if (string[position_of_next_bracket] == ',') {

                    // ulozenie pozicie
                    pozicia_ciarok[pocet_ciarok] = position_of_next_bracket;

                    //ulozenie poctu ciarok
                    pocet_ciarok++;
                }
                position_of_next_bracket++;
            };

            // tmp postition zacina na position
            int tmp_position = position;

            // string do ktoreho ulozim priestor medzi kazdymi dvomi ciarkami
            // stanovena max dlzka zo zadania
            char tmp_string[MAX_ARGUMENT_LENGTH] = "";
            int cele_cislo = 0;

            //cele_cislo = (int)strtol(tmp_string, NULL, 10);

            // kolko cifier ma aktualne cislo
            int dlzka_cisla = 0;

            // ulozenie kazdeho cisla do parametra selekcia structu aktualna_selekcia
            for (int i = 0; i < pocet_ciarok + 1; i++) {

                // zistenie v akom rozsahu sa nachadza dalsie cislo
                if (pocet_ciarok == 1) {

                    if (i==0) {
                        dlzka_cisla = pozicia_ciarok[i] - tmp_position - 1;
                    } else {
                        dlzka_cisla = position_of_next_bracket - tmp_position - 1;
                    }
                // ak mam 3 ciarky a teda 4 parametre
                } else if (pocet_ciarok == 3) {

                    if (i < 3) {

                        // od poslednej ciarky po dalsiu ciarku
                        dlzka_cisla = pozicia_ciarok[i] - tmp_position - 1;
                    } else {

                        // od poslednej ciarky po dalsiu zatvorku ']'
                        dlzka_cisla = position_of_next_bracket - tmp_position - 1;
                    };
                }

                // skopirovanie jedneho cisla do tmp_string
                for (int x = 0; x < dlzka_cisla; x++) {
                    tmp_string[x] = string[tmp_position + 1 + x];
                }
                cele_cislo = (int)strtol(tmp_string, NULL, 10);

                // uzavretie tmp_string na poslendom mieste
                tmp_string[dlzka_cisla] = '\0';

            // ak narazim na parameter bez cisla [_,1]
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

            // overenie ci je zadany korektny parameter
            //if(!verified_int(tmp_string)) return NULL;

            // pretypovanie zo str na int


            if (pocet_ciarok == 1) {

                // ak som na prvom cisle z dvoch
                if (i == 0) {

                    // ulozenie prveho cisla
                    aktualna_selekcia->selection[0] = cele_cislo;

                    // ulozenie prveho cisla
                    aktualna_selekcia->selection[2] = cele_cislo;

                    // nastavenie typu selekcie
                    aktualna_selekcia->selection_type = 0;

                // som na druhom cisle z dvoch
                } else {
                    // nastavenie druheho cisla
                    aktualna_selekcia->selection[1] = cele_cislo;

                    // nastavenie druheho cisla
                    aktualna_selekcia->selection[3] = cele_cislo;
                }

            // ak mam 3 ciarky, teda 4 parametre
                } else {

                    // nastavenie i-teho cisla na aktualny parameter
                    aktualna_selekcia->selection[i] = cele_cislo;
                }
                // posun na poziciu dlasej ciarky
                tmp_position = pozicia_ciarok[i];
            }
            // posuvam sa v hlavnom stringu o 1 lebo chcem znak za najblizsou dvojbodkou
            position = position_of_next_bracket + 1;
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

                int cislo;
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
                                aktualna_selekcia->selection[0] = row;
                                aktualna_selekcia->selection[2] = row;
                                aktualna_selekcia->selection[1] = col;
                                aktualna_selekcia->selection[3] = col;
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
                                    found_max = true;

                                    // kontrola ci je najdene cislo vacsie ako doterajsie max_number
                                }

                                if (curr_number < min_number) {

                                    found_min = true;

                                    // zmena max_number na current_number
                                    min_number = curr_number;
                                }
                            }

                            // ak nasiel aspon jedno cislo tak ulozi

                            if (found_max) {
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
            }
        }

        // ak narazim na [find STR]
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

        // TODO dorobit [_] pre obnovenie vyberu z docasnej premennej

        else if (string[position] == 'i' && string[position + 1] == 'r') {
            int position_of_next_delim = position;
            char command[100] = "";
            int position_in_command = 0;
            while (string[position_of_next_delim] != ';' && string[position_of_next_delim] != '\0') {
                command[position_in_command] = string[position_of_next_delim];
                position_in_command++;
                position_of_next_delim++;
            }
            command[position_of_next_delim] = '\0';

            // overenie ci to je skutocne irow
            if (strstr(command, "irow") != NULL) {
                irow(table);
            }

        }


        else {
            position++;
        }
    }

}

void irow(Table tabulka)

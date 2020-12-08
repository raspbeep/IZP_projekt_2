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

// struct of a variable
typedef struct variable {
    int type_of_var;        // -1 = unindentified, 0 = floar var, 1 = stringvar
    int selection[4];       // selection memory
    double number_f;        // float mem
    char *string;           // string mem
} Variable;

// struct for the actual selection
typedef struct selection_of_cells {
    int selection[4];       // defines a window of cells
    int selection_type;     // 0 = selection of 2 params [1,1], 1 = selection of window [1,1,1,2]
    bool find;
    char *find_str;         // find str arg memory
} Selection;

typedef struct cell {
    int length_of_cell_content;
    int *cell_content;      // contains the pointer of first char (saved as int) in a cell
} Cell;

typedef struct row {
    int n_of_cells;
    Cell *list_of_cells;    // points to the first cell
} Row;

typedef struct table {
    int n_of_rows;
    Row *list_of_rows;    // points to the first row
} Table;

int is_delim (const char *delim, char *delim_string[], const bool *multi_character_delim, int letter);
char * save_delim_and_args(char **argv, char *delim, char delim_string[], bool *multi_character_delim);
Table * load_table_from_file(char *delim, char *delim_string, bool multi_character_delim);
void print_table (Table *table, char delim);
void dealloc_table (Table *table);
bool level_table (Table *table);
void dealloc_variables(Variable *list);
Variable *init_list_of_variables();
bool parse_arguments (const char *string_of_all_params, Table *table, Variable *list_of_variables);
void init_selection(Selection *actual_selection);
void zero_selection(Selection *actual_selection);
bool refill_table (Table *table, Selection *actual_selection);
bool set_selection (Table *table, Selection *actual_selection, int *position, int *position_of_next_bracket, const char *string);
bool extract_params(const char *string, int *position, int pos_of_first_n, int *param1, int *param2);
bool swap_cells (Table *table, int row1, int col1, int row2, int col2);
bool max_min(Table *table, Selection *actual_selection, int *position, const char *string);
bool find_str(Table *table, Selection *actual_selection, int *position, const char *string, int *position_of_next_bracket);
char *str_strip(char *s);
bool is_content_in_col(Table *table, int col);
int shrink_table_cols(Table *table);

// FUNCTIONS FOR TABLE STRUCTURE EDITING
bool irow(Table *table, Selection *actual_selection);
bool arow(Table *table, Selection *actual_selection);
bool drow(Table *table, Selection *actual_selection);
bool icol(Table *table, Selection *actual_selection);
bool acol(Table *table, Selection *actual_selection);
bool dcol(Table *table, Selection *actual_selection);

// FUNCTIONS FOR TABLE CONTENTS EDITING
bool set_str (Table *table, Selection *actual_selection, int *position, const char *string);
bool clear(Table *table, Selection *actual_selection);
bool swap(Table *table, Selection *actual_selection, int row, int col);
bool sum(Table *table, Selection *actual_selection, int row, int col);
bool avg(Table *table, Selection *actual_selection, int row, int col);
bool count(Table *table, Selection *actual_selection, int row, int col);
bool len(Table *table, Selection *actual_selection, int row, int col);

// FUNCTIONS FOR VARIABLES
bool def_var (Table *table, Selection *actual_selection, Variable *list_of_variables, int *position, const char *string);
bool use_var(Table *table, Selection *actual_selection, Variable *list_of_variables, int *position, const char *string);
bool inc_var(Variable *list_of_variables, int *position, const char *string);
bool set_var(Selection *actual_selection, Variable *list_of_variables, int *position);
bool set_var_use(Selection *actual_selection, Variable *list_of_variables, int *position);


int main(int argc, char *argv[]) {

    if (argc != 5 && argc != 3) return -1;
    FILE *f;

    if (argc == 3) {
        f = freopen(argv[2], "r", stdin);
    } else {
        f = freopen(argv[4], "r", stdin);
    }

    // one character delimiter
    char delim;

    // multicharacter delim buffer
    char delim_string[MAX_DELIM_SIZE] = "";
    bool multi_character_delim = false;

    // string of all args from input
    char *string_of_all_params;

    // init of table struct
    Table *table;

    // initialization of list of variables
    Variable *list_of_variables;
    list_of_variables = init_list_of_variables();

    // saves delim and list of arguments
    string_of_all_params = save_delim_and_args(argv, &delim, delim_string, &multi_character_delim);

    // loads table to 3d struct
    table = load_table_from_file(&delim, delim_string, multi_character_delim);
    fclose(f);
    if (table == NULL) return -1;

    if (level_table(table) == false) return -1;

    // finds and executes all args
    if (parse_arguments(string_of_all_params, table, list_of_variables) == false) return -1;

    // redirect output to stdout
    if (argc == 3) {
        f = freopen(argv[2], "w", stdout);
    } else {
        f = freopen(argv[4], "w", stdout);
    }

    print_table(table, delim);

    fclose(f);

    // DEALLOCATION
    free (string_of_all_params);
    dealloc_table(table);
    dealloc_variables(list_of_variables);

    return 0;
}

Table *load_table_from_file(char *delim, char *delim_string, bool multi_character_delim) {
    // saves table into a 3d struct

    // initial allocation of table, first row, first cell
    Table *table;
    table = malloc(sizeof(Table));
    if (table == NULL) return NULL;
    int length_of_content = 1;
    table->n_of_rows = 1;

    Row *new_row;
    new_row = malloc(sizeof(Row) * table->n_of_rows);
    if (new_row == NULL) return NULL;
    table->list_of_rows = new_row;

    Cell *new_cell;
    new_cell = malloc(sizeof(Cell) * 1);
    if (new_cell == NULL) return NULL;
    table->list_of_rows->list_of_cells = new_cell;

    table->list_of_rows->n_of_cells = 1;
    table->list_of_rows->list_of_cells->length_of_cell_content = length_of_content;

    int *new_content;
    new_content = malloc(sizeof(int));
    *new_content = '\0';
    if (new_content == NULL) return NULL;
    table->list_of_rows->list_of_cells->cell_content = new_content;

    int curr_n_of_rows = 1;
    int curr_n_of_cells = 1;

    // loading of new char
    int letter;
    letter = fgetc(stdin);

    // quotes mode, escape mode
    bool in_quotes = false;
    bool escape_char = false;

    if (letter == 34) in_quotes = true;

    // main loop
    while (letter != EOF) {

        // current letter is delim
        if (is_delim(delim, &delim_string, &multi_character_delim, letter) && !in_quotes && !escape_char) {
            letter = fgetc(stdin);

            // if next letter is not EOF or end of line, create space for next cell
            if (letter != '\n' && letter != EOF) {

                curr_n_of_cells++;
                Cell *newptr;
                newptr = realloc(table->list_of_rows[curr_n_of_rows - 1].list_of_cells, sizeof(Cell) * curr_n_of_cells);
                if (newptr == NULL) return NULL;
                table->list_of_rows[curr_n_of_rows - 1].list_of_cells = newptr;

                table->list_of_rows[curr_n_of_rows - 1].n_of_cells++;

                //if the curr letter is doublequotes, turn on quotes mode
                if (letter == 34) {
                    in_quotes = true;

                    // add quotes to curr cell
                    int *new_ptr = malloc(sizeof(int) * 2);
                    if (new_ptr == NULL) return NULL;
                    table->list_of_rows[curr_n_of_rows - 1].list_of_cells[curr_n_of_cells - 1].cell_content = new_ptr;
                    table->list_of_rows[curr_n_of_rows - 1].list_of_cells[curr_n_of_cells - 1].cell_content[0] = 34;    //ascii code for "
                    table->list_of_rows[curr_n_of_rows - 1].list_of_cells[curr_n_of_cells - 1].length_of_cell_content = 2;
                    length_of_content = 2;

                }else {

                    // allocates mem for new cell with zero content (only '\0')
                    int *new_ptr = malloc(sizeof(int));
                    if (new_ptr == NULL) return NULL;
                    table->list_of_rows[curr_n_of_rows - 1].list_of_cells[curr_n_of_cells - 1].cell_content = new_ptr;
                    table->list_of_rows[curr_n_of_rows - 1].list_of_cells[curr_n_of_cells - 1].length_of_cell_content = 1;
                    length_of_content = 1;
                }

            } else {
                if (letter == '\n' ) {

                    // the previous char was delim and now is the end of line, so create empty cell
                    curr_n_of_cells++;
                    Cell *newptr;
                    newptr = realloc(table->list_of_rows[curr_n_of_rows - 1].list_of_cells, sizeof(Cell) * curr_n_of_cells);
                    if (newptr == NULL) return NULL;
                    table->list_of_rows[curr_n_of_rows - 1].list_of_cells = newptr;
                    table->list_of_rows[curr_n_of_rows - 1].n_of_cells++;

                    int *new_cell_content;
                    new_cell_content = malloc(sizeof(int));
                    if (new_cell_content == NULL) return NULL;
                    table->list_of_rows[curr_n_of_rows - 1].list_of_cells[curr_n_of_cells - 1].cell_content = new_cell_content;
                    table->list_of_rows[curr_n_of_rows - 1].list_of_cells[curr_n_of_cells - 1].length_of_cell_content = 1;
                    length_of_content = 1;
                }
            }

        // if current letter is not delim
        } else if (letter != '\n'){

            // escape char mode, 92 is ascii backslash
            if (letter == 92) escape_char = true;

            if (letter == 34 && !in_quotes) return NULL;

            // doublequotes mode
            if (letter == 34 && in_quotes) {
                letter = fgetc(stdin);
                if (letter == 92) escape_char = true;

                // add quotes and letter to cell content
                if (is_delim(delim, &delim_string, &multi_character_delim, letter)) {
                    in_quotes = false;
                    table->list_of_rows[curr_n_of_rows - 1].list_of_cells[curr_n_of_cells - 1].cell_content[length_of_content - 1] = 34;
                    table->list_of_rows[curr_n_of_rows - 1].list_of_cells[curr_n_of_cells - 1].length_of_cell_content = length_of_content;
                } else {
                    table->list_of_rows[curr_n_of_rows - 1].list_of_cells[curr_n_of_cells - 1].cell_content[length_of_content - 1] = letter;
                    table->list_of_rows[curr_n_of_rows - 1].list_of_cells[curr_n_of_cells - 1].length_of_cell_content = length_of_content;

                    // turns off escape mode after escaped char is saved
                    if (letter != 92 && escape_char) escape_char = false;
                    letter = fgetc(stdin);
                }
            } else {

                // add curr letter to cell
                table->list_of_rows[curr_n_of_rows - 1].list_of_cells[curr_n_of_cells - 1].cell_content[length_of_content - 1] = letter;
                table->list_of_rows[curr_n_of_rows - 1].list_of_cells[curr_n_of_cells - 1].length_of_cell_content = length_of_content;

                // turns off escape mode
                if (letter != 92 && escape_char) escape_char = false;
                letter = fgetc(stdin);
            }

            // if next letter is not eof, allocate memory for new letter
            if (letter != EOF && letter != '\n' && !escape_char && !is_delim(delim, &delim_string, &multi_character_delim, letter)) {
                length_of_content++;
                int *new_cell_content;
                new_cell_content = realloc(table->list_of_rows[curr_n_of_rows - 1].list_of_cells[curr_n_of_cells - 1].cell_content, sizeof(int) * length_of_content);
                if (new_cell_content == NULL) return NULL;
                table->list_of_rows[curr_n_of_rows - 1].list_of_cells[curr_n_of_cells - 1].cell_content = new_cell_content;

            // if escape mode is on, add letter even if its delim
            } else if (escape_char) {
                length_of_content++;
                int *new_cell_content;
                new_cell_content = realloc(table->list_of_rows[curr_n_of_rows - 1].list_of_cells[curr_n_of_cells - 1].cell_content, sizeof(int) * length_of_content);
                if (new_cell_content == NULL) return NULL;
                table->list_of_rows[curr_n_of_rows - 1].list_of_cells[curr_n_of_cells - 1].cell_content = new_cell_content;
            }
        // end of line
        } else {
            letter = fgetc(stdin);

            // if next letter is not eof, create new row
            if (letter != EOF) {

                curr_n_of_rows++;
                Row *newptr;
                newptr = realloc(table->list_of_rows, sizeof(Row) * curr_n_of_rows);
                if (newptr == NULL) return NULL;
                table->list_of_rows = newptr;

                table->n_of_rows++;
                length_of_content = 1;

                curr_n_of_cells = 1;
                Cell *newcell;
                newcell = malloc(sizeof(Cell) * curr_n_of_cells);
                if (newcell == NULL) return NULL;
                table->list_of_rows[curr_n_of_rows - 1].list_of_cells = newcell;

                // allocation of new cell, its content and content length
                table->list_of_rows[curr_n_of_rows - 1].list_of_cells[curr_n_of_cells - 1].cell_content = malloc(sizeof(int));
                table->list_of_rows[curr_n_of_rows - 1].list_of_cells[curr_n_of_cells - 1].length_of_cell_content = 1;
                table->list_of_rows[curr_n_of_rows - 1].n_of_cells = 1;
            }
        }
    }

    return table;
}

void print_table (Table *table, char delim) {
    // basic table print with delims

    // removal of surplus (empty) cols at the end of each row
    int n_of_valid_cols = shrink_table_cols(table);

    for (int row = 0; row < table->n_of_rows; row++) {
        for (int cell = 0; cell < n_of_valid_cols; cell++) {
            if (table->list_of_rows[row].list_of_cells[cell].length_of_cell_content == 1) {
                if (*table->list_of_rows[row].list_of_cells[cell].cell_content != (int)'\0') {
                    for (int znak = 0; znak < table->list_of_rows[row].list_of_cells[cell].length_of_cell_content; znak++) {
                        printf("%c", table->list_of_rows[row].list_of_cells[cell].cell_content[znak]);
                    }
                }
            }else {
                for (int c = 0; c < table->list_of_rows[row].list_of_cells[cell].length_of_cell_content; c++) {
                    printf("%c", table->list_of_rows[row].list_of_cells[cell].cell_content[c]);
                }
            }
            if (cell + 1 < n_of_valid_cols) printf("%c", delim);
        }
        if (row + 1 < table->n_of_rows) printf("\n");
    }
}

int shrink_table_cols(Table *table) {
    int n_of_valid_cols = table->list_of_rows->n_of_cells;

    for(int cols = table->list_of_rows->n_of_cells - 1; cols > 0; cols--) {
        if (!is_content_in_col(table, cols)) {
            n_of_valid_cols--;
        } else {
            break;
        }
    }
    return n_of_valid_cols;
}

bool is_content_in_col(Table *table, int col) {
    // finds if there is a cell with some content

    for (int row = 0; row < table->n_of_rows - 1; row++) {
        if (table->list_of_rows[row].list_of_cells[col].length_of_cell_content == 1) {
            if (*table->list_of_rows[row].list_of_cells[col].cell_content != (int)'\0') {
                return true;
            }
        } else {
            return true;
        }
    }
    return false;
}

void dealloc_table (Table *table) {
    // deallocates contents of table and table

    for (int row = 0; row < table->n_of_rows; row++) {
        for (int cell = 0; cell < table->list_of_rows[row].n_of_cells; cell ++) {
            free(table->list_of_rows[row].list_of_cells[cell].cell_content);
        }
        free(table->list_of_rows[row].list_of_cells);
    }
    free(table->list_of_rows);
    free(table);
}

void zero_selection(Selection *actual_selection) {
    for (int i = 0; i < 4; i++) {
        actual_selection->selection[i] = 0;
    }
}

char *save_delim_and_args(char **argv, char *delim, char delim_string[MAX_DELIM_SIZE], bool *multi_character_delim) {
    // finds if delim was provided, saves all args

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

int is_delim (const char *delim, char *delim_string[], const bool *multi_character_delim, int letter) {
    // find if letter is in delim_string

    if (*multi_character_delim) {
        if (strchr(*delim_string, (char)letter) != NULL ) {
            return 1;
        }
        return 0;
    }else{
        if(*delim == letter){
            return 1;
        }
        return 0;
    }
}

bool level_table (Table *table) {
    // levels table to the max number of cols in either row

    int max_n_of_cols = 1;
    for (int row = 0; row < table->n_of_rows; row++) {
        if (table->list_of_rows[row].n_of_cells > max_n_of_cols) {
            max_n_of_cols = table->list_of_rows[row].n_of_cells;
        }
    }

    for (int row = 0; row < table->n_of_rows; row++) {

        // if new cells need to be added
        if (table->list_of_rows[row].n_of_cells != max_n_of_cols) {

            int n_of_cols_in_row = table->list_of_rows[row].n_of_cells;

            for (int actual_n_of_cols = n_of_cols_in_row + 1; actual_n_of_cols <= max_n_of_cols; actual_n_of_cols++) {

                Cell *newcell;
                newcell = realloc(table->list_of_rows[row].list_of_cells, sizeof(Cell) * actual_n_of_cols);
                if (newcell == NULL) return false;
                table->list_of_rows[row].list_of_cells = newcell;

                int *new_content;
                new_content = malloc(sizeof(int));
                *new_content = (int)'\0';
                if (new_content == NULL) return false;

                table->list_of_rows[row].list_of_cells[actual_n_of_cols - 1].cell_content = new_content;
                table->list_of_rows[row].list_of_cells[actual_n_of_cols - 1].length_of_cell_content = 1;
                table->list_of_rows[row].n_of_cells = actual_n_of_cols;
            }

        }
    }
    return true;
}

Variable *init_list_of_variables (){
    Variable *list = malloc(sizeof(Variable) * 11);

    for (int i = 0; i < 11; i++) {
        for (int pos = 0; pos < 4; pos++) list[i].selection[pos] = 0;
        list[i].string = NULL;
        list[i].type_of_var = -1;
        list[i].number_f = 0.0;
    }

    return list;
}

void dealloc_variables (Variable *list) {

    for (int i = 0; i < 11; i++) {
        if(list[i].string != NULL) {
            free(list[i].string);
        }
    }
    free(list);
}

void init_selection (Selection *actual_selection) {
    // initializes selection

    for (int i = 0; i < 4; i++) actual_selection->selection[i] = 1;
    actual_selection->selection_type = 0;
    actual_selection->find = false;
    actual_selection->find_str = NULL;
}

void dealloc_selection (Selection *aktualna_selekcia) {
    free(aktualna_selekcia->find_str);
}

char *str_strip(char *s)
{
    size_t size;
    char *end;

    size = strlen(s);

    if (!size)
        return s;
    end = s + size - 1;
    while (end >= s && isspace(*end))
        end--;
    *(end + 1) = '\0';

    while (*s && isspace(*s))
        s++;

    return s;
}

bool refill_table (Table *table, Selection *actual_selection) {

    // n of rows, cols that need to be added
    int rows_over = actual_selection->selection[2] - table->n_of_rows;
    int cols_over = actual_selection->selection[3] - table->list_of_rows->n_of_cells;

    // need to add rows
    if (rows_over > 0) {

        Row *new_rows = realloc(table->list_of_rows, sizeof(Row) * actual_selection->selection[2]);
        if (new_rows == NULL) return false;
        table->list_of_rows = new_rows;

        // new cells in each row
        for (int riadok = table->n_of_rows; riadok < actual_selection->selection[2]; riadok++) {

            Cell *new_cells;
            new_cells = malloc(sizeof(Cell) * table->list_of_rows->n_of_cells);
            if (new_cells == NULL) return false;
            table->list_of_rows[riadok].list_of_cells = new_cells;

            // each cell has '\0' content
            for (int bunka = 0; bunka < table->list_of_rows->n_of_cells; bunka++) {

                int *new_content = malloc(sizeof(int));
                if (new_content == NULL) return false;
                *new_content = (int)'\0';

                table->list_of_rows[riadok].list_of_cells[bunka].cell_content = new_content;
                table->list_of_rows[riadok].list_of_cells[bunka].length_of_cell_content = 1;
            }
        }
        table->n_of_rows =  actual_selection->selection[2];
    }

    // need to add cols
    if (cols_over > 0) {
        for (int row = 0; row < actual_selection->selection[2]; row++) {

            Cell *new_row;
            new_row = realloc(table->list_of_rows[row].list_of_cells, sizeof(Cell) * actual_selection->selection[3]);
            if(new_row == NULL) return false;
            table->list_of_rows[row].list_of_cells = new_row;

            for (int bunka = table->list_of_rows[row].n_of_cells; bunka < actual_selection->selection[3]; bunka++) {

                int *new_content = malloc(sizeof(int));
                if (new_content == NULL) return false;
                *new_content = '\0';

                table->list_of_rows[row].list_of_cells[bunka].cell_content = new_content;
                table->list_of_rows[row].list_of_cells[bunka].length_of_cell_content = 1;
            }
            table->list_of_rows[row].n_of_cells = actual_selection->selection[3];
        }
    }
    return true;
}

bool verify_command (const char *string_of_all_params, int *position, char verif_command[4]) {
    // verifies that the command is the same as verif command

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

    if (strstr(command, verif_command) != NULL) {
        return true;
    }else {
        return false;
    }

}

bool set_selection (Table *table, Selection *actual_selection, int *position, int *position_of_next_bracket, const char *string) {
    // saves selection of cells to actual_selection
    zero_selection(actual_selection);

    // chcem hladat ']' hned od dalsej pozicie
    *position_of_next_bracket = *position + 1;
    int n_of_commas = 0;
    int pos_of_commas[3] = {0};

    // find closing bracket and positions of commas inbetween
    while(string[*position_of_next_bracket] != ']') {
        if (string[*position_of_next_bracket] == ',') {
            pos_of_commas[n_of_commas] = *position_of_next_bracket;
            n_of_commas++;
        }
        *position_of_next_bracket = *position_of_next_bracket + 1;
    };

    // only valid n of commas is 1 or 3
    if (n_of_commas != 1 && n_of_commas != 3) return false;

    int tmp_position = *position;

    char tmp_string[MAX_ARGUMENT_LENGTH] = "";
    int cele_cislo = 0;

    int length_of_number = 0;


    for (int i = 0; i < n_of_commas + 1; i++) {

        if (n_of_commas == 1) {
            if (i==0) {
                length_of_number = pos_of_commas[i] - tmp_position - 1;
            } else {
                length_of_number = *position_of_next_bracket - tmp_position - 1;
            }
        } else {
            if (i < 3) {
                length_of_number = pos_of_commas[i] - tmp_position - 1;
            } else {
                length_of_number = *position_of_next_bracket - tmp_position - 1;
            };
        }

        // copy number into tmp_string
        for (int x = 0; x < length_of_number; x++) {
            tmp_string[x] = string[tmp_position + 1 + x];
        }
        tmp_string[length_of_number] = '\0';

        bool dash = false;

        if (n_of_commas == 3 && tmp_string[0] == '-' && tmp_string[1] == '\0') dash = true;

        if (tmp_string[0] == '_' && tmp_string[1] == '\0') {
            if (n_of_commas == 1) {
                if (i == 0) {
                    actual_selection->selection[0] = 1;
                    actual_selection->selection[2] = table->n_of_rows;
                    actual_selection->selection_type = 1;
                } else {

                    actual_selection->selection[1] = 1;
                    actual_selection->selection[3] = table->list_of_rows->n_of_cells;
                }
            }
        }

        else if (n_of_commas == 1) {
            if (i == 0) {
                cele_cislo = (int)strtol(tmp_string, NULL, 10);
                actual_selection->selection[0] = cele_cislo;
                actual_selection->selection[2] = cele_cislo;
                actual_selection->selection_type = 0;
            } else {
                cele_cislo = (int)strtol(tmp_string, NULL, 10);
                actual_selection->selection[1] = cele_cislo;
                actual_selection->selection[3] = cele_cislo;
            }
        } else {
            cele_cislo = (int)strtol(tmp_string, NULL, 10);
            if (dash) {
                if (i == 2) {
                    actual_selection->selection[i] = table->n_of_rows;
                }else if (i == 3){
                    actual_selection->selection[i] = table->list_of_rows->n_of_cells;
                }
            } else {
                actual_selection->selection[i] = cele_cislo;
            }
        }
    tmp_position = pos_of_commas[i];
    }
    *position = *position_of_next_bracket + 1;
    return true;
}

bool extract_params(const char *string, int *position, int pos_of_first_n, int *param1, int *param2) {
    // extracts params of arguments, e.g. xxxx [param1,param2]

    int tmp_position = *position + pos_of_first_n;
    char tmp_string[1000] = "";
    while (string[tmp_position] != ',' && string[tmp_position] != '\0') {
        tmp_position++;
    }
    for (int znak = *position + pos_of_first_n, position_in_string = 0; znak < tmp_position;znak++, position_in_string++) {
        tmp_string[position_in_string] = string[znak];
    }
    *position = tmp_position + 1;

    char *p1_ptr;
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
    char *p2_ptr;
    double curr_number2 = 0.0;
    curr_number2 = strtod(tmp_string, &p2_ptr);
    if (p2_ptr == NULL) return false;
    *param2 = (int)curr_number2;
    *position = tmp_position;

    return true;
}

bool parse_arguments(const char *string, Table *table, Variable *list_of_variables) {
    // finds and executes all arguments from input

    Selection *actual_selection;
    actual_selection = malloc(sizeof(Selection));
    init_selection(actual_selection);

    int position = 0;
    int position_of_next_bracket = 0;

    // main loop
    while (string[position] != '\0') {

        // selection input
        if (string[position] == '[' && string[position + 1] != 'm' && string[position + 2] != ']' && string[position + 1] != 'f' && string[position + 1] != 's') {
            if (!set_selection(table, actual_selection, &position, &position_of_next_bracket, string)) return false;
        }

        // [max]/[min]
        else if (string[position] == '[' && string[position + 1] == 'm') {
            if (!max_min(table, actual_selection, &position, string)) return false;
        }

        // [find STR]
        else if (string[position] == '[' && string[position + 1] == 'f') {
            if (!find_str(table, actual_selection, &position, string, &position_of_next_bracket)) return false;
        }

        // irow
        else if (string[position] == 'i' && string[position + 1] == 'r') {
            if (verify_command(string, &position, "irow")) {
                if(!irow(table, actual_selection)) return false;
            }
        }

        // arow
        else if (string[position] == 'a' && string[position + 1] == 'r') {
            if (verify_command(string, &position, "arow")) {
                if(!arow(table, actual_selection)) return false;
            }
        }

        // drow
        else if (string[position] == 'd' && string[position + 1] == 'r') {
            if (verify_command(string, &position, "drow")) {
                if(!drow(table, actual_selection)) return false;
            }
        }

        // icol
        else if (string[position] == 'i' && string[position + 1] == 'c') {
            if (verify_command(string, &position, "icol")) {
                if(!icol(table, actual_selection)) return false;
            }
        }

        // acol
        else if (string[position] == 'a' && string[position + 1] == 'c') {
            if (verify_command(string, &position, "acol")) {
                if(!acol(table, actual_selection)) return false;
            }
        }

        // dcol
        else if (string[position] == 'd' && string[position + 1] == 'c') {
            if (verify_command(string, &position, "dcol")) {
                if(!dcol(table, actual_selection)) return false;
            }
        }

        // set STR
        else if (string[position] == 's' && string[position + 1] == 'e' && string[position + 2] == 't' && string[position + 2] != ']') {
            if (!set_str(table, actual_selection, &position, string)) return false;
        }

        // clear
        else if (string[position] == 'c' && string[position + 1] == 'l' && string[position + 2] == 'e' &&
                string[position + 3] == 'a' && string[position + 4] == 'r') {
            if (!clear(table, actual_selection)) return false;
            position = position + 5;
        }

        // swap
        else if (string[position] == 's' && string[position + 1] == 'w' && string[position + 2] == 'a' &&
                string[position + 3] == 'p') {
            int param1, param2;
            if (!extract_params(string, &position, 6, &param1, &param2)) return false;
            if (!swap(table, actual_selection, param1, param2)) return false;
        }

        // sum
        else if (string[position] == 's' && string[position + 1] == 'u' && string[position + 2] == 'm') {
            int param1, param2;
            if (!extract_params(string, &position, 5, &param1, &param2)) return false;
            if (!sum(table, actual_selection, param1, param2)) return false;

        }

        // avg
        else if (string[position] == 'a' && string[position + 1] == 'v' && string[position + 2] == 'g') {
            int param1, param2;
            if (!extract_params(string, &position, 5, &param1, &param2)) return false;
            if (!avg(table, actual_selection, param1, param2)) return false;
        }

        // count
        else if (string[position] == 'c' && string[position + 1] == 'o' && string[position + 2] == 'u' &&
                 string[position + 3] == 'n' && string[position + 4] == 't') {
            int param1, param2;
            if (!extract_params(string, &position, 7, &param1, &param2)) return false;
            if (!count(table, actual_selection, param1, param2)) return false;
        }

        // len
        else if (string[position] == 'l' && string[position + 1] == 'e' && string[position + 2] == 'n') {
            int param1, param2;
            if (!extract_params(string, &position, 5, &param1, &param2)) return false;
            if (!len(table, actual_selection, param1, param2)) return false;
        }

        // def
        else if (string[position] == 'd' && string[position + 1] == 'e' && string[position + 2] == 'f') {
            if (!def_var(table, actual_selection, list_of_variables, &position, string)) return false;
        }

        // use
        else if (string[position] == 'u' && string[position + 1] == 's' && string[position + 2] == 'e') {
            if (!use_var(table, actual_selection, list_of_variables, &position, string)) return false;
        }

        // inc
        else if (string[position] == 'i' && string[position + 1] == 'n' && string[position + 2] == 'c') {
            if (!inc_var(list_of_variables, &position, string)) return false;
        }
        // [set]
        else if (string[position] == '[' && string[position + 1] == 's' && string[position + 2] == 'e' && string[position + 3] == 't'
                && string[position + 4] == ']') {
            if (!set_var(actual_selection, list_of_variables, &position)) return false;
        }

        // [_]
        else if (string[position] == '[' && string[position + 1] == '_' && string[position + 2] == ']') {
            if (!set_var_use(actual_selection, list_of_variables, &position)) return false;
        }

        else {
            position++;
        }
    }
    dealloc_selection(actual_selection);
    free(actual_selection);
    return true;

}

bool irow(Table *table, Selection *actual_selection) {
    // inserts one row before selected cells

    refill_table(table, actual_selection);

    // row before which to insert new row
    int row = actual_selection->selection[0];

    int n_of_cols = table->list_of_rows->n_of_cells;
    int n_of_rows = table->n_of_rows;
    table->n_of_rows++;

    // allocation of a new row
    Row *newptr = realloc(table->list_of_rows, sizeof(Row) * table->n_of_rows);
    if (newptr == NULL) return false;
    table->list_of_rows = newptr;

    // shifting rows to next index
    for (int rows = n_of_rows - 1; rows > row - 2; rows--) {
        table->list_of_rows[rows + 1] = table->list_of_rows[rows];
    }

    // allocation of new cells
    Cell *new_cells = malloc(sizeof(Cell) * n_of_cols);
    if (new_cells == NULL) return false;
    table->list_of_rows[row - 1].list_of_cells = new_cells;
    for (int bunka = 0; bunka < n_of_cols; bunka++) {
        int *new_content_of_cell = malloc(sizeof(int));
        if (new_content_of_cell == NULL) return false;
        *new_content_of_cell='\0';
        table->list_of_rows[row - 1].list_of_cells[bunka].cell_content = new_content_of_cell;
        table->list_of_rows[row - 1].list_of_cells[bunka].length_of_cell_content = 1;
    }
    return true;
}

bool arow(Table *table, Selection *actual_selection) {
    // inserts one row after selected cells

    refill_table(table, actual_selection);

    // row after which to insert new row
    int row = actual_selection->selection[2];
    int n_of_cols = table->list_of_rows[0].n_of_cells;
    int pocet_riadkov_v_tabulke = table->n_of_rows;
    table->n_of_rows++;

    // allocation of a new row
    Row *newptr = realloc(table->list_of_rows, sizeof(Row) * table->n_of_rows);
    table->n_of_rows++;
    if (newptr == NULL) return false;
    table->list_of_rows = newptr;

    // shifting of rows
    for (int rows = pocet_riadkov_v_tabulke - 1; rows > row - 1 ; rows--) {
        table->list_of_rows[rows + 1] = table->list_of_rows[rows];
    }

    // allocation of new cells
    Cell *new_cells = malloc(sizeof(Cell) * n_of_cols);
    if (new_cells == NULL) return false;
    table->list_of_rows[row].list_of_cells = new_cells;

    for (int bunka = 0 ; bunka < n_of_cols ; bunka++) {
        int *new_content_of_cell = malloc(sizeof(int));
        if (new_content_of_cell == NULL) return false;
        *new_content_of_cell='\0';
        table->list_of_rows[row].list_of_cells[bunka].cell_content = new_content_of_cell;
        table->list_of_rows[row].list_of_cells[bunka].length_of_cell_content = 1;
    }

    return true;
}

bool drow(Table *table, Selection *actual_selection) {
    // deleter rows in selection

    refill_table(table, actual_selection);

    int row_from = actual_selection->selection[0];
    int row_to = actual_selection->selection[2];

    int n_of_cols = table->list_of_rows->n_of_cells;
    int n_of_row_to_delete = row_to - row_from + 1;

    // contents of cells need to be freed
    for (int riadok = row_from - 1 ; riadok < row_to ; riadok++) {
        for (int bunka = 0; bunka < n_of_cols; bunka++) {
            free(table->list_of_rows[riadok].list_of_cells[bunka].cell_content);
        }
        free(table->list_of_rows[riadok].list_of_cells);
    }

    for (int new_pos = row_from - 1, old_pos = row_to; old_pos < table->n_of_rows; new_pos++, old_pos++) {
        table->list_of_rows[new_pos] = table->list_of_rows[old_pos];
    }
    table->n_of_rows = table->n_of_rows - n_of_row_to_delete;
    Row *new_rows = realloc(table->list_of_rows, sizeof(Row) * table->n_of_rows - 1);
    if (new_rows == NULL) return false;
    table->list_of_rows = new_rows;

    return true;
}

bool icol(Table *table, Selection *actual_selection) {
    // inserts a col before selection

    refill_table(table, actual_selection);

    int insert_before = actual_selection->selection[1];
    int new_n_of_cols = table->list_of_rows->n_of_cells + 1;

    for (int riadok = 0; riadok < table->n_of_rows; riadok++) {

        // allocation of memory for new cell
        table->list_of_rows[riadok].n_of_cells = new_n_of_cols;
        Cell *new_cells = realloc(table->list_of_rows[riadok].list_of_cells, sizeof(Cell) * new_n_of_cols);
        if (new_cells == NULL) return NULL;
        table->list_of_rows[riadok].list_of_cells = new_cells;

        // shifting of cells to new positions
        for (int nova_pozicia = new_n_of_cols - 1, stara_pozicia = new_n_of_cols - 2; stara_pozicia >= insert_before - 1; nova_pozicia--, stara_pozicia--) {
            table->list_of_rows[riadok].list_of_cells[nova_pozicia] = table->list_of_rows[riadok].list_of_cells[stara_pozicia];
        }

        // new cell
        int *new_content = malloc(sizeof(int));
        if (new_content == NULL) return NULL;
        *new_content = '\0';
        table->list_of_rows[riadok].list_of_cells[insert_before - 1].cell_content = new_content;
        table->list_of_rows[riadok].list_of_cells[insert_before - 1].length_of_cell_content = 1;
    }

    return true;
}

bool acol(Table *table, Selection *actual_selection) {
    // appends a col after selection

    refill_table(table, actual_selection);

    int insert_after = actual_selection->selection[3];
    int new_n_of_cols = table->list_of_rows->n_of_cells + 1;

    for (int riadok = 0; riadok < table->n_of_rows; riadok++) {

        // allocation of memory for new cell
        table->list_of_rows[riadok].n_of_cells = new_n_of_cols;
        Cell *new_cells = realloc(table->list_of_rows[riadok].list_of_cells, sizeof(Cell) * new_n_of_cols);
        if (new_cells == NULL) return NULL;
        table->list_of_rows[riadok].list_of_cells = new_cells;

        // shifting cells to new positions
        for (int nova_pozicia = new_n_of_cols - 1, stara_pozicia = new_n_of_cols - 2; stara_pozicia >= insert_after; nova_pozicia--, stara_pozicia--) {
            table->list_of_rows[riadok].list_of_cells[nova_pozicia] = table->list_of_rows[riadok].list_of_cells[stara_pozicia];
        }

        // new cell
        int *new_content = malloc(sizeof(int));
        if (new_content == NULL) return NULL;
        *new_content = '\0';
        table->list_of_rows[riadok].list_of_cells[insert_after].cell_content = new_content;
        table->list_of_rows[riadok].list_of_cells[insert_after].length_of_cell_content = 1;
    }

    return true;
}

bool dcol(Table *table, Selection *actual_selection) {
    // deletes columns in selection

    refill_table(table, actual_selection);

    int delete_from = actual_selection->selection[1];
    int delete_to = actual_selection->selection[3];
    int n_of_cols_to_delete = delete_to - delete_from + 1;
    int new_n_of_cols = table->list_of_rows->n_of_cells - n_of_cols_to_delete;

    for (int riadok = 0; riadok < table->n_of_rows; riadok++) {

        table->list_of_rows[riadok].n_of_cells = new_n_of_cols;

        // contents of deleted cells need to be deallocated
        for (int bunka = delete_from - 1; bunka < delete_to; bunka++) {
            free(table->list_of_rows[riadok].list_of_cells[bunka].cell_content);
        }

        // shifting cell to new positions
        for (int nova_pozicia = delete_from - 1, stara_pozicia = delete_to; stara_pozicia <= new_n_of_cols + 1; nova_pozicia++, stara_pozicia++) {
            table->list_of_rows[riadok].list_of_cells[nova_pozicia] = table->list_of_rows[riadok].list_of_cells[stara_pozicia];
        }

        // shrinking rows to new number of cells
        Cell *new_row = realloc(table->list_of_rows[riadok].list_of_cells, sizeof(Cell) * new_n_of_cols);
        table->list_of_rows[riadok].list_of_cells = new_row;
    }

    return true;
}

bool set_str (Table *table, Selection *actual_selection, int *position, const char *string) {
    // function that copies STR parameter into every cell in selection

    refill_table(table, actual_selection);

    int position_of_string_end = *position + 4;
    char string_param[1000] = "";
    int pos_in_str_param = 0;

    while(string[position_of_string_end]) {
        if ((string[position_of_string_end] == ';' && string[position_of_string_end - 1] != '\\') || string[position_of_string_end] == '\0') {
            break;
        } else {
            string_param[pos_in_str_param] = string[position_of_string_end];
            pos_in_str_param++;
            position_of_string_end++;
        }
    }
    string_param[pos_in_str_param + 1] = '\0';
    *position = position_of_string_end + 1;

    int row_from = actual_selection->selection[0];
    int row_to = actual_selection->selection[2];
    int col_from = actual_selection->selection[1];
    int col_to = actual_selection->selection[3];

    int length_of_str_param = (int)strlen(string_param);

    for (int row = row_from - 1; row < row_to; row++) {
        for (int col = col_from - 1; col < col_to; col++) {

            // if cell content needs to be reallocated to new length
            if (table->list_of_rows[row].list_of_cells[col].length_of_cell_content != length_of_str_param) {

                int *new_content;
                new_content = realloc(table->list_of_rows[row].list_of_cells[col].cell_content, sizeof(int) * length_of_str_param);
                if (new_content == NULL) return false;
                table->list_of_rows[row].list_of_cells[col].cell_content = new_content;

                table->list_of_rows[row].list_of_cells[col].length_of_cell_content = length_of_str_param;
            }
            for (int i = 0; i < length_of_str_param; i++) {
                table->list_of_rows[row].list_of_cells[col].cell_content[i] = (int)string_param[i];
            }
        }
    }
    return true;
}

bool clear(Table *table, Selection *actual_selection) {
    // function that clear the contents of cells in selection

    refill_table(table, actual_selection);

    int row_from = actual_selection->selection[0];
    int row_to = actual_selection->selection[2];
    int col_from = actual_selection->selection[1];
    int col_to = actual_selection->selection[3];

    for (int row = row_from - 1; row < row_to; row++) {
        for (int col = col_from - 1; col < col_to; col++) {

            if (table->list_of_rows[row].list_of_cells[col].length_of_cell_content == 1) {
                *table->list_of_rows[row].list_of_cells[col].cell_content = '\0';
            } else {
                table->list_of_rows[row].list_of_cells[col].length_of_cell_content = 1;

                int *new_content;
                new_content = realloc(table->list_of_rows[row].list_of_cells[col].cell_content, sizeof(int));
                if (new_content == NULL) return false;
                *new_content = '\0';
                table->list_of_rows[row].list_of_cells[col].cell_content = new_content;
            }
        }
    }
    return true;
}

bool swap(Table *table, Selection *actual_selection, int row, int col) {
    // swaps cells in selection with the one set by parameters swap [R,C]

    if (row > table->n_of_rows || col > table->list_of_rows->n_of_cells) return false;

    refill_table(table, actual_selection);

    int row_from = actual_selection->selection[0];
    int row_to = actual_selection->selection[2];
    int col_from = actual_selection->selection[1];
    int col_to = actual_selection->selection[3];

    for (int riadok = row_from - 1; riadok < row_to; riadok++) {
        for (int bunka = col_from - 1; bunka < col_to; bunka++) {
            swap_cells(table, riadok, bunka, row - 1, col - 1);
        }
    }
    return true;
}

bool sum(Table *table, Selection *actual_selection, int row, int col) {
    // function that save the sum of valid number cells in selection into cell provided by parameters sum [R,C]

    if (row > table->n_of_rows || col > table->list_of_rows->n_of_cells) return false;

    refill_table(table, actual_selection);

    int row_from = actual_selection->selection[0];
    int row_to = actual_selection->selection[2];
    int col_from = actual_selection->selection[1];
    int col_to = actual_selection->selection[3];

    double total_sum = 0.0;
    char tmp_string[1000] = "";

    // find the sum of numbers in selection
    for (int rows = row_from - 1; rows < row_to; rows++) {
        for (int cols = col_from - 1; cols < col_to; cols++) {

            char *p;
            double curr_number = 0.0;
            int position_in_str = 0;

            for (int znak = 0; znak < table->list_of_rows[rows].list_of_cells[cols].length_of_cell_content; znak++) {
                tmp_string[znak] = (char)table->list_of_rows[rows].list_of_cells[cols].cell_content[znak];
                position_in_str++;
            }
            tmp_string[position_in_str] = '\0';
            *tmp_string = *str_strip(tmp_string);

            curr_number = (double)strtold(tmp_string, &p);

            if (*p == '\0') {
                total_sum = total_sum + curr_number;
            }
        }
    }

    // save sum of numbers to cell
    char buffer[MAX_ARGUMENT_LENGTH];
    int output_int;
    output_int = snprintf(buffer, MAX_ARGUMENT_LENGTH, "%g", total_sum);

    if (output_int >= 0 && output_int < MAX_ARGUMENT_LENGTH) {

        if (table->list_of_rows[row - 1].list_of_cells[col - 1].length_of_cell_content != (int)strlen(buffer)) {
            int *new_content;
            new_content = realloc(table->list_of_rows[row - 1].list_of_cells[col - 1].cell_content, sizeof(int) * strlen(buffer));
            if (new_content == NULL) return false;
            table->list_of_rows[row - 1].list_of_cells[col - 1].cell_content = new_content;
        }
        for (int i = 0, length = (int)strlen(buffer); i < length; i++) {
            table->list_of_rows[row - 1].list_of_cells[col - 1].cell_content[i] = (int)buffer[i];
        }
        table->list_of_rows[row - 1].list_of_cells[col - 1].length_of_cell_content = (int)strlen(buffer);
    }
    return true;
}

bool avg(Table *table, Selection *actual_selection, int row, int col) {
    // function that save the sum of valid number cells in selection into cell provided by parameters sum [R,C]

    if (row > table->n_of_rows || col > table->list_of_rows->n_of_cells) return false;

    refill_table(table, actual_selection);

    int row_from = actual_selection->selection[0];
    int row_to = actual_selection->selection[2];
    int col_from = actual_selection->selection[1];
    int col_to = actual_selection->selection[3];

    double total_sum = 0.0;
    char tmp_string[1000] = "";
    int n_of_valid_cells = 0;

    for (int riadok = row_from - 1; riadok < row_to; riadok++) {
        for (int bunka = col_from - 1; bunka < col_to; bunka++) {

            char *p;
            double curr_number = 0.0;
            int position_in_str = 0;

            for (int znak = 0; znak < table->list_of_rows[riadok].list_of_cells[bunka].length_of_cell_content; znak++) {
                tmp_string[znak] = (char)table->list_of_rows[riadok].list_of_cells[bunka].cell_content[znak];
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
    int size_of_buffer = (int)strlen(buffer);

    if (output_int >= 0 && output_int < 100) {

        if (table->list_of_rows[row - 1].list_of_cells[col - 1].length_of_cell_content != size_of_buffer) {
            int *new_content;
            new_content = realloc(table->list_of_rows[row - 1].list_of_cells[col - 1].cell_content, sizeof(int) * size_of_buffer);
            if (new_content == NULL) return false;

            table->list_of_rows[row - 1].list_of_cells[col - 1].cell_content = new_content;
        }

        for (int i = 0, length = size_of_buffer; i < length; i++) {
            table->list_of_rows[row - 1].list_of_cells[col - 1].cell_content[i] = (int)buffer[i];
        }
        table->list_of_rows[row - 1].list_of_cells[col - 1].length_of_cell_content = size_of_buffer;
    }
    return true;
}

bool count(Table *table, Selection *actual_selection, int row, int col) {
    // function that save the sum of valid number cells in selection into cell provided by parameters sum [R,C]

    if (row > table->n_of_rows || col > table->list_of_rows->n_of_cells) return false;

    // if the selection is bigger that the dimensions of table
    refill_table(table, actual_selection);

    int row_from = actual_selection->selection[0];
    int row_to = actual_selection->selection[2];
    int col_from = actual_selection->selection[1];
    int col_to = actual_selection->selection[3];

    int n_of_valid_cells = 0;

    for (int rows = row_from - 1; rows < row_to; rows++) {
        for (int cols = col_from - 1; cols < col_to; cols++) {

            if (table->list_of_rows[rows].list_of_cells[cols].length_of_cell_content == 1) {
                if (table->list_of_rows[rows].list_of_cells[cols].cell_content[0] != '\0') {
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

    free(table->list_of_rows[row - 1].list_of_cells[col - 1].cell_content);
    table->list_of_rows[row - 1].list_of_cells[col - 1].cell_content = new_content;
    table->list_of_rows[row - 1].list_of_cells[col - 1].length_of_cell_content = new_length_of_content;

    for (int znak = 0; znak < new_length_of_content; znak++) {
        table->list_of_rows[row - 1].list_of_cells[col - 1].cell_content[znak] = (int)buffer[znak];
    }
    return true;
}

bool len(Table *table, Selection *actual_selection, int row, int col) {
    int col_last = actual_selection->selection[3] - 1;
    int row_last = actual_selection->selection[2] - 1;

    if (row > table->n_of_rows || col > table->list_of_rows->n_of_cells) return false;

    int length = table->list_of_rows[row_last].list_of_cells[col_last].length_of_cell_content;

    char tmp_string[MAX_ARGUMENT_LENGTH] = "";
    int output_int;

    output_int = snprintf(tmp_string, MAX_ARGUMENT_LENGTH, "%d", length);

    if (table->list_of_rows[row - 1].list_of_cells[col - 1].length_of_cell_content != output_int) {
        int *new_cell_content;
        new_cell_content = realloc(table->list_of_rows[row - 1].list_of_cells[col - 1].cell_content, sizeof(int)*output_int);
        if (new_cell_content == NULL) return false;
        table->list_of_rows[row - 1].list_of_cells[col - 1].cell_content = new_cell_content;
        table->list_of_rows[row - 1].list_of_cells[col - 1].length_of_cell_content = output_int;
    }
    for (int letter = 0; letter < output_int; letter++) {
        table->list_of_rows[row - 1].list_of_cells[col - 1].cell_content[letter] = (int)tmp_string[letter];
    }
    return true;
}

bool def_var(Table *table, Selection *actual_selection, Variable *list_of_variables, int *position, const char *string) {
    // saves content from actual selection to a variable

    refill_table(table, actual_selection);
    if (actual_selection->selection[0] != actual_selection->selection[2] || actual_selection->selection[1] != actual_selection->selection[3]) return false;

    int position_of_number = *position + 5;
    char number[2] = "";

    number[0] = string[position_of_number];
    number[1] = '\0';
    char *p;

    long int ret;
    ret = strtol(number, &p, 10);

    if (p == NULL) return false;
    if (ret < 0 || ret > 9) return false;

    char tmp_string[100] = "";
    double curr_number = 0.0;
    int length_of_content = table->list_of_rows[actual_selection->selection[0] - 1].list_of_cells[actual_selection->selection[1] - 1].length_of_cell_content;

    for (int letter = 0; letter < length_of_content; letter++) {
        tmp_string[letter] = (char)table->list_of_rows[actual_selection->selection[0] - 1].list_of_cells[actual_selection->selection[1] - 1].cell_content[letter];
    }
    curr_number = (double)strtold(tmp_string, &p);

    if (*p == '\0') {
        list_of_variables[ret].number_f = curr_number;
        list_of_variables[ret].type_of_var = 0;
    } else {
        char *new_string = malloc(sizeof(char) * (length_of_content + 1));
        if (new_string == NULL) return false;

        for (int letter = 0; letter < length_of_content; letter++) {
            new_string[letter] = (char)table->list_of_rows[actual_selection->selection[0] - 1].list_of_cells[actual_selection->selection[1] - 1].cell_content[letter];
        }
        new_string[length_of_content] = '\0';
        list_of_variables[ret].string = new_string;
        list_of_variables[ret].type_of_var = 1;
    }
    *position = *position + 6;
    return true;
}

bool use_var(Table *table, Selection *actual_selection, Variable *list_of_variables, int *position, const char *string) {
    // set content of cell from variable

    refill_table(table, actual_selection);

    int row_from = actual_selection->selection[0];
    int row_to = actual_selection->selection[2];
    int col_from = actual_selection->selection[1];
    int col_to = actual_selection->selection[3];

    int position_of_number = *position + 5;
    char cislo[2] = "";
    cislo[0] = string[position_of_number];
    cislo[1] = '\0';
    char *p;

    long int ret;
    ret = strtol(cislo, &p, 10);

    if (p == NULL) return false;
    if (ret < 0 || ret > 9) return false;

    char new_cell_content[MAX_ARGUMENT_LENGTH] = "";
    int length_of_cell_content = 0;

    if (list_of_variables[ret].type_of_var == 0) {
        length_of_cell_content = snprintf(new_cell_content, MAX_ARGUMENT_LENGTH, "%g", (float)list_of_variables[ret].number_f);
    } else {
        length_of_cell_content = (int)strlen(list_of_variables[ret].string);
        strcpy(new_cell_content, list_of_variables[ret].string);
    }
    if (length_of_cell_content >= 0 && length_of_cell_content < MAX_ARGUMENT_LENGTH) {
        for (int row = row_from - 1; row < row_to; row++) {
            for (int col = col_from - 1; col < col_to; col++) {

                if (table->list_of_rows[row].list_of_cells[col].length_of_cell_content != length_of_cell_content - 1) {
                    int *new_content = realloc(table->list_of_rows[row].list_of_cells[col].cell_content, sizeof(int) * (length_of_cell_content));
                    if (new_content == NULL) return false;
                    table->list_of_rows[row].list_of_cells[col].cell_content = new_content;
                }

                for (int znak = 0; znak < length_of_cell_content; znak++) {
                    table->list_of_rows[row].list_of_cells[col].cell_content[znak] = (int)new_cell_content[znak];
                }
                table->list_of_rows[row].list_of_cells[col].length_of_cell_content = length_of_cell_content;
            }
        }
    }
    *position = *position + 6;
    return true;
}

bool inc_var(Variable *list_of_variables, int *position, const char *string) {
    // increments variable value by 1, if it doesnt exist, sets to 1

    int position_of_number = *position + 5;
    char cislo[2] = "";
    cislo[0] = string[position_of_number];
    cislo[1] = '\0';
    char *p;

    long int ret;
    ret = strtol(cislo, &p, 10);

    if (p == NULL) return false;
    if (ret < 0 || ret > 9) return false;

    if (list_of_variables[ret].type_of_var == 0) {
        list_of_variables[ret].number_f = list_of_variables[ret].number_f + 1;

    } else if (list_of_variables[ret].type_of_var == 1){
        free(list_of_variables[ret].string);
        list_of_variables[ret].type_of_var = 0;
        list_of_variables[ret].number_f = 1;
    }else {
        list_of_variables[ret].type_of_var = 0;
        list_of_variables[ret].number_f = 1.0;
    }

    *position = *position + 6;
    return true;
}

bool set_var(Selection *actual_selection, Variable *list_of_variables, int *position) {
    // sets variable for selection from actual selection

    // last, 10th, is only for selection
    for (int i = 0; i < 4; i++) {
        list_of_variables[10].selection[i] = actual_selection->selection[i];
    }

    *position = *position + 4;
    return true;
}

bool set_var_use(Selection *actual_selection, Variable *list_of_variables, int *position) {
    // sets actual selection from selection variable

    // last, 10th, is only for selection
    for (int i = 0; i < 4; i++) {
        actual_selection->selection[i] = list_of_variables[10].selection[i];
    }

    *position = *position + 4;
    return true;
}

bool swap_cells (Table *table, int row1, int col1, int row2, int col2) {
    // helper function for swapping of two cells

    // initialization of temporary swap space
    int *tmp_cell_content = table->list_of_rows[row1].list_of_cells[col1].cell_content;
    int tmp_length_of_content = table->list_of_rows[row1].list_of_cells[col1].length_of_cell_content;

    // swap
    table->list_of_rows[row1].list_of_cells[col1].cell_content = table->list_of_rows[row2].list_of_cells[col2].cell_content;
    table->list_of_rows[row1].list_of_cells[col1].length_of_cell_content = table->list_of_rows[row2].list_of_cells[col2].length_of_cell_content;

    table->list_of_rows[row2].list_of_cells[col2].cell_content = tmp_cell_content;
    table->list_of_rows[row2].list_of_cells[col2].length_of_cell_content = tmp_length_of_content;

    return true;
}

bool max_min(Table *table, Selection *actual_selection, int *position, const char *string) {
    // finds max/min and changes actual selection to it

    int n_of_rows = actual_selection->selection[2] - actual_selection->selection[0] + 1;
    int n_of_cols = actual_selection->selection[3] - actual_selection->selection[1] + 1;

    if (n_of_rows == 1 && n_of_cols == 1) {
        *position = *position + 5;
        return true;
    }

    int min_row = actual_selection->selection[0] - 1;
    int min_col = actual_selection->selection[1] - 1;

    int max_row = actual_selection->selection[2] - 1;
    int max_col = actual_selection->selection[3] - 1;

    double min_number;
    double max_number;
    bool found_max = false;
    bool found_min = false;
    bool found_number = false;

    if (string[*position + 2] == 'a') {
        for (int row = min_row; row <= max_row; row++){
            for (int col = min_col; col <= max_col; col++) {

                found_max = false;
                char tmp_string[MAX_ARGUMENT_LENGTH] = "";

                for (int znak = 0; znak < table->list_of_rows[row].list_of_cells[col].length_of_cell_content; znak++) {
                    tmp_string[znak] = (char)table->list_of_rows[row].list_of_cells[col].cell_content[znak];
                }
                tmp_string[table->list_of_rows[row].list_of_cells[col].length_of_cell_content + 1] = '\0';

                char *p;
                double curr_number = 0.0;
                curr_number = strtod(tmp_string, &p);

                if (*p == '\0') {
                    if (!found_number) {
                        max_number = curr_number;
                        found_number = true;
                        found_max = true;
                    }
                    if (curr_number > max_number) {
                        found_max = true;
                        max_number = curr_number;
                    }
                    if (found_max) {
                        actual_selection->selection[0] = row + 1;
                        actual_selection->selection[2] = row + 1;
                        actual_selection->selection[1] = col + 1;
                        actual_selection->selection[3] = col + 1;
                        found_max = false;
                    }
                }
            }
        }
    }

    if (string[*position + 2] == 'i') {
        for (int row = min_row; row <= max_row; row++){
            for (int col = min_col; col <= max_col; col++) {
                char tmp_string[MAX_ARGUMENT_LENGTH] = "";

                for (int znak = 0; znak < table->list_of_rows[row].list_of_cells[col].length_of_cell_content; znak++) {
                    tmp_string[znak] = (char)table->list_of_rows[row].list_of_cells[col].cell_content[znak];
                }
                tmp_string[table->list_of_rows[row].list_of_cells[col].length_of_cell_content + 1] = '\0';

                char *p;
                double curr_number = 0.0;
                curr_number = strtod(tmp_string, &p);

                if (*p == '\0') {
                    if (!found_number) {
                        min_number = curr_number;
                        found_number = true;
                        found_min = true;
                    }
                    if (curr_number < min_number) {
                        found_min = true;
                        min_number = curr_number;
                    }
                    if (found_min) {
                        actual_selection->selection[0] = row + 1;
                        actual_selection->selection[2] = row + 1;
                        actual_selection->selection[1] = col + 1;
                        actual_selection->selection[3] = col + 1;
                        found_min = false;
                    }
                }
            }
        }
    }
    *position = *position + 5;
    return true;
}

bool find_str(Table *table, Selection *actual_selection, int *position, const char *string, int *position_of_next_bracket) {
    // finds first cell that contains STR and sets selection on it

    // where STR starts
    *position_of_next_bracket = *position + 6;

    while (string[*position_of_next_bracket] != ']' || string[*position_of_next_bracket - 1] == 92) {
        *position_of_next_bracket = *position_of_next_bracket + 1;
        if (string[*position_of_next_bracket] == ']' && string[*position_of_next_bracket - 1] !=  '\\') {
            break;
        }
    }
    char tmp_string[MAX_ARGUMENT_LENGTH] = "";
    for (int i = *position + 6, j = 0; i < *position_of_next_bracket; i++, j++) {
        tmp_string[j] = string[i];
    }
    tmp_string[*position_of_next_bracket] = '\0';
    *position = *position + 1;
    int n_of_rows = actual_selection->selection[2] - actual_selection->selection[0] + 1;
    int n_of_cols = actual_selection->selection[3] - actual_selection->selection[1] + 1;

    if (n_of_rows == 1 && n_of_cols == 1) {
        char content_of_cell[MAX_ARGUMENT_LENGTH] = "";

        for (int x = 0; x < table->list_of_rows[actual_selection->selection[0] - 1].list_of_cells[actual_selection->selection[1] - 1].length_of_cell_content; x++) {
            content_of_cell[x] = (char)table->list_of_rows[actual_selection->selection[0] - 1].list_of_cells[actual_selection->selection[1] - 1].cell_content[x];
        }
        content_of_cell[table->list_of_rows[actual_selection->selection[0] - 1].list_of_cells[actual_selection->selection[1] - 1].length_of_cell_content] = '\0';

        if (strstr(tmp_string, content_of_cell)) {
            *position = *position_of_next_bracket+1;
            return true;
        } else {
            return false;
        }

    } else {
        int min_row = actual_selection->selection[0];
        int min_col = actual_selection->selection[1];
        int max_row = actual_selection->selection[2];
        int max_col = actual_selection->selection[3];

        for (int row = min_row - 1; row < max_row; row++){
            for (int col = min_col - 1; col < max_col; col++) {

                char content_of_cell[MAX_ARGUMENT_LENGTH] = "";
                for (int znak = 0; znak < table->list_of_rows[row].list_of_cells[col].length_of_cell_content; znak++) {
                    content_of_cell[znak] = (char)table->list_of_rows[row].list_of_cells[col].cell_content[znak];
                }

                if(strstr(content_of_cell, tmp_string) != NULL) {
                    actual_selection->selection[0] = row + 1;
                    actual_selection->selection[1] = col + 1;
                    actual_selection->selection[2] = row + 1;
                    actual_selection->selection[3] = col + 1;
                    *position = *position_of_next_bracket+1;
                    return true;

                }
            }
        }
        return false;
    }
}

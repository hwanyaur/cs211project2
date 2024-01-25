/*main.c*/

//
// Yaurie Hwang
// Northwestern University
// CS 211, Winter 2023
//
// Hours spent on this MF assignment: 12
//
// not sure who thought this would take 3-6 hours....
// be fr who is spending 3 hours on this besides the people
// who cheat or use chatgpt... reading and understanding
// the FIFTEEN PAGES of instructions alone takes like 1 hour
// i feel lied to
//

// #include files
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "analyzer.h"
#include "ast.h"
#include "database.h"
#include "parser.h"

// your functions
void print_schema(struct Database *db) {
  int n = db->numTables;

  printf("**DATABASE SCHEMA**\n");
  printf("Database: %s\n", db->name);
  for (int i = 0; i < n; ++i) { // each table
    struct TableMeta tb = db->tables[i];
    
    printf("Table: %s\n", tb.name);
    printf("  Record size: %d\n", tb.recordSize);

    for (int j = 0; j < tb.numColumns; ++j) { // each column
      struct ColumnMeta col = tb.columns[j];

      // determining column type
      int colTypeNum = col.colType;
      char colType[10];

      if (colTypeNum == 1) {
        strcpy(colType, "int");
      } else if (colTypeNum == 2) {
        strcpy(colType, "real");
      } else if (colTypeNum == 3) {
        strcpy(colType, "string");
      }

      // determining column index status
      int colIndexNum = col.indexType;
      char colIndex[16];

      if (colIndexNum == 0) {
        strcpy(colIndex, "non-indexed");
      } else if (colIndexNum == 1) {
        strcpy(colIndex, "indexed");
      } else {
        strcpy(colIndex, "unique indexed");
      }

      printf("  Column: %s, %s, %s\n", col.name, colType, colIndex);
    }
  }
  printf("**END OF DATABASE SCHEMA**\n");
}

void print_ast(struct QUERY *query) {
  printf("**QUERY AST**\n");

  // SELECT
  struct SELECT *select = query->q.select; // select struct
  printf("Table: %s\n", select->table);
  struct COLUMN *curr =
      select->columns; // curr to iterate through lists of columns

  while (curr != NULL) {
    if (curr->function == -1) {
      printf("Select column: %s.%s\n", curr->table, curr->name);
    }
    else if (curr-> function == 0) {
      printf("Select column: MIN(%s.%s)\n", curr->table, curr->name);
    }
    else if (curr-> function == 1) {
      printf("Select column: MAX(%s.%s)\n", curr->table, curr->name);
    }
    else if (curr-> function == 2) {
      printf("Select column: SUM(%s.%s)\n", curr->table, curr->name);
    }
    else if (curr-> function == 3) {
      printf("Select column: AVG(%s.%s)\n", curr->table, curr->name);
    }
    else if (curr-> function == 4) {
      printf("Select column: COUNT(%s.%s)\n", curr->table, curr->name);
    }
    curr = curr->next;
  }

  // JOIN
  struct JOIN *join = query->q.select->join; //
  if (join == NULL) {
    printf("Join (NULL)\n");
  } else {
    printf("Join %s On %s.%s = %s.%s\n", join->table, join->left->table,
           join->left->name, join->right->table, join->right->name);
  }

  // WHERE
  struct WHERE *where = query->q.select->where;
  if (where == NULL) {
    printf("Where (NULL)\n");
  } else {
    struct EXPR *expr = where->expr;
    char operator[6];
    if (expr->operator== 0) {
      strcpy(operator, "<");
    } else if (expr->operator== 1) {
      strcpy(operator, "<=");
    } else if (expr->operator== 2) {
      strcpy(operator, ">");
    } else if (expr->operator== 3) {
      strcpy(operator, ">=");
    } else if (expr->operator== 4) {
      strcpy(operator, "=");
    } else if (expr->operator== 5) {
      strcpy(operator, "<>");
    } else if (expr->operator== 6) {
      strcpy(operator, "like");
    }

    if (expr->litType == 2) {                  // if string literal
      if (strchr(expr->value, '\'') != NULL) { // contains single quotes
        printf("Where %s.%s %s \"%s\"\n", expr->column->table,
               expr->column->name, operator, expr->value);
      } else { //doesn't contain single quotes
        printf("Where %s.%s %s \'%s\'\n", expr->column->table,
               expr->column->name, operator, expr->value);
      }

    } else { //not a string literal
      printf("Where %s.%s %s %s\n", expr->column->table,
             expr->column->name, operator, expr->value);
    }
  }

  //ORDER BY
  struct ORDERBY *ord = query->q.select->orderby; //
  if (ord == NULL) {
    printf("Order By (NULL)\n");
  } else {
    
    char asc[5];
    if (ord->ascending == true) {
      strcpy(asc, "ASC"); 
    }
    else {
      strcpy(asc, "DESC"); 
    }
    
    if (ord->column->function == -1) {
      printf("Order By %s.%s %s\n", ord->column->table, ord->column->name, asc);
    }
    else if (ord->column-> function == 0) {
      printf("Order By MIN(%s.%s) %s\n", ord->column->table, ord->column->name, asc);
    }
    else if (ord->column-> function == 1) {
      printf("Order By MAX(%s.%s) %s\n", ord->column->table, ord->column->name, asc);
    }
    else if (ord->column-> function == 2) {
      printf("Order By SUM(%s.%s) %s\n", ord->column->table, ord->column->name, asc);
    }
    else if (ord->column-> function == 3) {
      printf("Order By AVG(%s.%s) %s\n", ord->column->table, ord->column->name, asc);
    }
    else if (ord->column-> function == 4) {
      printf("Order By COUNT(%s.%s) %s\n", ord->column->table, ord->column->name, asc);
    }
  }


  //LIMIT
struct LIMIT *limit = query->q.select->limit; //
  if (limit == NULL) {
    printf("Limit (NULL)\n");
  } else {
    printf("Limit %d\n", limit->N);
    }

  //INTO
  struct INTO *into = query->q.select->into; //
  if (into == NULL) {
    printf("Into (NULL)\n");
  } else {
    printf("Into %s\n", into->table);
    }
  printf("**END OF QUERY AST**\n");
} 

void execute_query(struct Database *db, struct QUERY *query) {
  //Select * from ridership where Riders >= 1000 order by count(ID) limit 100;
  struct SELECT *select = query->q.select;
  
  char filename[DATABASE_MAX_ID_LENGTH + 50]; 
  strcpy(filename, db->name);
  strcat(filename, "/");

  char tbName[DATABASE_MAX_ID_LENGTH + 1];
  strcpy(tbName,select->table);
  struct TableMeta *tb = db->tables; 
  int bufferSize = 0;

  for (int i = 0; i < db->numTables; i++) { //iterate over tables to find table name match
    if (strcasecmp(tbName, tb[i].name) ==0) {
      strcpy(tbName, tb[i].name);
      strcat(filename, tbName);
      strcat(filename, ".data"); //filename complete
      //printf("filename = %s\n", filename);
    }
  }

  FILE* input = fopen(filename, "r");
  
  if (input == NULL) {
    printf("**ERROR: Wrong file\n");
    exit(-1);
  }
  else { //input exists, not NULL
    
    bufferSize = tb->recordSize + 33;
    //printf("buffer size = %d\n", bufferSize);
    char* buffer = (char*) malloc(sizeof(char) *bufferSize);
    //printf("buffer = %s\n", buffer);
    
    if (buffer == NULL){
      printf("**ERROR: out of memory\n");
      exit(-1);
      }
 
    for (int j = 0; j < 5; j++) { //gets each line
      fgets(buffer, bufferSize, input); 
      printf("%s", buffer);
    }
//select * from Stations;
    
    
  
  }
  }
      

// int main()
int main() {
  char dbName[DATABASE_MAX_ID_LENGTH + 1];
  struct Database *db;

  printf("database? ");
  scanf("%s", dbName);
  db = database_open(dbName);

  if (db == NULL) {
    printf("**Error: unable to open database ‘%s’\n", dbName);
    exit(-1);
  } else {
    print_schema(db);

    parser_init(); // initializes parser
    char query[100];

    while (true) {
      printf("query? ");
      FILE *input = stdin;
      struct TokenQueue *tokens = parser_parse(input);

      if (tokens == NULL) {
        if (parser_eof() == false) { // syntax error, output error message
          continue;
        } else { // added $
          break;
        }
        printf("hi\n");
        continue;
      }      // end of if NULL
      else { // tokens have real tokens
        struct QUERY *query = analyzer_build(db, tokens);
        if (query == NULL) { // semantic error
          continue;
        }
        print_ast(query);

        
        execute_query(db, query);
        analyzer_destroy(query);
      }
    } // end of while (true)
    database_close(db);
  }
}

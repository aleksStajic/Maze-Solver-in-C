#define _CRT_SECURE_NO_WARNINGS

/* Preprocessor directives to include libraries */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "mazesolver.h"

/*
 Contains the majority of the maze solver logic
 PRE:       The file specified by the MACRO in lines 47 and 60 exists in the project
 POST:      Prints data about maze solutions.
 RETURN:    None
 */
void process()
{
  /* Variables */
  int dimension = 0;
  FILE* maze_file = NULL;
  maze_cell** maze = NULL;
  char outputstring[BUFFER]; // holds output for shortest/cheapest path

  /* Declare variables for holding generated path information */
  char** paths = NULL;
  int paths_found = 0;

  /* Opens and parses the maze file.  Replace the first parameter of fopen with
    different file names defined in the preprocessor section of the header file
    to test your mazesolver with our sample mazes. */
  maze_file = fopen(MAZE3877, "r");

  if (maze_file) {

    /* Calls the functions that:
      a) get the size of the maze and stores it in the dimension variable
      b) copies the maze into memory */
	  dimension = get_maze_dimension(maze_file); //tested dimension return value with MAZE1 and found that it works
	  maze = parse_maze(maze_file, dimension); //tested contents of maze array with MAZE1 and found that it works

  }
  else {
    fprintf(stderr, "Unable to parse maze file: %s\n", MAZE1);
    system("pause");
  }

  /* Traverses maze and generates all solutions */
  generate_all_paths(&paths, &paths_found, maze, dimension, 0, 0, ""); 
  //We initially start searching at maze[0][0], and the 'current path' is empty

  /* Calculates and displays required data */
  sprintf(outputstring, "Total number of solutions: %d\n", paths_found);

  //call the construct_shortest_path_info function to obtain the shortest path found in the maze
  construct_shortest_path_info(paths, paths_found, outputstring);
  
  //call the construct_cheapest_path_info function to obtain the cheapest path found in the maze
  //this function will write the cheapest path and its cost into the output string
  construct_cheapest_path_info(paths, paths_found, outputstring);

  printf("%s", outputstring);
}

/*
 Acquires and returns the maze size.  Since the maze is always a square, all we
 need to do is find the length of the top row!
 PARM:      maze_file is a pointer to a filestream
 PRE:       maze_file is an initialized pointer to a correctly-formatted maze file
 POST:      maze_file's internal pointer is set to beginning of stream
 RETURN:    length of the first line of text in the maze file EXCLUDING any EOL characters
            ('\n' or '\r') and EXCLUDING the string-terminating null ('\0') character.
 */
int get_maze_dimension( FILE* maze_file )  {

  int  dimension = 0;
  char line_buffer[BUFFER];

  dimension = strlen( fgets ( line_buffer, BUFFER, maze_file ) );

  fseek( maze_file, 0, SEEK_SET );

    /* Checks if text file was created in Windows and contains '\r'
	   IF TRUE reduce strlen by 2 in order to omit '\r' and '\n' from each line
	   ELSE    reduce strlen by 1 in order to omit '\n' from each line */
  if ( strchr( line_buffer, '\r' ) != NULL ) {
	  dimension = dimension - 2;
	  return dimension;
  } else {
	  dimension = dimension - 1;
	  return dimension;
  }
}

/*
 Parses and stores maze as a 2D array of maze_cell.  This requires a few steps:
 1) Allocating memory for a 2D array of maze_cell, e.g., maze_cell[rows][columns]
    a) Dynamically allocates memory for 'dimension' pointers to maze_cell, and assign
	   the memory (case as a double pointer to maze_cell) to maze, which is a
	   double pointer to maze_cell (this makes the maze[rows] headers)
	b) For each row of the maze, dynamically allocate memory for 'dimension' maze_cells
	   and assign it (cast as a pointer to maze_cell) to maze[row]
 2) Copying the file to the allocated space
    a) For each row obtained from the file using fgets and temporarily stored in line_buffer
	   i) For each of the 'dimension' columns in that row
	   ii)Assign the character from the file to the struct, and set the struct to unvisited
 3) Returns the double pointer called maze.
 PARAM:  maze_file pointer to an open filestream
 PARAM:  dimension pointer to an int
 PRE:    maze_file is a pointer to a correctly-formatted maze file
 POST:   dimension contains the correct size of the square maze
 POST:   maze contains a dynamically allocated copy of the maze stored in the maze_file
 RETURN: maze, a maze_cell double pointer, which points to 'dimension' single pointers
         to maze_cell, each of which points to 'dimension' maze_cell structs.
 */
maze_cell** parse_maze( FILE* maze_file, int dimension ) {
	/* Variables */
  char        line_buffer[BUFFER];
  int         row = 0;
  int         column = 0;
  maze_cell** maze = NULL; 

  /* Allocates memory for correctly-sized maze */
  maze = (maze_cell**) calloc(dimension, sizeof(maze_cell*));

  for ( row = 0; row < dimension; ++row ) {
	  maze[row] = (maze_cell*)calloc(dimension, sizeof(maze_cell));
	  //maze[row] is a pointer to a row of maze_cell structures (1D array of structures)
  }

  /* Copies maze file to memory */
	row = 0;
  while ( fgets ( line_buffer, BUFFER, maze_file ) ) { //gets one line of maze
    for ( column = 0; column < dimension; ++column ) {
		maze[row][column].character = line_buffer[column]; //writes all characters in a given row into 2D array
		maze[row][column].visited = UNVISITED; //sets the unvisited property in the struct to UNVISITED
	  }
    row++;
  }
 return maze;
}

/**
 Generates all paths through a maze recursively.
 PARAM:     pointer to a 2D array of all generated paths
            (pass the 2D array by reference - actual parameter to be modified by function)
 PARAM:     pointer to number of paths found
            (pass the integer by reference - actual parameter to be modified by function)
 PARAM:     pointer to a 2D array of maze_cell
 PARAM:     dimension of the square maze
 PARAM:     starting position row
 PARAM:     starting position column
 PARAM:     pointer to a string of chars which contains the 'current' path
 PRE:       maze contains a representation of the maze
 PRE:       dimension contains the correct dimension of the maze
 PRE:       row contains a valid row coordinate
 PRE:       column contains a valid column coordinate
 PRE:       path contains a sequence of characters (the first time you invoke this
            function, the passed parameter should be an empty string, e.g., "" (not NULL)
 POST:      IF current coordinate is not out of maze bounds &&
               current coordinate is not a wall
            THEN path contains current coordinate
 POST:      IF current coordinate is at maze finish line (right boundary)
            THEN paths contains the path from start to finish.
 POST:      dereferenced pathset contains all paths found
 POST:      dereferences numpaths contains the number of paths found
 */
void generate_all_paths( char*** pathsetref, int* numpathsref, maze_cell** maze, int dimension, int row, int column, char* path )
{
	/* Variables */
	int path_length   = 0;
	char* new_point  = NULL;
	char* new_path   = NULL;
	//char temp = maze[row][column].character;

  /* Checks for base cases - this corresponds to backtracking through the maze */
  //Base case 1: we have hit a wall
  //Base case 2: out of maze bounds
  //Base case 3: current position in maze has been visited before
  //Simply return if we hit one of the base cases (there are more than 1)
  if (column > dimension - 1 || column < 0 || row > dimension - 1 || row < 0 || maze[row][column].character == '*' || maze[row][column].visited == VISITED) {
    return;
  } 

  /* Otherwise deals with the recursive case.  Pushes the current coordinate onto the path
	  and checks to see if the right boundary of the maze has been reached
	  IF   right boundary reached
	  THEN path is added to the list as a successful path and function returns
	  ELSE the current location is marked as used and the search continues
	      in each cardinal direction using a recursive call using these steps:
		1. get length of path
		2. allocate space for a larger new path
		3. allocate space for a new point to add to the path
		4. assign the value in the maze cell to the new point
		5. concatenate old path to new path
		6. concatenate new point to new path */	
  else {
	  path_length = strlen( path ); //gets the length of current path
	  new_path = ( char * ) calloc( path_length + 2, sizeof( char ) ); //allocates memory for the next path_character
	  new_point = ( char * ) calloc( 2, sizeof( char ) );
	  new_point[0] = maze[row][column].character; //gets the next character in the current trajectory
	  if ( path_length ) { //if path_length != 0
		new_path = strcat( new_path, path ); //concatenates old path into new_path
	  }
	  new_path = strcat( new_path, new_point ); //concatenates the next path point into the new_path
          free(new_point); //frees temporary pointer

	//If we have reached the right boundary, store associated path into path matrix
          if ( column == ( dimension - 1 ) ) {
	    /* 1. Reallocate memory in global paths array to make room
			    for a new solution string
			 2. Copy the solution path to the location of new string
			 3. Increment paths counter */
	    *pathsetref = ( char** ) realloc ( *pathsetref, ( (*numpathsref) + 1 ) * sizeof( char* ) );
            (*pathsetref)[*numpathsref] = ( char* ) calloc( strlen( new_path ) + 1, sizeof( char ));
	    strcpy( (*pathsetref)[*numpathsref], new_path );
	    (*numpathsref)++;
        return;
    } else {
		  /* 1. Mark point as visited
			   2. Recursively search in each direction using the new path, and the same pathsetref and numpathsref
			   3. Mark point as unvisited */
	    maze[row][column].visited = VISITED;
	    generate_all_paths(pathsetref, numpathsref, maze, dimension, row, column + 1, new_path); //move horizontally to the right
	    generate_all_paths(pathsetref, numpathsref, maze, dimension, row, column - 1, new_path); //move horizontally to the left
	    generate_all_paths(pathsetref, numpathsref, maze, dimension, row + 1, column , new_path); //move vertically down
	    generate_all_paths(pathsetref, numpathsref, maze, dimension, row - 1, column, new_path); //move vertically up
	    maze[row][column].visited = UNVISITED; //start at new point
	    return;
    }
  }
}

/*
 Calculates the cost for a given path.
 Examples:
 ""    -> 0
 "2"   -> 2
 "871" -> 16
 PARM:   path_string is a pointer to an array of char
 PRE:    path_string is a pointer to a null-terminated array of char (a string)
 RETURN: the 'cost' of the path.
 */
int path_cost ( char* path_string )
{
  int cost = 0;
  int i = 0;
  
  while (path_string[i] != '\0') {
	  cost += (path_string[i] - '0'); //trick for converting an integer stored as a character into an integer
	  i++;
  }
  
  return cost;
}

/*
 Writes the shortest path in the paths global variable into the outputbuffer parameter,
 where the shortest path has the fewest cells.
 In the event of a tie, use the first 'shortest' path found.
 Uses this format:
 "Shortest path: XXXXXXXX"
 Don't forget to add a null character at the end.
 PARAM: 2D array containing all paths found
 PARAM: number of paths in pathset
 PARAM: outputbuffer is a character array of sufficient length
 POST:  outputbuffer contains information about the shortest path
 */
void construct_shortest_path_info ( char** pathset, int numpaths, char* outputbuffer ){
	int path_length;
	int min_length = -1;
	int i;
	int min_length_index = 0;

	for (i = 0; i < numpaths; i++) {
		path_length = strlen(pathset[i]);
		if (min_length == -1) { //after checking only first possible path, this automatically becomes first min path
			min_length = path_length;
		}
		else if (min_length > path_length) { //if we have found a new minimum length, place this into the output string
			min_length = path_length; //sets new min_length
			min_length_index = i; //stores the row index of the smallest length path
		}
	}

	strcat(outputbuffer, "Shortest path: ");
	strcat(outputbuffer, pathset[min_length_index]);
	strcat(outputbuffer, "\0"); //concatenates null character ---> Doesn't C already do this automatically?
}

/*
 Writes the cheapest path in the paths global variable into the outputbuffer parameter,
 where the cheapest path has the lowest sum value of its cells.
 In the event of a tie, use the first 'cheapest' path found.
 Uses this format:
 "Cheapest path: XXXXXXXX
  Cheapest path cost: 9999"
 Don't forget to use a newline and to add a null character at the end.
 Use sprintf to put an integer into a buffer which can then be concatenated using strcat.
 PARAM: 2D array containing all paths found
 PARAM: number of paths in pathset
 PARAM: outputbuffer is a character array of sufficient length
 POST:  outputbuffer contains information about the cheapest path and its cost
 */
void construct_cheapest_path_info ( char** pathset, int numpaths, char* outputbuffer ){
	int cost;
	int min_cost = -1;
	int i;
	int min_cost_index = 0;
	char int_buffer[BUFFER];

	for (i = 0; i < numpaths; i++) {
		cost = path_cost(pathset[i]);
		if (min_cost == -1) {
			min_cost = cost;
		}
		else if (min_cost > cost) {
			min_cost = cost; 
			min_cost_index = i; //saves the row index of the minimum cost path
		}
	}
	sprintf(int_buffer, "%d", min_cost);
	strcat(outputbuffer, "\nCheapest path: ");
	strcat(outputbuffer, pathset[min_cost_index]); //prints the minimum cost path into outputbuffer
	strcat(outputbuffer, "\nCheapest path cost: ");
	strcat(outputbuffer, int_buffer);
	strcat(outputbuffer, "\n\0"); //appends NULL character at the end of outputbuffer
}

/* End of file */

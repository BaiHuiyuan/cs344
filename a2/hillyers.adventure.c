// hillyers.adventure.c

/*******************************************************************************
* Author:       Shawn S Hillyer
* Date:         April 24, 2016
* Course:       OSU CSS 344-400: Assignment 02
* Description:  
*
*
*******************************************************************************/

/* Includes */
#include <dirent.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h> // For getpid()

/* Constants */
#define MIN_CONNECTIONS 3
#define MAX_CONNECTIONS 6
#define MAX_ROOMS 7
#define TOTAL_ROOM_NAMES 10
#define MAX_NAME_LEN 20

const char *USERNAME = "hillyers";
const char *ROOM_NAMES[] = {
	  "Hell's Kitchen"
	, "Manhattan"
	, "Lincoln Tunnel"
	, "SOHO"
	, "Brooklyn Bridge"
	, "Roosevelt Island"
	, "Washington Heights"
	, "Queens"
	, "Rikers Island"
	, "Bronx"
 };

/* Enums */
enum room_type { START_ROOM=0, END_ROOM=1, MID_ROOM=2 };

/* Structs */
struct Room {
	char name[MAX_NAME_LEN];           // a name from the ROOM_NAMES[] array
	int connections;         // Tally of actual number of connections
	char conn_names[MAX_CONNECTIONS][MAX_NAME_LEN]; // array of strings
	enum room_type type;
	int connected_matrix[MAX_ROOMS]; // 1=connected, 0=not
};

/* Forward-declarations */
struct Room;
void shuffle_array(int arr[], int n);
void swap(int arr[], int i, int j);
// int rand_int_in_range(int min, int max);
void generate_connections(struct Room rooms[], int size);
void write_to_file(struct Room rooms[], int size);
char * get_directory_name();
void print_rooms(struct Room rooms[], int size);

/*******************************************************************************
* generate_rooms()
* Create 7 different Room structs, one room per file, in a directory called
* <username>.rooms.<process id>
* <username> is hardcoded (See constant USERNAME near header)
* Each room has a room name, 3-6 connections (random), and a type. Connections
* are randomly assigned. All connections must be bidirectional. No room type 
* can be used more than once.
*******************************************************************************/
void generate_rooms(struct Room rooms[], int size) {
	int i, j;
	// Assign names by shuffling an array of values [0..TOTAL_ROOM_NAMES] and
	// selecting first size integers in the array during the init loop
	int rand_indices[10];
	for (i = 0; i < TOTAL_ROOM_NAMES; i++) {
		rand_indices[i] = i;
	}
	shuffle_array(rand_indices, TOTAL_ROOM_NAMES);

	// Initialize each room before making connections
	for (i = 0; i < size; i++) {
		// Set # of connections to 0 and set all connections to 0 except self
		rooms[i].connections = 0;
		for (j = 0; j < MAX_ROOMS; j++) {
			rooms[i].connected_matrix[j] = 0;
		}

		// Set name to name in unique and random index
		strcpy(rooms[i].name, ROOM_NAMES[rand_indices[i]]);
		
		// Set type; random order already so pick the first two as start/end
		enum room_type current_type;
		if (i == 0) current_type = START_ROOM;
		else if (i == 1) current_type = END_ROOM;
		else current_type = MID_ROOM; // All other elements are middle
		rooms[i].type = current_type;
	}

	generate_connections(rooms, size);
	write_to_file(rooms, size);
}



/*******************************************************************************
* generate_connections()
* Subroutine for generate_rooms. Takes an array of rooms and creates logical
* connections between each of them randomly.
* NOTE: We know that a graph with 7 nodes and every node having 3 undirected 
* edges must be connected, so no need to verify that a path from START to END 
* actually exists. This can be proved by diagramming connections on a 7 vertex
* graph with only 2 edges per node and then adding a third to any one node. This
* will force the graph to be connected as the two circuits are joined.
*******************************************************************************/
void generate_connections(struct Room rooms[], int size) {
	int i,         // Loop counter 
	    rand_indx, // Random index to attempt a connection
	    extra;     // Number of extra connections to make

	// Iterate through rooms; connect to 3+extra other rooms
	for (i = 0; i < size; i++) {
		// Give a chance to add extra connections. Using 0-3 causes a float up
		// leaving no nodes with only 3; 0 to 2 tends to not
		extra = rand() % 2; // rand_int_in_range(0, 3);

		// While we still need to make connections, try to make more
		while (rooms[i].connections < MIN_CONNECTIONS + extra) {
			rand_indx = rand() % size;
			// Try to connect to random nodes until it succeeds
			do {
				// Increment forward if random node failed to connect
				rand_indx = (++rand_indx) % size;
			} while( !connect_rooms(rooms, i, rand_indx) );
		}
	}
	return;
}


/*******************************************************************************
* connect_rooms()
* Subroutine for generate_connections. Associates two rooms together by creating
* a bidirectional linkage between the two rooms. 
*******************************************************************************/
int connect_rooms(struct Room rooms[], int i, int j) {
	if (i == j) {
		// printf("Connect rooms[%d] to itself! Derp\n", i); // TODO delete debug statement
		return 0; // cannot connect to self	
	} 
	// If already connected, return 0 (false)
	else if (rooms[i].connected_matrix[j] == 1) {
		// printf("Connect rooms[%d] to rooms[%d] but ALREADY CONNECTED\n", i, j); // TODO delete debug statement
		return 0;
	} 
	// else if target connection already at max connections return false
	else if (rooms[j].connections == MAX_CONNECTIONS || rooms[i].connections == MAX_CONNECTIONS) {
		// printf("Connect rooms[%d] to rooms[%d] but FULL.\n", i, j);
		return 0;
	}
	else {
		rooms[i].connected_matrix[j] = 1;
		rooms[j].connected_matrix[i] = 1;
		rooms[i].connections = rooms[i].connections + 1;
		rooms[j].connections = rooms[j].connections + 1;
		return 1;
	}
}


/*******************************************************************************
* shuffle_array()
* Shuffles an array of values
* 
* cite: www.vogella.com/tutorials/JavaAlgorithmsShuffle/article.html
*******************************************************************************/
void shuffle_array(int arr[], int n) {
	// Get random number from i to n, swap element at i with ele at n
	int i;
	for (i = 0; i < n; i++) {
		int random = i + (rand() % (n - i));
		swap(arr, i, random);
	}
}


/*******************************************************************************
* swap()
* Swaps two elements of an array
* cite: www.vogella.com/tutorials/JavaAlgorithmsShuffle/article.html
*******************************************************************************/
void swap(int arr[], int i, int j) {
	int temp = arr[i];
	arr[i] = arr[j];
	arr[j] = temp;
}


/*******************************************************************************
* write_to_file()
* 
* 
* 
*******************************************************************************/
void write_to_file(struct Room rooms[], int size) {
	int i, j;
	char * directory = get_directory_name();

	// Make the directory using mkdir()
	// Cite: http://linux.die.net/man/3/mkdir
	mkdir(directory, 0777); // make directory rwx/rwx/rwx for user/grp/world
	
	// Move into the new directory and start writing the files
	// http://linux.die.net/man/3chdir
	chdir(directory);

	// For each Room in the array, write the data
	for (i = 0; i < size; i++) {
		// Open the file for writing, then write each section per specification
		FILE * room_file = fopen (rooms[i].name, "w");
		fprintf(room_file, "ROOM NAME: %s\n", rooms[i].name);
		
		// Loop through the connected_matrix to get strings to write
		int n = 1;
		for (j = 0; j < MAX_ROOMS; j++) {
			if (rooms[i].connected_matrix[j] == 1) {
				fprintf(room_file, "CONNECTION %d: %s\n", n++, rooms[j].name);
			}
		}
		
		// Print the room type depending on setting and an extra newline at end
		fprintf(room_file, "ROOM TYPE: ");
		int type = rooms[i].type;
		switch (type) {
			case START_ROOM: fprintf(room_file, "START_ROOM\n"); 
					break;
			case END_ROOM: fprintf(room_file, "END_ROOM\n"); 
					break;
			case MID_ROOM: fprintf(room_file, "MID_ROOM\n"); 
					break;
		}
		// close the file
		fclose(room_file);
	}

	chdir(".."); // Go back up a directory
}

/*******************************************************************************
* ()
* 
* 
* 
*******************************************************************************/
void read_from_file(struct Room rooms[], int size) {
	int i = 0;
	char * directory = get_directory_name();
	chdir(directory);

	// Get a list of all the files using opendir()
	// Cite: stackoverflow.com/questions/612097, 
	// Cite: pubs.opengroup.org/onlinepubs/007908775/xsh/dirent.h.html

	DIR *dir;
	struct dirent *entry;

	// We've already changed into the directory we want, so just open cwd
	if ((dir = opendir (".")) != NULL) {
		// While there are files in the directory, get & process next entry
		// Also ends loop if we read too many files (shouldn't happen!)
		while ((entry = readdir(dir)) != NULL && i < size) {
			// Skip entry if filename is "." or ".." (current or parent directory)
			if (strcmp(entry->d_name, ".") == 0 
				|| strcmp(entry->d_name, "..") == 0) {
				continue; 
			}

			// Open the current file for reading, then read in each part of struct
			FILE * room_file = fopen(entry->d_name, "r");
			int buff_size = 250;
			char line[buff_size];
			char word_1[20];
			char contents[buff_size]; // used to store name of connections/type
			int conn_number; // Not really using, just dmping from sscanf
			int n = 0;
			
			rooms[i].connections = 0; // init before entering next loop

			// While we can read lines from the file, parse and fill rooms[i] struct
			// Cite: sscanf basics: www.cplusplus.com/reference/cstdio/sscanf
			while(fgets(line, buff_size - 1, room_file)) {
				// If the line is the ROOM NAME, read name from file into struct
				if(strstr(line, "ROOM NAME") != NULL) {
					sscanf(line, "ROOM NAME: %19[ a-zA-Z']s", rooms[i].name);
					// printf("rooms[%d].name=%s\n", i, rooms[i].name);
				}

				// If line is a CONNECTION line, increment .connections and read
				// the name of connection to the array of connection strings
				else if (strstr(line, "CONNECTION") != NULL) {
					// Read up to 19 characters into contents[] and copy into struct arr
					// This saves room for the null byte. 
					sscanf(line, "CONNECTION %d: %19[ a-zA-Z']s", &conn_number, contents);
					char * conn_name = rooms[i].conn_names[n++]; // readability purposes
					strcpy(conn_name, contents);
					rooms[i].connections++;
				}

				// If line is ROOM TYPE line, parse the type and set type as appropriate
				else if (strstr(line, "ROOM TYPE") != NULL) {
					sscanf(line, "ROOM TYPE: %s", contents);
					if (strstr(contents, "MID_ROOM") != NULL) {
						rooms[i].type = MID_ROOM;
					}
					else if (strstr(contents, "START_ROOM") != NULL) {
						rooms[i].type = START_ROOM;
					}
					else if (strstr(contents, "END_ROOM") != NULL) {
						rooms[i].type = END_ROOM;
					}
					else {
						printf("This is bad!!");
						return;
					}					
				}
			} // end while fgets
			// Close file, move to next element of array
			fclose(room_file);
			i++;
		} // end while readdir
		closedir(dir);
	} // end if read worked

	else {
		// Could not open directory
		perror("");
		return;
	}
}


/*******************************************************************************
* ()
* 
* 
* 
*******************************************************************************/
char * get_directory_name() {
	pid_t pid = getpid();
	char roomstr[] = ".rooms.";

	// Copy three parts of string into a string so we can make directory
	// Cite: www.cplusplus.com/reference/cstdio/sprintf
	int buff_size = strlen(USERNAME) + 20 + strlen(roomstr);
	char* directory = malloc(sizeof(char) * buff_size); // 

	sprintf(directory, "%s%s%d", USERNAME, roomstr, pid);
	return directory;
}

/*******************************************************************************
* ()
* 
* 
* 
*******************************************************************************/
void play_game() {
	char * directory = get_directory_name();

	// Allocate array of Room structs to read data back in
	struct Room * rooms = malloc (sizeof(struct Room) * MAX_ROOMS);
	read_from_file(rooms, MAX_ROOMS);
	print_rooms(rooms, MAX_ROOMS);
}


// TODO: Get rid of this debug funciton and related calls
void print_rooms(struct Room rooms[], int size) {
	int i, j;
	for (i = 0; i < size; i++) {
		printf("ROOM NAME: %s\n", rooms[i].name);
		for (j = 0; j < rooms[i].connections; j++) {
			printf("CONNECTION %d: %s\n", (j+1), rooms[i].conn_names[j]);
		}
		printf("ROOM TYPE: %d\n\n", rooms[i].type);
	}
}


/*******************************************************************************
* main()
* Creates a series of files that hold descriptions of "rooms" and how they are
* connected; offer to player an interface for playing game using generated
* rooms; exit and display path taken by player
*******************************************************************************/
int main(int argc, char const *argv[]) {
	// Seed random numbers
	srand(time(NULL));

	struct Room rooms[MAX_ROOMS];
	// Generate rooms
	generate_rooms(rooms, MAX_ROOMS);

	// Initiate player input loop / main 'game' logic
	play_game();

	// Game loop ends when exit found, return 0 for success
	return 0;
}
/// threads.c
/// Author's Name: Himani Munshi (hxm3443@rit.edu)
/// CSCI- 243 - 05
/// Date: 12/8/2018 

#define _DEFAULT_SOURCE
#define GROUND 0	  //Represents the ground level 
#define VERTICAL 1       //Represents a part of the building
#define HORIZONTAL 2    //Represents the roof
#define EMPTY_SPACE 3  //Represents the empty space within a building
#define HIT 4         //Represents that a missile has been hit  
#define MAX_SPEED 2  //Macro used for random speed calculation

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <ncurses.h>
#include <unistd.h>
#include <pthread.h>

char defender[81];           //String containing defender's name 
char attacker[81];          //String containing attacker's name
int NUM_BUFSIZE = 256;     //Buffer to store number of missiles
int TALLEST_BUILDING;     //Stores the height of the tallest building 
int HEIGHT;              //Stores the height of the terminal
int WIDTH;              //Stores the width of the terminal 
long int MAX_MISSILES; //Stores the maximum number of missiles 
int **storeLayout;    //2-D matrix containing different values to distinguish
                     //what has been hit by the missile

pthread_mutex_t p_mutex = PTHREAD_MUTEX_INITIALIZER;

//A Structure representing the defense shield
typedef struct {
    char *shield;
    int row;
    int col;
} defenseShield;

defenseShield *SHIELD;    //Global variable for the defense shield 

//A Structure representing the missile 
typedef struct {
    char *c;
    int row;
    int col;
    long speed;
} missile;

/// This function uses the ncurses library to print the entire city layout
/// on the terminal. 
/// The curses library code operates and controls the terminal display. 
/// @param cityLayout contains the entire city layout configuration after the
///        given text file has been read.  
void printCityLayout( int cityLayout[] ) {
    initscr();
    cbreak();
    
    move( 0, 5 );
    printw( "Enter 'q' to quit at end of attack, or control-C" );
    refresh();

    int i;

    for(i = 0; i < WIDTH; i++ ) {
        if( cityLayout[i] == 2 ) {
            move( HEIGHT-2, i );
	    storeLayout[HEIGHT-2][i] = GROUND;
            printw( "_" );
            refresh();
        }
        else {
            int count = 1, flag = 0, k, prevVal, nextVal;

            if( i > 0 && ( cityLayout[i] == cityLayout[i-1] ) )
                count++;

            for( k = i; k < WIDTH-1; k++ ) {
                if( cityLayout[i] == cityLayout[k+1] )
                    count++;
                else
                    break;
	    }

            if( i > 0 && ( cityLayout[i] != cityLayout[i-1] ) ) {
                prevVal = cityLayout[i-1];
            }
	    else if( i > 0 && ( cityLayout[i] == cityLayout[i-1]) ) {
                int check = i;

                while( check > 0 && cityLayout[i] == cityLayout[check] ) {
                    check--;
                }

                prevVal = cityLayout[check];   
            }
            else
                prevVal = 2;

            if( k < WIDTH-1 ) {
                nextVal = cityLayout[k+1];
            }
            else
                nextVal = 2;

            int j, storeCurr = i;
            if( prevVal < cityLayout[i] ) {
                if( ( cityLayout[i] != cityLayout[i-1] ) ) {
                    for( j = 2; j < cityLayout[i]; j++ ) {
	                storeLayout[HEIGHT-j][i] = VERTICAL;
                        move( HEIGHT-j, i );
                        printw( "|" );
                        refresh();
                    }

                    i++;
                }
                else if ( ( cityLayout[i] == cityLayout[i-1] ) ) {
                    flag = 1;
                    if( nextVal < cityLayout[storeCurr] ) {
                        for( int idx = 0; idx < count-2; idx++ ) {
			    storeLayout[HEIGHT-cityLayout[storeCurr]][i] = HORIZONTAL;
                            move( HEIGHT-cityLayout[storeCurr], i );
                            printw( "_" );
                            refresh();
                            i++;
                        }

                        if( cityLayout[i] == cityLayout[storeCurr] ) {
                            for( j = 2; j < cityLayout[i]; j++ ) {
		                storeLayout[HEIGHT-j][i] = VERTICAL;
                                move( HEIGHT-j, i );
                                printw( "|" );
                                refresh();
                            }
		        }
		    } 
                    else if( nextVal > cityLayout[storeCurr] ) {
                        for( int idx = 0; idx < count-1; idx++ ) {
		            storeLayout[HEIGHT-cityLayout[storeCurr]][i] = HORIZONTAL;
                            move( HEIGHT-cityLayout[storeCurr], i );
                            printw( "_" );
                            refresh();
                            i++;
			}

                        for( int idx = 2; idx < cityLayout[i]; idx++ ) {
		            storeLayout[HEIGHT-idx][i] = VERTICAL;
                            move( HEIGHT-idx, i );
                            printw( "|" );
                            refresh();
                        }
                    }
		}

                if( flag == 1 )
                    continue;

                if( nextVal < cityLayout[storeCurr] ) {
                    for( int idx = 0; idx < count-2; idx++ ) {
		        storeLayout[HEIGHT-cityLayout[storeCurr]][i] = HORIZONTAL;
                        move( HEIGHT-cityLayout[storeCurr], i );
                        printw( "_" );
                        refresh();
                        i++;
                    }

                    for( int idx = 2; idx < cityLayout[i]; idx++ ) {
			storeLayout[HEIGHT-idx][i] = VERTICAL;
                        move( HEIGHT-idx, i );
                        printw( "|" );
                        refresh();
                    }
		}
                else if( nextVal > cityLayout[storeCurr] ) {
                    for( int idx = 0; idx < count-1; idx++ ) {
		        storeLayout[HEIGHT-cityLayout[storeCurr]][i] = HORIZONTAL;
                        move( HEIGHT-cityLayout[storeCurr], i );
                        printw( "_" );
                        refresh();
                        i++;
                    }

                    for( int idx = 2; idx < cityLayout[i]; idx++ ) {
		        storeLayout[HEIGHT-idx][i] = VERTICAL;
                        move( HEIGHT-idx, i );
                        printw( "|" );
                        refresh();
                    }
		}
	    }
            else if( prevVal > cityLayout[i] ) {
                if( nextVal < cityLayout[storeCurr] ) {
                    for( int idx = 0; idx <= count-2; idx++ ) {
	                storeLayout[HEIGHT-cityLayout[storeCurr]][i] = HORIZONTAL;
                        move( HEIGHT-cityLayout[storeCurr], i );
                        printw( "_" );
                        refresh();
                        i++;
                    }
                    
		    for( int idx = 2; idx < cityLayout[i]; idx++ ) {
			storeLayout[HEIGHT-idx][i] = VERTICAL;
                        move( HEIGHT-idx, i );
                        printw( "|" );
                        refresh();
                    }
		}
                else if( nextVal > cityLayout[storeCurr] ) {
                    for( int idx = 0; idx <= count-1; idx++ ) {
			storeLayout[HEIGHT-cityLayout[storeCurr]][i] = HORIZONTAL;
                        move( HEIGHT-cityLayout[storeCurr], i );
                        printw( "_" );
                        refresh();
                        i++;
                    }

                    for( int idx = 2; idx < cityLayout[i]; idx++ ) {
			storeLayout[HEIGHT-idx][i] = VERTICAL;
                        move( HEIGHT-idx, i );
                        printw( "|" );
                        refresh();
                    }
		}
	    }
	}
    }
}

/// This function is responsible for creating one defense shield by intializing
/// the values in the structure.
/// @param col specifies the column in which the shield is to be placed 
/// @return a new structure of defense shield with initialized values 
defenseShield *make_DefenseShield( int col ) {
    defenseShield *new = (defenseShield*) malloc( sizeof(defenseShield) );
    new->shield = "#####";
    new->row = (HEIGHT-TALLEST_BUILDING)-5;
    new->col = col;

    return new;
}

/// This function is responsible for creating a missile by intializing the
/// the values in the structure.
/// @return a new structure of missile with initialized values  
missile *make_missile() {
    missile *new = malloc( sizeof(missile) );
    new->c = "|";
    new->row = 2;
    new->col = rand() % WIDTH;

    return new;
}

/// This function is responsible for printing a missile/message to the terminal.
/// @param message the message to be printed 
/// @row the row in which the message will be displayed 
/// @col the column in which the message will be displayed 
void print( char *message, int row, int col ) {
    pthread_mutex_lock( &p_mutex );
    move( row, col );
    printw( message );
    refresh();
    pthread_mutex_unlock( &p_mutex ); 
}

/// This function is responsible for the interaction that takes place between the
/// user and the curses terminal. If the user enters the right key, the shield moves to 
/// the right by one row and the same happens to the left if the user hits the left key. 
/// The shield stops moving if the user hits 'q'.
/// @param new a struct pointer for the defense shield    
void interactionWithKeys( defenseShield *new ) {
    noecho();
    cbreak();
    
    int ch;
    ch = getch();
    int flag1 = 0, flag2 = 0, flag3 = 0;

    while( 1 ) {
        switch( ch ) {
	    case 68: //KEY_LEFT
		flag1 = 1;
	        break;
	    case 67: //KEY_RIGHT
                flag2 = 1;
		break;
	    case 'q':
		flag3 = 1;
		break;
       }

       if( flag1 == 1 && ( (new->col) - 1 ) >= 0 ) {
           print( "     ", new->row, new->col );
	   new->col -= 1;
	   print( new->shield, new->row, new->col );
          
       }
       else if( flag2 == 1 && ( (new->col) + 1 ) < (WIDTH-4) ) {
	   print( "     ", new->row, new->col );
	   new->col += 1;
	   print( new->shield, new->row, new->col );
       }
       else if( flag3 == 1 ) {
           break;   
       }

       ch = getch();
       flag1 = 0;
       flag2 = 0;
       flag3 = 0; 
    }
}

/// This is the defense thread which makes a shield, prints it on the terminal, and
/// responds according to the user input. 
void *defenseThread() {
    print( SHIELD->shield, SHIELD->row, SHIELD->col );
    interactionWithKeys(SHIELD);
    
    return NULL;
}

/// This is the missile thread which makes a missile, prints it on the terminal
/// in a random column, moves it through that column from the starting row till
/// it hits the ground or the building. When there is a hit, it replaces the missile
/// with special characters to represent that the missile has been exploded and 
/// smoke has been produced. 
void *missileThread() {
    missile *new = make_missile();
    for( int row = 2; row <= HEIGHT-2; row++ ) {
        print( " ", (new->row)-1, new->col );

        if( (new->row == (SHIELD->row)-1) && ( (new->col >= (SHIELD->col)) && (new-> col < (SHIELD->col) + 5 ) ) ) {
            print( "?", new->row, new->col );
	    print( "*", SHIELD->row, new->col );
	 
	    break;   
        }
        else if( (storeLayout[new->row][new->col] == GROUND) || (row == HEIGHT-2) ) {
            print( "?", new->row, new->col );
	    print( "*", (new->row)+1, new->col);
	  
	    break;
	}
	else if( storeLayout[new->row][new->col] == VERTICAL ) {
            storeLayout[new->row][new->col] = HIT;
	    print( "?", (new->row)-1, new->col );
	    print( "*", new->row, new->col );
	  
	    break;    
	}
	else if( storeLayout[new->row][new->col] == HORIZONTAL ) {
	    storeLayout[new->row][new->col] = EMPTY_SPACE;
	    storeLayout[(new->row)+1][new->col] = HIT;
	    print( "?", new->row, new->col );
            print( "*", (new->row)+1, new->col);
	   
	    break;
	}
	else if( storeLayout[new->row][new->col] == HIT ) {
	    storeLayout[new->row][new->col] = EMPTY_SPACE;
	    storeLayout[(new->row)+1][new->col] = HIT;
	    print( " ", (new->row)-1, new->col );
	    print( "?", new->row, new->col );
	    print( "*", (new->row)+1, new->col );
	  
	    break;
	}
        else {
	    print( new->c, new->row, new->col );
	}

	new->row++;
	usleep( ((rand() % 100) + 170)*1000 );
    }

    if( new != NULL )
        free( new );

    return NULL;
}

/// This is the attack thread which is responsible for the parallel running 
/// execution of the missiles. This is where the attack threading happens by
/// creating all the missiles and then joining all the missiles so that they
/// can run simultaneously. If maximum number of missiles is 0, then the 
/// program loops infinitely till the user enters control-C.
void *attackThread() {
    if( MAX_MISSILES != 0 ) {
        pthread_t thread[MAX_MISSILES];

        for( int i = 0; i < MAX_MISSILES; i++ ) {
	    pthread_create( &thread[i], NULL, missileThread, NULL );
	    sleep( 1 );
        }

	for( int i = 0; i < MAX_MISSILES; i++ ) {
	    pthread_join( thread[i], NULL );
	}

	move( 2, 5 );
	printw( "The %s attack has ended.", attacker );
	refresh();
   }
   else {
       while(1) {
           pthread_t thread_id;
           pthread_create( &thread_id, NULL, missileThread, NULL);
           sleep(1);	   
       }
   }

   return NULL;
}

/// This function is responsible for freeing the storeLayout which is 
/// a 2-D matrix containing information about the ground and the building.
/// @param storeLayout A 2-D matrix containing 0, 1, 2, and 3.
//         0 for ground
//         1 if it is a part of the building 
//         2 if it is the roof 
//         3 if it is a empty space within the building  
void free_storeLayout( int **storeLayout ) {
    if( storeLayout != NULL ) {
        for( int i = 0; i < HEIGHT; i++ ) {
            free( storeLayout[i] );
        } 
        free( storeLayout );
    }
}

/// This is the main() function where all the reading takes place. After 
/// reading the config file, the city layout is printed on the terminal and 
/// the height of the tallest building has been computed. The defense shield 
/// is then created followed by the attack thread. 
/// Finally, it frees all the memory which was allocated in the program.  
int main( int argc, char **argv ) {
    if( argc != 2 ) {
        fprintf( stderr, "usage: ./threads config-file\n" );
	exit( EXIT_FAILURE );
    }

    struct winsize w;
    ioctl( 0, TIOCGWINSZ, &w );
    int width = w.ws_col;
    int height = w.ws_row;
    
    HEIGHT = height;
    WIDTH = width;

    FILE *fp;
    fp = fopen( argv[1], "r" );

    if( fp == NULL ) {
        perror( argv[1] );
	exit( EXIT_FAILURE );
    }
 
    int hasRead1 = 0, hasRead2 = 0, hasRead3 = 0, hasRead4 = 0, count = 0, 
	layoutSize = 0, flag1 = 0, flag2 = 0;
    int cityLayout[width];
    char *storeNum;
    int ch;
    long int maxMissiles;

    storeNum = malloc( sizeof( char ) * NUM_BUFSIZE );
    storeLayout = malloc( sizeof( int* ) * HEIGHT );

    for( int i = 0; i < HEIGHT; i++ ) {
        storeLayout[i] = malloc( sizeof(int) * WIDTH );
    }	

    for( int i = 0; i < HEIGHT; i++ ) {
        for( int j = 0; j < WIDTH; j++ ) {
	    storeLayout[i][j] = EMPTY_SPACE;
        }
    }	
   
    while( ( ch = fgetc(fp) ) != EOF ) {
        if( ch == '#' ) {
	    while( ch != '\n' ) {
	        ch = fgetc(fp);
	    } 
	}
    
        //reading defender's name
	else if( hasRead1 == 0 ) {    
	    if( isspace( ch ) ) {
                continue;    
	    }
                
	    int i = 0;

	    defender[i] = ch;
	    i++;

            while( (ch = fgetc(fp)) != '\n' ) {
                defender[i] = ch;
                i++;
            }
	    defender[i] = '\0';

	    hasRead1 = 1;
	}
    
        //reading attacker's name
	else if( hasRead2 == 0 ) {
            if( isspace( ch ) ) {
                continue;
            }

            int i = 0;

	    attacker[i] = ch;
	    i++;

            while( ( (ch = fgetc(fp)) != '\n' ) ) {
                attacker[i] = ch;
                i++;
            }
	    attacker[i] = '\0';

	    hasRead2 = 1;
	}

        //reading the maximum number of missiles 
	else if( hasRead3 == 0 ) {
            int check1 = 0, check2 = 0;

	    if( ch == '-' ) {
		flag2 = 1;
		break;
	    }

            if( !isdigit(ch) && !isspace(ch) ) {
		flag1 = 1;
	        break;	
	    }
            
	    if( isspace(ch) ) 
	        continue;
            

            if( isdigit(ch) ) {	    
                storeNum[count] = (char) ch;
                count++;
		check1 = 1;
	    }
	    
            while( ( ch = fgetc(fp) ) != '\n' ) {

		if( count >= NUM_BUFSIZE ) {
		    storeNum = realloc( storeNum, (NUM_BUFSIZE+1) );
		    NUM_BUFSIZE++;
		}

	        if( isspace(ch) ) {
	            while( ( ch = fgetc(fp) ) != '\n' );
		    break; 
	        }
                
		if( isdigit(ch) ) { 
                    storeNum[count] = (char) ch;
	            count++;
		    check2++;
		}
	    }

	    storeNum[count] = '\0';

	    if( (check1 == 1) && (check2 == count-1) && storeNum[0] != '\0' ) {
		hasRead3 = 1; 
	        sscanf( storeNum, "%ld" , &maxMissiles);
		MAX_MISSILES = maxMissiles; 
            }
	
	}

	else if( hasRead4 == 0 ) {
	    char storeN[HEIGHT+1];
	    int i = 0;
            
            if( isspace(ch) ) {
	        continue;
	    }
  
	    if( isdigit(ch) ) {
                storeN[i] = ch;
	        i++;
            }

	    while( !isspace( ch = fgetc(fp) ) ) {
                if( isdigit(ch) ) {
	            storeN[i] = ch;
		    i++;
		}
 		else {
		    continue; 
		}
	    }
	    storeN[i] = '\0';
	   
            if( storeN[0] != '\0' ) {
	        sscanf( storeN, "%d", &cityLayout[layoutSize] );
	        layoutSize++;
	    }

            while( ( layoutSize != width ) && ( ( ch = fgetc(fp) ) != EOF ) ) {
                if( isspace( ch ) || ch == '\n' ) 
		    continue;
		else {
		    char store[HEIGHT+1];
                    int j = 0;

	            if( isdigit(ch) ) {	    
		        store[j] = ch;
                        j++;
		    }
		    
		    while( ( ch = fgetc(fp) ) != '\n' && !isspace(ch) && ch != EOF ) {
		        store[j] = ch;
			j++;
		    }
                    store[j] = '\0';
                    
		    if( store[0] != '\0' ) {
		        sscanf( store, "%d", &cityLayout[layoutSize]);  
		        layoutSize++;
		    }
		}
	    }

	    hasRead4 = 1;
	}
    }
    
    if( layoutSize < WIDTH-1 ) {
        for( int i = layoutSize; i < WIDTH; i++ )  
	    cityLayout[i] = 2;
    } 

    if( hasRead1 == 0 ) 
        fprintf( stderr, "Error: missing defender name.\n" );
    else if( hasRead2 == 0 ) 
	fprintf( stderr, "Error: missing attacker name.\n" );
    else if( flag1 == 1 )
        fprintf( stderr, "Error: missing missile specification.\n" );
    else if( flag2 == 1 )
        fprintf( stderr, "Error: missile specification < 0.\n" );
    else if( hasRead3 == 0 ) 
        fprintf( stderr, "Error: missing missile specification.\n" );
    else if( hasRead4 == 0 ) 
	fprintf( stderr, "Error: missing city layout.\n");

    if( !hasRead1 || !hasRead2 || !hasRead3 || !hasRead4 ) {
        if( storeNum != NULL )
	    free( storeNum );

	free_storeLayout( storeLayout );

	if( fp != NULL )
            fclose( fp );
        
	exit( EXIT_FAILURE );
    }
 
    int max = 0;
    for( int i = 0; i < WIDTH; i++ ) {
        if( cityLayout[i] > max )
            max = cityLayout[i];
    }

    TALLEST_BUILDING = max;

    printCityLayout( cityLayout );
    
    SHIELD = make_DefenseShield( (WIDTH/2) + 5 );
    pthread_t thread_id;
    pthread_create( &thread_id, NULL, defenseThread, NULL );

    pthread_t attack_id;
    pthread_create( &attack_id, NULL, attackThread, NULL );
    pthread_join( attack_id, NULL );
    pthread_join( thread_id, NULL );
        
    
    move( 4, 5 );
    printw( "The %s defense has ended.", defender );
    refresh();
    
    move( 5, 5 );
    printw( "hit enter to close..." );
    refresh();
    

    getch();
    endwin();
    
    if( SHIELD != NULL ) {
        free( SHIELD );
    }

    if( storeNum != NULL )
        free( storeNum );
    
    free_storeLayout( storeLayout );

    if( fp != NULL ) 
        fclose( fp );
    
    return 0;
}

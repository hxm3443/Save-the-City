#define _DEFAULT_SOURCE
#define GROUND 0
#define VERTICAL 1
#define HORIZONTAL 2
#define EMPTY_SPACE 3 
#define HIT 4
#define MAX_SPEED 2

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <curses.h>
#include <unistd.h>
#include <pthread.h>

char defender[81];
char attacker[81];
int NUM_BUFSIZE = 256; 
int TALLEST_BUILDING;
int HEIGHT;
int WIDTH;
int MAX_MISSILES; 
int **storeLayout;

pthread_mutex_t p_mutex = PTHREAD_MUTEX_INITIALIZER;

typedef struct {
    char *shield;
    int row;
    int col;
} defenseShield;

defenseShield *SHIELD;

typedef struct {
    char *c;
    int row;
    int col;
    long speed;
} missile;
 
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
            for( k = i; k < WIDTH; k++ ) {
                if( cityLayout[i] == cityLayout[k+1] )
                    count++;
                else
                    break;

            }

            if( i > 0 && ( cityLayout[i] != cityLayout[i-1] ) ) {
                prevVal = cityLayout[i-1];
            }
            else if( i > 0 && (cityLayout[i] == cityLayout[i-1]) ) {
                int check = i;
                while( cityLayout[i] == cityLayout[check] ) {
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
    
    refresh();
    
}

defenseShield *make_DefenseShield( int col ) {
    defenseShield *new = malloc( sizeof(defenseShield) );
    new->shield = "#####";
    new->row = (HEIGHT-TALLEST_BUILDING)-5;
    new->col = col;

    return new;
}

missile *make_missile() {
    missile *new = malloc( sizeof(missile) );
    new->c = "|";
    new->row = 2;
    new->col = rand() % WIDTH;

    return new;
}

void print( char *message, int row, int col ) {
    pthread_mutex_lock( &p_mutex );
    move( row, col );
    printw( message );
    refresh();
    pthread_mutex_unlock( &p_mutex ); 
}

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

void *defenseThread() {
    SHIELD = make_DefenseShield( (WIDTH/2)+5 );

    print( SHIELD->shield, SHIELD->row, SHIELD->col );
    interactionWithKeys(SHIELD);
 

    if( SHIELD != NULL ) 
        free( SHIELD );

    return NULL;
}

void *missileThread() {
    missile *new = make_missile();
    for( int row = 2; row <= HEIGHT-2; row++ ) {
        

	print( " ", (new->row)-1, new->col );
        if( (new->row == (SHIELD->row)-1) && ( (new->col >= (SHIELD->col)) && (new-> col <= (SHIELD->col) + 5 ) ) ) {
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
	//change within sleep
	usleep( ((rand() % 100) + 170)*1000 );
	
	
    }
    if( new != NULL )
        free( new );

    return NULL;
}

void destroy_missile( missile *m ) {
   if( m != NULL ) {
       free( m );
   } 
}

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


void free_storeLayout( int **storeLayout ) {
    if( storeLayout != NULL ) {
        for( int i = 0; i < HEIGHT; i++ ) {
            free( storeLayout[i] );
        } 
        free( storeLayout );
    }
}

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
 
   
    int hasRead1 = 0, hasRead2 = 0, hasRead3 = 0, hasRead4 = 0, count = 0, layoutSize = 0;
    int cityLayout[width];
    char *storeNum;
    storeNum = malloc( sizeof( char ) * NUM_BUFSIZE );
    int ch;
    long int maxMissiles;
    
    storeLayout = malloc( sizeof(int*) * HEIGHT );
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

        //reading the maxmum number of missiles 
	else if( hasRead3 == 0 ) {
            int check1 = 0, check2 = 0;

            if( !isdigit(ch) && !isspace(ch) ) { 
	        fprintf( stderr, "Error: missing missile specification." );
	        break;	
	    }
            
	    if( isspace(ch) ) 
	        continue;
            
	    if( isdigit(ch) == '-' ) {
		fprintf( stderr, "Error: missile specification < 0." );
		break;
	    }

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
	        }
                
	        //check if you need to reallocate memory for storeNum
		if( isdigit(ch) ) { 
                    storeNum[count] = (char) ch;
	            count++;
		    check2++;
		}
	    }

	    if( (check1 == 1) && (check2 == count-1) ) {
		hasRead3 = 1; 
	        sscanf( storeNum, "%ld" , &maxMissiles);
		MAX_MISSILES = maxMissiles; 
            }
	    else {
		fprintf( stderr, "Error: missing missile specification." );
		break;
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
            
	    sscanf( storeN, "%d", &cityLayout[layoutSize] );
	    layoutSize++;
	    
            while( ( layoutSize != width ) && ( ( ch = fgetc(fp) ) != EOF ) ) {
                if( isspace( ch ) || ch == '\n' ) 
		    continue;
		else {
                    char store[height+1];
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

		    sscanf( store, "%d", &cityLayout[layoutSize]);  
		    layoutSize++;
		}
	    }

	    hasRead4 = 1;
	}
    }

    
    //check if it is width or width-1
    if( layoutSize < WIDTH-1 ) {
        for( int i = layoutSize; i < WIDTH; i++ )  
	    cityLayout[i] = 2;
    } 

    if( hasRead1 == 0 ) 
        fprintf( stderr, "Error: missing defender name." );
    else if( hasRead2 == 0 ) 
	fprintf( stderr, "Error: missing attacker name." );
    else if( hasRead4 == 0 ) 
	fprintf( stderr, "Error: missing city layout.");
    if( !hasRead1 || !hasRead2 || !hasRead3 || !hasRead4 ) {
	
	if( storeNum != NULL )
	    free( storeNum );
	free_storeLayout( storeLayout );
	if( fp != NULL )
            fclose( fp );
        //error
	exit( EXIT_FAILURE );
    }

    for( unsigned int i = 0; defender[i] != '\0'; i++ ) {
        printf( "%c", defender[i] );
    }

    printf( "\n" );

    for( unsigned int i = 0; attacker[i] != '\0'; i++ ) {
        printf( "%c", attacker[i] );
    }

    printf( "\n" );

    printf( "%ld", maxMissiles );
    
    printf( "\n" );
    
    for( int i = 0; i < WIDTH; i++ ) {
	//size == width 
        printf( "%d ", cityLayout[i] );
    }
    
    int max = 0;
    for( int i = 0; i < WIDTH; i++ ) {
        if( cityLayout[i] > max )
            max = cityLayout[i];
    }

    TALLEST_BUILDING = max;

    printCityLayout( cityLayout );
    
    
    SHIELD = make_DefenseShield( WIDTH/2 );
    pthread_t thread_id;
    pthread_create( &thread_id, NULL, defenseThread, NULL );

    pthread_t attack_id;
    pthread_create( &attack_id, NULL, attackThread, NULL );
    pthread_join( attack_id, NULL );
    pthread_join( thread_id, NULL );

    move( 4, 5 );
    printw( "The %s defense has ended.", defender );
    refresh();
    print( "hit enter to close...", 5, 5 );

    getch();
    endwin();
    
    if( storeNum != NULL )
        free( storeNum );
    
    free_storeLayout( storeLayout );

    if( fp != NULL ) 
        fclose( fp );
    
    return 0;
}

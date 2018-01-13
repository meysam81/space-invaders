#include <stdio.h> // C library to perform Input/Output operations
#include <stdlib.h> // random
#include <curses.h> // visual programming c
#include <time.h> // time of system
#include <unistd.h>

#define MAX_BOMBS 1000
/* Space Invaders */

/* header file for invaders.c */

struct player {
	int r,c; // row, column
	char ch; // symbol
};

struct alien {
	int r,c; // row, column
	int pr,pc; // previous row, previous column; we keep both the current position and the last position
	int alive; // 1=alive 0=destroyed
	char direction; // 'l'=left 'r'=right
	char ch; // symbol
};

struct shoot {
	int r,c; // row, column
	int active; /* 1=active 0=inactive */
	char ch; // symbol
};

struct bomb {
	int r,c; // row, column
	int active; // 1=active 0=inactive
	int loop; // used to prevent alien from flashing when bomb is dropped
	char ch; // symbol
};

struct options { // keeps the score
	int overall,alien,shots,bombs,
        bombchance; // should we drop bomb?
};
	
void menu(struct options *settings); // list for user
void gameover(int win); // won or lost?

/* The main function handles user input, the game visuals, and checks for win/loss conditions */
int main() {
    // struct for each type
   struct player tank;
   struct alien aliens[30]; // nomber of invaders in 3 rows of 10 columns
   struct shoot shot[3]; // number of shots the player can have
   struct bomb bomb[MAX_BOMBS]; // a big number of bombs to drop down
   struct options settings; // score of the game
   unsigned int input, loops=0, i=0, j=0, currentshots=0, currentbombs=0, 
    currentaliens=30; // number of initial ALIVE aliens; if reaches zero we are won
   int random=0, score=0, win=-1;
   char tellscore[30];
   
   initscr();// Initialize the window screen for control of the C program
   clear(); // the screen is cleared completely on the next call to wrefresh() for that window and repainted from scratch.
   noecho(); // Don't echo any keypresses
   cbreak(); // making characters typed by the user immediately available to the program
   nodelay(stdscr,1); /* The nodelay option causes getch to be a non-blocking call. If no input is ready, getch returns ERR. If disabled (bf is FALSE), getch waits until a key is pressed. */
   keypad(stdscr,1); /*The keypad option enables the keypad of the user's terminal. If enabled (bf is TRUE), the user can press a function key (such as an arrow key) and wgetch returns a single value representing the function key, as in KEY_LEFT. If disabled (bf is FALSE), curses does not treat function keys specially and the program has to interpret the escape sequences itself*/
   srand(time(NULL));  // seed the random so that it's pseudo randoms are more random-like

   /* Set default settings */
   settings.overall = 15000;
   settings.alien = 12;
   settings.shots = 3;
   settings.bombs = 10;
   settings.bombchance = 5; // lower this if you don't want a lot of bombs; current value is 5 percent

   /* Set tank settings */
    // The integer variables LINES and COLS are defined in <curses.h> and will be filled in by initscr with the size of the screen.
   tank.r = LINES - 1;
   tank.c = COLS / 2;
   tank.ch = '^';

   /* Set aliens settings */
   for (i=0; i<10; ++i) { // row 1: 10 columns, all alive and direction is right for moving
      aliens[i].r = 1;
      aliens[i].c = i*3;
      aliens[i].pr = 0;
      aliens[i].pc = 0;
      aliens[i].ch = '#';
      aliens[i].alive = 1;
      aliens[i].direction = 'r';
   }
   for (i=10; i<20; ++i) { // row 2
      aliens[i].r = 2;
      aliens[i].c = (i-10)*3;
      aliens[i].pr = 0;
      aliens[i].pc = 0;
      aliens[i].ch = '#';
      aliens[i].alive = 1;
      aliens[i].direction = 'r';
   }
   for (i=20; i<30; ++i) { // row 3
      aliens[i].r = 3;
      aliens[i].c = (i-20)*3;
      aliens[i].pr = 0;
      aliens[i].pc = 0;
      aliens[i].ch = '#';
      aliens[i].alive = 1;
      aliens[i].direction = 'r';
   }
   
   /* Set shot settings */
   for (i=0; i<3; ++i) { // we have 3 shots, with the symbol start(*)
      shot[i].active = 0;
      shot[i].ch = '*';
   }
   
   /* Set bomb settings */
   for (i=0; i<MAX_BOMBS; ++i) { // enemies bombs
      bomb[i].active = 0;
      bomb[i].ch = 'o';
      bomb[i].loop = 0; // loop: used to prevent alien from flashing when bomb is dropped
   }
   
    // consider the screen as a 2D board starting from top-left as (0, 0) which it's horizontal line is x and vertical line is y
    // now we move cursor to the right place, draw something, and so on

   /* Display game title,score header,options */
   move(0,(COLS/2)-9); // move functions moves the cursor to the place where it's parameters points to
   addstr("--SPACE INVADERS--"); // print the string in the parameter in the place of the current cursor
   move (0,1); // move to the next place for the next print
   addstr("SCORE: "); // another print
   move(0,COLS-19); // and so on
   addstr("m = menu  q = quit");
   
   while(1) { // the real game starts from here
      /* Show score */
        // go right a little bit for beauty; actually we are filling the place with spaces with the help of sprintf
      sprintf(tellscore, "%d", score);  // right pad the string with spaces to display them with even width.
      move(0,8); // move ...
      addstr(tellscore); // ... and print
      
      /* Move tank */
      move(tank.r,tank.c); // move the tanks and ...
      addch(tank.ch); // ... display
      
      /* Move bombs */
        // loop: used to prevent alien from flashing when bomb is dropped
      if (loops % settings.bombs == 0) // drop some bombs while moving the enemies
      for (i=0; i<MAX_BOMBS; ++i) {
         if (bomb[i].active == 1) {
            // don't go pass the screen
            if (bomb[i].r < LINES) { // The integer variables LINES and COLS are defined in <curses.h> and will be filled in by initscr with the size of the screen.
               if (bomb[i].loop != 0) { // we don't want the bomb to be displayed when it's position is in the position of the enemy itself, so whenever a bomb is going to drop, we show it only if it is lower than the corresponding enemy
                  move(bomb[i].r-1,bomb[i].c); // move but ...
                  addch(' '); // print nothing
               }
               else
                  ++bomb[i].loop; // now that it is dropped, we can set loop so that it IS printed in the next iteration
               
               move(bomb[i].r,bomb[i].c); // if lower that enemy: print *; else print nothing (actually prints a space)
               addch(bomb[i].ch);
               
               ++bomb[i].r; // bomb row: come down one unit
            }
            else { // if we are pass the screen! come down one unit and clear the current row
               bomb[i].active = 0;
               bomb[i].loop = 0;
               --currentbombs;
               move(bomb[i].r - 1,bomb[i].c);
               addch(' ');
            }
         }
      }
      
      /* Move shots */
      if (loops % settings.shots == 0)
      for (i=0; i<3; ++i) { // we have only 3 shots
         if (shot[i].active == 1) {
            if (shot[i].r > 0) {
               if (shot[i].r < LINES - 2) {
                  move(shot[i].r + 1,shot[i].c); // print space to wipe the previous shot so that we can see that the shot ascends to the enemy
                  addch(' ');
               }
               
               for (j=0; j<30; ++j) { // if an enemy's position coincides with our shot, wipe it from screen, meaning KILL IT
                  if (aliens[j].alive == 1 && aliens[j].r == shot[i].r && aliens[j].pc == shot[i].c) {
                     score += 20;
                     aliens[j].alive = 0;
                     shot[i].active = 0;
                     --currentshots;
                     --currentaliens;
                     move(aliens[j].pr,aliens[j].pc);
                     addch(' ');
                     break;
                  }
               }
               
               if (shot[i].active == 1) {
                  move(shot[i].r,shot[i].c); // move the cursor to the right position and ..
                  addch(shot[i].ch); // ... print whether a shot or wipe the one that passed a second ago
                  
                  --shot[i].r; // ascend the shot to the enemy
               }
            }
            else { // obvious I assume
               shot[i].active = 0;
               --currentshots;
               move(shot[i].r + 1,shot[i].c);
               addch(' ');
            }
         }
      }     
      
      /* Move aliens */
      if (loops % settings.alien == 0)
      for (i=0; i<30; ++i) { // for all 30 enemies
         if (aliens[i].alive == 1) {
            move(aliens[i].pr,aliens[i].pc); // if enemy passed the current position, wipe it
            addch(' ');
            
            move(aliens[i].r,aliens[i].c);
            addch(aliens[i].ch);
            
            aliens[i].pr = aliens[i].r;
            aliens[i].pc = aliens[i].c;
            
            /* Check if alien should drop bomb */
            random = 1+(rand()%100); // we drop bomb with a random probability
            if ((settings.bombchance - random) >= 0 && currentbombs < MAX_BOMBS) {
               for (j=0; j<MAX_BOMBS; ++j) {
                  if (bomb[j].active == 0) {
                     bomb[j].active = 1;
                     ++currentbombs;
                     bomb[j].r = aliens[i].r + 1;
                     bomb[j].c = aliens[i].c;
                     break;
                  }
               }
            }
            
            /* Set alien's next position */
            if (aliens[i].direction == 'l')
               --aliens[i].c;
            else if (aliens[i].direction == 'r')
               ++aliens[i].c;
               
            /* Check alien's next positions */
            if (aliens[i].c == COLS - 2) { // if it's the end of screen
               ++aliens[i].r;
               aliens[i].direction = 'l';
            }
            else if (aliens[i].c == 0) { // if it's the beginning of the screen
               ++aliens[i].r;
               aliens[i].direction = 'r';
            }
         }
      }
      
      
      /* See if game has been won or lost*/
      if (currentaliens == 0) {
         win = 1;
         break;
      }
      for (i=0; i<30; ++i) { // if enemies reach the bottom of the screen
         if (aliens[i].r == LINES-1) {
            win = 0;
            break;
         }
      }
      for (i=0; i<MAX_BOMBS; ++i) {
         if (bomb[i].r == tank.r && bomb[i].c == tank.c) {
            win = 0;
            break;
         }
      }    
      
      move(0,COLS-1);


/*
The refresh and wrefresh routines (or wnoutrefresh and doupdate) must be called to get actual output to the terminal, as other routines merely manipulate data structures. The routine wrefresh copies the named window to the physical terminal screen, taking into account what is already there to do optimizations. The refresh routine is the same, using stdscr as the default window. Unless leaveok has been enabled, the physical cursor of the terminal is left at the location of the cursor for that window.
*/
      refresh();
      usleep(settings.overall); // sleep for specific MILLISECONDS
      ++loops; // number of movements of the whole game, which is added in each refreshing the page and printing
      
      input = getch(); // get the inputs of the user; if he wanted the menu, give it to him/her
      move(tank.r,tank.c);
      addch(' ');
      
      /* Check input */
      if (input == 'q')
         win = 2;
      else if (input == KEY_LEFT)
         --tank.c;
      else if (input == KEY_RIGHT)
         ++tank.c;
      else if (input == ' ' && currentshots < 3) {
         for (i=0; i<3; ++i) {
            if (shot[i].active == 0) {
               shot[i].active = 1;
               ++currentshots;
               --score;
               shot[i].r = LINES - 2;
               shot[i].c = tank.c;
               break;
            }
         }
      }
      else if (input == 'm')
         menu(&settings); 
      
      if (win != -1)
         break;
      
      /* Check for valid tank position */
      if (tank.c > COLS-2)
         tank.c = COLS - 2;
      else if (tank.c < 0)
         tank.c = 0;     
   }
   
   gameover(win);
   endwin();
   return 0;
}

/* This function handles the menu options available to the user */
void menu(struct options *settings) {
   char option, buf[30];
   int new;

   clear();
   echo();
   nocbreak();
   nodelay(stdscr,0);

   move(0,0);
   addstr("1. Change overall game speed");
   move(1,0);
   addstr("2. Change alien motion speed");
   move(2,0);
   addstr("3. Change tank shot speed");
   move(3,0);
   addstr("4. Change alien bomb speed");
   move(4,0);
   addstr("5. Change alien bomb dropping frequency");
   move(5,0);
   addstr("6. Return to the game");
   move(6,0);
   addstr("7. Exit the game");
   move(8,0);
   addstr("Enter your option: ");
   refresh();
   
   while(1) {
      move(8,19);
      option = getch();
      move(9,0);
      deleteln();
      move(10,0);
      deleteln();
      move(11,0);
      deleteln();
      
      if (option == '1') {
         sprintf(buf,"Current value: %d\n", settings->overall);
         
         move(9,0);
         addstr(buf);
         move(10,0);
         addstr("Enter new value: ");
         move(10,17);
         refresh();
         getch();
         getstr(buf);
         
         new = atoi(buf);
         
         /* Check for valid new value */
         if (new < 0) {
            move(11,0);
            addstr("ERROR: Inalid value");
         }
         else {
            settings->overall = new;
         }  
      }
      else if (option == '2') {
         sprintf(buf,"Current value: %d\n", settings->alien);
         
         move(9,0);
         addstr(buf);
         move(10,0);
         addstr("Enter new value: ");
         move(10,17);
         refresh();
         getch();
         getstr(buf);
         
         new = atoi(buf);
         
         /* Check for valid new value */
         if (new <= 0) {
            move(11,0);
            addstr("ERROR: Inalid value");
         }
         else {
            settings->alien = new;
         }  
      }
      else if (option == '3') {
         sprintf(buf,"Current value: %d\n", settings->shots);
         
         move(9,0);
         addstr(buf);
         move(10,0);
         addstr("Enter new value: ");
         move(10,17);
         refresh();
         getch();
         getstr(buf);
         
         new = atoi(buf);
         
         /* Check for valid new value */
         if (new <= 0) {
            move(11,0);
            addstr("ERROR: Inalid value");
         }
         else {
            settings->shots = new;
         }  
      }
      else if (option == '4') {
         sprintf(buf,"Current value: %d\n", settings->bombs);
         
         move(9,0);
         addstr(buf);
         move(10,0);
         addstr("Enter new value: ");
         move(10,17);
         refresh();
         getch();
         getstr(buf);
         
         new = atoi(buf);
         
         /* Check for valid new value */
         if (new <= 0) {
            move(11,0);
            addstr("ERROR: Inalid value");
         }
         else {
            settings->bombs = new;
         }  
      }
      else if (option == '5') {
         sprintf(buf,"Current value: %d\n", settings->bombchance);
         
         move(9,0);
         addstr(buf);
         move(10,0);
         addstr("Enter new value: ");
         move(10,17);
         refresh();
         getch();
         getstr(buf);
         
         new = atoi(buf);
         
         /* Check for valid new value */
         if (new > 100 || new < 0) {
            move(11,0);
            addstr("ERROR: Inalid value");
         }
         else {
            settings->bombchance = new;
         }  
      }
      else if (option == '6') {
         break;
      }
      else if (option == '7') {
         endwin();
         exit(0);
      }
      else {
         move(9,0);
         addstr("ERROR: Invalid selection");
         move(8,19);
         addstr(" ");
         refresh();        
      }
   }
   
   clear();
   noecho();
   cbreak();
   nodelay(stdscr,1);
   
   move(0,(COLS/2)-9);
   addstr("--SPACE INVADERS--");
   move (0,1);
   addstr("SCORE: ");
   move(0,COLS-19);
   addstr("m = menu  q = quit");
}

/* This function handles displaying the win/lose screen */
void gameover(int win) {

   nodelay(stdscr, 0);
   
   if (win == 0) {
      clear();
      move((LINES/2)-1,(COLS/2)-5);
      addstr("YOU LOSE!");
      move((LINES/2),(COLS/2)-11);
      addstr("PRESS ANY KEY TO EXIT");
      move(0,COLS-1);
      refresh();
      getch();
   }
   
   else if (win == 1) {
      clear();
      move((LINES/2)-1,(COLS/2)-5);
      addstr("YOU WIN!");
      move((LINES/2),(COLS/2)-11);
      addstr("PRESS ANY KEY TO EXIT");
      move(0,COLS-1);
      refresh();
      getch();
   }
}

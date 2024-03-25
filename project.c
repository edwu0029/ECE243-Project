#include "stdbool.h"
#include "stdlib.h"
#include <string.h>
#include <globals.h>

// Double Buffer
volatile int pixel_buffer_start;       // global variable
short int Buffer1[240][512];  // 240 rows, 512 (320 + padding) columns
short int Buffer2[240][512];

//Game State
char gameState[20][26][3];
#define ROWS 20
#define COLS 26

// Function Declarations
void move_tile(int x, int y, int dirX, int dirY);
bool check_move_bounds(int x, int y, int dirX, int dirY);
bool check_box_move(int boxX, int boxY, int dirX, int dirY);

void plot_pixel(int x, int y, short int line_color);
void clear_screen();
void undraw_gamestate();
void draw_gamestate();
void draw_background();
void wait_for_vsync();
void erase_temp(int x, int y);
void draw_character(int x, int y);
void draw_box(int x, int y);

int main(void) {
  volatile int *pixel_ctrl_ptr = (int *)0xFF203020;
  volatile int *keys_ptr = (int *)0xFF200050;

  /* set front pixel buffer to Buffer 1 */
  *(pixel_ctrl_ptr + 1) =
      (int)&Buffer1;  // first store the address in the  back buffer
  /* initialize a pointer to the pixel buffer, used by drawing functions */
  pixel_buffer_start = *pixel_ctrl_ptr;
  clear_screen();  // pixel_buffer_start points to the pixel buffer

  /* set back pixel buffer to Buffer 2 */
  *(pixel_ctrl_ptr + 1) = (int)&Buffer2;
  pixel_buffer_start = *(pixel_ctrl_ptr + 1);  // we draw on the back buffer
  clear_screen();  // pixel_buffer_start points to the pixel buffer
  wait_for_vsync();
  draw_background();

  //Initial game state (TEMPORARY)
  gameState[15][15][0] = 'P';
  gameState[0][0][0] = 'B';
  gameState[2][5][0] = 'B';
  gameState[2][6][0] = 'B';
  gameState[19][25][0] = 'B';
  gameState[10][23][0] = 'B';
  gameState[15][17][0] = 'B';

  //Main game loop
  int characterX = 15;
  int characterY = 15;

  while(1){
    /*-------------------- Undraw -------------------------*/
    undraw_gamestate();
    //Shift gameStates
    for(int r = 0;r<ROWS;r++){
      for(int c = 0;c<COLS;c++){
        gameState[r][c][2] = gameState[r][c][1];
        gameState[r][c][1] = gameState[r][c][0];
      }
    }

    /*-------------------- Redraw -------------------------*/
    draw_gamestate();

    /*-------------------- Getting & Process Input -------------------------*/
    int edgecapture = *(keys_ptr+3) & 0xF;
    int dirX = 0;
    int dirY = 0;
    if(edgecapture & 0b1){
      //Key 0 pressed [UP]
      dirY--;
      *(keys_ptr+3) = edgecapture;
    }else if(edgecapture & 0b10){
      //Key 1 pressed [DOWN]
      dirY++;
      *(keys_ptr+3) = edgecapture;
    }else if(edgecapture & 0b100){
      //Key 2 pressed [LEFT]
      dirX--;
      *(keys_ptr+3) = edgecapture;
    }else if(edgecapture & 0b1000){
      //Key 3 pressed [RIGHT]
      dirX++;
      *(keys_ptr+3) = edgecapture;
    }

    //Check if character move is in bounds
    if(check_move_bounds(characterX, characterY, dirX, dirY)){
      //If there is a box to push, move box first
      bool validBoxPush = gameState[characterY+dirY][characterX+dirX][0]=='B' && check_box_move(characterX+dirX, characterY+dirY, dirX, dirY);
      if(gameState[characterY+dirY][characterX+dirX][0]=='B' && check_box_move(characterX+dirX, characterY+dirY, dirX, dirY)){
        move_tile(characterX+dirX, characterY+dirY, dirX, dirY); //Move box
        move_tile(characterX, characterY, dirX, dirY); //Move character
        characterX+=dirX;
        characterY+=dirY;
      }else if(gameState[characterY+dirY][characterX+dirX][0]!='B'){
        //No box, can move character
        move_tile(characterX, characterY, dirX, dirY);
        characterX+=dirX;
        characterY+=dirY;
      }
    }
  }

  /*-------------------- Wait for double buffer -------------------------*/
  wait_for_vsync(); // swap front and back buffers on VGA vertical sync
  pixel_buffer_start = *(pixel_ctrl_ptr + 1); // new back buffer

  // draw_character(120-1, 179);
  // draw_box(132-1, 179);
}

/*-------------------- MOVEMENT -------------------------*/
//Move a tile(character, box, etc) in a certain direction
void move_tile(int x, int y, int dirX, int dirY){
  char temp = gameState[y][x][0];
  gameState[y][x][0] = ' '; //Replace with empty
  gameState[y+dirY][x+dirX][0] = temp;
}
//Check if this move is valid based on its bounds
bool check_move_bounds(int x, int y, int dirX, int dirY){
  return 0<=x+dirX && x+dirX<COLS && 0<=y+dirY && y+dirY<ROWS;
}
//check if box can be moved in the certain direction
bool check_box_move(int boxX, int boxY, int dirX, int dirY){
  //Check if it is blocked by bounds
  if(!check_move_bounds(boxX, boxY, dirX, dirY)){
    return false;
  }
  //Check if it is blocked another box
  if(gameState[boxY+dirY][boxX+dirX][0]=='B'){
    return false;
  }
  return true;
}

/*-------------------- DRAWING -------------------------*/
void plot_pixel(int x, int y, short int line_color) {
  volatile short int *one_pixel_address;

  one_pixel_address = pixel_buffer_start + (y << 10) + (x << 1);

  *one_pixel_address = line_color;
}

// Function to clear the screen (display all pixels as black)
void clear_screen() {
	for (int i = 0; i < 320; i++) {
		for (int j = 0; j < 240; j++) {
			plot_pixel(i, j, 0);
		}
	}
}

//Undraws the previously rendered game state (2 frames before current)
void undraw_gamestate(){
  for(int r = 0;r<ROWS;r++){
    for(int c = 0;c<COLS;c++){
      if(gameState[r][c][2]!=gameState[r][c][0]){
        //this tile needs to be undrawn since it differes
        erase_temp(c*12, r*12);
      }else{
        continue;
      }
    }
  }
}

//Function to draw current game state
void draw_gamestate() {
  for(int r = 0;r<ROWS;r++){
    for(int c = 0;c<COLS;c++){
      if(gameState[r][c][0]=='P'){
        draw_character(c*12, r*12);
      }else if(gameState[r][c][0]=='B'){
        draw_box(c*12, r*12);
      }
    }
  }
}

// Draw initial game background
void draw_background() {
	int counter = 0;
    for (int i = 0; i < 240; i++) {
      for (int j = 0; j < 320; j++) {
        plot_pixel(j, i, background[counter]);
			  counter++;
      }
    }
}

// Function to draw character at specified location
void draw_character(int x, int y) {
  int counter = 0;
  for (int i = 0; i < 12; i++) {
    for (int j = 0; j < 12; j++) {
      if (character[counter] != 1) {
        plot_pixel(x + j, y + i, character[counter]);
      }
      counter++;
    }
  }
}

// Function to draw the crates that are pushed
void draw_box(int x, int y) {
  int counter = 0;
  for (int i = 0; i < 12; i++) {
    for(int j = 0; j < 12; j++) {
      plot_pixel(x + j, y + i, box[counter]);
      counter++;
    }
  }
}

// Temporary erase function with purple background (12x12)
void erase_temp(int x, int y) {
  for (int i = 0; i < 12; i++) {
    for (int j = 0; j < 12; j++) {
      plot_pixel(x + i, y + j, 0x99f7);
    }
  }
}

// Function to wait 1/60th of a second (for the current frame to finish rendering)
void wait_for_vsync() {
  volatile int *pixel_ctrl_ptr = (int *)0xFF203020;
  int status;

  *pixel_ctrl_ptr = 1;  // starts the sync process
  // Poll the S until it becomes 0
  status = *(pixel_ctrl_ptr + 3);
  while ((status & 0x1) != 0) {
    status = *(pixel_ctrl_ptr + 3);
  }
}



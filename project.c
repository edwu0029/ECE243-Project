#include "stdbool.h"
#include "stdlib.h"
#include <string.h>
#include <globals.h>

volatile int pixel_buffer_start;       // global variable
short int Buffer1[240][512];  // 240 rows, 512 (320 + padding) columns
short int Buffer2[240][512];

void plot_pixel(int x, int y, short int line_color);
void clear_screen();
void draw_background();
void wait_for_vsync();

int main(void) {
  volatile int *pixel_ctrl_ptr = (int *)0xFF203020;

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
}

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



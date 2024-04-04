#include "stdbool.h"
#include "stdlib.h"
#include <string.h>
#include <stdio.h>

volatile int *hex0_ptr;
unsigned int digit_to_hex_val(unsigned int val);
void set_hex(int hexNum, unsigned int val);

char make_code_to_letter(unsigned int val);

int main(void) {
  volatile int *ps2_ptr = (int *)0xFF200100;
  hex0_ptr = (int *)0xFF200020;
  
  int data, rvalid, val;
  while(1){
    data = *ps2_ptr;
    rvalid = data & 0x8000;
    if(rvalid){
        val = data&0xFF;
        char pressed = make_code_to_letter(val);
        printf("Pressed: %d or %c\n", val, pressed);
        set_hex(0, digit_to_hex_val(val));
    }
    //Poll until break code F0
    while(val!=0xF0){
        data = *ps2_ptr;
        val = data&0xFF;
    }
  }
}

//Converts PS2 makecode to its corresponding char
char make_code_to_letter(unsigned int val){
    if(val==0x1C){
        return 'A';
    }else if(val==0x32){
        return 'B';
    }else if(val==0x1D){
        return 'W';
    }else if(val==0x1B){
        return 'S';
    }else if(val==0x23){
        return 'D';
    }else if(val==0x29){
        return ' '; //Space
    }else{
        return ' '; //Return space by default
    }
}

unsigned int digit_to_hex_val(unsigned int val){
  if(val==1){
    return 0x06;
  }else if(val==2){
    return 0x5B;
  }else if(val==3){
    return 0x4F;
  }else if(val==4){
    return 0x66;
  }else if(val==5){
    return 0x6D;
  }else if(val==6){
    return 0x7D;
  }else if(val==7){
    return 0x07;
  }else if(val==8){
    return 0x7F;
  }else if(val==9){
    return 0x6F;
  }else{
    //Assume 0 by default
    return 0x3F;
  }
}
void set_hex(int hexNum, unsigned int val){
  unsigned int t = *hex0_ptr;
  if(hexNum==0){
    *hex0_ptr = (t & 0xFFFFFF00)+(val);
  }else if(hexNum==1){
    *hex0_ptr = (t & 0xFFFF00FF)+(val << 8);
  }else if(hexNum==2){
    *hex0_ptr = (t & 0xFF00FFFF)+(val << 16);
  }else if(hexNum==3){
    *hex0_ptr = (t & 0x00FFFFFF)+(val << 24);
  }
}





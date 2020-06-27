#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/shm.h>
#include "struct.h"
#include "Memtools.h"
#include <string.h>
#include <stdlib.h>

//http://web.theurbanpenguin.com/adding-color-to-your-output-from-c///
/**
 * Prints text in color
 * 1: Bold Red
 * 2: Bold Blue
 * 3: Bold Green
 * 4: Bold Yellow
 * 5: Bold Magenta
 * 6: Bold Cyan
*/
void printc(char *msg, int color){
  if (color == 1){
    printf("\033[1;31m");
    printf("%s",msg);
    printf("\033[0m");
  } else if (color == 2){
    printf("\033[1;34m");
    printf("%s",msg);
    printf("\033[0m");
  } else if (color == 3){
    printf("\033[1;32m");
    printf("%s",msg);
    printf("\033[0m");
  }else if (color == 4){
    printf("\033[1;33m");
    printf("%s",msg);
    printf("\033[0m");
  }else if (color == 5){
    printf("\033[1;35m");
    printf("%s",msg);
    printf("\033[0m");
  }else if (color == 6){
    printf("\033[1;36m");
    printf("%s",msg);
    printf("\033[0m");
  }
}
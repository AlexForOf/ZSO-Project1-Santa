#include "santa.h"
#include <stdio.h>

int main(void) {
  printf("--- Starting Santa Claus Simulation (10x times stress test) ---\n");

  for(int i = 1; i <= 10; i++) {
    printf("\n========================================\n");
    printf("          STARTING RUN #%d\n", i);
    printf("========================================\n");

    project_zso();
    
    printf("Run #%d completed successfully.\n", i);
  }
 

  printf("--- All 10 simulations finished safely! ---\n");

  return 0;
}

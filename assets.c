#include <stdio.h>
#include "assets.h"


void printlist(int list[][3]){
    for (int floor=3; floor >-1; floor--){
        for (int button=0; button<3; button++){
            printf("%d",list[floor][button]);
        }
        printf("\n");
    }
    printf("\n");
    printf("\n");      
}

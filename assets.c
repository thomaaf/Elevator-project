#include <stdio.h>
#include "assets.h"


void assets_Print_Status(int lastEngineDir, int INTRANSITION, int currentFloor, int state, int currentDestination, int list ){

	for (int y=0; y<4; y++){
		for (int x=0; x<3; x++){
			printf("| %d",list[y][x] );
			printf(" ");
		}
		printf("\n");

	}
	printf(" |\n");
	printf("|----------------|\n");  //18 stk
	printf("|");printf("lsdir: % d",lastEngineDir); printf("       |\n");
	printf("|----------------|\n");  //18 stk
	printf("|");printf("trans: % d",INTRANSITION); printf("       |\n");
	printf("|----------------|\n");  //18 stk
	printf("|");printf("floor: % d",currentFloor); printf("       |\n");
	printf("|----------------|\n");  //18 stk
	printf("|");printf("state: % d",state); printf("       |\n");
	printf("|----------------|\n");
	for (int x=0; x<4; x++){
		printf("| %d",currentDestination[x] );
		printf(" ");
	}
	printf(" |\n");
	printf("|----------------|\n");
	printf("\n");
	printf("\n");
	printf("\n");
}
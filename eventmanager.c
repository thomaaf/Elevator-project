#include "elev.h"
#include "eventmanager.h"
#include <stdio.h>

int list[4][3]={
	{0,0,0},
	{0,0,0},
	{0,0,0},
	{0,0,0}
};



void emButtonPressed( int pressedFloor, int pressedButton ){
	for (int floor=0; floor<4; floor++ ){
		for (int button=0; button<3; button++){
			list [pressedFloor][pressedButton]=1;
			printf("%d",list[floor][button]);

		}
		printf("\n");
	}
	printf("\n");
	printf("\n");

}
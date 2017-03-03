#include "elev.h"
#include "eventmanager.h"
#include "assets.h"
#include <stdio.h>


static int list[4][3]={ //[floor][button]
	{0,0,0},
	{0,0,0},
	{0,0,0},
	{0,0,0}
};

static int Currentfloor;


void emQueueUpdater( int pressedFloor, int pressedButton ){ //updates the queue with new orderd
	list [pressedFloor][pressedButton]=1;
	printlist(list);
	emExecutePriorityOrder(pressedFloor,pressedButton);
}

void emElevatorFeedBack(int newFloor){ //Updates the queue when/if and order has been executed
	if (Currentfloor==newFloor){ //if there's no new floor update, break to avoid more work
		//do nothing
	}
	else{
		Currentfloor=newFloor;
		for (int button=0; button <3; button++){
			if (list[Currentfloor][button]==1 ){
				list[Currentfloor][button]=0;
				printlist(list);
			}
		}
	}
}

void emExecutePriorityOrder(int pressedFloor,int pressedButton){
	printf("pressedFloor %d\n",pressedFloor);	
	printf("currentFloor %d\n",Currentfloor);	
	if (pressedFloor>Currentfloor){
		elev_set_motor_direction(DIRN_UP);
		printf("test1");	

	}
	else if(pressedFloor<Currentfloor){
		elev_set_motor_direction(DIRN_DOWN);
		printf("test2");
	}
	else{
		elev_set_motor_direction(DIRN_STOP);
		printf("test3");
	}

}



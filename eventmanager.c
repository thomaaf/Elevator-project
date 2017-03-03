#include "elev.h"
#include "eventmanager.h"
#include "assets.h"
#include <stdio.h>


static int list[4][3]={ //[floor][button]		//reminder: 1st floor in top of list, 4th floor at bottom, need to fix (the asset.c/prinlist prints opposite -> 1st at bottom)
	{0,0,0},
	{0,0,0},
	{0,0,0},
	{0,0,0} //
};

static int Currentfloor;//private variable, holds first floor


void emQueueUpdater( int pressedFloor, int pressedButton ){ //updates the queue with new orderd
	list [pressedFloor][pressedButton]=1;					//sets new order, internal or external
	printlist(list);										
	emExecutePriorityOrder(pressedFloor,pressedButton);		//tells it that there is a order, and that something should be done
}

void emElevatorFeedBack(int newFloor){ 						//Updates the queue when/if and order has been executed 		//should also have a better name;
	if (Currentfloor==newFloor){ 							//if there's no new floor update, break to avoid more work
		//if the order is is equal to the current floor, 
		//then the door should open, should this be 
		//implemented here? if so, how often shoud
		//this function run
	}
	else{
		Currentfloor=newFloor;						
		for (int button=0; button <3; button++){
			if (list[Currentfloor][button]==1 ){
				emExecutePriorityOrder(newFloor); 			//stops the elevator, finishes the execution
				list[Currentfloor][button]=0;				//clears the order, and it is marked as complete
				EMDoorControl();							//Opens the door, whole program busy for 3 seconds
				printlist(list);							//prints the new list with the order marked as cleared
			}
		}
	}
}


void emExecutePriorityOrder(int pressedFloor,int pressedButton){

	if (pressedFloor>Currentfloor){
		elev_set_motor_direction(DIRN_UP);
	

	}
	else if(pressedFloor<Currentfloor){
		elev_set_motor_direction(DIRN_DOWN);
	
	}
	else{
		elev_set_motor_direction(DIRN_STOP);
		
	}

}


void EMDoorControl(){
	elev_set_door_open_lamp(1);	//opens the door
	sleep(3);					//Timer at 3 seconds (bad implementation)
	elev_set_door_open_lamp(0);	//closes the door
}
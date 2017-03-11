#include "elev.h"
#include "eventmanager.h"
#include "assets.h"
#include <stdio.h>
#include "lights.h"
#include <time.h>


static int list[4][3]={ //[floor][button]		//reminder: 1st floor in top of list, 4th floor at bottom, need to fix (the asset.c/prinlist prints opposite -> 1st at bottom)
	{0,0,0},
	{0,0,0},
	{0,0,0},
	{0,0,0} //
};

static int CurrentDestination[4]={0,0,0,0};
static int starttime;
static int timeractive=0;
static int Currentfloor;//private variable, holds first floor
static int state; //1 = moving upwards, -1 = moving downwards, 0 = idle. 


void emQueueUpdater( int pressedFloor, int pressedButton ){ //updates the queue with new orderd
	if (pressedFloor==Currentfloor){
		emDoorControl(pressedFloor);
		break;
	}
	list [pressedFloor][pressedButton]=1;					//sets new order, internal or external
	lightsSetButtonLight(pressedFloor,pressedButton); 		//sets the lights of the pushed button
	printlist(list);										//prints the order list.
	emStatemachine(pressedFloor);
}

void emStatemachine (int pressedFloor, int pressedButton){
	if (state ==0){
		if (pressedFloor>Currentfloor){
			state = 1;
			if (pressedButton==BUTTON_CALL_DOWN){
				state = -1;							//elevator goes up, but we only want people whos going down
				CurrentDestination[pressedFloor]=2; //This is a exception case. Elevator needs to go up, to pick up the passenger whos going down. he has priority
				emSetEngine(DIRN_UP);

			}
			else {
				CurrentDestination[pressedFloor]=1; //This is the case where Button call up, and elevator needs to go up, state's set to 1, and normal op
				emSetEngine(DIRN_UP);
			}
			
			

		}													
		else if(pressedFloor<Currentfloor){
			state = -1;

			if (pressedButton==BUTTON_CALL_UP){
				state = 1;						//elevator goes down, but we only want people whos going up
				CurrentDestination[pressedFloor]==2; //This is a exception case. Elevator needs to go up, to pick up the passenger whos going down. he has priority
				emSetEngine(DIRN_DOWN);
			}
			else{
				CurrentDestination[pressedFloor]==1;
				emSetEngine(DIRN_DOWN);
			}
		}
		else {
			state = 0;
			//at place already
		}


	}
	else if(state ==1){
		if ((pressedFloor>Currentfloor)&&((pressedButton== BUTTON_CALL_UP)||(pressedButton==BUTTON_COMMAND))){  //on the way upwards, sets orders upwards
			CurrentDestination[pressedFloor]=1;
		}
		else if((pressedFloor<Currentfloor)&&(pressedButton==BUTTON_CALL_UP)){
			for (int x=0; x<4; x++){
				if ((CurrentDestination[x]==2)&&(pressedFloor<x)){	//Exeption case where theres a guy below the original dude who's going up
					CurrentDestination[x]=1;
					CurrentDestination[pressedFloor]=2;
					break;
				}
				else{
					CurrentDestination[pressedFloor]=2;
				}
			}
		}
		else if (pressedFloor==Currentfloor){
			//state =0; //perhaps? i dont know
		}
	}


	else if (state==-1){
		if (pressedFloor>Currentfloor){
			for (int x=0; x<4; x++){
				if ((CurrentDestination[x]==2)&&(pressedFloor>x)){	//Exeption case where theres a guy below the original dude who's going up
					CurrentDestination[x]=1;
					CurrentDestination[pressedFloor]=2;
					break;
				}
			}
		}
		else if((pressedFloor<Currentfloor)&&((pressedButton== BUTTON_CALL_DOWN)||(pressedButton==BUTTON_COMMAND))){
			CurrentDestination[pressedFloor]=1;
			
		}
		else if (pressedFloor==Currentfloor){
			//state =0; //perhaps? i dont know
		}			
	}
}

void emFloorControl(int newFloor){ 						//Updates the queue when/if and order has been executed 		//should also have a better name;
	Currentfloor = newFloor;
	lightsIndicatorFloorLights( newfloor)
	if (state == 1){
		if (CurrentDestination[Currentfloor]==1){
			emDoorControl(Currentfloor);
			CurrentDestination[Currentfloor]=0;
			list[Currentfloor][BUTTON_COMMAND]=0;
			list[Currentfloor][BUTTON_CALL_UP]=0;
			for (int x=0; x<4; x++){
				if (CurrentDestination[x]==2){
					emSetEngine(DIRN_DOWN);				//special case, needs to go down to pick up people going up.
					return;								//Needs to exit here VERY IMPORTANT THAT IT BREAKS HERE!!!! ESSENTIAL UNO MUNDO
				}
			}
			

		}
		else if (CurrentDestination[Currentfloor]==2){
			emDoorControl(Currentfloor);
			CurrentDestination[Currentfloor]=0;
			list[Currentfloor][BUTTON_COMMAND]=0;
			list[Currentfloor][BUTTON_CALL_UP]=0;
		}

		for (int x = Currentfloor + 1; x<4; x++ ){ //chechs if there's any more orders. if there's 
			if (CurrentDestination[x]==1){
				emSetEngine(DIRN_UP);
				break;
			}
			else{
				state=0;
				queuecheck();
			}
		}
	}


	else if (state == -1){
		if (CurrentDestination[Currentfloor]==1){
			emDoorControl(Currentfloor);
			CurrentDestination[Currentfloor]=0;
			list[Currentfloor][BUTTON_COMMAND]=0;
			list[Currentfloor][BUTTON_CALL_DOWN]=0;
			for (int x=0 ; x<4; x++ ){
				if (CurrentDestination[x]==2){
					emSetEngine(DIRN_UP);			//special case, needs to go up to pick up people going down
					return;						//Needs to exit here VERY IMPORTANT THAT IT BREAKS HERE!!!! ESSENTIAL UNO MUNDO
				}
			}
		}
		else if(CurrentDestination[Currentfloor]==2){
			emDoorControl(Currentfloor);
			CurrentDestination[Currentfloor]=0;
			list[Currentfloor][BUTTON_COMMAND]=0;
			list[Currentfloor][BUTTON_CALL_DOWN]=0;
		}

		for (int x = Currentfloor -1; x>-1; x-- ){ //chechs if there's any more orders. if there's 
			if (CurrentDestination[x]==1){
				emSetEngine(DIRN_DOWN);
				break;
			}
			else{
				state=0;
				queuecheck();
			}
		}
	}
}

void queuecheck(){
	for (int y=0; y<4; y++){
		for (int x=0; x<3; x++ ){
			if (list[x][y]==1){
				emStatemachine(y,x);
			}

		}
	}
}

void emSetEngine(int direction){

	if (direction==DIRN_UP){
		elev_set_motor_direction(DIRN_UP);
	

	}
	else if(direction==DIRN_DOWN){
		elev_set_motor_direction(DIRN_DOWN);
	
	}
	else{ //pressedFloor==Currentfloor
		elev_set_motor_direction(DIRN_STOP);
		
	}
}

void emDoorControl(int floor){
	if (state = 1){
		lightsClearOrderButtonLight(floor, BUTTON_COMMAND);
		lightsClearOrderButtonLight(floor, BUTTON_CALL_UP);
	}
	else if (state = -1){
		lightsClearOrderButtonLight(floor, BUTTON_COMMAND);
		lightsClearOrderButtonLight(floor, BUTTON_CALL_DOWN);
	}
	elev_set_door_open_lamp(1);	//opens the door
	emToggleTimer();
}

void emStopButton(){
	elev_set_stop_lamp(1);						//sets the stop lamp 								//should this just be a call from lights?
 	list =0; 									//does this work?									//Resets the queue list with zero orders
	elev_set_motor_direction(DIRN_STOP);		//Stops the elevator
	if (elev_get_floor_sensor_signal()>-1){		//opens the door IF the sensors puts out a floor
		elev_set_door_open_lamp(1);
	}
	while (elev_get_stop_signal()==1){			//loops untill the stopbutton is released
		//do nothing
	}
	elev_set_door_open_lamp(0);
	elev_set_stop_lamp(0);		

	//exits and reinitiates normal operating mode
}

void emToggleTimer(int active ){
	if (active =1){
		timeractive = 1;
		starttime =clock(); 
	}
	else{
		active =0;
	}
}

int emCheckTimer(){
	if (timeractive==1){
		if (clock() - starttime() > 3){
			elev_set_door_open_lamp(0);
		}

	}
}
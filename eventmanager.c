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

static int CurrentDestination[4]={0,0,0,0};	//Current state order queue
static int starttime;						//is the starttime for the timer
static int timeractive=0;					//states if the timer is active. 1= True
static int Currentfloor;					//private variable, holds current floor
int intransition;							//states if the elevator is in between two floors. 1 = true. 
static int state; 							//orderstate: 1 = picking upwards orders. -1 = picking downwards orders. 0 = idle
static int lastenginedir;					//Holds the last known engine direction. 1 = DIRN_UP.   -1 = DIRN_DOWN


void emQueueUpdater( int pressedFloor, int pressedButton ){ //updates the queue with new orderd
	if ((pressedFloor==Currentfloor)&&(intransition==0)){
		emDoorControl(pressedFloor);
		return;
	}
	list [pressedFloor][pressedButton]=1;					//sets new order, internal or external
	lightsSetOrderButtonLight(pressedFloor,pressedButton); 		//sets the lights of the pushed button
										//prints the order list.
	emprintshit();
	emStatemachine(pressedFloor, pressedButton);

	

}

void emStatemachine (int pressedFloor, int pressedButton){
	//printf("emStatemachine ( pressedFloor= %d , pressedButton = %d )",pressedFloor,pressedButton);
	if (state ==0){
		if (pressedFloor>Currentfloor){
			state = 1;
		
			if (pressedButton==BUTTON_CALL_DOWN){
				state = -1;							//elevator goes up, but we only want people whos going down
				CurrentDestination[pressedFloor]=2; //This is a exception case. Elevator needs to go up, to pick up the passenger whos going down. he has priorit
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
				CurrentDestination[pressedFloor]=2; //This is a exception case. Elevator needs to go up, to pick up the passenger whos going down. he has priority
				emSetEngine(DIRN_DOWN);

			}
			else{
				CurrentDestination[pressedFloor]=1;
				emSetEngine(DIRN_DOWN);

			}


		}
		else if (pressedFloor == Currentfloor){//happens only after emqueuehandler has run
			if (intransition==1){
				if (lastenginedir==1){		//if we haven driven a bit over the last floor
					emSetEngine(DIRN_DOWN);	//then we have to go back down to the prev floor
					if ((pressedButton==BUTTON_COMMAND)||(pressedButton==BUTTON_CALL_DOWN)){	//Rule: if the elevator is slightly over, then going back down counst as state =-1
						state=-1;
					}
					else if (pressedButton==BUTTON_CALL_UP){
						state=1;
					}
				}
				else if(lastenginedir==-1){
					emSetEngine(DIRN_UP);
					if((pressedButton=BUTTON_COMMAND)||(pressedButton==BUTTON_CALL_UP)){
						state = 1;
					}
					else if(pressedButton==BUTTON_CALL_DOWN){
						state=-1;
					}
				}
				CurrentDestination[pressedFloor]=1;
				return;

			}
			else{
				if (pressedFloor==0){
					state=1; 
					CurrentDestination[pressedFloor]=1;

				}
				else if(pressedFloor==3){
					state=-1;
					CurrentDestination[pressedFloor]=1;

				}
			}
		}
		return;
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
			}
		}
		else if (pressedFloor==Currentfloor){

			CurrentDestination[pressedFloor]=1;	
		}
		return;
	}

	else if (state==-1){
		if ((pressedFloor>Currentfloor)&&(pressedButton==BUTTON_CALL_DOWN)){
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

			CurrentDestination[pressedFloor]=1;
			emFloorControl(pressedFloor);


		}
		return;
	}
}

void emFloorControl(int newFloor){ 						//Updates the queue when/if and order has been executed 		//should also have a better name;
	if (intransition==1){
		return;
	}
	else{
		

		Currentfloor = newFloor;
		lightsIndicatorFloorLights( newFloor);

		if (state == 1){
			if (CurrentDestination[Currentfloor]==1){

				emDoorControl(Currentfloor);
				CurrentDestination[Currentfloor]=0;
				list[Currentfloor][BUTTON_COMMAND]=0;
				list[Currentfloor][BUTTON_CALL_UP]=0;
				emprintshit();

				

			}
			else if (CurrentDestination[Currentfloor]==2){
				emDoorControl(Currentfloor);
				CurrentDestination[Currentfloor]=0;
				list[Currentfloor][BUTTON_COMMAND]=0;
				list[Currentfloor][BUTTON_CALL_UP]=0;
				emprintshit();
			}


		}


		else if (state == -1){
			if (CurrentDestination[Currentfloor]==1){
				emDoorControl(Currentfloor);
				CurrentDestination[Currentfloor]=0;
				list[Currentfloor][BUTTON_COMMAND]=0;
				list[Currentfloor][BUTTON_CALL_DOWN]=0;
				emprintshit();

			}
			else if(CurrentDestination[Currentfloor]==2){
				emDoorControl(Currentfloor);
				CurrentDestination[Currentfloor]=0;
				list[Currentfloor][BUTTON_COMMAND]=0;
				list[Currentfloor][BUTTON_CALL_DOWN]=0;
				emprintshit();
			}


		}
	}
}

void queuecheck(){
	printf("queue is running \n");
	for (int y=0; y<4; y++){
		for (int x=0; x<3; x++ ){
			if (list[y][x]==1){
				emStatemachine(y,x);
			}

		}
	}
	printf("queue done \n");
	emprintshit();
}

void emSetEngine(int direction){

	if (direction==DIRN_UP){
		elev_set_motor_direction(DIRN_UP);
		lastenginedir=1;

	}
	else if(direction==DIRN_DOWN){
		elev_set_motor_direction(DIRN_DOWN);
		lastenginedir=-1;
	
	}
	else{ //pressedFloor==Currentfloor
		elev_set_motor_direction(DIRN_STOP);
		
	}
}

void emDoorControl(int floor){
	if (state == 1){
		lightsClearOrderButtonLight(floor, BUTTON_COMMAND);
		lightsClearOrderButtonLight(floor, BUTTON_CALL_UP);
	}
	else if (state == -1){
		lightsClearOrderButtonLight(floor, BUTTON_COMMAND);
		lightsClearOrderButtonLight(floor, BUTTON_CALL_DOWN);
	}
	
	emSetEngine(DIRN_STOP);
	emToggleTimer(1);
}

void emStopButton(){

	elev_set_stop_lamp(1);						//sets the stop lamp 								//should this just be a call from lights?
	for (int y=0; y<4; y++){
		CurrentDestination[y]=0;
		for (int x=0; x<3; x++){
			list[y][x]=0;
			lightsClearOrderButtonLight(y,x);
		}
	}
	
 	elev_set_motor_direction(DIRN_STOP);		//Stops the elevator

	while (elev_get_stop_signal()==1){			//loops untill the stopbutton is released
		if (elev_get_floor_sensor_signal()>-1){		//opens the door IF the sensors puts out a floor
			emToggleTimer(1);
		}
	}

	
	elev_set_stop_lamp(0);		
	state=0;
	emprintshit();

	//exits and reinitiates normal operating mode
}

void emToggleTimer(int active ){
	if (active ==1){
		elev_set_door_open_lamp(1);
		timeractive = 1;
		starttime =clock()/1000000; 
	}
	else if (active == 0){
		active =0;
	}
}

void emCheckTimer(){
	if (timeractive==1){

		if (clock()/1000000 - starttime > 3){
			
			elev_set_door_open_lamp(0);
			timeractive=0;
			if (emcheckSpecialCase()==1){
				printf("specialcase\n");
				emprintshit();
				return;
			}
			else if (emcheckForMoreOrder()==1){
				printf("more orders\n");
				emprintshit();
				return;
			}


		}

	}
}


int emcheckSpecialCase( ){
	if (state == 1){
		for (int x=0; x<4; x++){
			if (CurrentDestination[x]==2){
				emSetEngine(DIRN_DOWN);
				return 1;
			}
		}
	}
	else if(state == -1){
		for (int x=0; x<4; x++ ){
			if (CurrentDestination[x]==2){
				emSetEngine(DIRN_UP);
				return 1;
			}
		}
	}
	return 0;
}

int emcheckForMoreOrder(){
	if (state == 1){
		for (int x = 0 ; x<4; x++ ){ //checks if there's any more orders. if there's 
			if (CurrentDestination[x]==1){
				emSetEngine(DIRN_UP);
				return 1;
			}
			
		}
	}
	else if (state == -1){
		for (int x = 0 ; x<4; x++ ){ //checks if there's any more orders. if there's 
			if (CurrentDestination[x]==1){
				emSetEngine(DIRN_DOWN);
				return 1;
			}
			
		}
	}
	state =0;
	emprintshit();
	queuecheck();
	return 0;

}


void emprintshit(){
	printf("|----------------|\n");  //18 stk
	printf("|");printf("lsdir: % d",lastenginedir); printf("       |\n");
	printf("|----------------|\n");  //18 stk
	printf("|");printf("trans: % d",intransition); printf("       |\n");
	printf("|----------------|\n");  //18 stk
	printf("|");printf("floor: % d",Currentfloor); printf("       |\n");
	printf("|----------------|\n");  //18 stk
	printf("|");printf("state: % d",state); printf("       |\n");
	printf("|----------------|\n");
	for (int x=0; x<4; x++){
		printf("| %d",CurrentDestination[x] );
		printf(" ");
	}
	printf(" |\n");
	printf("|----------------|\n");
	printf("\n");
	printf("\n");
	printf("\n");
}
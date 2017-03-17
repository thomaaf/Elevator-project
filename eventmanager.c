#include "elev.h"
#include "eventmanager.h"
#include "assets.h"
#include <stdio.h>
#include "lights.h"
#include "fsm.h"



static int list[4][3]={ //[floor][button]		//reminder: 1st floor in top of list, 4th floor at bottom, need to fix (the asset.c/prinlist prints opposite -> 1st at bottom)
	{0,0,0},
	{0,0,0},
	{0,0,0},
	{0,0,0} //
};

static int currentDestination[4]={0,0,0,0};	//Current state order queue

static int currentFloor;					//private variable, holds current floor
int INTRANSITION;							//states if the elevator is in between two floors. 1 = true. 
static int state=0; 							//orderstate: 1 = picking upwards orders. -1 = picking downwards orders. 0 = idle
static int lastEngineDirection=0;					//Holds the last known engine direction. 1 = DIRN_UP.   -1 = DIRN_DOWN
static int lastFloorPressed=-1;
static int lastButtonPressed=-1;


void em_Queue_Updater( int pressedFloor, int pressedButton ){ //updates the queue with new order
	if ((pressedFloor==currentFloor)&&(INTRANSITION==0)){
		em_Door_Control(pressedFloor);
		return;
	}
	if (list[pressedFloor][pressedButton]==1){
		return;
	}
	list [pressedFloor][pressedButton]=1;						//sets new order, internal or external
	lightsSetOrderButtonLight(pressedFloor,pressedButton); 		//sets the lights of the pushed button
																//prints the order list.
	printf("queue\n");
	em_Print_Status ();
	fsm_Finite_State_Machine(pressedFloor, pressedButton, *state, currentFloor,lastEngineDirection);

	

}



void em_Floor_Control(int newFloor){ 						//Updates the queue when/if and order has been executed 		//should also have a better name;
	if (INTRANSITION==1){								//Marks that the elevator is in transition, and that it should not rund
		return;
	}
	else{
		

		currentFloor = newFloor;						//Updates the current floor to the new arrived floor
		lightsIndicatorFloorLights( newFloor);			//Updates the floorindicator light with the new arrived floor

		if (state == 1){								//if we are in state of picking people going up
			if ((currentDestination[currentFloor]==1)||(currentDestination[currentFloor]==2)){	//if theres and UP or COMMAND order at the new arrived floor

				em_Door_Control(currentFloor);			//fixes doors and lights.
				currentDestination[currentFloor]=0;		//clears currentDestination orders
				list[currentFloor][BUTTON_COMMAND]=0;	//clears Listorders
				list[currentFloor][BUTTON_CALL_UP]=0;	//clears Listorders
				printf("floorcontrol state =1\n");

				em_Print_Status ();							//Prints shit
			}
		}


		else if (state == -1){
			if ((currentDestination[currentFloor]==1)||(currentDestination[currentFloor]==2)){
				em_Door_Control(currentFloor);
				currentDestination[currentFloor]=0;
				list[currentFloor][BUTTON_COMMAND]=0;
				list[currentFloor][BUTTON_CALL_DOWN]=0;
				printf("floorcontrol state =-1\n");
				em_Print_Status ();

			}
		}
	}
}

void em_Queue_Check(){						//Checks the main queue list for any more orders that hasnt been handled yet.
	printf("queue is running \n");
	for (int y=0; y<4; y++){
		for (int x=0; x<3; x++ ){
			if (list[y][x]==1){
				fsm_Finite_State_Machine(y,x);	//If found, they are just input as regular buttonpushed, and everything should work as with a single button press.
			}

		}
	}
	printf("queue done \n");

	em_Print_Status ();
}

void em_Set_Engine(int direction){

	if (direction==DIRN_UP){
		elev_set_motor_direction(DIRN_UP);
		if(elev_get_floor_sensor_signal()!=-1){
			lastEngineDirection=1;	
		}
		

	}
	else if(direction==DIRN_DOWN){
		elev_set_motor_direction(DIRN_DOWN);
		if (elev_get_floor_sensor_signal()!=-1){
			lastEngineDirection=-1;	
		}
		
	
	}
	else{ //pressedFloor==currentFloor
		elev_set_motor_direction(DIRN_STOP);
		
	}
}

void em_Door_Control(int floor){
	if (state == 1){											//If the state is =1 then the only orders that should be handled are UP and Command
		lights_Clear_Order_Button_Light(floor, BUTTON_COMMAND);		//therefore they are the only lights that are cleared, given stop at a floor
		lights_Clear_Order_Button_Light(floor, BUTTON_CALL_UP);
	}
	else if (state == -1){
		lights_Clear_Order_Button_Light(floor, BUTTON_COMMAND);
		lights_Clear_Order_Button_Light(floor, BUTTON_CALL_DOWN);
	}
	else if (state ==0){
		lights_Clear_Order_Button_Light(floor, BUTTON_COMMAND);
		lights_Clear_Order_Button_Light(floor, BUTTON_CALL_DOWN);
		lights_Clear_Order_Button_Light(floor, BUTTON_CALL_UP);
		currentDestination[currentFloor]=0;
		list[currentFloor][BUTTON_COMMAND]=0;
		list[currentFloor][BUTTON_CALL_DOWN]=0;
		list[currentFloor][BUTTON_CALL_UP]=0;
	}
	
	em_Set_Engine(DIRN_STOP);										//stops the engine at the floor
	timer_Toggle_Timer(1);											//toggles the door timer
}

void em_Stop_Button(){

	elev_set_stop_lamp(1);						//sets the stop lamp 								
	for (int y=0; y<4; y++){					//clears all orders
		currentDestination[y]=0;
		for (int x=0; x<3; x++){
			list[y][x]=0;
			lights_Clear_Order_Button_Light(y,x);	//clears all orderlights
		}
	}
	
 	elev_set_motor_direction(DIRN_STOP);		//Stops the elevator

	while (elev_get_stop_signal()==1){			//loops untill the stopbutton is released, this asserts that no new orders can be recieve while stop button is pressed.
		if (elev_get_floor_sensor_signal()>-1){		//opens the door IF the sensors puts out a floor
			timer_Toggle_Timer(1);
		}
	}

	
	elev_set_stop_lamp(0);					
	state=0;
	printf("stop button =1\n");
	em_Print_Status ();

	//exits and reinitiates normal operating mode
}




int em_Check_Special_Case( ){
	if (state == 1){							//if the state is = 1, and there's and order labeled 2, then the elevator should proceed DOWN
		for (int x=0; x<4; x++){
			if (currentDestination[x]==2){		
				//if (lastEngineDirection==1){
				//	return 0;
				//}
				em_Set_Engine(DIRN_DOWN);
				return 1;
			}
		}
	}
	else if(state == -1){						//if the state is = -1, and there's and order labeled 2, then the elevator should proceed UP
		for (int x=0; x<4; x++ ){
			if (currentDestination[x]==2){
				//if (lastEngineDirection==-1){
				//	return 0;
				//}
				em_Set_Engine(DIRN_UP);
				return 1;
			}
		}
	}
	return 0;
}

int em_Check_For_More_Orders(){
	if (state == 1){							//if the state is = 1, and there's and order labeled 1, then the elevator should proceed UP
		for (int x = 0 ; x<4; x++ ){ 			
			
			if (currentDestination[x]==1){
				em_Set_Engine(DIRN_UP);
				return 1;
			}
			
		}
	}
	else if (state == -1){
		for (int x = 0 ; x<4; x++ ){ 			//if the state is = -1, and there's and order labeled 1, then the elevator should proceed DOWN
			if (currentDestination[x]==1){
				em_Set_Engine(DIRN_DOWN);
				return 1;
			}
			
		}
	}
	state =0;
	printf("check for more orders = return 0\n");
	em_Print_Status ();
	em_Queue_Check(); //if there's no orders in currentDestination, then we need to check for new order, and "restart" the elevator to state =0 
	return 0;

}

void em_Print_Status(){

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

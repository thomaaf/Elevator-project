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
static int lastfloorpressed=-1;
static int lastbuttonpressed=-1;


void emQueueUpdater( int pressedFloor, int pressedButton ){ //updates the queue with new orderd
	if ((pressedFloor==Currentfloor)&&(intransition==0)){
		emDoorControl(pressedFloor);
		return;
	}
	if (list[pressedFloor][pressedButton]==1){
		return;
	}
	list [pressedFloor][pressedButton]=1;					//sets new order, internal or external
	lightsSetOrderButtonLight(pressedFloor,pressedButton); 		//sets the lights of the pushed button
										//prints the order list.
	printf("queue\n");
	emprintshit();
	emStatemachine(pressedFloor, pressedButton);

	

}

void emStatemachine (int pressedFloor, int pressedButton){
	
	//if (list[lastfloorpressed][lastbuttonpressed]==1){ 						//makes sure that only one order is recieved, by checking if ther's already a order for this floor
																	//If there's already an order stored, then the function extits, and no new information is written.
	//	return;
	//}

	if (state ==0){													//State=0 => Elevator is stationary
		lastfloorpressed=pressedFloor;								//asserts that this order can only be called once.
		lastbuttonpressed=pressedButton;
		if (pressedFloor>Currentfloor){								//If there's an order above the current floor
			state = 1;												//simplification. Assumes that the order is an UP or Command
		
			if (pressedButton==BUTTON_CALL_DOWN){					//if the order is not a UP or COMMAND, then the state should be -1, which is asserted here
				state = -1;																		
				CurrentDestination[pressedFloor]=2; 				//To keep a tab on the upper command of DOWNgoing orders. As long as there's a 2 in the list
				emSetEngine(DIRN_UP);								//and the state is -1, then the elevator will move UPwards
				return;
	

			}
			else {
				CurrentDestination[pressedFloor]=1; 				//The order was just a regular UP or COMMAND, and the elevator moves UP with state =1 
				emSetEngine(DIRN_UP);
				return; 
			
			}

			

		}													
		else if(pressedFloor<Currentfloor){							//If there's an order below the current floor
			state = -1;												//simplification. Assumes that the order is an DOWN or COMMAND

			if (pressedButton==BUTTON_CALL_UP){						//if the order is not a DOWN or COMMAND, then the state should be 1, which is asserted here
				state = 1;												
				CurrentDestination[pressedFloor]=2; 				//To keep a tab on the lower command of UPgoing orders. As long as there's a 2 in the list
				emSetEngine(DIRN_DOWN);								//and the state is 1, then the elevator will move DOWNwards
				return;

			}
			else{
				CurrentDestination[pressedFloor]=1;					//The order was just a regular UP or COMMAND, and the elevator moves UP with state =1 
				emSetEngine(DIRN_DOWN);
				return;

			}


		}
		else if (pressedFloor == Currentfloor){						//Should only happen after quehandler or after a STOPprocedure
			if (intransition==1){									//If the elevator is stopped between two floors

				if (lastenginedir==1){								//if we haven driven a bit over the last floor
					emSetEngine(DIRN_DOWN);							//then we have to go back DOWN to the prev floor
					if ((pressedButton==BUTTON_COMMAND)||(pressedButton==BUTTON_CALL_DOWN)){	//Rule: if the elevator is slightly over, then going back DOWN counst as state =-1
						state=-1;
						CurrentDestination[pressedFloor]=1;	
					}
					else if (pressedButton==BUTTON_CALL_UP){
						state=1;
						CurrentDestination[pressedFloor]=2;			//we are going down to pick up a person going up. 2 marks the turning point
					}
				}

				else if(lastenginedir==-1){							//if we haven driven a bit below the last floor
					emSetEngine(DIRN_UP);							//then we have to go back UP to the prev floor
					if((pressedButton=BUTTON_COMMAND)||(pressedButton==BUTTON_CALL_UP)){		//Rule: if the elevator is slightly below, then going back UP counst as state =1
						state = 1;
						CurrentDestination[pressedFloor]=1;	
					}
					else if(pressedButton==BUTTON_CALL_DOWN){
						state=-1;
						CurrentDestination[pressedFloor]=2;			//We are going UP to pick up a person going DOWN. 2 marks the turning point.	
					}
				}
				
				return;

			}
			else{
				//printf("Exeption: Should be handled by QueueUpdater\n");
				//if (pressedFloor==0){								//These cases should be handled in QueueUpdater, and should therefore never happen
					//state=1; 
					//CurrentDestination[pressedFloor]=1;
					list[pressedFloor][pressedButton]=0;
					emDoorControl(pressedFloor);
					
					printf("test\n");

				//}
				//else if(pressedFloor==3){
				//	state=-1;
					//CurrentDestination[pressedFloor]=1;
				//	emDoorControl(pressedFloor);

				//}
			}
		}
		printf("Exception: Has reached end of State == 0. Should exit before this.\n");
		return;
	}

	else if(state ==1){												//state = 1 => Elevator is picking up people going UP.
		lastfloorpressed=pressedFloor;								//asserts that this order can only be called once.
		lastbuttonpressed=pressedButton;

		if ((pressedFloor>Currentfloor)&&((pressedButton== BUTTON_CALL_UP)||(pressedButton==BUTTON_COMMAND))){  //Picking people who's going up. And these orders are above current location
			CurrentDestination[pressedFloor]=1; 
			return;
		}
		else if((pressedFloor<Currentfloor)&&(pressedButton==BUTTON_CALL_UP)){	//exeptioncase. The order is below current elevator position, Elevator needs to go down to pick up
		
			for (int x=0; x<4; x++){
				if ((CurrentDestination[x]==2)&&(pressedFloor<x)){	//Checks if there's any more orders further below the current new order.
					CurrentDestination[x]=1;						//if yes, then the found order should be reset to 1, and the new order should have the turningpoint and be = 2; 
					CurrentDestination[pressedFloor]=2;				//Updates the destination list with the order
					return;
				}
				//if (CurrentDestination[pressedFloor]!=2){
				//	CurrentDestination[pressedFloor]=1;
				//}
			}

			return;
		}
		else if (pressedFloor==Currentfloor){						//Should also be handled by Queuehandler; 
			//printf("Exeption: Should be handled by QueueUpdater; pressedFloor == Currentfloor, state = 1 \n");
			if (lastenginedir==-1){
				CurrentDestination[pressedFloor]=1;
			}
			else if(lastenginedir==1){
				CurrentDestination[pressedFloor]=2;
			}
			return;
		}
		//printf("Exception: Has reached end of State == 1. Should exit before this.\n");
		return;
	}

	else if (state==-1){											//state = -1 => Elevator is picking up people going DOWN.
	

		if ((pressedFloor>Currentfloor)&&(pressedButton==BUTTON_CALL_DOWN)){	//exeptioncase. The order is above current elevator position, Elevator needs to go UP to pick up
			for (int x=0; x<4; x++){
				if ((CurrentDestination[x]==2)&&(pressedFloor>x)){	//Checks if there's any more orders further above the current new order.
					CurrentDestination[x]=1;						//if yes, then the found order should be reset to 1, and the new order should have the turningpoint and be =2;
					CurrentDestination[pressedFloor]=2;				//Updates the destination list with the order
					return;
				}
			}
			return;
		}
		else if((pressedFloor<Currentfloor)&&((pressedButton== BUTTON_CALL_DOWN)||(pressedButton==BUTTON_COMMAND))){  //Picking people who's going dwon. And these orders are below current location
			CurrentDestination[pressedFloor]=1;
			return;	
		}
		else if (pressedFloor==Currentfloor){
			//printf("Exeption: Should be handled by QueueUpdater; pressedFloor == Currentfloor, state =- 1 \n");
			if (lastenginedir==-1){
				CurrentDestination[pressedFloor]=2;
			}
			else if(lastenginedir==1){
				CurrentDestination[pressedFloor]=1;
			}
			
			return;
		}
		//printf("Exception: Has reached end of State == -1. Should exit before this.\n");
		return;
	}
}

void emFloorControl(int newFloor){ 						//Updates the queue when/if and order has been executed 		//should also have a better name;
	if (intransition==1){								//Marks that the elevator is in transition, and that it should not rund
		return;
	}
	else{
		

		Currentfloor = newFloor;						//Updates the current floor to the new arrived floor
		lightsIndicatorFloorLights( newFloor);			//Updates the floorindicator light with the new arrived floor

		if (state == 1){								//if we are in state of picking people going up
			if ((CurrentDestination[Currentfloor]==1)||(CurrentDestination[Currentfloor]==2)){	//if theres and UP or COMMAND order at the new arrived floor

				emDoorControl(Currentfloor);			//fixes doors and lights.
				CurrentDestination[Currentfloor]=0;		//clears currentDestination orders
				list[Currentfloor][BUTTON_COMMAND]=0;	//clears Listorders
				list[Currentfloor][BUTTON_CALL_UP]=0;	//clears Listorders
				printf("floorcontrol state =1\n");

				emprintshit();							//Prints shit

				

			}
/*			
			else if (CurrentDestination[Currentfloor]==2)
				emDoorControl(Currentfloor);
				CurrentDestination[Currentfloor]=0;
				list[Currentfloor][BUTTON_COMMAND]=0;
				list[Currentfloor][BUTTON_CALL_UP]=0;
				emprintshit();
			}
*/

		}


		else if (state == -1){
			if ((CurrentDestination[Currentfloor]==1)||(CurrentDestination[Currentfloor]==2)){
				emDoorControl(Currentfloor);
				CurrentDestination[Currentfloor]=0;
				list[Currentfloor][BUTTON_COMMAND]=0;
				list[Currentfloor][BUTTON_CALL_DOWN]=0;
				printf("floorcontrol state =-1\n");
				emprintshit();

			}
			/*
			else if(CurrentDestination[Currentfloor]==2){
				emDoorControl(Currentfloor);
				CurrentDestination[Currentfloor]=0;
				list[Currentfloor][BUTTON_COMMAND]=0;
				list[Currentfloor][BUTTON_CALL_DOWN]=0;
				emprintshit();
			}
			*/



		}
	}
}

void queuecheck(){						//Checks the main queue list for any more orders that hasnt been handled yet.
	printf("queue is running \n");
	for (int y=0; y<4; y++){
		for (int x=0; x<3; x++ ){
			if (list[y][x]==1){
				emStatemachine(y,x);	//If found, they are just input as regular buttonpushed, and everything should work as with a single button press.
			}

		}
	}
	printf("queue done \n");

	emprintshit();
}

void emSetEngine(int direction){

	if (direction==DIRN_UP){
		elev_set_motor_direction(DIRN_UP);
		if(elev_get_floor_sensor_signal()!=-1){
			lastenginedir=1;	
		}
		

	}
	else if(direction==DIRN_DOWN){
		elev_set_motor_direction(DIRN_DOWN);
		if (elev_get_floor_sensor_signal()!=-1){
			lastenginedir=-1;	
		}
		
	
	}
	else{ //pressedFloor==Currentfloor
		elev_set_motor_direction(DIRN_STOP);
		
	}
}

void emDoorControl(int floor){
	if (state == 1){											//If the state is =1 then the only orders that should be handled are UP and Command
		lightsClearOrderButtonLight(floor, BUTTON_COMMAND);		//therefore they are the only lights that are cleared, given stop at a floor
		lightsClearOrderButtonLight(floor, BUTTON_CALL_UP);
	}
	else if (state == -1){
		lightsClearOrderButtonLight(floor, BUTTON_COMMAND);
		lightsClearOrderButtonLight(floor, BUTTON_CALL_DOWN);
	}
	else if (state ==0){
		lightsClearOrderButtonLight(floor, BUTTON_COMMAND);
		lightsClearOrderButtonLight(floor, BUTTON_CALL_DOWN);
		lightsClearOrderButtonLight(floor, BUTTON_CALL_UP);
		CurrentDestination[Currentfloor]=0;
		list[Currentfloor][BUTTON_COMMAND]=0;
		list[Currentfloor][BUTTON_CALL_DOWN]=0;
		list[Currentfloor][BUTTON_CALL_UP]=0;
	}
	
	emSetEngine(DIRN_STOP);										//stops the engine at the floor
	emToggleTimer(1);											//toggles the door timer
}

void emStopButton(){

	elev_set_stop_lamp(1);						//sets the stop lamp 								
	for (int y=0; y<4; y++){					//clears all orders
		CurrentDestination[y]=0;
		for (int x=0; x<3; x++){
			list[y][x]=0;
			lightsClearOrderButtonLight(y,x);	//clears all orderlights
		}
	}
	
 	elev_set_motor_direction(DIRN_STOP);		//Stops the elevator

	while (elev_get_stop_signal()==1){			//loops untill the stopbutton is released, this asserts that no new orders can be recieve while stop button is pressed.
		if (elev_get_floor_sensor_signal()>-1){		//opens the door IF the sensors puts out a floor
			emToggleTimer(1);
		}
	}

	
	elev_set_stop_lamp(0);					
	state=0;
	printf("stop button =1\n");
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

		if (clock()/1000000 - starttime > 3){			//if the timer has run out, the it reinitiaes the elevator prosess
			
			elev_set_door_open_lamp(0);
			timeractive=0;
			if (emcheckSpecialCase()==1){				//checks if there's any orders with label 2
				printf("specialcase\n");				//if yes, then the elevator should proceed down
				emprintshit();
				return;
			}
			else if (emcheckForMoreOrder()==1){			//checks for more orders, either in CurrentDestination, or in list if Currentdestiantion is empty
				printf("more orders\n");
				emprintshit();
				return;
			}


		}

	}
}


int emcheckSpecialCase( ){
	if (state == 1){							//if the state is = 1, and there's and order labeled 2, then the elevaotr should proceed DOWWN
		for (int x=0; x<4; x++){
			if (CurrentDestination[x]==2){		
				//if (lastenginedir==1){
				//	return 0;
				//}
				emSetEngine(DIRN_DOWN);
				return 1;
			}
		}
	}
	else if(state == -1){						//if the state is = -1, and there's and order labeled 2, then the elevaotr should proceed UP
		for (int x=0; x<4; x++ ){
			if (CurrentDestination[x]==2){
				//if (lastenginedir==-1){
				//	return 0;
				//}
				emSetEngine(DIRN_UP);
				return 1;
			}
		}
	}
	return 0;
}

int emcheckForMoreOrder(){
	if (state == 1){							//if the state is = 1, and there's and order labeled 1, then the elevaotr should proceed UP
		for (int x = 0 ; x<4; x++ ){ 			
			
			if (CurrentDestination[x]==1){
				emSetEngine(DIRN_UP);
				return 1;
			}
			
		}
	}
	else if (state == -1){
		for (int x = 0 ; x<4; x++ ){ 			//if the state is = -1, and there's and order labeled 1, then the elevaotr should proceed DOWN
			if (CurrentDestination[x]==1){
				emSetEngine(DIRN_DOWN);
				return 1;
			}
			
		}
	}
	state =0;
	printf("check for more orders = return 0\n");
	emprintshit();
	queuecheck(); //if there's no orders in CUrrentdestination, then we need to check for new order, and "restart" the elevator to state =0 
	return 0;

}


void emprintshit(){

	for (int y=0; y<4; y++){
		for (int x=0; x<3; x++){
			printf("| %d",list[y][x] );
			printf(" ");
		}
		printf("\n");

	}
	printf(" |\n");
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

#include <stdio.h>
#include "fsm.h"










void finite_state_machine (int pressed_floor, int pressed_button, int state){
	
	//if (list[last_floor_pressed][last_button_pressed]==1){ 						//makes sure that only one order is recieved, by checking if ther's already a order for this floor
																	//If there's already an order stored, then the function extits, and no new information is written.
	//	return;
	//}

	if (state ==0){													//State=0 => Elevator is stationary
		last_floor_pressed=pressed_floor;								//asserts that this order can only be called once.
		last_button_pressed=pressed_button;
		if (pressed_floor>current_floor){								//If there's an order above the current floor
			state = 1;												//simplification. Assumes that the order is an UP or Command
		
			if (pressed_button==BUTTON_CALL_DOWN){					//if the order is not a UP or COMMAND, then the state should be -1, which is asserted here
				state = -1;																		
				current_destination[pressed_floor]=2; 				//To keep a tab on the upper command of DOWNgoing orders. As long as there's a 2 in the list
				emSetEngine(DIRN_UP);								//and the state is -1, then the elevator will move UPwards
				return;
	

			}
			else {
				current_destination[pressed_floor]=1; 				//The order was just a regular UP or COMMAND, and the elevator moves UP with state =1 
				emSetEngine(DIRN_UP);
				return; 
			
			}

			

		}													
		else if(pressed_floor<current_floor){							//If there's an order below the current floor
			state = -1;												//simplification. Assumes that the order is an DOWN or COMMAND

			if (pressed_button==BUTTON_CALL_UP){						//if the order is not a DOWN or COMMAND, then the state should be 1, which is asserted here
				state = 1;												
				current_destination[pressed_floor]=2; 				//To keep a tab on the lower command of UPgoing orders. As long as there's a 2 in the list
				emSetEngine(DIRN_DOWN);								//and the state is 1, then the elevator will move DOWNwards
				return;

			}
			else{
				current_destination[pressed_floor]=1;					//The order was just a regular UP or COMMAND, and the elevator moves UP with state =1 
				emSetEngine(DIRN_DOWN);
				return;

			}


		}
		else if (pressed_floor == current_floor){						//Should only happen after quehandler or after a STOPprocedure
			if (intransition==1){									//If the elevator is stopped between two floors

				if (last_engine_direction==1){								//if we haven driven a bit over the last floor
					emSetEngine(DIRN_DOWN);							//then we have to go back DOWN to the prev floor
					if ((pressed_button==BUTTON_COMMAND)||(pressed_button==BUTTON_CALL_DOWN)){	//Rule: if the elevator is slightly over, then going back DOWN counst as state =-1
						state=-1;
						current_destination[pressed_floor]=1;	
					}
					else if (pressed_button==BUTTON_CALL_UP){
						state=1;
						current_destination[pressed_floor]=2;			//we are going down to pick up a person going up. 2 marks the turning point
					}
				}

				else if(last_engine_direction==-1){							//if we haven driven a bit below the last floor
					emSetEngine(DIRN_UP);							//then we have to go back UP to the prev floor
					if((pressed_button=BUTTON_COMMAND)||(pressed_button==BUTTON_CALL_UP)){		//Rule: if the elevator is slightly below, then going back UP counst as state =1
						state = 1;
						current_destination[pressed_floor]=1;	
					}
					else if(pressed_button==BUTTON_CALL_DOWN){
						state=-1;
						current_destination[pressed_floor]=2;			//We are going UP to pick up a person going DOWN. 2 marks the turning point.	
					}
				}
				
				return;

			}
			else{
				//printf("Exeption: Should be handled by QueueUpdater\n");
				//if (pressed_floor==0){								//These cases should be handled in QueueUpdater, and should therefore never happen
					//state=1; 
					//current_destination[pressed_floor]=1;
					list[pressed_floor][pressed_button]=0;
					emDoorControl(pressed_floor);
					
					printf("test\n");

				//}
				//else if(pressed_floor==3){
				//	state=-1;
					//current_destination[pressed_floor]=1;
				//	emDoorControl(pressed_floor);

				//}
			}
		}
		printf("Exception: Has reached end of State == 0. Should exit before this.\n");
		return;
	}

	else if(state ==1){												//state = 1 => Elevator is picking up people going UP.
		last_floor_pressed=pressed_floor;								//asserts that this order can only be called once.
		last_button_pressed=pressed_button;

		if ((pressed_floor>current_floor)&&((pressed_button== BUTTON_CALL_UP)||(pressed_button==BUTTON_COMMAND))){  //Picking people who's going up. And these orders are above current location
			current_destination[pressed_floor]=1; 
			return;
		}
		else if((pressed_floor<current_floor)&&(pressed_button==BUTTON_CALL_UP)){	//exeptioncase. The order is below current elevator position, Elevator needs to go down to pick up
		
			for (int x=0; x<4; x++){
				if ((current_destination[x]==2)&&(pressed_floor<x)){	//Checks if there's any more orders further below the current new order.
					current_destination[x]=1;						//if yes, then the found order should be reset to 1, and the new order should have the turningpoint and be = 2; 
					current_destination[pressed_floor]=2;				//Updates the destination list with the order
					return;
				}
				//if (current_destination[pressed_floor]!=2){
				//	current_destination[pressed_floor]=1;
				//}
			}

			return;
		}
		else if (pressed_floor==current_floor){						//Should also be handled by Queuehandler; 
			//printf("Exeption: Should be handled by QueueUpdater; pressed_floor == current_floor, state = 1 \n");
			if (last_engine_direction==-1){
				current_destination[pressed_floor]=1;
			}
			else if(last_engine_direction==1){
				current_destination[pressed_floor]=2;
			}
			return;
		}
		//printf("Exception: Has reached end of State == 1. Should exit before this.\n");
		return;
	}

	else if (state==-1){											//state = -1 => Elevator is picking up people going DOWN.
	

		if ((pressed_floor>current_floor)&&(pressed_button==BUTTON_CALL_DOWN)){	//exeptioncase. The order is above current elevator position, Elevator needs to go UP to pick up
			for (int x=0; x<4; x++){
				if ((current_destination[x]==2)&&(pressed_floor>x)){	//Checks if there's any more orders further above the current new order.
					current_destination[x]=1;						//if yes, then the found order should be reset to 1, and the new order should have the turningpoint and be =2;
					current_destination[pressed_floor]=2;				//Updates the destination list with the order
					return;
				}
			}
			return;
		}
		else if((pressed_floor<current_floor)&&((pressed_button== BUTTON_CALL_DOWN)||(pressed_button==BUTTON_COMMAND))){  //Picking people who's going dwon. And these orders are below current location
			current_destination[pressed_floor]=1;
			return;	
		}
		else if (pressed_floor==current_floor){
			//printf("Exeption: Should be handled by QueueUpdater; pressed_floor == current_floor, state =- 1 \n");
			if (last_engine_direction==-1){
				current_destination[pressed_floor]=2;
			}
			else if(last_engine_direction==1){
				current_destination[pressed_floor]=1;
			}
			
			return;
		}
		//printf("Exception: Has reached end of State == -1. Should exit before this.\n");
		return;
	}
}
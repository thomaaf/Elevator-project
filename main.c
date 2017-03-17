#include "elev.h"
#include <stdio.h>
#include "eventmanager.h"



void main_New_Order     ( void );
void main_At_Floor      ( void );
void main_If_Stop       ( void );
void main_Start         ( void );
void main_Floor_Check   ( void );







int main() {
    // Initialize hardware
    if (!elev_init()) {
        printf("Unable to initialize elevator hardware!\n");
        return 1;
    }

    
    main_Start();

    printf("Press STOP button to stop elevator and exit program.\n");
    em_Print_Status();

	
    elev_set_motor_direction(DIRN_STOP);

    while (1) {

    	main_New_Order();
    	main_At_Floor();
        main_If_Stop();
        timer_Check_Timer();
        
  

    }

    return 0;
}



void main_New_Order(){
	for (int floor=0; floor<4; floor++ ){
		for (int button=0; button<3; button++){ //USe enum instead of numbers to be sure whiovh floor is being called
			if (!(button == BUTTON_CALL_UP && floor == N_FLOORS - 1)&&(!(button == BUTTON_CALL_DOWN && floor == 0))){
				if (elev_get_button_signal( button, floor)==1){
					em_Queue_Updater(floor, button);
					
				}
			}

		}
	}

}
void main_At_Floor(){

        if (elev_get_floor_sensor_signal()==-1){
        	INTRANSITION =1; 
        }
        else if (elev_get_floor_sensor_signal()!=-1){
        	emFloorControl(elev_get_floor_sensor_signal());
        	INTRANSITION=0;
        }
        
 
}

void main_If_Stop(){ //bruke annet navn?
    if (elev_get_stop_signal()==1){
        emStopButton();

    }
}

void main_Start(void){ //Moves the elevator to a defined floor
    while (elev_get_floor_sensor_signal()==-1){
        elev_set_motor_direction(DIRN_DOWN);
    }  
}


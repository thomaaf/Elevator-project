#include "elev.h"
#include <stdio.h>
#include "eventmanager.h"



void newOrder( void );
void atFloor(void);
void ifStop(void);
void startUp(void);
void floorCheck(void);
int PrevFloor=-1;







int main() {
    // Initialize hardware
    if (!elev_init()) {
        printf("Unable to initialize elevator hardware!\n");
        return 1;
    }

    
    startUp();

    printf("Press STOP button to stop elevator and exit program.\n");
    emprintshit();

	
    elev_set_motor_direction(DIRN_STOP);

    while (1) {

    	newOrder();
    	atFloor();
        ifStop();
        emCheckTimer();
        
  

    }

    return 0;
}



void newOrder(){
	for (int floor=0; floor<4; floor++ ){
		for (int button=0; button<3; button++){ //USe enum instead of numbers to be sure whiovh floor is being called
			if (!(button == BUTTON_CALL_UP && floor == N_FLOORS - 1)&&(!(button == BUTTON_CALL_DOWN && floor == 0))){
				if (elev_get_button_signal( button, floor)==1){
					emQueueUpdater(floor, button);
					
				}
			}

		}
	}

}
void atFloor(){

        if (elev_get_floor_sensor_signal()==-1){
        	intransition =1; 
        }
        else if (elev_get_floor_sensor_signal()!=-1){
        	emFloorControl(elev_get_floor_sensor_signal());
        	intransition=0;
        }
        
 
}

void ifStop(){ //bruke annet navn?
    if (elev_get_stop_signal()==1){
        emStopButton();

    }
}

void startUp(void){ //Moves the elevator to a defined floor
    while (elev_get_floor_sensor_signal()==-1){
        elev_set_motor_direction(DIRN_DOWN);
    }  
}


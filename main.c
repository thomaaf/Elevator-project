#include "elev.h"
#include <stdio.h>
#include "eventmanager.h"





void isButtonPressed( void );
void atFloor(void);
//void ifStop(void);






int main() {
    // Initialize hardware
    if (!elev_init()) {
        printf("Unable to initialize elevator hardware!\n");
        return 1;
    }

    printf("Press STOP button to stop elevator and exit program.\n");
    printf("Hello there, I'm here to make u confused")
	
    elev_set_motor_direction(DIRN_STOP);

    while (1) {
    	isButtonPressed();
    	atFloor();
    	//ifStop();

    }

    return 0;
}



void isButtonPressed(){
	for (int floor=0; floor<4; floor++ ){
		for (int button=0; button<3; button++){ //USe enum instead of numbers to be sure whiovh floor is being called
			if (!(button == BUTTON_CALL_UP && floor == N_FLOORS - 1)&&(!(button == BUTTON_CALL_DOWN && floor == 0))){
				if (elev_get_button_signal( button, floor)==1){
					emButtonPressed(floor, button);
					
				}
			}

		}
	}

}

Void atFloor()

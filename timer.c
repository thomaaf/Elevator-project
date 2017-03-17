#include <time.h>
#include "timer.h"





void emToggleTimer(int active ){
	if (active ==1){
		elev_set_door_open_lamp(1);
		timer_active = 1;
		start_time =clock()/1000000; 
	}
	else if (active == 0){
		active =0;
	}
}

void emCheckTimer(){
	if (timer_active==1){

		if (clock()/1000000 - start_time > 3){			//if the timer has run out, then it reinitiaes the elevator prosess
			
			elev_set_door_open_lamp(0);
			timer_active=0;
			if (emcheckSpecialCase()==1){				//checks if there's any orders with label 2
				printf("specialcase\n");				//if yes, then the elevator should proceed down
				emprintshit();
				return;
			}
			else if (emcheckForMoreOrder()==1){			//checks for more orders, either in current_destination, or in list if Currentdestiantion is empty
				printf("more orders\n");
				emprintshit();
				return;
			}


		}

	}
}
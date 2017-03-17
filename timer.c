#include <time.h>
#include "timer.h"





void emToggleTimer(int active ){
	if (active ==1){
		elev_set_door_open_lamp(1);
		timerActive = 1;
		startTime =clock()/1000000; 
	}
	else if (active == 0){
		active =0;
	}
}

void emCheckTimer(){
	if (timerActive==1){

		if (clock()/1000000 - startTime > 3){			//if the timer has run out, then it reinitiaes the elevator prosess
			
			elev_set_door_open_lamp(0);
			timerActive=0;
			if (emcheckSpecialCase()==1){				//checks if there's any orders with label 2
				printf("specialcase\n");				//if yes, then the elevator should proceed down
				emprintshit();
				return;
			}
			else if (emcheckForMoreOrder()==1){			//checks for more orders, either in currentDestination, or in list if Currentdestiantion is empty
				printf("more orders\n");
				emprintshit();
				return;
			}


		}

	}
}
#include "lights.h"
#include <stdio.h>
#include "elev.h"

void lightsIndicatorFloorLights(int floor){
	if (floor==-1){
		return;
	}
	elev_set_floor_indicator( floor);
}

void lightsSetOrderButtonLight (int floor, int button){
	if (!(button == BUTTON_CALL_UP && floor == N_FLOORS - 1)&&(!(button == BUTTON_CALL_DOWN && floor == 0))){
		elev_set_button_lamp( button, floor, 1);
	}
}

void lightsClearOrderButtonLight(int floor, int button){
	if (!(button == BUTTON_CALL_UP && floor == N_FLOORS - 1)&&(!(button == BUTTON_CALL_DOWN && floor == 0))){
		elev_set_button_lamp( button, floor, 0);
	}
}

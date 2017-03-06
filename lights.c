#include "lights.h"
#include <stdlib.h>

void lightsIndicatorFloorLights(int floor){
	elev_set_floor_indicator(int floor);
}

void lightsSetOrderButtonLight (int floor, int button){
	elev_set_button_lamp( button, floor, 1);
}

void lightsClearOrderButtonLight(int floor, int button){
	elev_set_button_lamp( button, floor, 0);
}

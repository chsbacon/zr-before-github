//Begin page main
float position1[3];
float position2[3];
float position3[3];

float myPosition[12];

int point_number;

float xdist;
float ydist;


void init(){
	point_number = 0;
	
	position1[0] = 1;
	position1[1] = 0; 
	position1[2] = 0;
	
	position2[0] = 1;
	position2[1] = 1;
	position2[2] = 0;
	
	position3[0] = 0; 
	position3[1] = 0;
	position3[2] = 0;
	
}

void loop(){
	api.getMyZRState(myPosition);
	
	if (point_number == 0) {
	    xdist = position1[0] - myPosition[0];
	    ydist = position1[1] - myPosition[1];
	    DEBUG(("\n %f", xdist));
	    DEBUG(("\n %f", ydist));
	    api.setPositionTarget(position1);
	    if (sqrtf(xdist*xdist + ydist*ydist) <= 0.05) 
	        point_number = 1;
	}
	else if (point_number == 1) {
	    xdist = position2[0] - myPosition[0];
	    ydist = position2[1] - myPosition[1];
	    api.setPositionTarget(position2);
	    if (sqrtf(xdist*xdist + ydist*ydist) <= 0.05) 
	        point_number = 2;
	}
	else if (point_number == 2) {
	    xdist = position3[0] - myPosition[0];
	    ydist = position3[1] - myPosition[1];
	    api.setPositionTarget(position3);
	    if (sqrtf(xdist*xdist + ydist*ydist) <= 0.05) 
	        point_number = 3;
	}
}  

//End page main

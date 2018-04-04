//Begin page main
//#define DECISION_MESSAGES
#define DEBUG_MESSAGES
#define MOV_DEBUG_MESSAGES
//#define MOV_DEBUG_MESSAGES2

#define  tol  0.05f
#define  MIN_ANGLE  0.79f
#define  GRAY_AREA -1
#define  DARK_AREA   0
#define  LIGHT_AREA  1
#define  MIRROR_COOR 0.4f
#define  MINIM_FUEL    5.0f
#define  VEC_SIZE    12
#define  LIMIT   0.62f

//Declare any variables shared between functions here
short               photos_taken, step, TIME; 
bool                mirror, oponent_in_dark, //(0 for blue, 1 for red)
                    oponent_close, hardstop, slow_down, economy,
                    can_take_a_picture, oponent_converges, wait_flag;
short               light, score_item, old_score;
float               destination[3], *cpos, *cvel, *catt, *crot, *oatt,
                    o_distance, velocity, rot_vel, fuel, energy, distance_to_point,
                    oponent_direction[3], State[12], oState[12], *opos, *ovel, action_energy,
                    desired_attitude[3], distance_to_light;


void init(){
	//This function is called once when your code is first loaded.
	//IMPORTANT: make sure to set any variables that need an initial value.
	//Do not assume variables will be set to 0 automatically!
	step = 2;
	hardstop = false;
	old_score = 100;
	mirror = false;
	economy = false;
}

void loop(){
	//This function is called once per second.  Use it to control the satellite.
	
	float           flocal, fvector[3], light_border;
	
	short           energy_item, mirror_item;
	
	float           vcoef;
	
	bool            must_upload_pictures, dummy = false;
	
	
	TIME = game.getCurrentTime ();
	
	//Read state of opponent's Sphere
	api.getOtherZRState(oState);
	opos = oState;          //opponent's position
	ovel = &oState[3];      //oponent's velocity
	oatt = &oState[6];      //opponent's attitude
	//ovelocity = mathVecMagnitude(ovel,3);
	
	//Read state of our Sphere
	api.getMyZRState(State);
	//copy information to vectors
	cpos = State;               //our position
	cvel = &State[3];           //our velocity
	catt = &State[6];           //attitude (the direction that our camera faces)
	crot = &State[9];           //rotation vector
	
	rot_vel = mathVecMagnitude(crot,3);             //rotational velocity
	velocity = mathVecMagnitude(cvel,3);            //velocity
	photos_taken = game.getMemoryFilled();          //how many photos into memory
	//moving_outwards = (mathVecInner(cpos,cvel,3)>0);  //true if we are moving outwards, false otherwise
	//fuel = game.getFuelRemaining();             //fuel 
    energy = game.getEnergy();                  //energy
    //oenergy = game.getOtherEnergy();          //oponent energy
	
	

    //Sets desired attitude. We want to face the oponent in order to take pictures
    mathVecSubtract(oponent_direction,opos,cpos,3);
    flocal = o_distance;
    o_distance=mathVecNormalize(oponent_direction,3);
    
    
  	
    //check if oponent is close or converges
    oponent_converges = ((o_distance<0.4f)&&(flocal > o_distance)) ? true : false;
    #ifdef MOV_DEBUG_MESSAGES2
    DEBUG(("Converges %d.", oponent_converges));
    #endif

    /*oponent_close = (o_distance<0.4f) ? 1 : 0;
    #ifdef MOV_DEBUG_MESSAGES2
    DEBUG(("Antipalos konta 3: %d", oponent_close));
    #endif*/
    
    //additional information
    //Sets the light variable to 1 (if we are in the light zone), 0 (if we are in grey zone) or -1
    //if we are in dark zone
    light = (game.posInLight(cpos)) ? LIGHT_AREA :  DARK_AREA;
	oponent_in_dark = game.posInDark(opos);
	light_border = game.getLightInterfacePosition();
	distance_to_light = cpos[1] + cvel[1] - light_border - 0.025f;
	if (distance_to_light<-0.07f)
	    distance_to_light = 2.0f;
	//dark_border = (light_border>0) ? light_border - 0.8f : light_border + 0.8f;
	
	can_take_a_picture = SmallCanTakePicture();
	

	//determine the velocity (variable: vcoef) for our movement
	//If the Sphere is in light, then move fast. Otherwise move slower.
	//Also, determine the required energy to take photo and upload (action_energy)
	//action_energy = ((light!=DARK_AREA)&&(distance_to_light > 0.15f)) ? 1.5f  : 2.7f;
	action_energy = ((light!=LIGHT_AREA)&&(distance_to_light>0.90f))||(distance_to_light < 0.15f) ? 2.9f : 1.1f;
	
	must_upload_pictures = MustUpload();
	
    //Messages (printed only if DEBUG_MESSAGES is defined)
	#ifdef DEBUG_MESSAGES
	DEBUG(("TIME: %d, Step: %d, Energy: %f.\n", TIME, step, energy));
	#endif
	
	vcoef = (step<=6) ? 0.7f : 0.55f;
	if (light!=LIGHT_AREA){
	    vcoef = ((energy>2.0f)&&(step<6)) ? 0.7f : (energy>1.2f) ? 0.5f : 0.3f;
	    if (must_upload_pictures)
	        vcoef = 0.3f;
	}
	if (oponent_converges)
	    vcoef = 0.5f;
	 
	
	    
	
	//stop, no more energy...
	flocal = (velocity<0.02f) ? 0.40f : (velocity<0.03f) ? 0.6f : 1.2f;
	hardstop = (light!=LIGHT_AREA)&&(energy<=flocal) ? 1 : hardstop;
	
	
	
	//best suited items (-1 if no items are present)
	score_item = ItemSelection2(3,6);
	mirror_item = ItemSelection2(7,8);
	//energy_item = ItemSelection2(0,2);
	//energy_item = cpos[0]>0 ? 0 : 1;
	//DEBUG(("ENERGY ITEM : %d", energy_item));


    if (TIME<60)
        economy = true;

    //-----------------------------------------------------------------
	//Strategic Steps (step is initialized at 0)
	switch(step){
	  
	
    /*case 0:
        label0:
        vcoef = 0.9f;
        if (energy_item>=0){
            if ((old_energy!=energy_item)&&(TIME>1)){
                goto label_next_step0;
            }
            else{
                game.getItemLoc(destination,energy_item);
                if (fabs(destination[0])<0.15){
                    economy = true; //set economic movement
                    goto label_next_step0;
                }
            }
        }
        else{
            label_next_step0:
            step = 2;
            goto label2;
        }
    break;
    */

    
    case 2: //Score+ selection. Set destination for item picking
        label2:
	    if (score_item>=0){
	        if (old_score!=score_item){
                //DEBUG(("OponentIsAbove : %d", OponentIsAbove())); 
                //if opponent is above 
                if ((distance_to_light<1.05f)&&(destination[2]-opos[2]>0.48f)){
                    step = 6;
                    goto label6;
                }
    	    }
            game.getItemLoc(destination,score_item);
        }
        else{
            //decide when to go for mirror
            if ((distance_to_light<0.70f)||(cpos[2]>0.25)){
                step = 6;
                goto label6;
            }
        }
    break;
    
    
    
    case 6:
        label6:
        if (game.getNumMirrorsHeld()>0){
            step=8;
            mirror = true;
            goto label8;
        }
        if (mirror_item>=0){
            game.getItemLoc(destination,mirror_item);
        }
        else{
            step = 8;
            goto label8;
        }
    break;
    
    
    case 8://go up for pictures
        label8:
        destination[2] = -0.40f;
        destination[0] = (cpos[0]>0) ? 0.45f : -0.45f;
        destination[1] = (distance_to_light<0.90f) ? light_border-0.1f : 0.54f;
        if (mirror){
            if (TIME>141){
                destination[0] = 0.0f;
                destination[1] = 0.0f;
                destination[2] = 0.0f;
            }
            if (TIME>156){
                game.useMirror();
            }
        }
        if ((TIME>150)&&(photos_taken==0)){
             dummy = true;//game.takePic();
        }
        
    break;

	}
	
	if ((energy<1)||(dummy))
	    game.takePic();
	
    //--------------------------------------------------------
	//moving stuff
	economy = (step>2) ? true : economy;
	if (hardstop==0){
        //MoveToDestination(vcoef);
        MoveToDestinationBACON2(vcoef , economy);
	}
    else{
            #ifdef DEBUG_MESSAGES
            DEBUG(("STOP MOVEMENT"));
            #endif
            //stop and wait
            if (velocity>0.0035f){
    	        //HardStabilize();
    	        memcpy(fvector, cvel, VEC_SIZE);
                vrescale(fvector, -4.0f);
                api.setForces(fvector);
                #ifdef MOV_DEBUG_MESSAGES
                DEBUG(("FRENOOOOOOOOOOOOOOOOOOOOOOOOO"));
                #endif
            }
    	    else{
                hardstop=0;
    	    }
    }
    
    
    
    
    //DEBUG(("ACTION ENERGY %f", action_energy));
    //picture taking stuff
    //Normal Picture
    if ( can_take_a_picture ) {
        game.takePic();
        #ifdef DEBUG_MESSAGES
        DEBUG(("TIME %d. TAKING PICTURE of oponent.\n",TIME));
        #endif
        //energy--;
        //photos_taken++;
    }
    
    //mirror code
    if ( (distance_to_light>0.5f)&&(distance_to_light<0.88f)&&(o_distance>0.44f) ){
        if ((mirror)&&(energy<4)&&(TIME<156)){
            mirror = false;
            game.useMirror();
        }
    }
    

    //if memory is full we need to upload.
    if (must_upload_pictures){
        //uploading stuff
        if ( (catt[2]>0.969f)&&(rot_vel<0.05f)&&((energy>1.4f)||(TIME>130)) ){
	        game.uploadPics();
        }
        //memcpy(desired_attitude, EARTH, VEC_SIZE);
        mathVecAdd(desired_attitude, (float *) EARTH, cvel, 3);
    }
    else{
        mathVecAdd(desired_attitude, oponent_direction, ovel, 3);
        mathVecSubtract(desired_attitude, desired_attitude, cvel, 3);
    }
    //mathVecNormalize(desired_attitude,3);
    if ( ((TIME%2==0)||(energy>1.9f)||(light!=DARK_AREA)||(TIME<90)||(!must_upload_pictures))&&(energy>0.7f)&&(TIME>20) )
        api.setAttitudeTarget(desired_attitude);
    
    old_score = score_item;
}












//--------------------------------------------------------------
//------------- Our own functions ------------------------------

short       MirrorSelection(){
    short   blue_mirror = game.hasItem(7);
    short   red_mirror = game.hasItem(8);
    return (cpos[0]>0) ? (blue_mirror>=0 ? 7 : (red_mirror>=0 ? 8 : -1)) 
                       : (red_mirror>=0 ? 8 : (blue_mirror>=0 ? 7 : -1));
}


short       SimpleMirrorSelection(){
    return (cpos[0]>0) ? 7 : 8; 
}

short   SimpleEnergySelection(){
    return (cpos[0]>0) ? 0 : 1;
}



//Determines whether the sphere must upload its photos or not
bool    MustUpload(){
    //if there are 2 photos in memory, the sphere must always upload the photos
    if (photos_taken>1)
        return true;
    
    //if there is only 1 photo in memory, then under some conditions the sphere must upload the photos
    if (photos_taken==1){
        //A. if there aren't enough fuel 
        /*if (fuel<MINIM_FUEL)
            return true;*/
        //B. If we are inside the dark zone
        if ((light==DARK_AREA)&&(energy<2.9f))
            if (distance_to_light>1.0f)
                return true;
        //C. if the game is near the end
        if (TIME>=161)
            return true;
        //D. if  oponent is inside the dark zone
        /*if ((oponent_in_dark)&&(energy>2.0f))
            return true;*/
    }
    return false;
}



//---------------------------------------------------------------
//Determines whether the sphere can upload the photos
bool CanUpload(){
    if ((catt[2]<=0.969f)||(rot_vel>=0.05))
        return false;
    //DEBUG(("I can upload-------------------------------------"));
    return true;
}



//----------------------------------------------
//Determines whether we can take a photo of the opponent
bool    CanTakePicture(){
    //if we  don't have enough energy to move and take photo return false
    if (energy<action_energy)
        return 0;
    //if memory is full return false
    if (photos_taken>1)
        return 0;
    //if we are not facing the oponent return false
    if (!game.isFacingOther())
        return 0;
    //if oponent is in the dark zone return false
    if (oponent_in_dark)
        return 0;
    //if the opponent's distance is greater than 0.5 return false.
    if (o_distance<0.5f)
        return 0;
    //if the camera is off return false
    if (!game.isCameraOn())
        return 0;
    //if the photo scores negative points return false
    if (game.getPicPoints()<=0)
        return 0;
    return 1;
}


bool SmallCanTakePicture(){
    if ((energy<action_energy)||(photos_taken>1)||(!game.isFacingOther())||(oponent_in_dark)||(!game.isCameraOn()))
        return 0;
    if (game.getPicPoints()<=0)
        return 0;
    return 1;   
}



//-----------------------------------------------------------------
//Main Movement function. Directs the sphere to destination
//with a velocity proportional to coef
void MoveToDestination(float coef){
     
    float        vel = 0.07f*coef, vlocal[3], local;
    bool         moving_towards_target=0;

    
    mathVecSubtract(vlocal,destination,cpos,3);
	distance_to_point = mathVecNormalize(vlocal,3);
	
	//determines whether the sphere is moving towards the target
	if (mathVecInner(vlocal,cvel,3)>0.80f*velocity)
	    moving_towards_target = 1;
	
	//rescales the velocity vector.
	vrescale(vlocal, vel);
	
	//If velocity is small move using setPositionTarget
    if (distance_to_point>tol)
        if ((velocity<0.015f)||(!moving_towards_target)){
    	    api.setPositionTarget(destination);
    	    #ifdef MOV_DEBUG_MESSAGES
    	    DEBUG(("SET POSITION!!!"));
    	    #endif
        }
    else
        slow_down = false;
    
	
	//if the sphere is getting closer to destination slow down.
	local =  velocity*3.4f+0.02f;
	if (oponent_converges)
	    local += 0.05f;
	if (distance_to_point<=local){
	    slow_down = true;
	    if (velocity>0.01f)
	        HardStabilize();
	}
	else{//otherwise set velocity proportional to coef.
	    if (( fabs(vel-velocity)>0.008f)&&(!slow_down)){
	        #ifdef MOV_DEBUG_MESSAGES
	        DEBUG(("Setting Velocity %1.4f, %1.4f, %1.4f.",vlocal[0], vlocal[1], vlocal[2]));
	        #endif
	        api.setVelocityTarget(vlocal);
	    }
	}
	
	#ifdef MOV_DEBUG_MESSAGES
    DEBUG(("TIME: %d. Going To: %1.4f %1.4f %1.4f. Vel = %1.4f, des Vel = %1.4f\n", TIME, destination[0], destination[1], destination[2], velocity, mathVecMagnitude(vlocal,3)));
    #endif
}



void MoveToDestinationBACON(){
    float usToIt[3];
    mathVecSubtract(usToIt, destination, cpos, 3);
    float distance = mathVecNormalize(usToIt, 3);
    if (distance > .2) {
        // DEBUG(("VELOCITY CONTROL"));
        vrescale(usToIt, distance/9);
        api.setVelocityTarget(usToIt);
    }
    else {
        //DEBUG(("POSITION CONTROL"));
        api.setPositionTarget(destination);
    }
}

void MoveToDestinationBACON2(float vcoef, bool economy){
    float usToIt[3];
    mathVecSubtract(usToIt, destination, cpos, 3);
    float distance = mathVecNormalize(usToIt, 3), des_vel;
    if (distance > 0.2f) {
        des_vel = distance > 0.4f ? vcoef*0.07f : vcoef*0.175f*distance;
        if (( fabs(des_vel - velocity) > 0.008f)||(distance<0.4f)||(!economy)){
            //DEBUG(("VELOCITY CONTROL : vel = %f  des_vel = %f", velocity, des_vel));
            vrescale(usToIt, des_vel);
            api.setVelocityTarget(usToIt);
        }
    }
    else {
        //DEBUG(("POSITION : vel = %f", velocity));
        api.setPositionTarget(destination);
    }
}



//-----------------------------------------------------------------
//Main Movement function. Directs the sphere to destination
//with a velocity proportional to coef
void MoveToDestinationSmall(float coef){
     
    float        vel = 0.07f*coef, vlocal[3], local;
    bool         moving_towards_target=0;

    
    mathVecSubtract(vlocal,destination,cpos,3);
	distance_to_point = mathVecNormalize(vlocal,3);
	
	//determines whether the sphere is moving towards the target
	if (mathVecInner(vlocal,cvel,3)>0.90f*velocity)
	    moving_towards_target = 1;
	
	//rescales the velocity vector.
	vrescale(vlocal, vel);
	
	//If velocity is small move using setPositionTarget
    if ((distance_to_point>tol)&&((fabs(velocity-vel)>0.008f)||(!moving_towards_target))){
	    api.setPositionTarget(destination);
	    #ifdef MOV_DEBUG_MESSAGES
	    DEBUG(("SET POSITION!!!"));
	    #endif
    }
        
	//if the sphere is getting closer to destination slow down.
	local = velocity*2.86f+0.05f;
	local = oponent_close ? local+0.12f : local;
	if (distance_to_point<=local){
	    if (velocity>0.015f)
	        HardStabilize();
	}
	
	#ifdef MOV_DEBUG_MESSAGES
    DEBUG(("TIME: %d. Going To: %1.4f %1.4f %1.4f. Vel = %1.4f, des Vel = %1.4f\n", TIME, destination[0], destination[1], destination[2], velocity, mathVecMagnitude(vlocal,3)));
    #endif
	
	/*if (distance_to_point<tol){
        r=1;
    }
	return r;*/
}





//------------------------------------------------------
//rescales a vector a --> l*a
void    vrescale(float *vector, float local)
{
    vector[0] *=local;
    vector[1] *=local;
    vector[2] *=local;
}


//--------------------------------------------------
//Stabilization function - Hard breaks
void  HardStabilize(){
    float    force[3];
    
    memcpy(force, cvel, VEC_SIZE);
    vrescale(force, -4.0f);
    api.setForces(force);
    #ifdef MOV_DEBUG_MESSAGES
    DEBUG(("FRENOOOOOOOOOOOOOOOOOOOOOOOOO"));
    #endif
}



//--------------------------------------------------------
//find the closest (free) item with item id between i_start and i_stop.
//The function disregards the items that are closer to the oponent
//returns -1 if there are no other free items.
short    ItemSelection(short i_start, short i_stop){
    float       pos1[3],d1,d2,d0;
    short       i,i0=-1;
    
    d0 = 100;
    for (i=i_start;i<=i_stop;i++){
        if (game.hasItem(i)==-1){
            game.getItemLoc(pos1,i);
            d1 = DistanceBetweenPoints(pos1,cpos);
            d2 = DistanceBetweenPoints(pos1,opos);
            if ((d1<=d0)&&(d1<d2)){
                d0=d1;
                i0=i;
            }
        }
    }
    if (i0==-1)
        return -1;
    else
        return i0;    
}


//--------------------------------------------------------
//find the closest (free) item with item id between i_start and i_stop.
//The function disregards the items that are closer to the oponent
//and the oponent moves toward them.
//returns -1 if there are no other free items.
short    ItemSelection2(short i_start, short i_stop){
    float       pos1[3],diff[3],d1,d2,d0;
    short       i,i0=-1;
    
    d0 = 100 ;
    for (i=i_start;i<=i_stop;i++){
        if (game.hasItem(i)==-1){
            game.getItemLoc(pos1,i);
            d1 = DistanceBetweenPoints(pos1,cpos);
            mathVecSubtract(diff, pos1, opos,3);
            d2 = mathVecNormalize(diff,3);
            /*if (i_start == 0){
                DEBUG(("oponent direct: %f", mathVecInner(diff,ovel,3)/mathVecMagnitude(ovel,3))); 0.988f
                DEBUG(("my distance = %f, op distance = %f", d1+temp, d2*d2));
            }*/
            if ( ( ((mathVecInner(diff,ovel,3) < mathVecMagnitude(ovel,3)*0.988f))&&(d2>0.18f))||(d1<d2*d2) )
                if (d1<=d0){
                    d0 = d1;
                    i0=i;
                }
        }
    }
    return i0;    
}


float   DistanceBetweenPoints(float  *A,  float *B){
    return (A[0]-B[0])*(A[0]-B[0])+(A[1]-B[1])*(A[1]-B[1])+(A[2]-B[2])*(A[2]-B[2]);
}
//End page main

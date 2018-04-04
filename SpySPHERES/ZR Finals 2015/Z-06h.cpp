//Begin page main
//#define DECISION_MESSAGES
#define DEBUG_MESSAGES
//#define MOV_DEBUG_MESSAGES
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
bool                must_upload_pictures, oponent_in_dark, //(0 for blue, 1 for red)
                    oponent_close, hardstop, slow_down, pack_detected,
                    can_take_a_picture, oponent_converges, can_take_a_picture_no_mirror;
short               light, score_item, old_score, old_energy, tt, mirrors, hugger;
float               destination[3], *cpos, *cvel, *catt, *crot, *oatt,
                    o_distance, velocity, rot_vel, fuel, energy,
                    oponent_direction[3], State[12], oState[12], *opos, *ovel, action_energy,
                    desired_attitude[3], distance_to_light, distance_to_dark, distance,
                    dark_border, light_border;


void init(){
	//This function is called once when your code is first loaded.
	//IMPORTANT: make sure to set any variables that need an initial value.
	//Do not assume variables will be set to 0 automatically!
	step = 6;
	hardstop = false;
	old_score = 100;
	//old_energy = 100;
	must_upload_pictures = false;
	hugger = 0;
}



void loop(){
	//This function is called once per second.  Use it to control the satellite.
	
	float           flocal, fvector[3], score_location[3], energy_location[3], oenergy;
	
	short           energy_item, mirror_item;
	
	float           vcoef;
	
	bool            sphere_is_getting_into_light;
	
	
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
    oenergy = game.getOtherEnergy();          //oponent energy
	

    //Sets desired attitude. We want to face the oponent in order to take pictures
    mathVecSubtract(oponent_direction,opos,cpos,3);
    //flocal = o_distance;
    o_distance=mathVecNormalize(oponent_direction,3);
    
    
  	
    //check if oponent is close or converges
    //oponent_converges = ((o_distance<0.4f)&&(flocal > o_distance)) ? true : false;
    //#ifdef MOV_DEBUG_MESSAGES2
    //DEBUG(("Converges %d.", oponent_converges));
    //#endif

    //oponent_close = (o_distance<0.3f) ? true : false;
    //#ifdef MOV_DEBUG_MESSAGES2
    //DEBUG(("Antipalos konta 3: %d", oponent_close));
    //#endif*/
    
    //additional information
    //Sets the light variable to 1 (if we are in the light zone), 0 (if we are in grey zone) or -1
    //if we are in dark zone
    //light = (game.posInLight(cpos)) ? LIGHT_AREA : game.posInDark(cpos) ? DARK_AREA : GRAY_AREA;
    light = (game.posInLight(cpos));
	oponent_in_dark = game.posInDark(opos);
	light_border = game.getLightInterfacePosition();
	distance_to_light = cpos[1] + cvel[1] - light_border - 0.025f;
	dark_border = (light_border>0) ? light_border - 0.8f : light_border + 0.8f;
	distance_to_dark = cpos[1] + cvel[1] - dark_border - 0.025f;
	
	sphere_is_getting_into_light = ((dark_border>light_border)^(distance_to_light<-0.06f))
	                        &(distance_to_dark<0.06f);
	
	can_take_a_picture_no_mirror = ((tt==0)&(energy>action_energy)&(photos_taken<=1)
	            &(game.isFacingOther())&(!oponent_in_dark)&(game.isCameraOn()));

	can_take_a_picture = false;
	if (can_take_a_picture_no_mirror)
	    can_take_a_picture = (game.getPicPoints()>0);

	//determine the velocity (variable: vcoef) for our movement
	//If the Sphere is in light, then move fast. Otherwise move slower.
	//Also, determine the required energy to take photo and upload (action_energy)
	//action_energy = ((light!=DARK_AREA)&&(distance_to_light > 0.15f)) ? 1.5f  : 2.7f;
	action_energy = (TIME>53) ? 1.3f : 3.8f;
	//DEBUG(("Action energy : %f", action_energy));
	
	must_upload_pictures = (photos_taken>1)
	                    |( (photos_taken==1)&(((energy<action_energy)&(distance_to_dark>0.2f))|(tt>0)|(TIME>=163)) )
	                    |(must_upload_pictures&(photos_taken>0));
	
    
	tt = game.getMirrorTimeRemaining();
	
	
    //Messages (printed only if DEBUG_MESSAGES is defined)
	#ifdef DEBUG_MESSAGES
	DEBUG(("TIME: %d, Step: %d, Energy: %f.\n", TIME, step, energy));
	#endif
	
	//best suited items (-1 if no items are present)
	mirror_item = ItemSelection(7,8);
	score_item = ItemSelection(3,6);
	game.getItemLoc(score_location,score_item);
	#ifdef DEBUG_MESSAGES
	DEBUG(("SCORE ITEM : %d", score_item));
	#endif

	
	vcoef = (light) ? 0.55f : 0.3f;
	
	//for Hugme
	hugger = (step==8)&(o_distance<0.42f)&(tt<=4) ? hugger+1 : 0;
	DEBUG(("HUGGER = %d", hugger));
	
	//stop, no more energy...
	flocal = (velocity<0.02f) ? 0.70f : (velocity<0.03f) ? 0.9f : 1.3f;
	if (TIME>50)                                                            //for Hugme
	    hardstop = ((!light)&(energy<=flocal))|((cpos[1]>-0.66f)&(hugger>4)&((oenergy>0.9f)|(energy<2.8f))) ? 1 : hardstop;
	
	
	 
	
    DEBUG(("STEP  %d",step));

    //-----------------------------------------------------------------
	//Strategic Steps (step is initialized at 0)
	switch(step){
	
	case 4:
    label4:
        if (score_item>=0){
            memcpy(destination,score_location, VEC_SIZE);
            //if dark is close or if the oponent is moving towards the dark zone
            if (  ((distance_to_light<0.45f)&(distance_to_light>-0.05f)&(o_distance>0.4f))
                |((opos[1]-cpos[1]<-0.1f)&(ovel[1]<-0.15f)) )
                destination[1] = light_border - 0.1f;
        }
        else{
            step = 8;
        }
    break;


    case 5:
    label5:
        memcpy(destination,score_location, VEC_SIZE);
        vcoef = 0.5f;
        if (light){
            step = 4;
        }
    break;



    case 6:
        label6:
        mirrors = 0;
        if (game.hasItem(7)==0){
            mirrors++;
        }
        if (game.hasItem(8)==0){
            mirrors++;
        }
        DEBUG(("Number of mirrors: %d",mirrors));
        if (mirror_item>=0){
            game.getItemLoc(destination,mirror_item);
            vcoef = 0.9f;
            if (o_distance<0.3f)
                vcoef = 0.3f;
        }
        else{
            step = (mirrors==2) ? 8 : 5;
        }
    break;
    
    
    case 8://go up for pictures
        label8:
        if ((TIME<75)&(light))
            vcoef = 0.85f;
        destination[2] = -0.45f;
        destination[0] = 0.0f;
        if ((TIME<130)&(!light)&(!oponent_in_dark)&(opos[1]-cpos[1]<0.55f)){
            destination[1] = max(-0.7, opos[1] - 0.6f);
        }
        else
            destination[1] = sphere_is_getting_into_light ? max(-0.78, dark_border-0.85f) 
                                                     : ( (TIME<100) ? max(-0.2f, cpos[1]-0.2f) : 0.1f );
        
        if (TIME>130){
            if (photos_taken==0){
                //for Hugme
                if (game.getScore()>6){
                    hardstop=false;
                    //vcoef = 0.5f;
                }
                else{
                    destination[1] = 0.3f;
                    vcoef = 0.5f;
                }
            }
            else{//stop to upload photos
                if (o_distance<0.43f)
                    hardstop=true;
            }
            //if we couldn't take photos on a downstream movement, go for scores.
            if ((score_item>=0)&(energy>3)){
                step = 4;
            }
        }
        //DEBUG(("oponent distance = %f", o_distance));
        //if we are close to opponent go for score items
        else if ((score_item>=0)&(
                            (opos[1]<cpos[1])
                            |((ovel[1]<-0.010f)&((o_distance<0.35f)|(opos[1]-cpos[1]<0.25f)))
                            )){
            step = 4;
        }
    break;
    
    
    

	}
	
	
	/*if (((TIME>150)&(photos_taken==0))|((tt>8)&(o_distance<0.35f)))
	    dummy = true;*/
    //--------------------------------------------------------
	//moving stuff
	if (hardstop==0){
	    //MoveToDestinationBACON();
        //MoveToDestinationBACON2(vcoef , (step>2)||(light==LIGHT_AREA));
        mathVecSubtract(fvector, destination, cpos, 3);
        distance = mathVecNormalize(fvector, 3);
        if (distance > 0.30f) {
            flocal = vcoef*0.07;
            if ((  fabs(flocal - velocity) > 0.008f)|(step!=6)|(light)){
                //DEBUG(("VELOCITY CONTROL : vel = %f  des_vel = %f", velocity, flocal));
                vrescale(fvector, flocal);
                api.setVelocityTarget(fvector);
            }
        }
        else {
            DEBUG(("POSITION : vel = %f", velocity));
            api.setPositionTarget(destination);
        }
        DEBUG(("GOING TO %f  %f  %f", destination[0], destination[1], destination[2]));
	}
    else{
        #ifdef DEBUG_MESSAGES
        DEBUG(("STOP MOVEMENT"));
        #endif
        //stop and wait
        if (velocity>0.0035f){
	        HardStabilize();
        }
	    else{
            hardstop=0;
	    }
    }
    
    
    
    
    //DEBUG(("ACTION ENERGY %f", action_energy));
    //picture taking stuff
    //Normal Picture
    if (can_take_a_picture ) {
        game.takePic();
        #ifdef DEBUG_MESSAGES
        DEBUG(("TIME %d. TAKING PICTURE of oponent.\n",TIME));
        #endif
        //energy--;
        //photos_taken++;
    }
    //mirror code
    if ( ((step == 8)&(sphere_is_getting_into_light))|
            ((step!=8)&(mathVecInner(oatt,oponent_direction,3)<-0.80f)&(o_distance>0.44f)&(sphere_is_getting_into_light)) ){
        if (tt<=1){
            game.useMirror();
            DEBUG(("USING MIRROR"));
        }
    }
    //if memory is full we need to upload.
    if (must_upload_pictures){
        //uploading stuff
        if ( (catt[2]>0.969f)&(rot_vel<0.05f)&(energy>1.4f) ){
	        game.uploadPics();
        }
        //memcpy(desired_attitude, EARTH, VEC_SIZE);
        mathVecAdd(desired_attitude, (float *) EARTH, cvel, 3);
    }
    else{
        vrescale(oponent_direction, o_distance);
        mathVecAdd(desired_attitude, oponent_direction, ovel, 3);
        mathVecSubtract(desired_attitude, desired_attitude, cvel, 3);
    }
    //mathVecNormalize(desired_attitude,3);
    //for hugger
    if ( (step!=6)&(((hugger<4)&(energy>0.5f))|(must_upload_pictures)) ){
        api.setAttitudeTarget(desired_attitude);
        //DEBUG(("ROTATE"));
    }
    
}












//--------------------------------------------------------------
//------------- Our own functions ------------------------------
/*bool  SphereInLight(){
    if ((dark_border>light_border)&(distance_to_dark<0.08f)&(distance_to_light>-0.08f))
        return true;
    if ((dark_border<light_border)&(distance_to_dark<0.08f)&(distance_to_light<-0.08f))
        return true;
    return false;
}*/

bool  SphereInLight(){
    return (distance_to_dark<0.08f)&(
                ((dark_border>light_border)&(distance_to_light>-0.08f))
                    |(dark_border<light_border)&(distance_to_light<-0.08f)   );
 
}


float  max(float a, float b){
    return (a>b) ? a : b;
}

float  min(float a, float b){
    return (a<b) ? a : b;
}


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
        //B. If we are inside the dark zone
        if (((energy<action_energy)&(distance_to_dark>0.2f))|(tt>0))
                return true;
        //C. if the game is near the end
        if (TIME>=161)
            return true;

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
    if ((tt>0)|(energy<action_energy)|(photos_taken>1)|(!game.isFacingOther())|(oponent_in_dark)|(!game.isCameraOn()))
        return 0;
    return 1;   
}





void MoveToDestinationBACON(){
    float usToIt[3];
    mathVecSubtract(usToIt, destination, cpos, 3);
    float distance = mathVecNormalize(usToIt, 3);
    if (distance > .2) {
        // DEBUG(("VELOCITY CONTROL"));
        vrescale(usToIt, distance/7);
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
            if ((d1<=d0)&(d2>0.017f)){
                d0=d1;
                i0=i;
            }
        }
    }
    
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
            if ( ( ((mathVecInner(diff,ovel,3) < mathVecMagnitude(ovel,3)*0.991f))&(d2>0.18f))|(d1<d2*d2) )
                if (d1<=d0){
                    d0 = d1;
                    i0=i;
                }
        }
    }
    return i0;    
}


//--------------------------------------------------------
//find the closest (free) item with item id between i_start and i_stop.
//If there are score packs it returns the nearest pack
//The function disregards the items that are closer to the oponent
//and the oponent moves toward them.
//returns -1 if there are no other free items.
short    ItemSelection3(short i_start, short i_stop){
    float       pos1[3],diff[3],d1,d2,d0;
    short       i,i0=-1;
    
    d0 = 100 ;
    pack_detected = false;
    for (i=i_start; i<=i_stop; i++){
        if (game.hasItem(i)==-1){
            game.getItemLoc(pos1,i);
            if ((i_start==3)&(fabs(pos1[0])<0.18f)&(pos1[2]<0.2f)&(TIME<30)){
                pack_detected = true;
                #ifdef DEBUG_MESSAGES
                DEBUG(("Pack Detected --- %d", pack_detected));
                #endif
                return (cpos[0]>0) ? i : i+1;
            }
            d1 = DistanceBetweenPoints(pos1,cpos);
            mathVecSubtract(diff, pos1, opos,3);
            d2 = mathVecNormalize(diff,3);
            if ( ( ((mathVecInner(diff,ovel,3) < mathVecMagnitude(ovel,3)*0.988f))&(d2>0.18f))|(d1<d2*d2) )
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



/*
short   InitialDecision(){
    short   Up, Down;
    float   pos[6],*Uppos,*Downpos;
    
    Up = cpos[0]>0 ? 3 : 4;
    Down = Up + 2;
    game.getItemLoc(pos,Up);
    game.getItemLoc( &pos[3], Down);
    
    if (pos[2]>pos[5]){
        Up = Down;
        Down = Up - 2;
        Uppos = &pos[3];
        Downpos = pos;
    }
    else{
        Uppos = pos;
        Downpos = &pos[3];
    }
    DEBUG(("upper item : %d z = %f", Up, Uppos[2]));
    DEBUG(("lower item : %d z = %f", Down, Downpos[2]));
    
    if (Uppos[0] - Downpos[0])
}*/

//End page main

//Begin page main
//#define DECISION_MESSAGES
// #define DEBUG_MESSAGES
//#define MOV_DEBUG_MESSAGES
//#define MOV_DEBUG_MESSAGES2

#define  tol  0.05f
#define  MIN_ANGLE  0.79f
#define  GRAY_AREA -1
#define  DARK_AREA   0
#define  LIGHT_AREA  1
#define  MINIM_FUEL    5.0f
#define  VEC_SIZE    12
#define  LIMIT   0.62f

//Declare any variables shared between functions here
short               photos_taken, step, TIME; 
bool                must_upload_pictures, oponent_in_dark,
                    oponent_close, slow_down, light, pack_detected, hugme;
                    // can_take_a_picture, oponent_converges, can_take_a_picture_no_mirror;
short               score_item, mirror_item/*, old_score, old_energy*/, tt, mirrors;
float               destination[3], *cpos, *cvel, *catt, *crot, *oatt,
                    o_distance, velocity, rot_vel, fuel, energy,
                    oponent_direction[3], State[12], oState[12], *opos, *ovel,
                    desired_attitude[3], distance_to_light, distance_to_dark, distance,
                    dark_border, light_border, ovelocity;

    
void init(){
	//This function is called once when your code is first loaded.
	//IMPORTANT: make sure to set any variables that need an initial value.
	//Do not assume variables will be set to 0 automatically!
	step = 6;
	mirrors=0;
	mirror_item=-1;
// 	old_score = 100;
	//old_energy = 100;
	must_upload_pictures = 0;
	hugme = false;
	api.setAttGains(1.0f,.0f,1.0f);
}



void loop(){
	//This function is called once per second.  Use it to control the satellite.
	
	float           flocal, fvector[3], score_location[3], oenergy, vcoef;
	
// 	bool            /*dummy = false,*/ sphere_is_getting_into_light;
	
	
	TIME = game.getCurrentTime ();
	
	//Read state of opponent's Sphere
	api.getOtherZRState(oState);
	opos = oState;          //opponent's position
	ovel = &oState[3];      //oponent's velocity
	oatt = &oState[6];      //opponent's attitude
	ovelocity = mathVecMagnitude(ovel,3);
	
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
    if (((mathVecInner(ovel,oponent_direction,3)<-0.997f*ovelocity)&(ovelocity>.01f)))hugme=true;
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
    // light = (game.posInLight(cpos)) ? LIGHT_AREA : DARK_AREA;
	light = game.posInLight(cpos);
	oponent_in_dark = game.posInDark(opos);
	light_border = game.getLightInterfacePosition();
	distance_to_light = cpos[1] + cvel[1] - light_border - 0.025f;
	dark_border = (light_border>0) ? light_border - 0.8f : light_border + 0.8f;
	distance_to_dark = cpos[1] + cvel[1] - dark_border - 0.025f;
	
// 	sphere_is_getting_into_light = ((dark_border>light_border)^(distance_to_light<0.1f)) //changed last num
//	                        &(distance_to_dark<0.08f);
	
// 	can_take_a_picture_no_mirror = ((tt==0)&(energy>action_energy)&(photos_taken<=1)
	           // &(game.isFacingOther())&(!oponent_in_dark)&(game.isCameraOn()));

// 	can_take_a_picture = false;
// 	if (can_take_a_picture_no_mirror)
	   // can_take_a_picture = (game.getPicPoints()>0);
    // can_take_a_picture= ((tt==0)&(energy>action_energy)&(photos_taken<=1)
	           // &(game.isFacingOther())&(!oponent_in_dark)&(game.isCameraOn()))
	           // && (game.getPicPoints()>0); //boolean and for short-circuiting
	//determine the velocity (variable: vcoef) for our movement
	//If the Sphere is in light, then move fast. Otherwise move slower.
	//Also, determine the required energy to take photo and upload (action_energy)
	//action_energy = ((light!=DARK_AREA)&&(distance_to_light > 0.15f)) ? 1.5f  : 2.7f;

// 	#define action_energy 1.3f
	
	//DEBUG(("Action energy : %f", action_energy));
	
	must_upload_pictures = (photos_taken>1)
	                    |((((((energy<1.1f)&(distance_to_dark>0.2f))|(tt>0)|(TIME>=163)) )
	                    |(must_upload_pictures))&(photos_taken>0));
	
    
	tt = game.getMirrorTimeRemaining();
	
	
    //Messages (printed only if DEBUG_MESSAGES is defined)
	
	if ((mirror_item!=-1)&(game.hasItem(mirror_item)==0)) mirrors++;
	//best suited items (-1 if no items are present)
	mirror_item = ItemSelection(7,8);
	score_item = ItemSelection(3,6);
	game.getItemLoc(score_location,score_item);

	
	vcoef = (light) ? 0.55f : 0.28f;
	
	
	//stop, no more energy...
// 	flocal = (velocity<0.02f) ? 0.70f : (velocity<0.03f) ? 0.9f : 1.3f;
	
	
    // DEBUG(("vcoef:%f",vcoef));
    //-----------------------------------------------------------------
	//Strategic Steps (step is initialized at 0)
	switch(step){
	
	case 4:
    label4:
        if (score_item>=0){
            memcpy(destination,score_location, VEC_SIZE);
            //if dark is close or if the oponent is moving towards the dark zone
            // DEBUG(("needed speed: %f",(tt>0?distance_to_light/tt:-1)));
            //previously included (distance_to_light<0.4f)&(distance_to_light>-0.05f)
            if ( ((tt>0) bitand (distance_to_light>0.025f*(tt-1))) | ((tt<8)&(distance_to_light>-0.1f)&(o_distance>0.4f))
                        |((opos[1]-cpos[1]<-0.1f)&(ovel[1]<-0.15f)) )
                destination[1] = light_border - 0.2f;
        }
        else {
            step = 8;
        }
    break;


    /*case 5:
    label5:
        memcpy(destination,score_location, VEC_SIZE);
        vcoef = 0.5f;
        if (light){
            step = 4;
        }
    break;*/



    case 6:
        label6:
        // mirrors = game.getNumMirrorsHeld();
        DEBUG(("Number of mirrors: %d",mirrors));
        if (mirror_item!=-1){
            game.getItemLoc(destination,mirror_item);
            vcoef=0.9f;
            if (o_distance<0.3f)
                vcoef = .4f;
                
            float theirDist = DistanceBetweenPoints(destination,opos);
            float ourDist = DistanceBetweenPoints(destination,cpos);
            mathVecSubtract(fvector,destination,opos,3);
            mathVecNormalize(fvector,3);
            if ((theirDist>.02f) & mathVecInner(fvector,ovel,3)>.93f*ovelocity
            & (ourDist>.03f) & (mirrors==1) & ((theirDist>ourDist-.2f)) )// MAJOR CHANGE !!!!!!!
                vcoef = .27f*ourDist/theirDist;
                
            
        }
        else{
            if (mirrors == 2){
                step = 8;
            }
            else{
                step = 4;
            }
        }
    break;
    
    
    case 8://go up for pictures
        label8:
        if ((TIME<75)&(light))
            vcoef = 1.428f;
        destination[2] = -0.35f;
        destination[0] = cpos[0]*0.85f;
        if (hugme bitand (TIME<130)){
            vcoef = 1.0f;
            destination[1]=-0.72f;
            if (/*(cpos[0]<-.5f) bitand */ (o_distance<.42f)){
                destination[2]=-.1f;
            }
        }
        else{
            destination[1] = light ? max(-0.78f, dark_border-1.1f) 
                                                     : ( (TIME<100)&(energy>1) ? min(-0.2f, dark_border+0.2f) : -0.1f );
            if (hugme){
                destination[1]=-0.2f;
                vcoef = 0.15f;
            }
            else if (TIME>100){
                if (photos_taken==0){
                    vcoef = 0.5f;
                }
                //if we couldn't take photos on a downstream movement, go for scores.
                if ((score_item>=0)&(energy>3)){
                    step = 4;
                }
            }
            else if ((!hugme)&(score_item>=0)&(
                                (opos[1]<cpos[1])
                                |((ovel[1]<-0.010f)&((o_distance<0.35f)|(opos[1]-cpos[1]<0.25f)))
                                )){
                step = 4;
            }
        }
        
    break;
    
    
    

	}
    //--------------------------------------------------------
	//moving stuff
	if ((TIME<60)|(light)|(energy>(((velocity<0.03f) ? 0.7f : 0.9f)))) { // hardstop-esque logic
        mathVecSubtract(fvector, destination, cpos, 3);
        distance = mathVecNormalize(fvector, 3);
        if (distance > 0.40f) {
            flocal = vcoef*0.07f;
            if ( ( fabs(flocal - velocity) > 0.008f)|(step!=6)|(light==LIGHT_AREA)){
                //DEBUG(("VELOCITY CONTROL : vel = %f  des_vel = %f", velocity, flocal));
                vrescale(fvector, flocal);
                api.setVelocityTarget(fvector);
            }
        }
        else{
            api.setPositionTarget(destination);
        }
	}
	else {
	    memset(fvector,0.0f,VEC_SIZE);
	    api.setVelocityTarget(fvector);
	    DEBUG(("stopping"));
	}
    
    
    
    //DEBUG(("ACTION ENERGY %f", action_energy));
    //picture taking stuff
    //Normal Picture
    if ((((tt==0)&(energy>1.2f)&(photos_taken<=1)
	        &(game.isFacingOther())&(!oponent_in_dark)&(game.isCameraOn()))
	        && (game.getPicPoints()>0))
	        |((energy<1) | ((TIME>150)&(photos_taken==0))|((tt>8)&(o_distance<0.35f)))) {
        game.takePic();
    }
    DEBUG(("%f   %f",distance_to_light,distance_to_dark));
    DEBUG(("HugMe=%d",hugme));
    //mirror code
    // if ( !(hugme|TIME<45)/*&(mathVecInner(oatt,oponent_direction,3)<-0.80f)*/&((o_distance>0.4f) bitor step==8)&(tt<=1)&((light_border>dark_border)?(distance_to_light<.2f & distance_to_light>-.6f):((distance_to_light<.2f) bitor (distance_to_dark>-.2f)))){
    if ( !(hugme)
        &(((step == 8)|
        ((mathVecInner(oatt,oponent_direction,3)< -0.80f)&(o_distance>0.4f)))
        &(tt<=1) & ((distance_to_dark>.65f) bitor (distance_to_dark<.15f bitand distance_to_light> -.15f))) ) {
            game.useMirror();
            DEBUG(("USING MIRROR"));
    }
    //if memory is full we need to upload.
    // DEBUG(("must_upload_pictures:%d",must_upload_pictures));
    vrescale(oponent_direction, o_distance);
    mathVecAdd(desired_attitude, oponent_direction, ovel, 3);
    mathVecSubtract(desired_attitude, desired_attitude, cvel, 3);
    if (must_upload_pictures/*&&!huggingNow*/){
        //uploading stuff
        if ( (catt[2]>0.969f)&(rot_vel<0.05f)&(energy>((TIME>165)?1.f:1.2f)) ){
	        game.uploadPics();
        }
        memcpy(desired_attitude, EARTH, VEC_SIZE);
        // mathVecAdd(desired_attitude, (float *) EARTH, cvel, 3);
    }
    //mathVecNormalize(desired_attitude,3);
    if ( (TIME>50)&(energy>0.5f)){
        DEBUG(("Rotating to %s",must_upload_pictures?"upload":"take pic"));
        api.setAttitudeTarget(desired_attitude);
        //DEBUG(("ROTATE"));
    }
    
}












//--------------------------------------------------------------
//------------- Our own functions ------------------------------

float  max(float a, float b){
    return (a>b) ? a : b;
}

float  min(float a, float b){
    return (a<b) ? a : b;
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
            if ((d1<=d0)&((d2>0.02f)|(ovelocity>0.02f))){ //just happen to be the same
                d0=d1;
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

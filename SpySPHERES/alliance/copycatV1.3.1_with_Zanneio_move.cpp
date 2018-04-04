//Begin page main
//Begin page main
//Begin page main
//Begin page Awareness
bool enSoonInLight() {
    #define seconds 4
    float soonlightTail = (LIGHT_SPEED*seconds)+lightTail;
    if(soonlightTail>0.8f) soonlightTail-=1.6f; // if it's out of bounds, correct it
    float soonlightFront = (soonlightTail + 0.9f); // 0.7 for dark and 0.2 for both grey
    if(soonlightFront>0.8f) soonlightFront-=1.6f; // if it's out of bounds, correct it
    float predictedPos=enPos[1]+(enVel[1]*seconds);
        //if their predicted pos is past either end of the ligh and grey zones return true
    return (predictedPos>soonlightTail || predictedPos<soonlightFront);
}
//End page Awareness
//Begin page Item Selection
short closestInRange(short start, short stop) {
    short closest = start;
        //make sure starting item is available
    while (game.hasItem(closest)!=-1 && closest<stop) closest++;
    
    for (short i=closest+1; i<=stop; i++) {
        if (game.hasItem(i)==-1 &&
        distanceBetween(ItemLoc[i],myPos)<distanceBetween(ItemLoc[closest],myPos)
        &&distanceBetween(ItemLoc[i],enPos)>0.25f)
            closest = i;
    }
    return closest; //closest may have been gotten but it will never return -1
}
//End page Item Selection

//Begin page Vectors
float angleBetween(float vec1[3], float vec2[3])
{
	return acosf(mathVecInner(vec1, vec2, 3)/(mathVecMagnitude(vec1,3) * mathVecMagnitude(vec2,3)));
}

float distanceBetween(float pos1[3],float pos2[3]){//2%
    float distance[3];
    mathVecSubtract(distance,pos1,pos2,3);
    return mathVecMagnitude(distance,3);
}

void scale(float vector[], float scalar) {//2%  does edit the passed array
    for (int i = 0; i < 3; i++) {
        vector[i] *= scalar;
    }
}


//End page Vectors
//Begin page main
//state pointers
float myState[12];
float *myPos;
float *myVel;
float *myAtt;
float *myRot;
float myEnergy;

float enState[12];
float *enPos;
float *enVel;
float enEnergy;

short myPosInArea, enPosInArea;

float uploadVec[3];
float zeroVec[3];

float ItemLoc[NUM_ITEMS][3];

short gameTime;
bool enemyHasMirror;
short mirrorTimerEnd;
bool enemyMirrorActivated;

float lightTail, lightFront, myDistToLightFront;

short closeMirror, closePP, closeEnergy;
// bool firstLightPassWithMirror
bool abandoningPPs, slip;
short pps;

void init()
{
    memcpy(uploadVec, EARTH, 12);
    memset(zeroVec, 0.0f, 12);
    mirrorTimerEnd = 240;
    enemyMirrorActivated=false;
    gameTime = -1;

	//The following are accessed as 3 element arrays
	myPos=myState;
	myVel=&myState[3];
	myAtt=&myState[6];
	myRot=&myState[9];
	enPos=enState;
	enVel=&enState[3];

	//Filling ItemLoc
	for(int i=0;i<NUM_ITEMS;i++)
	{
	    game.getItemLoc(ItemLoc[i],i);
	}
	    //we have to have a defined pos to determine closest pointpack 
	api.getMyZRState(myState);
    closePP = 3;
    // firstLightPassWithMirror=true;  
    abandoningPPs=slip=false;
    pps=0;
}


void loop()
{
    update();
    float waypoint[3]; //used for getting out of the light
    int memoryFilled = game.getMemoryFilled();
    if (game.hasItem(closePP)==0) pps++;
    closePP=closestInRange(3,6);
    
    //get point-packs    
	if ((gameTime<45 || myPosInArea!=1)&&pps<2&&!abandoningPPs) {
    	DEBUG(("going for point-packs(slip:%d)", slip));
	    closeMirror = closestInRange(7,8);
	    if (myEnergy>.6f){
	        mathVecSubtract(waypoint, ItemLoc[closePP], myPos, 3);
            float distance = mathVecNormalize(waypoint, 3);
            if (distance > 0.30f) {
                scale(waypoint, (distance > 0.4f ? 
                    (gameTime<13? 1.6f : 0.7f) *0.07f :
                    (gameTime<13? 1.6f : 0.7f)*0.175f*distance));
                api.setVelocityTarget(waypoint);
            }
            else {
                api.setPositionTarget(ItemLoc[closePP]);
            }
	    }
	    else{
	        api.setVelocityTarget(zeroVec);
	    }
        slip=(enEnergy<1.1f && myEnergy>0.75f
        && enPos[1]-lightFront>0.2f);
    }
    
    //slip through light before getting mirror
    else if (slip && !(myPosInArea==-1/*&&myDistToLightFront<-0.1*/)) { //check this
        DEBUG(("slipping through"));
        abandoningPPs=true;
        memcpy(waypoint, ItemLoc[closeMirror], 12);
        waypoint[1]=lightTail - 0.2f;
        //if (myDistToLightFront<0.1 || myPosInArea==-1)
        api.setPositionTarget(waypoint);
    }
    
    //pick up mirror and wait
    else if (game.hasItem(closeMirror)==-1) {//||(game.getNumMirrorsHeld()>0 && myDistToLightFront>0.15f&& myDistToLightFront<0.35f)) {
        DEBUG(("going for mirror"));
        slip=false;
        abandoningPPs=true;
        api.setPositionTarget(ItemLoc[closeMirror]);
        closeEnergy=closestInRange(0,2);
    }
    
    //pass through the light with mirror to get energy
    else if ((game.getMirrorTimeRemaining()>0) || 
    (myDistToLightFront < 0.15f && myDistToLightFront > 0.0f &&game.getNumMirrorsHeld()>0)){
        DEBUG(("going for energy waypoint and deploying mirror"));
        game.useMirror();
        memcpy(waypoint,ItemLoc[closeEnergy],12);
        waypoint[1] = lightTail<0.0f?lightTail:-0.4f;
        api.setPositionTarget(waypoint);
    }
    
    //get energy
    else if (gameTime > 135){
        DEBUG(("going for energy/pp"));
        api.setPositionTarget(ItemLoc[(myEnergy<3.0f&&game.hasItem(closeEnergy)==-1)?closeEnergy:closePP]);
    }
    
    //get remaining point-packs
    // if (gameTime > 135) {
    //     DEBUG(("cleaning up pointpacks"));
    //     api.setPositionTarget(ItemLoc[closePP]);
    // }
    else {
        DEBUG(("staying still"));
        api.setVelocityTarget(zeroVec);
    }
    
    if (memoryFilled > 0 && myPosInArea != -1 && gameTime > 30 && gameTime < 80) {
            DEBUG(("leaving light"));
            memcpy(waypoint,myPos,12);
            waypoint[1] = - 0.2f;
            api.setPositionTarget(waypoint);
    }
    //part of mirror logic
    // if (game.hasItem(closeMirror)==0 && myDistToLightFront>0.15f&& myDistToLightFront<0.25f) firstLightPassWithMirror=false;
    
    // DEBUG(("lightTail:%f myPos[1]-lightTail:%f",lightTail,myPos[1]-lightTail));
    
    
    
// 	pic logic (37%)
	float picPoints = game.getPicPoints();
	if ((game.isFacingOther() and distanceBetween(myPos,enPos)>PHOTO_MIN_DISTANCE
        and (myEnergy>1.6f or myPosInArea==1) and enPosInArea!=-1
        and memoryFilled<2 && game.getMirrorTimeRemaining() == 0) or myEnergy < 1.0f or (memoryFilled == 0 and game.hasItem(closePP)!=-1 and gameTime>150)){
        if (!enemyHasMirror or (!enemyMirrorActivated && (picPoints > -0.1f))) {
            game.takePic();
        }
        else if (!enemyMirrorActivated) {
            if (picPoints < 0.0f) {
                enemyMirrorActivated = true;
                mirrorTimerEnd = gameTime + 24;
            }
        }
    }
    
	if (( ((memoryFilled == 2 and myEnergy>1.0f) 
            or (gameTime > 160)) || game.getFuelRemaining()<10
            or (myPosInArea==-1 and enPosInArea==-1 and myEnergy > 2.5f ))and memoryFilled>0){
        api.setAttitudeTarget(uploadVec);
        // DEBUG(("rotation: %f angle: %f", mathVecMagnitude(myRot,3),angleBetween(myAtt, uploadVec)));
        if (angleBetween(myAtt, uploadVec) /*>0.97*/ <.25f && mathVecMagnitude(myRot, 3) < UPLOAD_ANG_VEL) {
            game.uploadPics();
        }
    }
    else if ((myEnergy > 1.2f || myPosInArea==1)&& (enPosInArea != -1
              || enSoonInLight())
    ) {
        float vecToTarget[3];
        mathVecSubtract(vecToTarget, enPos, myPos, 3);
        //mathVecAdd(vecToTarget, vecToTarget, enVel, 3);
        // mathVecAdd(vecToTarget, vecToTarget, myVel, 3);
        //mathVecNormalize(vecToTarget,3);
        api.setAttitudeTarget(vecToTarget);
        // DEBUG(("Rotating to face (dist between: %f)", distanceBetween(myPos,enPos)));
    }
}

void update() {
    gameTime++;
	enEnergy=game.getOtherEnergy();
    myEnergy=game.getEnergy();
    api.getMyZRState(myState);
	api.getOtherZRState(enState);
	
    myPosInArea=game.posInArea(myPos);
    enPosInArea=game.posInArea(enPos);
	
    lightTail = game.getLightInterfacePosition()-0.05f;
    lightFront = lightTail+ 0.9f; // 0.7 for dark and 0.2 for both grey
    DEBUG(("myDistToLightFront %f", myDistToLightFront));
    if(lightFront>0.8f) lightFront-=1.6f; // if it's out of bounds, correct it
    // DEBUG(("lightFront:%f lightTail:%f",lightFront,lightTail));
    myDistToLightFront = myPos[1] - lightFront;
    enemyHasMirror = ((gameTime >= mirrorTimerEnd) ? false : (game.hasItem(7) == 1 || game.hasItem(8) == 1));
}


//End page main
//End page main
//End page main
//End page main

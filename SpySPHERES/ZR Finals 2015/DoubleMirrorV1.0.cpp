//Begin page main
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
// float *enAtt;
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
// 	enAtt=&enState[6];

	//Filling ItemLoc
	for(int i=0;i<NUM_ITEMS;i++)
	{
	    game.getItemLoc(ItemLoc[i],i);
	}
// 	api.getMyZRState(myState);
    
}


void loop()
{
    update();
    float waypoint[3]; //used for getting out of the light
    short memoryFilled = game.getMemoryFilled();
    closeMirror = closestInRange(7,8);
    closePP = closestInRange(3,6);
    closeEnergy=closestInRange(0,2);
    float relPos = (myPos[1] + myVel[1]-LIGHT_SPEED);
    bool sphereInLight = (( (lightFront>lightTail)^(relPos-lightTail<0))
        &&(relPos-lightFront<0.08f));

    // Zanneio checks if they're pointing at us. Too big?        
    if (sphereInLight && game.getNumMirrorsHeld()>0 &&distanceBetween(myPos,enPos)>0.4f
    && game.getMirrorTimeRemaining()==0 && enEnergy>0.49f)
         game.useMirror();
         
    if (game.hasItem(closeMirror)==-1 ){//&& (distanceBetween(myPos,ItemLoc[closeMirror])<0.5f || myEnergy>2.0f)) {
        // memcpy(waypoint, ItemLoc[closeMirror], 12);
        // waypoint[0] *= distanceBetween(myPos,ItemLoc[closeMirror])>0.3?0.7f:1.0f;
        // api.setPositionTarget(waypoint);
        moveToPoint(ItemLoc[closeMirror]);
        DEBUG(("going to mirror loc"));
    }
    
    else if (//(distanceBetween(myPos,ItemLoc[closeMirror])>0.3f || game.hasItem(closeMirror)!=-1) &&
    game.getMirrorTimeRemaining()>0&& sphereInLight)
    {
        // memcpy(waypoint,ItemLoc[closeMirror],12);
        waypoint[0] = enPos[0]*0.7f;        
        waypoint[2] = ItemLoc[closeEnergy][2];
        waypoint[1] = distanceBetween(myPos,enPos)<0.5f ?
                                (enPos[1]<-0.1f)?-0.7:enPos[1]-0.6f :
                                (lightTail<0.0f&&lightTail>-0.7)? lightTail-0.1f :-0.75f;
        moveToPoint(waypoint);
        DEBUG(("leaving light"));
    }
    else {
        //slow
        moveToPoint(ItemLoc[closeEnergy]);
    }
    /*
    else if (memoryFilled>0 && myPos[1]>-0.5 && myEnergy<1.5f) {
        memcpy(waypoint, myPos, 12);
        waypoint[1]-=0.2f;
        api.setPositionTarget(waypoint);
        DEBUG(("trying to get into light"));
    }
    
    else if (memoryFilled==0&&gameTime>110&&game.getNumMirrorsHeld()==0 && myEnergy>0.75f) {
        moveToPoint(ItemLoc[ (myEnergy>3.0f)?closePP:closeEnergy ]);
        DEBUG(("getting energy/pps"));
    }
    else {
        DEBUG(("staying still"));
        api.setVelocityTarget(zeroVec);
    }*/
    // DEBUG(("dist to item: %f",distanceBetween(myPos,ItemLoc[closeMirror])));
    
    
    
// 	pic logic (37%)
	if ((game.isFacingOther() and distanceBetween(myPos,enPos)>PHOTO_MIN_DISTANCE 
        and (myEnergy>1.6f or myPosInArea==1) and enPosInArea!=-1
        and memoryFilled<2 && game.getMirrorTimeRemaining() == 0 && gameTime>36)||myEnergy<1.0f){
        if (!enemyHasMirror or (!enemyMirrorActivated && (game.getPicPoints() > 0.0f))) {
            game.takePic();
        }
        else if (!enemyMirrorActivated) {
            if (game.getPicPoints() < 0.0f) {
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
    else if (gameTime>36 && (myEnergy > 1.2f || myPosInArea==1)&& (enPosInArea != -1
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
bool enSoonInLight() {
    #define seconds 4
    float soonlightTail = (LIGHT_SPEED*seconds)+lightTail;
    if(soonlightTail>0.8f) soonlightTail-=1.6f; // if it's out of bounds, correct it
    float soonlightFront = (soonlightTail + 0.9f); // 0.7 for dark and 0.2 for both grey
    if(soonlightFront>0.8f) soonlightFront-=1.6f; // if it's out of bounds, correct it
    float predictedPos=enPos[1]+(enVel[1]*seconds);
        //if their predicted pos is past either end of the ligh and grey zones return true
    return (predictedPos>soonlightTail || predictedPos<soonlightFront)? true : false;
}
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

//Begin page Movement
void moveToPoint(float targetPos[3]) {
    float velocityTarget[3];
    mathVecSubtract(velocityTarget, targetPos, myPos, 3);
    float dist = mathVecNormalize(velocityTarget, 3); // returns magnitude pre-normalization
    scale(velocityTarget, (dist < 0.14f) ? dist*dist : 0.01f);
	api.setVelocityTarget(velocityTarget);
    api.setPositionTarget(targetPos);
    //using both functions is deliberate
}
//End page Movement
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
    for (short i = 0; i < 3; i++) {
        vector[i] *= scalar;
    }
}


//End page Vectors
//End page main

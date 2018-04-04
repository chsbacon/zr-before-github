//Begin page Awareness
// bool hasEnemyPickedUpAMirror() {
//     return ((gameTime >= mirrorTimerEnd) ? false : (game.hasItem(7) == 1 || game.hasItem(8) == 1));
    
// }

// short enemyItemTargeted() {
//     short targetItem = 0;
//     float minAngle = PI/6.0f;
//     float itemLoc[3];
//     float enemyVecToItem[3];
//     for (short i = 0; i < 9; i++) {
//         game.getItemLoc(itemLoc, i);
//         mathVecSubtract(enemyVecToItem, itemLoc, enPos, 3);
//         float angle = angleBetween(enVel, enemyVecToItem);
//         if (angle < minAngle) {
//             minAngle = angle;
//             targetItem = i;
//         }
//     }
//     return (minAngle < PI/6.0f && game.hasItem(targetItem) == -1) ? targetItem : -1;   
// }

bool enSoonInLight() {
    #define seconds 4
    float soonlightTail = (LIGHT_SPEED*seconds)+lightTail;
    if(soonlightTail>0.8) soonlightTail-=1.6; // if it's out of bounds, correct it
    float soonlightFront = (soonlightTail + 0.9); // 0.7 for dark and 0.2 for both grey
    if(soonlightFront>0.8) soonlightFront-=1.6; // if it's out of bounds, correct it
    float predictedPos=enPos[1]+(enVel[1]*seconds);
    return (predictedPos>soonlightTail || predictedPos<soonlightFront)? true : false;
}

// short timeToLight(float point[3]) {
//     short timeToFront = (short) ((point[1]-lightFront)/ (LIGHT_SPEED));
//     timeToFront=timeToFront<0?500:timeToFront;
//     // timeToTail=timeToTail<0?500:timeToTail;
//     // return (timeToTail<timeToFront? timeToTail : (timeToFront==500?-1:timeToFront));
//     return timeToFront;
//                 // y = LIGHT_SPEED(t) + lightPoint
//                 // y = point
//                 //  -lightPoint + point = LIGHT_SPEED(t)
//                 // (-lightPoint + point)/ (LIGHT_SPEED) = t
// }
//End page Awareness
//Begin page Item Selection
short closestInRange(short start, short stop) {
    short closest = start;
    while (game.hasItem(closest)!=-1 && closest<=stop) closest++;
    for (short i=closest+1; i<=stop; i++) {
        // DEBUG(("item:%d hasItem:%d closest:%f i:%f",i,game.hasItem(i),distanceBetween(ItemLoc[closest],myPos),distanceBetween(ItemLoc[i],myPos)));
        if (game.hasItem(i)==-1 &&
        distanceBetween(ItemLoc[i],myPos)<distanceBetween(ItemLoc[closest],myPos))
            closest = i;
    }
    return closest; //closest may have been gotten but it will never return -1
}
//End page Item Selection
//Begin page Movement
void moveToPoint(float targetPos[3]) {
    float velocityTarget[3];
    mathVecSubtract(velocityTarget, targetPos, myPos, 3);
    float dist = mathVecNormalize(velocityTarget, 3); // returns magnitude pre-normalization
    scale(velocityTarget, (dist*dist < 0.02f) ? dist*dist : 0.02f);
	api.setVelocityTarget(velocityTarget);
    api.setPositionTarget(targetPos);
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

// float* clone(float inVec[3]) {
//     float returnVec[3];
//     for (short i=0;i<3;i++) returnVec[i]=inVec[i];
//     return returnVec;
// }

void scale(float vector[], float scalar) {//2%  does edit the passed array
    for (int i = 0; i < 3; i++) {
        vector[i] *= scalar;
    }
}
// void vecManip(float outVec[3], float c, float inVec[3])
// {   
//     /*This function sets a vec that you task in (outVec) to a scalar multiple of another vec*/
// 	outVec[0] = c * inVec[0];
// 	outVec[1] = c * inVec[1];
// 	outVec[2] = c * inVec[2];
// }
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
// float *enAtt;
float enEnergy;

short myPosInArea, enPosInArea;

// float zeroVec[3];
float uploadVec[3];

float ItemLoc[NUM_ITEMS][3];

short gameTime;
bool enemyHasMirror;
short mirrorTimerEnd;
bool enemyMirrorActivated;

float lightTail, lightFront, myDistToLightFront;

short closeMirror, closePP, closeEnergy;
bool firstLightPassWithMirror, abandoningPPs, slip;
bool waitToUpload;
void init()
{
// 	memset(zeroVec, 0.0f, 12); //sizeof(float) is 4
    memcpy(uploadVec, EARTH, 12);
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
	api.getMyZRState(myState);
    closePP = closestInRange(3,6);
    firstLightPassWithMirror=true;  
    abandoningPPs=slip=waitToUpload=false;
    // DEBUG(("LOL! We are TheMachKeppleriansCorElevenWhiteHole2 and we think we're soooo cool"));
}


void loop()
{
    update();
    float waypoint[3];
    int memoryFilled = game.getMemoryFilled();
// 	if (game.hasItem(closePP)!=-1) {
//     	closePP = closestInRange(3,6);
// 	}
        closePP=closestInRange(3,6);


	if ((gameTime<45 || myPosInArea!=1)&&game.getScore()<3.0f&&!abandoningPPs) {
    	DEBUG(("going for point-packs(slip:%d)", slip));
	    closeMirror = closestInRange(7,8);
	    moveToPoint(ItemLoc[closePP]);
        slip=(enEnergy<1.1f && myEnergy>0.75f
        && enPos[1]-lightFront>0.2f);
    }
    else if (slip && !(myPosInArea==-1/*&&myDistToLightFront<-0.1*/)) { //check this
        DEBUG(("slipping through"));
        abandoningPPs=true;
        // float targetPos[3];
        memcpy(waypoint, ItemLoc[closeMirror], 12);
        waypoint[1]=lightTail;
        if (myDistToLightFront<0.1 || myPosInArea==-1) api.setPositionTarget(waypoint);
    }
    //-----------------
    else if (game.hasItem(closeMirror)==-1||(firstLightPassWithMirror&&(myDistToLightFront>0.1f||lightFront<0))) {
        DEBUG(("going for mirror (first light pass:%d)",firstLightPassWithMirror));
        slip=false;
        abandoningPPs=true;
        api.setPositionTarget(ItemLoc[closeMirror]);
        closeEnergy=closestInRange(0,2);
        if (memoryFilled > 0 && myPosInArea != -1) {
            // float getAwaySpot[3];
            memcpy(waypoint,myPos,12);
            waypoint[1] = - 0.2f;
            api.setPositionTarget(waypoint);
        }
    }
    //-----------------------
    else if ((game.getMirrorTimeRemaining()>0) || (myDistToLightFront < 0.1f && myDistToLightFront > 0.0f)){
        DEBUG(("going for energy waypoint and deploying mirror"));
        game.useMirror();
        // waypoint[0]=ItemLoc[closeMirror][0];
        // waypoint[1]=-0.4;
        // waypoint[2]=ItemLoc[closeEnergy][2];
        memcpy(waypoint,ItemLoc[closeEnergy],12);
        waypoint[1] = - 0.4f;
        //if (myDistToLightFront < -0.05f||myPosInArea==-1) 
        api.setPositionTarget(waypoint);
    }
    else if (myEnergy<2.5&&game.hasItem(closeEnergy)==-1||game.hasItem(closePP)!=-1){
        //waitToUpload = (game.hasItem(closeEnergy)==-1);
        DEBUG(("going for energy"));
        DEBUG(("game.hasItem(closeMirror)==-1:%d (firstLightPassWithMirror:%d myDistToLightFront>0.1f: %d)",game.hasItem(closeMirror)==-1,firstLightPassWithMirror,myDistToLightFront>0.1f));
        api.setPositionTarget(ItemLoc[closeEnergy]);
    }
    else {
        DEBUG(("cleaning up pointpacks"));
        api.setPositionTarget(ItemLoc[closePP]);
    }
    if (game.hasItem(closeMirror)==0&&myDistToLightFront<=0.11) firstLightPassWithMirror=false;

    // else  {
    //     DEBUG(("default (high-ish spot)"));
    //     api.setPositionTarget(ItemLoc[closeMirror]);
    // }
// 	pic logic (37%)
	if (game.isFacingOther() and distanceBetween(myPos,enPos)>PHOTO_MIN_DISTANCE 
        and (myEnergy>1.2f or myPosInArea==1) and enPosInArea!=-1
        and memoryFilled<2 && game.getMirrorTimeRemaining() == 0){
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
            or (gameTime > 170)) 
            or (myPosInArea==-1 and enPosInArea==-1 and myEnergy > 2.5 ))and memoryFilled>0){
        api.setAttitudeTarget(uploadVec);
        // DEBUG(("rotation: %f angle: %f", mathVecMagnitude(myRot,3),angleBetween(myAtt, uploadVec)));
        if (angleBetween(myAtt, uploadVec) /*>0.97*/ <.25f && mathVecMagnitude(myRot, 3) < UPLOAD_ANG_VEL
        && !waitToUpload) {
            game.uploadPics();
        }
    }
    else if ((myEnergy > 1.2 || myPosInArea==1)&& (enPosInArea != -1
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
// 	DEBUG(("energy usage: us:%f them:%f", game.getEnergy()-myEnergy, game.getOtherEnergy()-enEnergy));
	enEnergy=game.getOtherEnergy();
    myEnergy=game.getEnergy();
    api.getMyZRState(myState);
	api.getOtherZRState(enState);
	
    myPosInArea=game.posInArea(myPos);
    enPosInArea=game.posInArea(enPos);
	
    lightTail = game.getLightInterfacePosition()-0.05f;
    lightFront = lightTail+ 0.9; // 0.7 for dark and 0.2 for both grey
    DEBUG(("myDistToLightFront %f", myDistToLightFront));
    if(lightFront>0.8f) lightFront-=1.6f; // if it's out of bounds, correct it
    // DEBUG(("lightFront:%f lightTail:%f",lightFront,lightTail));
    myDistToLightFront = myPos[1] - lightFront;
    enemyHasMirror = ((gameTime >= mirrorTimerEnd) ? false : (game.hasItem(7) == 1 || game.hasItem(8) == 1));
}

//End page main

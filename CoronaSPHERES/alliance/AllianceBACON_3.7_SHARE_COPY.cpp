//Begin page main
float myPos[3];
float myAtt[3];
float enemyPos[3];
bool POIPhotoTaken[6];
float zeroVec[3];
short nextFlare;
short gameTime;
short chosenPOIID;
float zVec[3];
float lattitudeAngle;
float chosenPOILoc[3]; 
float height;
bool bottom; 
bool onRight;
bool inShadowZone;
bool chaser;
void init() {
    gameTime = -1;
    zeroVec[0]=zeroVec[1]=zeroVec[2]=zVec[0]=zVec[1]=0.0f;
    zVec[2]=-1.0f;
    flushPOIPhotos();
    chosenPOIID = 2;
    bottom=onRight=chaser=false;
}

void loop(){
    gameTime++;
    int upload = 0;
    float state[12];
    float myVelo[3];
    api.getMyZRState(state);
    myPos[0] = state[0];
	myPos[1] = state[1];
	myPos[2] = state[2];
	myVelo[0] = state[3];
	myVelo[1] = state[4];
	myVelo[2] = state[5];
	myAtt[0] = state[6];
	myAtt[1] = state[7];
	myAtt[2] = state[8];
    api.getOtherZRState(state);
    enemyPos[0] = state[0];
    enemyPos[1] = state[1];
	enemyPos[2] = state[2];
	 
	inShadowZone = myPos[0] > 0 && sqrtf(myPos[1]*myPos[1] + myPos[2]*myPos[2]) < 0.2f;
	
	if (gameTime == 3) {
	    if (enemyPos[0] < -0.012) 
	        chaser=bottom=true;
	        
	    if (enemyPos[2] < -0.005f)
	        bottom = true;
	}
	
	if (enemyPos[0] < -0.15)
	    chaser = true;
	    
	if (inShadowZone) {
	    if (myPos[2] > enemyPos[2] || chaser)
	        bottom = true;
	    else
	        bottom = false;
	}
	    
	int fuelRemaining = game.getFuelRemaining();
	nextFlare = game.getNextFlare();
	if (nextFlare < 0)
	    nextFlare = 100;
	    
	if (myPos[1] > 0) 
    	onRight = true;
    else
        onRight = false;
    	
	if (gameTime % 60 == 0){ 
        flushPOIPhotos(); 
	}
    getNewChosenPoint();
    
    game.getPOILoc(chosenPOILoc, chosenPOIID);
    float vecToPOI[3] = {chosenPOILoc[0], 0.0f, chosenPOILoc[2]};
    float angleFromTop = angleCalc(vecToPOI, zVec);
    float timeToTop = 10*angleFromTop;
    if (fabs(PI - angleFromTop) >= 0.1f)
        lattitudeAngle = (timeToTop - (int)(timeToTop))*0.1f;
        
    float futurePOISpot[3];
    POIPredict(futurePOISpot, chosenPOILoc, getRotationAngle(chosenPOILoc));
    float futurePhotoSpot[3];
    vecManip(futurePhotoSpot, zeroVec, height*5, futurePOISpot);
    
    mathVecSubtract(vecToPOI, futurePOISpot, futurePhotoSpot, 3);
    if (distance(myPos, futurePhotoSpot) < 0.05f)
	    mathVecSubtract(vecToPOI, futurePOISpot, myPos, 3); 
	    
    mathVecNormalize(vecToPOI, 3);
    api.setAttitudeTarget(vecToPOI);
    
    
    short preRoundPhotos = game.getMemoryFilled();
    


    if ((game.getMemoryFilled() > 0) && (nextFlare < 15 || 
        preRoundPhotos == game.getMemorySize() || gameTime >= 220)) {
        uploadPhotos();
        api.setVelocityTarget(zeroVec);
        upload = 1;
    }
    else if (game.alignLine(chosenPOIID) || timeToLattitude(chosenPOILoc) > 4 || fuelRemaining == 0) {
            if (!(bottom && chosenPOILoc[2] < 0))
	            game.takePic(chosenPOIID);
            if (game.getMemoryFilled() > preRoundPhotos) 
                markPhotoTaken();
    }
	
    if ((nextFlare <= 15 || realTimeToLat(chosenPOILoc, chosenPOIID) + 15 > nextFlare)
        || (fuelRemaining < 9 && (nextFlare < 20 || myPos[0] > 0.03f))) {
        float darkSpot[3] = {0.41f, 0.0f, -0.1f};
        if (bottom)
    	    darkSpot[2] *= -1;
        if (game.getMemoryFilled() > 0)
            uploadPhotos();
        moveToPoint(darkSpot);
    }
        
    else if (upload == 0)
        moveToPoint(futurePhotoSpot);
    
    if (!(inShadowZone) && nextFlare == 1) {
        game.turnOff();
        game.turnOn();
    }
    //if (gameTime < 3) 
    //    moveToPoint(zeroVec);
    
    //DEBUG(("futurePOILoc %f, %f, %f", futurePOISpot[0], futurePOISpot[1], futurePOISpot[2]));
    //DEBUG(("LattitudeAngle %f", lattitudeAngle));
    DEBUG(("\nTIME %i\nChosenPOIID %i\nTimeToLat %i\nFuturePOISpot %f, %f, %f\nChosenPOILoc %f, %f, %f\n", gameTime,
        chosenPOIID, timeToLattitude(chosenPOILoc), futurePOISpot[0], futurePOISpot[1], futurePOISpot[2], chosenPOILoc[0],
        chosenPOILoc[1], chosenPOILoc[2]));
    if ( (!(inShadowZone)&&((myPos[0]>=0.3f))&&(myVelo[0]>0))||(myPos[0]>=0.48f) ){
        float temp1[3];            
        DEBUG(("********HARD TURN*********"));
        temp1[0] = 10*(0.3f-myPos[0]-4.3f*myVelo[0]);
        temp1[1] = 10*(0.1-myPos[1]-4.3f*myVelo[1]);
        temp1[2] = 10*(-myPos[2]-4.3f*myVelo[2]);
        api.setForces(temp1);
    }
}

void markPhotoTaken() {
    short outerAddition = 0;
    if (mathVecMagnitude(myPos, 3) > 0.43f)
        outerAddition = 1;
    POIPhotoTaken[chosenPOIID * 2 + outerAddition] = true;
}

void uploadPhotos() {
    float vecToEarthPos[3] = {0.64 - myPos[0], -myPos[1], -myPos[2]};
    mathVecNormalize(vecToEarthPos, 3);
    api.setAttitudeTarget(vecToEarthPos);
    if (angleCalc(myAtt, vecToEarthPos) < 0.05f)
        game.uploadPic();
}
short realTimeToLat(float givenPOILoc[3], short POIID) {
    float futurePOILoc[3];
    POIPredict(futurePOILoc, givenPOILoc, getRotationAngle(givenPOILoc));
    short timeToLat = timeToLattitude(givenPOILoc);
    short timeToPoint = angleCalc(futurePOILoc, myPos) * 15 + (fabs(mathVecMagnitude(myPos, 3) - height)*40) + 2;
    if (timeToPoint > timeToLat && !(timeToPoint  < 8 && timeToLat < 8)) {
        timeToLat += 31;
    }
    return (timeToLat);
}

short timeToLattitude(float givenPOILoc[3]) {
    float vecCircleToPOILoc[3] = {givenPOILoc[0], 0.0f, givenPOILoc[2]};
    float timeToTop = (angleCalc(vecCircleToPOILoc, zVec) - lattitudeAngle) * 10;
    if (bottom) {
    	if (givenPOILoc[0] > 0)
    		return 1;
    	return timeToTop + 2;
    }
    return timeToTop;
}

float getRotationAngle(float POILoc[3]) {
    float vecCircToPOI[3] = {POILoc[0], 0.0f, POILoc[2]};
    float theta = angleCalc(vecCircToPOI, zVec) - lattitudeAngle;
    if (bottom)
    	theta += PI + 2 * lattitudeAngle;
    return theta;
}

void POIPredict(float futurePOILoc[3], float currentPOILoc[3], float theta) {
    float a1 = cosf(theta);
    float a2 = -sinf(theta);
    
    futurePOILoc[0] = currentPOILoc[0]*a1 + currentPOILoc[2]*a2;
    futurePOILoc[1] = currentPOILoc[1];
    futurePOILoc[2] = -currentPOILoc[0]*a2 + currentPOILoc[2]*a1;
}

void moveToPoint(float targetPos[3]) {
    float velocityTarget[3];
    float targetCopy[3];
    float posCopy[3];
    memcpy(targetCopy,targetPos,3*sizeof(float));
    memcpy(posCopy,myPos,3*sizeof(float));
    mathVecNormalize(targetCopy, 3);
    mathVecNormalize(posCopy, 3);
    if (angleCalc(targetPos, myPos) > 1.16f || mathVecMagnitude(myPos, 3) < 0.35f) {
        mathVecAdd(targetPos, targetCopy, posCopy, 3);
        mathVecNormalize(targetPos, 3);
        vecManip(targetPos, zeroVec, 0.45f, targetPos);
    }
    mathVecSubtract(velocityTarget, targetPos, myPos, 3);
    
	mathVecNormalize(velocityTarget, 3);
	vecManip(velocityTarget, zeroVec, 0.02f, velocityTarget);
    
    
    api.setPositionTarget(targetPos);
	if (distance(targetPos, myPos) > 0.1f) 
	    api.setVelocityTarget(velocityTarget);
}



void getNewChosenPoint() {
    float POILoc[3];
    float minTimeToLat = 50.0f;
    short timeToPOISwitch = 60 - gameTime % 60;
    height = 0.44f;
    for (short POIID = 0; POIID < 3; POIID++) {
        game.getPOILoc(POILoc, POIID);
        float timeToLat = realTimeToLat(POILoc, POIID);
        short weight = 0;
        if (POIPhotoTaken[POIID * 2 + 1])
            weight += 8;
        short farPOIID = 0;
        if (onRight)
        	farPOIID = 1;
        if (POIID == farPOIID)
            weight += 5;
        if (((enemyPos[1] * POILoc[1] > 0) && (enemyPos[2] * myPos[2] > 0))
            && fabs(enemyPos[1]) > 0.01f)
            weight += 15;
        //DEBUG(("FUTURE POI %i: %f, %f, %f\n\n", POIID, futurePOI[0], futurePOI[1], futurePOI[2]));
        //DEBUG(("TIME  %i\n LAT %f\n Flare %i\n POISwitch %i\n POIID %i\n\n\n",
        //        gameTime, timeToLat, nextFlare, timeToPOISwitch, POIID));
        if (timeToLat < timeToPOISwitch && (timeToLat + 15 < nextFlare)
            && timeToLat + weight < minTimeToLat) { 
            chosenPOIID = POIID;
            minTimeToLat = timeToLat + weight;
        }
    }
    if (minTimeToLat > 49) 
        chosenPOIID = 2;

    if (POIPhotoTaken[chosenPOIID * 2 + 1] && timeToLattitude(chosenPOILoc) < timeToPOISwitch)
        height = 0.4f;
}
void flushPOIPhotos() { //Sets POIPhotoTaken to all false
    POIPhotoTaken[0]=POIPhotoTaken[1]=POIPhotoTaken[2]=POIPhotoTaken[3]=POIPhotoTaken[4]=POIPhotoTaken[5]=false;
}

void vecManip(float outVec[3], float inVec1[3], float c, float inVec2[3])
{   
    /*This function sets a vec that you task in (outVec) to a manipulation 
      of two other already defined vecs, quite powerful and versatile*/
	outVec[0] = inVec1[0] + c * inVec2[0];
	outVec[1] = inVec1[1] + c * inVec2[1];
	outVec[2] = inVec1[2] + c * inVec2[2];
}

float angleCalc(float vec1[3], float vec2[3])
{
    //uses a variation of the dot product to calculate the angle between two vectors
	return acosf(mathVecInner(vec1, vec2, 3)/(mathVecMagnitude(vec1,3) * mathVecMagnitude(vec2,3)));
}

float distance(float pos1[3], float pos2[3]) {
    /*Calculates the vector between two points, then returns the distance between
      those two points by getting the magnitude of that vector*/
    float differenceVec[3];
    mathVecSubtract(differenceVec, pos1, pos2, 3);
    return mathVecMagnitude(differenceVec, 3);
}
//End page main

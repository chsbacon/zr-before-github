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
float futurePOISpot[3];
float height;
bool bottom; 
bool onRight;
int flareCount;

void init() {
    gameTime = -1;
    zeroVec[0]=zeroVec[1]=zeroVec[2]=zVec[0]=zVec[1]=0.0f;
    zVec[2]=-1.0f;
    flushPOIPhotos(); 
    chosenPOIID = 2;
    bottom = false;
    onRight = false;
    flareCount = 0;
}


void loop(){

    gameTime++;

    int upload = 0;
    float state[12];
    api.getMyZRState(state);
    myPos[0] = state[0];
	myPos[1] = state[1];
	myPos[2] = state[2];
	myAtt[0] = state[6];
	myAtt[1] = state[7];
	myAtt[2] = state[8];
    api.getOtherZRState(state);
    enemyPos[0] = state[0];
    enemyPos[1] = state[1];
	enemyPos[2] = state[2];
	
	if (gameTime == 3) {
	    if (enemyPos[2] < -0.005f)
	        bottom = true;
	}
	
	if (inDarkZone()) {
	    if ((enemyPos[2] < 0) && fabs(enemyPos[2]) > 0.02f)
	        bottom = true;
	    else
	        bottom = false;
	}
	    
	int fuelRemaining = game.getFuelRemaining();
	nextFlare = game.getNextFlare();
	if (nextFlare < 0)
	    nextFlare = 100;
	    
	if (nextFlare == 0)
	    flareCount++;
	    
	if (myPos[1] > 0)
    	onRight = true;
    	
	if (gameTime % 60 == 0) 
        flushPOIPhotos(); 

    getNewChosenPoint();
    
    game.getPOILoc(chosenPOILoc, chosenPOIID);
    float vecCircToPOI[3] = {chosenPOILoc[0], 0.0f, chosenPOILoc[2]};
    float angleFromTop = angleCalc(vecCircToPOI, zVec);
    float timeToTop = 10*angleFromTop;
    if (fabs(PI - angleFromTop) >= 0.1f)
        lattitudeAngle = (timeToTop - (int)(timeToTop))*0.1f;
    POIPredict(futurePOISpot, chosenPOILoc, getRotationAngle(chosenPOILoc));
    float futurePhotoSpot[3];
    vecManip(futurePhotoSpot, zeroVec, height*5, futurePOISpot);
    
    float vecToPOI[3];
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
    else if ((game.alignLine(chosenPOIID) && inCone())
            || timeToLattitude(chosenPOILoc) > 3 || fuelRemaining == 0) {
            if (!(bottom && chosenPOILoc[2] < 0))
	            game.takePic(chosenPOIID);
            if (game.getMemoryFilled() > preRoundPhotos) 
                markPhotoTaken();
    }
	
    if ((nextFlare <= 15 || realTimeToLat(chosenPOILoc, chosenPOIID) + 15 > nextFlare)
        || (fuelRemaining < 9 && flareCount < 3 && (nextFlare < 20 || myPos[0] > 0.03f))) {
        float darkSpot[3] = {0.4f, 0.05f, -0.05f};
        if (!(onRight))
         	darkSpot[1] *= -1;
        if (bottom)
    	    darkSpot[2] *= -1;
        if (game.getMemoryFilled() > 0)
            uploadPhotos();
        moveToPoint(darkSpot);
    }
        
    else if (upload == 0)
        moveToPoint(futurePhotoSpot);
    
    if (!(inDarkZone()) && nextFlare == 1) {
        game.turnOff();
        game.turnOn();
    }
    if (gameTime < 3) 
        moveToPoint(zeroVec);
    
    //DEBUG(("\nTIME %i\nChosenPOIID %i\nTimeToLat %i\nFuturePOISpot %f, %f, %f\nChosenPOILoc %f, %f, %f\n", gameTime,
    //    chosenPOIID, timeToLattitude(chosenPOILoc), futurePOISpot[0], futurePOISpot[1], futurePOISpot[2], chosenPOILoc[0],
    //    chosenPOILoc[1], chosenPOILoc[2]));
}

bool inDarkZone() {
    return myPos[0] > 0 && sqrtf(myPos[1]*myPos[1] + myPos[2]*myPos[2]) < 0.2f;
}    

bool inCone() {
    float POIToSphere[3];
    mathVecSubtract(POIToSphere, myPos, futurePOISpot, 3);
    if (angleCalc(futurePOISpot, POIToSphere) < 0.4f)
        return true;
    return false;
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
    short timeToLat = timeToLattitude(givenPOILoc);
    short timeToPoint = timeToSpot(POIID);
    if (timeToPoint > timeToLat && !(timeToPoint  < 8 && timeToLat < 8)) {
        timeToLat += 31;
    }
    return timeToLat;
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

float timeToSpot(short POIID) {
    float POILoc[3];
    float futurePOILoc[3];
    game.getPOILoc(POILoc, POIID);

    POIPredict(futurePOILoc, POILoc, getRotationAngle(POILoc));
    return angleCalc(futurePOILoc, myPos) * 15 + (fabs(mathVecMagnitude(myPos, 3) - height)*40) + 2;
}

float getRotationAngle(float POILoc[3]) {
    float vecCircToPOI[3] = {POILoc[0], 0.0f, POILoc[2]};
    float theta = angleCalc(vecCircToPOI, zVec) - lattitudeAngle;
    if (bottom)
    	theta += PI + 2 * lattitudeAngle;
    return theta;
}


void POIPredict(float futurePOILoc[3], float currentPOILoc[3], float theta) {
    float yVec[3] = {0.0f, -1.0f, 0.0f};
    float cosTheta = cosf(theta);
    float axisCrossVec[3];
    float vectorOne[3];
    float vectorTwo[3];
    mathVecCross(axisCrossVec, yVec, currentPOILoc);
    vecManip(vectorOne, zeroVec, cosTheta, currentPOILoc);
    vecManip(vectorTwo, vectorOne, sinf(theta), axisCrossVec);
    vecManip(futurePOILoc, vectorTwo, (1 - cosTheta) * mathVecInner(currentPOILoc, yVec, 3), yVec);
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
            weight = 8;
        short farPOIID = 0;
        if (onRight)
        	farPOIID = 1;
        if (POIID == farPOIID)
            weight += 0.1;
        if ((enemyPos[1] * POILoc[1] > 0) && (enemyPos[2] * myPos[2] > 0))
            weight += 15;
        //DEBUG(("FUTURE POI %i: %f, %f, %f\n\n", POIID, futurePOI[0], futurePOI[1], futurePOI[2]));
        //DEBUG(("TIME  %i\nPOINT %f \n LAT %f\n Flare %i\n POISwitch %i\n POIID %i\n\n\n",
        //        gameTime, timeToPoint, timeToLat, nextFlare, timeToPOISwitch, POIID));
        if (timeToLat < timeToPOISwitch && (timeToLat + 15 < nextFlare)
            && timeToLat + weight < minTimeToLat) { 
            chosenPOIID = POIID;
            minTimeToLat = timeToLat + weight;
        }
    }
    if (minTimeToLat > 49) {
        if (realTimeToLat(chosenPOILoc, chosenPOIID) < timeToPOISwitch) 
            chosenPOIID = 2;
    }
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

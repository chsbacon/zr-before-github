//Begin page Movement
void moveToPoint(float targetPos[3], float SPEEDMULTIPLIER) {
    float velocityTarget[3];
    findShortPath(targetPos);
    vecManip(velocityTarget, targetPos, -1.0f, myPos);
    float dist = distance(targetPos, myPos);

    if (dist>=0.15f){
	    mathVecNormalize(velocityTarget, 3);
	    vecManip(velocityTarget, zeroVec, 0.02f, velocityTarget);
	}
	else{
	    vecManip(velocityTarget, zeroVec, SPEEDMULTIPLIER * dist, velocityTarget);
	}
    
    api.setPositionTarget(targetPos);
	if (dist > 0.08f) {
	    api.setVelocityTarget(velocityTarget);
	}
}

void findShortPath(float targetPos[3]) {
    float angle = angleCalc(targetPos, myPos);
    float intermediatePos[3];
    float targetCopy[3];
    float posCopy[3];
    vecManip(targetCopy, targetPos, 0.0f, zeroVec);
    vecManip(posCopy, myPos, 0.0f, zeroVec);
    mathVecNormalize(targetCopy, 3);
    mathVecNormalize(posCopy, 3);
    if (angle > 1.16f || mathVecMagnitude(myPos, 3) < 0.35f) {
        mathVecAdd(intermediatePos, targetCopy, posCopy, 3);
        mathVecNormalize(intermediatePos, 3);
        vecManip(targetPos, zeroVec, 0.45f, intermediatePos);
    }
}
//End page Movement
//Begin page PhotoSpots
void getNewChosenPoint() {
    float POILoc[3];
    float minTimeToLat = 50.0f;
    int timeToPOISwitch = 60 - gameTime % 60;
    height = 0.44f;
    for (int POIID = 0; POIID < 3; POIID++) {
        game.getPOILoc(POILoc, POIID);
        float timeToLat = realTimeToLat(POILoc, POIID);
        float timeToPoint = timeToSpot(POIID);
        int weight = 0;
        if (POIPhotoTaken[POIID * 2 + 1])
            weight = innerWeight;
        int farPOIID = 0;
        if (blue)
        	farPOIID = 1;
        //DEBUG(("FUTURE POI %i: %f, %f, %f\n\n", POIID, futurePOI[0], futurePOI[1], futurePOI[2]));
        DEBUG(("TIME  %i\nPOINT %f \n LAT %f\n Flare %i\n POISwitch %i\n POIID %i\n\n\n",
                gameTime, timeToPoint, timeToLat, nextFlare, timeToPOISwitch, POIID));
        if (timeToLat < timeToPOISwitch && (timeToLat + FLARERUNTIME < nextFlare)
            && timeToLat + weight < minTimeToLat && POIID != farPOIID) { //Last condition only works for blue
            chosenPOIID = POIID;
            minTimeToLat = timeToLat + weight;
        }
    }
    if (minTimeToLat > 49) {
        if (realTimeToLat(futurePOISpot, chosenPOIID) < timeToPOISwitch)
            chosenPOIID = 2;
    }
    if (POIPhotoTaken[chosenPOIID * 2 + 1] && timeToLattitude(chosenPOILoc) < timeToPOISwitch)
        height = 0.4f;
}

void flushPOIPhotos() { //Sets POIPhotoTaken to all false
    POIPhotoTaken[0] = false;
    POIPhotoTaken[1] = false;
    POIPhotoTaken[2] = false;
    POIPhotoTaken[3] = false;
    POIPhotoTaken[4] = false;
    POIPhotoTaken[5] = false;
}
//End page PhotoSpots
//Begin page Picture-Taking
bool inPictureTakingPosition() {
	if (game.alignLine(chosenPOIID) && inCone(chosenPOIID)) 
	   return true;
	return false;
}

bool facing(float target[3]) {
    float attTarget[3];
    vecManip(attTarget, target, -1.0f, myPos);
    if ((angleCalc(myAtt, attTarget) < 0.05f))
        return true;
    return false;
}
//End page Picture-Taking
//Begin page Utilities
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
    vecManip(differenceVec, pos1, -1.0f, pos2);
    return mathVecMagnitude(differenceVec, 3);
}
//End page Utilities
//Begin page main
float myPos[3];
float myAtt[3];
bool POIPhotoTaken[6];
float zeroVec[3];
int nextFlare;
int gameTime;
float targetPos[3];
float zVec[3];
int chosenPOIID;
float futurePhotoSpot[3];
float futurePOISpot[3];
float lattitudeAngle;
float chosenPOILoc[3];
float height;
int FLARERUNTIME;
bool bottom; 
bool blue;
float earthPos[3];
float movementConstant;
int innerWeight;


void init() {
    FLARERUNTIME = 15;
    movementConstant = 1.0f; ////MESS WITH THIS
    innerWeight = 5; /////MESS WITH THIS
    gameTime = -1;
    zeroVec[0] = 0.0f;
    zeroVec[1] = 0.0f;
    zeroVec[2] = 0.0f;
    zVec[0] = 0.0f;
    zVec[1] = 0.0f;
    zVec[2] = -1.0f;
    flushPOIPhotos(); 
    chosenPOIID = 2;
    bottom = false; ////MESS WITH THIS, TRUE MAKES IT GO BOTTOM
    blue = false;
    earthPos[0] = 0.64f;
    earthPos[1] = 0.0f;
    earthPos[2] = 0.0f;
}


void loop(){
    update();
    int preRoundPhotos = game.getMemoryFilled();
    if ((game.getMemoryFilled() > 0) && (nextFlare < FLARERUNTIME || 
        game.getMemoryFilled() == game.getMemorySize() || gameTime >= 220))
        moveToUploadPhotos();
    else if (inPictureTakingPosition() || timeToLattitude(chosenPOILoc) > 4) {
	    game.takePic(chosenPOIID);
	    if (game.getMemoryFilled() > preRoundPhotos)
            markPhotoTaken();
	}
	
    if ((nextFlare <= FLARERUNTIME || realTimeToLat(chosenPOILoc, chosenPOIID) + FLARERUNTIME > nextFlare)) {
        float darkSpot[3] = {0.42f, 0.05f, -0.05f};
        if (!(blue))
        	darkSpot[1] *= -1;
        if (bottom)
        	darkSpot[2] *= -1;
        if (game.getMemoryFilled() > 0)
            moveToUploadPhotos();
        moveToPoint(darkSpot, 1.0f);
    }
        
    else 
        moveToPoint(futurePhotoSpot, movementConstant);
        
}

void update() {
    gameTime++;
    float myState[12];
    api.getMyZRState(myState);
	nextFlare = game.getNextFlare();
	if (nextFlare < 0)
	    nextFlare = 100;
	for (int i = 0; i < 3; i++) {
        myPos[i] = myState[i];
        myAtt[i] = myState[i+6];
	}
	if (gameTime <= 1 && myPos[1] > 0)
    	blue = true;
	if (gameTime % 60 == 0) {
        flushPOIPhotos(); 
	}
	if (gameTime <= 220 || game.getMemoryFilled() == 0)
        getNewChosenPoint();
    game.getPOILoc(chosenPOILoc, chosenPOIID);
    float vecCircToPOI[3] = {chosenPOILoc[0], 0.0f, chosenPOILoc[2]};
    lattitudeAngle = fmodf(angleCalc(vecCircToPOI, zVec), .1f);
    float theta = angleCalc(vecCircToPOI, zVec) - lattitudeAngle;
    if (bottom)
    	theta += PI + 2 * lattitudeAngle;
    POIPredict(futurePOISpot, chosenPOILoc, theta);
    vecManip(futurePhotoSpot, zeroVec, height*5, futurePOISpot);
    float vecToPOI[3];
    vecManip(vecToPOI, futurePOISpot, -1.0f, futurePhotoSpot);
    if (distance(myPos, futurePhotoSpot) < 0.05f)
	    vecManip(vecToPOI, futurePOISpot, -1.0f, myPos); 
    mathVecNormalize(vecToPOI, 3);
    api.setAttitudeTarget(vecToPOI);
}

    
bool inCone(int POIID) {
    float POIToSphere[3];
    float POILoc[3];
    float futurePOILoc[3];
    game.getPOILoc(POILoc, POIID);
    POIPredict(futurePOILoc, POILoc, timeToLattitude(POILoc)*0.1f);
    vecManip(POIToSphere, myPos, -1.0f, futurePOILoc);
    float maxAngleVariance = 0.4f; //assume outer radius
	if (height < 0.42f) { //if inner, relax constraint
	    maxAngleVariance = 0.8f;
	}
    if ((angleCalc(futurePOILoc, POIToSphere) < maxAngleVariance)) //&& (dist < maxRadius && dist > minRadius))
        return true;
    return false;
}

void markPhotoTaken() {
    int outerAddition = 0;
    if (height > 0.43f)
        outerAddition = 1;
    POIPhotoTaken[chosenPOIID * 2 + outerAddition] = true;
}

void moveToUploadPhotos() {
    float vecToEarthPos[3];
    mathVecSubtract(vecToEarthPos, earthPos, myPos, 3);
    mathVecNormalize(vecToEarthPos, 3);
    api.setAttitudeTarget(vecToEarthPos);
    api.setVelocityTarget(zeroVec);
    if (facing(earthPos))
        game.uploadPic();
}

int realTimeToLat(float givenPOILoc[3], int POIID) {
    int timeToLat = timeToLattitude(givenPOILoc);
    int timeToPoint = timeToSpot(POIID);
    if (timeToPoint > timeToLat && !(timeToPoint  < 8 && timeToLat < 8)) {
        timeToLat += 31;
    }
    return timeToLat;
}

int timeToLattitude(float givenPOILoc[3]) {
    float vecCircleToPOILoc[3] = {givenPOILoc[0], 0.0f, givenPOILoc[2]};
    float timeToTop = (angleCalc(vecCircleToPOILoc, zVec) - lattitudeAngle) * 10;
    if (bottom) {
    	if (givenPOILoc[0] > 0)
    		return 1;
    	return timeToTop + 2;
    }
    return timeToTop;
}

float timeToSpot(int POIID) {
    float POILoc[3];
    float futurePOILoc[3];
    game.getPOILoc(POILoc, POIID);
    float vecCircToPOI[3] = {POILoc[0], 0.0f, POILoc[2]};
    float theta = angleCalc(vecCircToPOI, zVec) - lattitudeAngle;
    if (bottom)
    	theta += PI + 2 * lattitudeAngle;
    POIPredict(futurePOILoc, POILoc, theta);
    float angleBetween = angleCalc(futurePOILoc, myPos);
    unsigned char extraTime = (unsigned char)(fabs(mathVecMagnitude(myPos, 3) - height)*40) + 2;
    float totalTime = angleBetween * 15 + extraTime;
    return totalTime;
}

void POIPredict(float futurePOILoc[3], float currentPOILoc[3], float theta) {
    float yVec[3] = {0.0f, -1.0f, 0.0f};
    rotateVec(futurePOILoc, currentPOILoc, yVec, theta);
}

void rotateVec(float outVec[3], float vector[3], float axis[3], float angle) {
    float axisCrossVec[3];
    float vectorOne[3];
    float vectorTwo[3];
    mathVecCross(axisCrossVec, axis, vector);
    float axisDotVector = mathVecInner(vector, axis, 3);
    vecManip(vectorOne, zeroVec, cosf(angle), vector);
    vecManip(vectorTwo, vectorOne, sinf(angle), axisCrossVec);
    vecManip(outVec, vectorTwo, (1 - cosf(angle)) * axisDotVector, axis);
}
//End page main

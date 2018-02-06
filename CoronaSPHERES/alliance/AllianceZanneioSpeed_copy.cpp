//Begin page Flare
void antiFlare() {
    drift();
	if (nextFlare == 1) { //Turns off if flare about to happen
		game.turnOff();
		game.turnOn();
	}
}

void drift() { //more like stop, no drifting here lol
    api.setVelocityTarget(zeroVec);
}


//End page Flare
//Begin page Movement
/*void moveToPoint(float targetPos[3], float SPEEDMULTIPLIER) {
    float vecToTarget[3];
    findClearPath(targetPos, myPos, zeroVec, 0.35, ASTAVOIDANCERADIUS);
    vecManip(vecToTarget, targetPos, -1.0, myPos);
	vecManip(targetPos, zeroVec, SPEEDMULTIPLIER, vecToTarget);//Makes overshot proportional to distance
	vecManip(targetPos, myPos, 1.0, vecToTarget);                                                                     //the overshot becomes closer to 0 as we get closer and closer 
	api.setPositionTarget(targetPos); If targetPos, wasn't reset in the above loop,
	                                    it remains unchanged (though sped up) 
	                                    from what was originally
	                                    passed in, and we go straight there
}*/


void moveToPoint(float targetPos[3], float SPEEDMULTIPLIER) {
    float velocityTarget[3];
    findShortPath(targetPos, ASTAVOIDANCERADIUS);
    vecManip(velocityTarget, targetPos, -1.0, myPos);
    float dist = distance(targetPos, myPos);
    float vel = 0.02f;

    if (dist>=0.15f){
	    mathVecNormalize(velocityTarget, 3);
	    vecManip(velocityTarget, zeroVec, vel, velocityTarget);
	}
	else{
	    vecManip(velocityTarget, zeroVec, SPEEDMULTIPLIER * dist, velocityTarget);
	}
    
    api.setPositionTarget(targetPos);
	if (dist > 0.08f){
	    api.setVelocityTarget(velocityTarget);
	}
	if (dist < 0.05)
	    api.setVelocityTarget(zeroVec);
}

void findShortPath(float targetPos[3], float dodgeDist) {
    float angle = angleCalc(targetPos, myPos);
    float intermediatePos[3];
    float targetCopy[3];
    float posCopy[3];
    vecManip(targetCopy, targetPos, 0, zeroVec);
    vecManip(posCopy, myPos, 0, zeroVec);
    mathVecNormalize(targetCopy, 3);
    mathVecNormalize(posCopy, 3);
    if (angle > 0.7f || distance(myPos, targetPos) > 0.35) {
        mathVecAdd(intermediatePos, targetCopy, posCopy, 3);
        mathVecNormalize(intermediatePos, 3);
        vecManip(targetPos, zeroVec, dodgeDist, intermediatePos);
    }
}
/*
short gotopos2(float poi[3], float dest[3], float coef){
    short r;
    float att[3], v[3], d, vel =0.02f, temp[3];
    r=0;

    mathVecSubtract(v,dest,cpos,3);
	d = mathVecMagnitude(v,3);
    
    if (upload == 0){
        //compute attitude facing the POI when at final destination
        att[0] = -dest[0];att[1]=-dest[1];att[2]=-dest[2];
    }
    else{
        //Facing earth?
        temp[0]=0.64f;
        temp[1]=0;
        temp[2]=0;
        mathVecSubtract(att,temp,cpos,3);
        mathVecNormalize(att,3);
        #ifdef DEBUG_MESSAGES
        DEBUG(("ANGLE = %f\n", acosf(mathVecInner(att,catt,3))));
        #endif
        if (acosf(mathVecInner(att,catt,3))<0.05f){
            game.uploadPic();
            r=game.getMemoryFilled();
            DEBUG(("MEMORY: %d",r));
            if (r==0){//no need? only for bug
                upload = 0;
                DEBUG(("STOP UPLOADING... catt =\n"));
            }
        }
    }
    
	if (d>=0.15f){
	    mathVecNormalize(v,3);
	    v[0]*=vel;v[1]*=vel;v[2]*=vel;
	}
	else{
	    v[0]*=coef*d;v[1]*=coef*d;v[2]*=coef*d;
	}
    
    api.setPositionTarget(dest);
	if (d>0.08f){
	    api.setVelocityTarget(v);
	}
  
    //consider removing for codesize
    //mathVecNormalize(att,3);
    //------------------------
	api.setAttitudeTarget(att);

	r = 0;
    if (d<tol){
        v[0]=0;v[1]=0;v[2]=0;
        api.setVelocityTarget(v);
        r=1;
    }
	return r;
}*/
void findClearPath(float targetPos[3], float myPosition[3], float obstaclePos[3], float dangerDist, float dodgeDist) {
    float vecToObstacle[3];
    float vecToPoint[3];
    float vecToPointAdj[3];
    float c[3];
    float cAdj[3];
    float d[3];
	vecManip(vecToObstacle, obstaclePos, -1.0, myPosition); //gets the vector from us to the asteroid and stores it in vecToAsteroid
    vecManip(vecToPoint, targetPos, -1.0, myPosition); //gets the vector from us to the point we want to get to, stores in vecToPoint
	if (angleCalc(vecToObstacle, vecToPoint) < PI/2.0
	    && (mathVecMagnitude(vecToPoint, 3) > mathVecMagnitude(vecToObstacle, 3))) {
	    float vecToObstacleMag = mathVecMagnitude(vecToObstacle, 3); //vec to asteroid magnitude
	    float bAdjMag = fabs(vecToObstacleMag*cosf(angleCalc(vecToObstacle, vecToPoint)));
	    mathVecNormalize(vecToPoint, 3);
	    vecManip(vecToPointAdj, zeroVec, bAdjMag, vecToPoint); //sets the velocity vector to bAdjMag length
	    mathVecSubtract(c, vecToPointAdj, vecToObstacle, 3); //finds c by subtracting vecToAsteroid from veloAdj
	    float cMag = mathVecMagnitude(c, 3);
				
	    if (cMag < dangerDist)
    	{
    	    //if we need to dodge/adjust trajectory
    		mathVecNormalize(c, 3);
    		vecManip(cAdj, zeroVec, dodgeDist, c); //makes c length "dodgedist"
    		mathVecAdd(d, cAdj, vecToObstacle, 3); //d is a  new vector, similar to what used to be vecToPointAdj
    		vecManip(targetPos, myPosition, 1.0, d); //TARGET POS
	    }
	}
}
//End page Movement
//Begin page POISwitch
int timeUntilPOISwitch() {
    return (60 - (gameTime % 60));
}

void moveToPOISwitchSpot() {
    float targetSwitchSpot[3] = {-0.4, 0, 0};
    moveToPoint(targetSwitchSpot, SPEEDCONSTANT);
}




//End page POISwitch
//Begin page PhotoSpots
void getNewChosenPoint() {
    float POILoc[3];
    float PhotoSpot[3];
    float minTimeToLat = 50;
    height = 0.45;
    int invalidPOIs = 0;
    float futurePOI[3];
    for (int POIID = 0; POIID < 3; POIID++) {
        game.getPOILoc(POILoc, POIID);
        float vecCircToPOI[3];
        float centerOfCircle[3] = {0, POILoc[1], 0};
        vecManip(vecCircToPOI, POILoc, -1.0, centerOfCircle);
        POIPredict(futurePOI, POILoc, angleCalc(vecCircToPOI, zVec) - lattitudeAngle);
        vecManip(PhotoSpot, zeroVec, height/0.2, futurePOI);
        float timeToLat = timeToLattitude(POILoc);
        float timeToPoint = timeToSpot(PhotoSpot);
        if (timeToPoint > timeToLat && !(timeToPoint  < 5 && timeToLat < 5)) {
            timeToLat += (PI - lattitudeAngle) * 10;
        }
        //DEBUG(("FUTURE POI %i: %f, %f, %f\n\n", POIID, futurePOI[0], futurePOI[1], futurePOI[2]));
        //DEBUG(("TIME  %i\nPOINT %f \n LAT %f\n Flare %i\n POISwitch %i\n POIID %i\n\n\n",
        //        gameTime, timeToPoint, timeToLat, nextFlare, timeToPOISwitch, POIID));
        if (timeToLat < timeToPOISwitch && (timeToLat + FLARERUNTIME < nextFlare || nextFlare == -1)
            && timeToLat < minTimeToLat && !(POIPhotoTaken[POIID * 2 + 1])) {
            chosenPOIID = POIID;
            minTimeToLat = timeToLat;
        }
        else
            invalidPOIs++;
    }
    if (invalidPOIs == 3) {
        if (timeToLattitude(futurePOISpot) < timeToPOISwitch)
            chosenPOIID = 2;
        if (POIPhotoTaken[chosenPOIID * 2 + 1])
            height = 0.41;
    }
}

/*int getNewChosenPoint() { 
    float minWeight = 5.0;
    float POILoc[3];
    float futureSpot[3];
    height = 0.45;
    unsigned char chosenPOIID = 2;
    //float timeTest;
    //timeTest = timeToPhoto(chosenPOILoc);
    for (int POIID = 0; POIID < 3; POIID++) {
        game.getPOILoc(POILoc, POIID);
        float vecCircToPOI[3];
        float centerOfCircle[3] = {0, POILoc[1], 0};
        vecManip(vecCircToPOI, POILoc, -1.0, centerOfCircle);
        float angle = angleCalc(vecCircToPOI, zVec);
        POIPredict(futureSpot, POILoc, angle);
        float weight = angle + distance(myPos, futureSpot);
        if (POIPhotoTaken[POIID * 2 + 1] && !(POIPhotoTaken[POIID * 2]))
            weight += 0.15;
        if (weight < minWeight && !(POIPhotoTaken[POIID * 2] && POIPhotoTaken[POIID * 2 + 1])
            && timeToLattitude(POILoc) < timeToPOISwitch) {
            minWeight = weight;
            chosenPOIID = POIID;
        }
    }
    if (POIPhotoTaken[chosenPOIID * 2 + 1])
        height = 0.41;
    return chosenPOIID;
}
*/

void flushPOIPhotos() { //Sets POIPhotoTaken to all false
    for (int i = 0; i < 6; i++){
        POIPhotoTaken[i] = false;
    }  
}
//End page PhotoSpots
//Begin page Picture-Taking
void rotateToFace() {
    float vecToPOI[3];
    vecManip(vecToPOI, chosenPOILoc, -1.0, myPos);
    mathVecNormalize(vecToPOI, 3);
    api.setAttitudeTarget(vecToPOI);
}


bool inPictureTakingPosition() {
	if (facing(chosenPOILoc) && inCone()) 
	   return true;
	return false;
}

bool facing(float target[3]) {
    float attTarget[3];
    vecManip(attTarget, target, -1.0, myPos);
    if ((angleCalc(myAtt, attTarget) < 0.05))
        return true;
    return false;
}

void pointAt(float target[3]) {
    float attTarget[3];
    mathVecSubtract(attTarget, target, myPos, 3);
    mathVecNormalize(attTarget, 3);
    api.setAttitudeTarget(attTarget);
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
    vecManip(differenceVec, pos1, -1.0, pos2);
    return mathVecMagnitude(differenceVec, 3);
}


//End page Utilities
//Begin page main
float myState[12];
float enemy[12];
float myPos[3];
float myVelo[3];
float myAtt[3];
float enemyPos[3];
float enemyVelo[3];
bool POIPhotoTaken[6];
float zeroVec[3];
int nextFlare;
int gameTime;
float targetPos[3];
float ASTAVOIDANCERADIUS;
float SPEEDCONSTANT;
int preRoundPhotos;
float futurePOIPos[3];
float zVec[3];
int chosenPOIID;
float futurePhotoSpot[3];
float futurePOISpot[3];
bool pickedUp;
float lattitudeAngle;
float chosenPOILoc[3];
float height;
int timeToPOISwitch;
int FLARERUNTIME;

void init() {
    height = 0.44;
    lattitudeAngle = 0.2047;
    ASTAVOIDANCERADIUS = 0.45; //The "dodgedist" for the asteroid
    SPEEDCONSTANT = 2.0; //Constant that dictates our movement speed, roughly 1.75x normal speed
    FLARERUNTIME = 17;
    gameTime = -1;
    zeroVec[0] = 0;
    zeroVec[1] = 0;
    zeroVec[2] = 0;
    zVec[0] = 0;
    zVec[1] = 0;
    zVec[2] = -1.0;
    flushPOIPhotos(); //Presents our data type that stores what pics we've taken in a 60s period to false
    chosenPOIID = 2;
    POIPredict(futurePOISpot, chosenPOILoc, (angleCalc(zVec, chosenPOILoc) - lattitudeAngle));
    pickedUp = true;
}


void loop(){
    update();
    if (pickedUp == false)
        pickUpMemory();
    if (pickedUp == true) {    
	    if (inPictureTakingPosition() && game.getMemoryFilled() < game.getMemorySize()) {
	        game.takePic(chosenPOIID);
	    }
	    moveToPoint(futurePhotoSpot, SPEEDCONSTANT);
        if (game.getMemoryFilled() > preRoundPhotos) {
            //DEBUG(("PHOTO TAKEN!"));
            markPhotoTaken();
            //DEBUG(("POIPhotoTaken %i", POIPhotoTaken[chosenPOIID * 2 + 1]));
            //getNewChosenPoint();
        }
    }
    if ((game.getMemoryFilled() > 0) && ((nextFlare <= 9 && nextFlare != -1)|| 
        game.getMemoryFilled() == game.getMemorySize() || gameTime >= 220))
        moveToUploadPhotos();
    if ((nextFlare <= FLARERUNTIME || timeToLattitude(chosenPOILoc) + FLARERUNTIME > nextFlare) && nextFlare != -1) {
        float darkSpot[3] = {0.42, 0, -0.05};
        moveToPoint(darkSpot, 1.0);
        DEBUG(("RUNNN!\n"));
        //getNewChosenPoint();
    }
}

void update() {
    gameTime++;
    api.getMyZRState(myState);
    api.getOtherZRState(enemy);
	nextFlare = game.getNextFlare();
	timeToPOISwitch = timeUntilPOISwitch();
	for (int i = 0; i < 3; i++) {
        myPos[i] = myState[i];
        enemyPos[i] = enemy[i];
        myVelo[i] = myState[i+3];
        enemyVelo[i] = enemy[i+3];
        myAtt[i] = myState[i+6];
	}
	if (gameTime % 60 == 0) {
	    flushPOIPhotos(); //If the POIs change, wipe the picTaken data
	}
    getNewChosenPoint();
   // DEBUG(("chosenPOIID %i\n", chosenPOIID));
    preRoundPhotos = game.getMemoryFilled();
    game.getPOILoc(chosenPOILoc, chosenPOIID);
    float vecCircToPOI[3];
    float centerOfCircle[3] = {0, chosenPOILoc[1], 0};
    vecManip(vecCircToPOI, chosenPOILoc, -1.0, centerOfCircle);
    lattitudeAngle = fmodf(angleCalc(vecCircToPOI, zVec), .1f);
    //if (angleCalc(vecCircToPOI, zVec) < lattitudeAngle || gameTime % 60 == 0) {
    //    getNewChosenPoint();
    //    game.getPOILoc(chosenPOILoc, chosenPOIID);
    //}
    POIPredict(futurePOISpot, chosenPOILoc, (angleCalc(vecCircToPOI, zVec) - lattitudeAngle));
    DEBUG(("futurePOISpot %f, %f, %f\n", futurePOISpot[0], futurePOISpot[1], futurePOISpot[2]));
    vecManip(futurePhotoSpot, zeroVec, height/0.2, futurePOISpot);
    //if (!(inCone()))
    //    getConeSpot(futurePhotoSpot);
    //else
    //    vecManip(futurePhotoSpot, myPos, 0, zeroVec);
    float vecToPOI[3];
    vecManip(vecToPOI, futurePOISpot, -1.0, futurePhotoSpot);
    if (distance(myPos, futurePhotoSpot) < 0.05)
	    vecManip(vecToPOI, futurePOISpot, -1.0, myPos); //was futurePOISpot - futurePhotoSpot
	
    mathVecNormalize(vecToPOI, 3);
    api.setAttitudeTarget(vecToPOI);
}

bool inCone() {
    float POIToSphere[3];
    vecManip(POIToSphere, myPos, -1.0, futurePOISpot);
    float dist = mathVecMagnitude(myPos, 3);
    float maxAngleVariance = 0.4; //assume outer radius
	float minRadius = 0.42;
	float maxRadius = 0.53;
	if (height < 0.42) { //if inner, relax constraint
	    maxAngleVariance = 0.8;
	    minRadius = 0.31;
	    maxRadius = 0.42;
	}
    if ((angleCalc(futurePOISpot, POIToSphere) < maxAngleVariance) && (dist < maxRadius && dist > minRadius))
        return true;
    return false;
}

void markPhotoTaken() {
    int outerAddition = 0;
    if (height > 0.43)
        outerAddition = 1;
    POIPhotoTaken[chosenPOIID * 2 + outerAddition] = true;
}

void moveToUploadPhotos() {
    float earthPos[3] = {0.64, 0, 0};
    float vecEarthToSpot[3];
    float vecEarthToSPHERE[3];
    float targetVec[3];
    float comparisonVec[3];
    vecManip(targetPos, myPos, 0, zeroVec);
    findClearPath(targetPos, earthPos, zeroVec, 0.22, 0.22);
    vecManip(vecEarthToSpot, targetPos, -1.0, earthPos);
    vecManip(vecEarthToSPHERE, myPos, -1.0, earthPos);
    mathVecSubtract(comparisonVec, vecEarthToSpot, vecEarthToSPHERE, 3);
    float theta = 0;
    if (mathVecMagnitude(comparisonVec, 3) > 0.00001)
        theta = angleCalc(vecEarthToSPHERE, vecEarthToSpot);
    float vecEarthToSPHEREMag = mathVecMagnitude(vecEarthToSPHERE, 3);
    mathVecNormalize(vecEarthToSpot, 3);
    vecManip(vecEarthToSpot, zeroVec, cosf(theta) * vecEarthToSPHEREMag, vecEarthToSpot);
    mathVecSubtract(targetVec, vecEarthToSpot, vecEarthToSPHERE, 3);
    mathVecAdd(targetPos, myPos, targetVec, 3);
    moveToPoint(targetPos, 1.75);
    pointAt(earthPos);
    if (facing(earthPos))
        game.uploadPic();
}

void getConeSpot(float targetPoint[3]) {
    float rotAxis[3];
    getRotationAxis(rotAxis);
    float rotAngle = getRotationAngle(targetPoint);
    if ((targetPoint[1] < 0) || (fabs(targetPoint[1]) <= 0.0001 && myPos[1] > targetPoint[1]))
        vecManip(rotAxis, zeroVec, -1.0, rotAxis);
    rotateVec(targetPoint, targetPoint, rotAxis, rotAngle);
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

void getRotationAxis(float returnVec[3]) {
    float rotVec[3] = {0.0f, 1.0f, 0.0f};
    if (lattitudeAngle > PI/2)
        rotVec[1] = -1.0f;
    rotateVec(returnVec, zVec, rotVec, (PI/2 + lattitudeAngle));
    mathVecNormalize(returnVec, 3);
}

float getRotationAngle(float point[3]) {
    float alpha = 0.75;
    float dist = mathVecMagnitude(point, 3);
    if (dist > 0.43)
        alpha = 0.37;
    float gamma = asinf(sinf(PI - alpha) * 0.2f/dist);
    return alpha - gamma;
}

void pickUpMemory() {
    //Goes to pick up memory pack, returns true if it has successfully collected it.
    float memoryPackLocation[3] = {-0.5, 0.6, 0};
    int redBlue = 1;
    if (myPos[1] < 0) {
        redBlue = 2;
        memoryPackLocation[1] = -0.6;
    }
    float vecToMemory[3];
    mathVecSubtract(vecToMemory, memoryPackLocation, myPos, 3);
    float dist = mathVecMagnitude(vecToMemory, 3);
    moveToPoint(memoryPackLocation, 3.0); 
    if(dist < 0.04 && mathVecMagnitude(myVelo, 3) < 0.005)
	{
	    float rspeed[]={0, 0, .5};
	    api.setAttRateTarget(rspeed);
	}
	if (game.hasMemoryPack(redBlue - 1, redBlue))
        pickedUp = true;
}

int timeToLattitude(float givenPOILoc[3]) {
    float vecCircleToPOILoc[3];
    float centerOfCircle[3] = {0, givenPOILoc[1], 0};
    vecManip(vecCircleToPOILoc, givenPOILoc, -1.0, centerOfCircle);
    float undershoot = fmodf(angleCalc(vecCircleToPOILoc, zVec), .1f);
    //lattitudeAngle = undershoot;
    //DEBUG(("lattitudeAngle %f\n", lattitudeAngle));
    float timeToTop = (angleCalc(vecCircleToPOILoc, zVec) - undershoot) * 10;
    //DEBUG(("timeToTop! %f", timeToTop));
    return timeToTop; 
}

float timeToSpot(float spot[3]) {
    float angleBetween = angleCalc(spot, myPos);
    unsigned char radialTime = (unsigned char)(fabs(mathVecMagnitude(myPos, 3) - mathVecMagnitude(spot, 3))*50);
    return angleBetween * 20 + radialTime;
}

void POIPredict(float futurePOILoc[3], float currentPOILoc[3], float theta) {
    float yVec[3] = {0, -1.0, 0};
    rotateVec(futurePOILoc, currentPOILoc, yVec, theta);
//    if (futurePOILoc[0] > 0) {
//        float angleTraveled = angleCalc(currentPOILoc, zVec);
//        rotateVec(futurePOILoc, zVec, yVec, (PI + theta - angleTraveled));
//    }
}
//End page main

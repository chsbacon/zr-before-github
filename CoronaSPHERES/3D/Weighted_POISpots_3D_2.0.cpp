//Begin page Flare
bool flareImminent() {
	return (((nextFlare <= 15 && nextFlare != -1) || gameTime > 225)
	        && game.getMemoryFilled() > 0) ;
}

void antiFlare() {
	//ADD DRIFT CODE HERE
	if (nextFlare == 1) {
		game.turnOff();
		game.turnOn();
	}
}

///UNDER REVIEW
void driftToPoint(float point[3]){
    float vectorToDrift[3];
    mathVecSubtract(vectorToDrift,point,myPos,3);
    // + 5 for turn on time, +3 for solar flare time, and +1 for margin of error
    int timeToDrift = game.getNextFlare() + 9;
    vecManip(vectorToDrift, zeroVec, (1.0/timeToDrift), vectorToDrift);
    api.setVelocityTarget(vectorToDrift);
} 

//End page Flare
//Begin page Movement
void moveToPoint(float targetPos[3]) {
    float vecToTarget[3];
    float vecToAsteroid[3];
    float vecToPoint[3];
    float vecToPointAdj[3];
    float c[3];
    float cAdj[3];
    float d[3];
	vecManip(vecToAsteroid, zeroVec, -1.0, myPos); //gets the vector from us to the asteroid and stores it in vecToAsteroid
    vecManip(vecToPoint, targetPos, -1.0, myPos); //gets the vector from us to the point we want to get to, stores in vecToPoint
	if (angleCalc(vecToAsteroid, vecToPoint) < PI/2.0) {
	    float vecToAstMag = mathVecMagnitude(vecToAsteroid, 3); //vec to asteroid magnitude
	    float bAdjMag = fabs(vecToAstMag*cosf(angleCalc(vecToAsteroid, vecToPoint))); //gets ideal length of our veloVec for our triangle
	    mathVecNormalize(vecToPoint, 3);
	    vecManip(vecToPointAdj, zeroVec, bAdjMag, vecToPoint); //sets the velocity vector to bAdjMag length
	    mathVecSubtract(c, vecToPointAdj, vecToAsteroid, 3); //finds c by subtracting vecToAsteroid from veloAdj
	    float cMag = mathVecMagnitude(c, 3);
				
	    if (cMag < 0.35)
    	{
    	    //if we need to dodge/adjust trajectory
    	    float dodgeDist = 0.5;
    		mathVecNormalize(c, 3);
    		vecManip(cAdj, zeroVec, dodgeDist, c); //makes c length "dodgedist"
    		mathVecAdd(d, cAdj, vecToAsteroid, 3); //d is a  new vector, similar to what used to be vecToPointAdj
    		vecManip(targetPos, myState, 1.0, d); //TARGET POS
	    }
	}
	vecManip(vecToTarget, targetPos, -1.0, myPos);
	float distToTarget = mathVecMagnitude(vecToTarget, 3);
	mathVecNormalize(vecToTarget, 3);
	vecManip(targetPos, myPos, 2.0*distToTarget, vecToTarget);
	if (distance(myPos, targetPos))
	api.setPositionTarget(targetPos); /*If targetPos, wasn't reset in the above loop,
	                                    it remains unchanged from what was originally
	                                    passed in, and we go straight there*/
}


void moveToUploadPhoto() { //need to factor in our velo/Dist to upload spot
	float escapeVec[3];
	float targetPos[3];
	//float POILoc[3];
	//game.getPOILoc(POILoc, (indexPhotoSpot/2));
	vecManip(escapeVec, myPos, 0, zeroVec);
	mathVecNormalize(escapeVec, 3);
	vecManip(escapeVec, zeroVec, 0.6, escapeVec);
	float distToEscapePos = distance(myPos, escapeVec);
	mathVecNormalize(escapeVec, 3);
	vecManip(targetPos, myPos, 2.5*distToEscapePos, escapeVec);
	api.setPositionTarget(targetPos);
}

//End page Movement
//Begin page PhotoSpots
void photoTakingSpots() {
    float POI[3];
    float POIAdj[3];
    int numPOIs = 3;
    for (int i = 0; i < numPOIs; i++) {
        game.getPOILoc(POI, i);
        mathVecNormalize(POI, 3);
        vecManip(POIAdj, zeroVec, 0.40, POI); //The inner picTakingSpot for the POI
        vecManip(photoSpots[i*2], POIAdj, 0.0, zeroVec);
        vecManip(POIAdj, zeroVec, 0.50, POI); //The outer picTakingSpot for the POI
        vecManip(photoSpots[i*2 + 1], POIAdj, 0.0, zeroVec);
    }
}


int getBestPhotoSpot(){ 
    float myVel[3]= {myState[3], myState[4], myState[5]};
    float futurePos[3];
    mathVecAdd(futurePos,myPos,myVel,3);
    
    float weighting[6];
    bool eligible[6];
    float pointVal;
    float bestVal=0;
    int best=0;
    
    int iMax=6;
        
    for(int i=0;i<iMax;i++){
        eligible[i]=false;
        if(isSafePoint(photoSpots[i])&&!POIPhotoTaken[i]){
            eligible[i]=true;
            pointVal=2.0;
            if (gameTime < 24 && game.getMemoryFilled() == 0)
                pointVal = 20.0;
            if(i%2==1)//is it outer
                pointVal=3.0;
            weighting[i]=(PI-angleCalc(futurePos, photoSpots[i]))*pointVal;//really cheesy
        }
    }
    
    for(int i=0;i<iMax;i++){
        if(weighting[i]>bestVal&&eligible[i]){
            best=i;
            bestVal=weighting[i];
        }
    }
    //DEBUG(("Best! %i", best));
    return best;
}


bool isSafePoint(float point[3]) {
	float enemyPos[3] = {enemy[0], enemy[1], enemy[2]};
	float enemyVelo[3] = {enemy[3], enemy[4], enemy[5]};
	float enemyDist = distance(enemyPos, point);
	float myDist = distance(myPos, point);
	float enemyToPoint[3];
	mathVecSubtract(enemyToPoint, point, enemyPos, 3);
	if (enemyDist < myDist) {
		if (enemyDist < 0.15)
			return false;
		if (enemyDist < 0.25 && angleCalc(enemyVelo, enemyToPoint))
			return false;
	}		
	return true;
}


void flushPOIPhotos() { //Sets POIPhotoTaken to all false
    for (int i = 0; i < 6; i++){
        POIPhotoTaken[i] = false;
    }  
}
//End page PhotoSpots
//Begin page Picture-Taking
void rotateToPOI() {
    float POILoc[3];
    float attTarget[3];
    float myAtt[3] = {myState[6], myState[7], myState[8]};
    float myAttRate[3] = {myState[9], myState[10], myState[11]};
    float attRateTarget[3];
    int POIID = indexPhotoSpot / 2;
    game.getPOILoc(POILoc, POIID);
    vecManip(attTarget, POILoc, -1.0, myPos);
    mathVecNormalize(attTarget, 3);
    float angleBetweenAttAndAttTarget = angleCalc(attTarget, myAtt);
    vecManip(attRateTarget, zeroVec, 0.5*angleBetweenAttAndAttTarget, myAttRate);
    api.setAttRateTarget(attRateTarget);
    api.setAttitudeTarget(attTarget);
}

void takePhoto() {
    float POILoc[3];
    float POItoSphere[3];
    float attTarget[3];
    float myOrientation[3] = {myState[6], myState[7], myState[8]}; //which way we are facing
    int POIID = indexPhotoSpot / 2;
	game.getPOILoc(POILoc, POIID);
    vecManip(attTarget, POILoc, -1.0, myPos); //gets the vec we want to face toward
	vecManip(POItoSphere, myPos, -1.0, POILoc); //gets the vec from our sphere to the asteroid
	
	float maxAngleVariance = 0.3; //assume outer radius
	if (indexPhotoSpot % 2 == 0) //if inner, relax constraint
	    maxAngleVariance = 0.7;
	if (((angleCalc(myOrientation, attTarget) < 0.03)) &&
	    (angleCalc(POILoc, POItoSphere) < maxAngleVariance)) {
	   //Take a photo 
	   game.takePic(POIID);
	}
	//else if (angleCalc(POILoc, POItoSphere) > 1.2 && mathVecMagnitude(myPos, 3) < 0.5) {
	//    game.takePic(POIID);
	//}
}
/*
void takePhoto() {
    float POILoc[3];
    float POItoSphere[3];
    float attTarget[3];
    float myOrientation[3] = {myState[6], myState[7], myState[8]}; //which way we are facing
    int POIID = indexPhotoSpot / 2;
	game.getPOILoc(POILoc, POIID);
    vecManip(attTarget, POILoc, -1.0, myPos); //gets the vec we want to face toward
	vecManip(POItoSphere, myPos, -1.0, POILoc); //gets the vec from our sphere to the asteroid
	
	float maxAngleVariance = 0.375; //assume outer radius
	if (indexPhotoSpot % 2 == 0) //if inner, relax constraint
	    maxAngleVariance = 0.775;
	if (((angleCalc(myOrientation, attTarget) < 0.045)) &&
	    (angleCalc(POILoc, POItoSphere) < maxAngleVariance)) {
	   //Take a photo 
	   game.takePic(POIID);
	}
	//else if (angleCalc(POILoc, POItoSphere) > 1.2 && mathVecMagnitude(myPos, 3) < 0.5) {
	//    game.takePic(POIID);
	//}
}

*/
bool inPictureTakingPosition() {
    float POILoc[3];
    float POItoSphere[3];
    float attTarget[3];
    float myOrientation[3] = {myState[6], myState[7], myState[8]}; //which way we are facing
    int POIID = indexPhotoSpot / 2;
	game.getPOILoc(POILoc, POIID);
    vecManip(attTarget, POILoc, -1.0, myPos); //gets the vec we want to face toward
	vecManip(POItoSphere, myPos, -1.0, POILoc); //gets the vec from our sphere to the asteroid
	float maxAngleVariance = 0.375; //assume outer radius
	float minRadius = 0.42;
	float maxRadius = 0.53;
	if (indexPhotoSpot % 2 == 0) { //if inner, relax constraint
	    maxAngleVariance = 0.775;
	    minRadius = 0.31;
	    maxRadius = 0.42;
	}
	float dist = mathVecMagnitude(myPos, 3);
	    
	if (((angleCalc(myOrientation, attTarget) < 0.045)) &&
	    (angleCalc(POILoc, POItoSphere) < maxAngleVariance) && (dist < maxRadius && dist > minRadius)) {
	   //Take a photo 
	   return true;
	}
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
    vecManip(differenceVec, pos1, -1.0, pos2);
    return mathVecMagnitude(differenceVec, 3);
}
//End page Utilities
//Begin page main
float targetPos[3];
float myState[12];
float enemy[12];
float myPos[3];
float photoSpots[6][3];
bool POIPhotoTaken[6];
float zeroVec[3];
int indexPhotoSpot;
int nextFlare;
int gameTime;
int counter;

//TAKES A LONG TIME TO TAKE PHOTO SOMETIMES
//WORK ON TIMING OF FLARE HIDING
//IMPLEMENT DRIFT CODE
//STILL BUGGY, SOME GAMES IT MAY COMPLETELY FAIL

void init() {
    gameTime = -1;
    zeroVec[0] = 0;
    zeroVec[1] = 0;
    zeroVec[2] = 0;
    flushPOIPhotos();
    counter = 0;
}

void loop() {
    int preRoundPhotos = game.getMemoryFilled();
    update();
	antiFlare();
	rotateToPOI();
	if (inPictureTakingPosition() && game.getMemoryFilled() < game.getMemorySize()) {
	    game.takePic(indexPhotoSpot/2);
	    counter++;
	    DEBUG(("Tried To Take a Picture! %i\n", counter));
	}
	if (game.getMemoryFilled() == game.getMemorySize() || flareImminent())
		moveToUploadPhoto();
	else {
		moveToPoint(photoSpots[indexPhotoSpot]);
		//DEBUG(("indexPhotoSpot %i\n", indexPhotoSpot));
		//takePhoto();
	}
	if (game.getMemoryFilled() - preRoundPhotos > 0) { //If we took a photo this round
		POIPhotoTaken[indexPhotoSpot] = true;
		DEBUG(("Pic Taken!\n"));
	}
    uploadTakenPhotos();
}

void update() {
    gameTime++;
	photoTakingSpots();
	api.getMyZRState(myState);
    api.getOtherZRState(enemy);
	nextFlare = game.getNextFlare();
	myPos[0] = myState[0];
	myPos[1] = myState[1];
	myPos[2] = myState[2];
	if (gameTime % 60 == 0)
	    flushPOIPhotos();
	indexPhotoSpot = getBestPhotoSpot();
}

void uploadTakenPhotos() {
    if (mathVecMagnitude(myPos, 3) > 0.53 && game.getMemoryFilled() > 0)
		game.uploadPic();
}
//End page main

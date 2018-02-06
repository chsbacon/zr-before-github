//Begin page Flare
bool flareImminent() {
    return ((((nextFlare < 15 && nextFlare != -1) 
            || gameTime > 225)
            && game.getMemoryFilled() > 0) 
            || game.getMemoryFilled() == game.getMemorySize());
}


void antiFlare() {
    drift();
	if (nextFlare == 1) {
		game.turnOff();
		game.turnOn();
	}
}

void drift() { //more like stop, no drifting here lol
    api.setVelocityTarget(zeroVec);
}
//End page Flare
//Begin page Movement
void moveToPoint(float targetPos[3]) {
    float vecToTarget[3];
    float vecEnemyToTarget[3];
    float vecToEnemy[3];
    vecManip(vecToEnemy, enemyPos, -1.0, myPos);
    findClearPath(targetPos, zeroVec, 0.5);
   /* if (enemyInPath(targetPos)) {
        DEBUG(("GET OUT DA WAY!"));
        findClearPath(targetPos, enemyPos, AVOIDANCERADIUS);
        vecManip(vecEnemyToTarget, targetPos, -1.0, enemyPos);
        float points[4][3];
        vecManip(points[0], enemyPos, 1.0, vecEnemyToTarget);
        vecManip(points[1], enemyPos, -1.0, vecEnemyToTarget);
        mathVecCross(points[2], vecToEnemy, vecEnemyToTarget);
        mathVecCross(points[3], vecEnemyToTarget, vecToEnemy);
        float minWeight = 1.0;
        int chosenPoint = 0;
        
        for (int i = 0; i < 4; i++) {
            float weight = 5.0;
            float magnitude = mathVecMagnitude(points[i], 3);
            float enemyToPoint[3];
            vecManip(enemyToPoint, points[i], -1.0, enemyPos);
            if (magnitude > 0.35 && magnitude < 0.6)
                weight = mathVecInner(enemyVelo, enemyToPoint, 3);
            if (weight < minWeight) {
                minWeight = weight;
                chosenPoint = i;
            }
        }
        vecManip(targetPos, points[chosenPoint], 0, zeroVec);
    }*/
	vecManip(vecToTarget, targetPos, -1.0, myPos);
	float distToTarget = mathVecMagnitude(vecToTarget, 3);
	mathVecNormalize(vecToTarget, 3);
	vecManip(targetPos, myPos, SPEEDCONSTANT*distToTarget, vecToTarget);
	api.setPositionTarget(targetPos); /*If targetPos, wasn't reset in the above loop,
	                                    it remains unchanged from what was originally
	                                    passed in, and we go straight there*/
}

void findClearPath(float targetPos[3], float obstaclePos[3], float dodgeDist) {
    float vecToAsteroid[3];
    float vecToPoint[3];
    float vecToPointAdj[3];
    float c[3];
    float cAdj[3];
    float d[3];
	vecManip(vecToAsteroid, obstaclePos, -1.0, myPos); //gets the vector from us to the asteroid and stores it in vecToAsteroid
    vecManip(vecToPoint, targetPos, -1.0, myPos); //gets the vector from us to the point we want to get to, stores in vecToPoint
	if (angleCalc(vecToAsteroid, vecToPoint) < PI/2.0
	    && (mathVecMagnitude(vecToPoint, 3) > mathVecMagnitude(vecToAsteroid, 3))) {
	    float vecToAstMag = mathVecMagnitude(vecToAsteroid, 3); //vec to asteroid magnitude
	    float bAdjMag = fabs(vecToAstMag*cosf(angleCalc(vecToAsteroid, vecToPoint))); //gets ideal length of our veloVec for our triangle
	    mathVecNormalize(vecToPoint, 3);
	    vecManip(vecToPointAdj, zeroVec, bAdjMag, vecToPoint); //sets the velocity vector to bAdjMag length
	    mathVecSubtract(c, vecToPointAdj, vecToAsteroid, 3); //finds c by subtracting vecToAsteroid from veloAdj
	    float cMag = mathVecMagnitude(c, 3);
				
	    if (cMag < 0.35)
    	{
    	    //if we need to dodge/adjust trajectory
    		mathVecNormalize(c, 3);
    		vecManip(cAdj, zeroVec, dodgeDist, c); //makes c length "dodgedist"
    		mathVecAdd(d, cAdj, vecToAsteroid, 3); //d is a  new vector, similar to what used to be vecToPointAdj
    		vecManip(targetPos, myState, 1.0, d); //TARGET POS
	    }
	}
}

bool enemyInPath(float point[3]) {
    float vecToEnemy[3];
    float vecToPhotoSpot[3];
    vecManip(vecToEnemy, enemyPos, -1.0, myPos);
    vecManip(vecToPhotoSpot, photoSpots[indexPhotoSpot], -1.0, myPos);
    float theta = angleCalc(vecToEnemy, vecToPhotoSpot);
    if (mathVecMagnitude(vecToEnemy, 3) * sinf(theta) < AVOIDANCERADIUS
        && mathVecMagnitude(vecToEnemy, 3) < mathVecMagnitude(vecToPhotoSpot, 3)
        && theta < PI/2) {
        return true;
    }
    return false;
}

void moveToUploadPhoto() { 
	float escapeVec[3];
	vecManip(escapeVec, myPos, 0, zeroVec);
	mathVecNormalize(escapeVec, 3);
	vecManip(escapeVec, zeroVec, 0.56, escapeVec);
	moveToPoint(escapeVec);
	//moveToPoint(escapeVec);
}

//End page Movement
//Begin page POISwitch
bool POISwitchClose() {
    return  ((gameTime < 230 && 60 - (gameTime % 60) < 10)
            && distance(myPos, photoSpots[indexPhotoSpot]) > 0.2); 
}

int timeUntilPOISwitch() {
    return (60 - (gameTime % 60));
}

void moveToPOISwitchSpot() {
    DEBUG(("Switching!"));
    float targetSwitchSpot[3] = {-0.4, 0, 0};
    moveToPoint(targetSwitchSpot);
}
//End page POISwitch
//Begin page PhotoSpots
void photoTakingSpots() {
    float POI[3];
    float POIAdj[3];
    for (int i = 0; i < 3; i++) {
        game.getPOILoc(POI, i);
        mathVecNormalize(POI, 3);
        vecManip(POIAdj, zeroVec, 0.4, POI); //The inner picTakingSpot for the POI
        vecManip(photoSpots[i*2], POIAdj, 0.0, zeroVec);
        vecManip(POIAdj, zeroVec, 0.5, POI); //The outer picTakingSpot for the POI
        vecManip(photoSpots[i*2 + 1], POIAdj, 0.0, zeroVec);
    }
}


bool enemyInWay(float targetPos[3]) {
    float targetPosCopy[3];
	findClearPath(targetPos, zeroVec, 0.5);
	vecManip(targetPosCopy, targetPos, 0, zeroVec);
	findClearPath(targetPos, enemyPos, AVOIDANCERADIUS);
	for (int i = 0; i < 3; i++) {
	    if (targetPos[i] != targetPosCopy[i]) 
		    return true;
	}
	return false;
}

int getBestPhotoSpot(){ 
    float futurePos[3];
    mathVecAdd(futurePos,myPos,myVelo,3);
    
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
            pointVal=2.1;
            if(i%2==1) {//is it outer 
                pointVal=2.0;
                if ((game.getMemoryFilled() == game.getMemorySize() - 1) 
                   || ((nextFlare < 15 && nextFlare != -1) || timeUntilPOISwitch() < 15)) {
                    pointVal = 3.0;
                }
            }
            weighting[i]=(PI-angleCalc(futurePos, photoSpots[i]))*pointVal;//really cheesy
            if (enemyInWay(photoSpots[i]))
                weighting[i] -= 2;
        }
    }
    
    for(int i=0;i<iMax;i++){
        if(weighting[i]>bestVal&&eligible[i]){
            best=i;
            bestVal=weighting[i];
        }
    }
    return best;
}


bool isSafePoint(float point[3]) {
	/*float enemyPos[3] = {enemy[0], enemy[1], enemy[2]};
	float enemyVelo[3] = {enemy[3], enemy[4], enemy[5]};
	float enemyDist = distance(enemyPos, point);
	float myDist = distance(myPos, point);
	float enemyToPoint[3];
	mathVecSubtract(enemyToPoint, point, enemyPos, 3);
	if (enemyDist < myDist) {
		if (enemyDist < 0.25)
			return false;
		if (enemyDist < 0.35 && angleCalc(enemyVelo, enemyToPoint))
			return false;
	}*/		
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
    float vecToAsteroid[3];
    vecManip(vecToAsteroid, zeroVec, -1.0, myPos);
    mathVecNormalize(vecToAsteroid, 3);
    api.setAttitudeTarget(vecToAsteroid);
}


bool inPictureTakingPosition() {
    float POILoc[3];
    float POItoSphere[3];
    float attTarget[3];
    float myOrientation[3] = {myState[6], myState[7], myState[8]}; //which way we are facing
    int POIID = indexPhotoSpot / 2;
	game.getPOILoc(POILoc, POIID);
    vecManip(attTarget, POILoc, -1.0, myPos); //gets the vec we want to face toward
	vecManip(POItoSphere, myPos, -1.0, POILoc); //gets the vec from our sphere to the asteroid
	float maxAngleVariance = 0.4; //assume outer radius
	float minRadius = 0.42;
	float maxRadius = 0.53;
	if (indexPhotoSpot % 2 == 0) { //if inner, relax constraint
	    maxAngleVariance = 0.8;
	    minRadius = 0.31;
	    maxRadius = 0.42;
	}
	float dist = mathVecMagnitude(myPos, 3);
	if (((angleCalc(myOrientation, attTarget) < 0.05)) &&
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
float myState[12];
float enemy[12];
float myPos[3];
float myVelo[3];
float enemyPos[3];
float enemyVelo[3];
float photoSpots[6][3];
bool POIPhotoTaken[6];
float zeroVec[3];
int indexPhotoSpot;
int nextFlare;
int gameTime;
float targetPos[3];



//TESTING:
float AVOIDANCERADIUS;
float SPEEDCONSTANT;


void init() {
    AVOIDANCERADIUS = .35;
    SPEEDCONSTANT = 2.0;
    
    
    
    gameTime = -1;
    zeroVec[0] = 0;
    zeroVec[1] = 0;
    zeroVec[2] = 0;
    flushPOIPhotos();
}

void loop() {
    int preRoundPhotos = game.getMemoryFilled();
    update();
	rotateToPOI();
	if (nextFlare < 4 && nextFlare != -1)
	    antiFlare();
	else {
	    if (inPictureTakingPosition() && game.getMemoryFilled() < game.getMemorySize()) {
	        game.takePic(indexPhotoSpot/2);
	    }
	    if (flareImminent()) {
	        moveToUploadPhoto();
	    }
	    else if (POISwitchClose() && distance(myPos, photoSpots[indexPhotoSpot]) > 0.2) {
	        moveToPOISwitchSpot();
	    }
	    else {
	        moveToPoint(photoSpots[indexPhotoSpot]);
	    }
	    if (game.getMemoryFilled() - preRoundPhotos > 0) { //If we took a photo this round
	    	POIPhotoTaken[indexPhotoSpot] = true;
	    	DEBUG(("Pic Taken!\n"));
	    }
	}
    uploadTakenPhotos();
}

void update() {
    gameTime++;
	photoTakingSpots();
	api.getMyZRState(myState);
    api.getOtherZRState(enemy);
	nextFlare = game.getNextFlare();
	for (int i = 0; i < 3; i++) {
        myPos[i] = myState[i];
        enemyPos[i] = enemy[i];
        myVelo[i] = myState[i+3];
        enemyVelo[i] = enemy[i+3];
	}
	if (gameTime % 60 == 0) {
	    flushPOIPhotos();
	}
	indexPhotoSpot = getBestPhotoSpot();
}

void uploadTakenPhotos() {
    if (mathVecMagnitude(myPos, 3) > 0.53 && game.getMemoryFilled() > 0)
		game.uploadPic();
}
//End page main

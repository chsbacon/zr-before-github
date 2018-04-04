//Begin page Awareness
bool hasEnemyPickedUpAMirror() {
    return (gameTime >= mirrorTimerEnd) ? false : (game.hasItem(7) == 1 || game.hasItem(8) == 1);
    
}

short enemyItemTargeted() {
    short targetItem = 0;
    float minAngle = PI/6.0f;
    float itemLoc[3];
    float enemyVecToItem[3];
    for (short i = 0; i < 9; i++) {
        game.getItemLoc(itemLoc, i);
        mathVecSubtract(enemyVecToItem, itemLoc, enPos, 3);
        float angle = angleBetween(enVel, enemyVecToItem);
        if (angle < minAngle) {
            minAngle = angle;
            targetItem = i;
        }
    }
    return (minAngle < PI/6.0f && game.hasItem(targetItem) == -1) ? targetItem : -1;   
}

bool enSoonInLight() {
    #define seconds 4
    float soonlightTail = (LIGHT_SPEED*seconds)+lightTail; // 0.05 accounts for half of grey zone
    if(soonlightTail>0.8) soonlightTail-=1.6; // if it's out of bounds, correct it
    float soonlightFront = (soonlightTail + 0.9); // 0.7 for dark and 0.2 for both grey
    if(soonlightFront>0.8) soonlightFront-=1.6; // if it's out of bounds, correct it
    float predictedPos=enPos[1]+(enVel[1]*seconds);
    return (predictedPos>soonlightTail || predictedPos<soonlightFront)? true : false;
}

short timeToLight(float point[3]) {
    short timeToFront = (short) ((point[1]-lightFront)/ (LIGHT_SPEED));
    timeToFront=timeToFront<0?500:timeToFront;
    // timeToTail=timeToTail<0?500:timeToTail;
    // return (timeToTail<timeToFront? timeToTail : (timeToFront==500?-1:timeToFront));
    return timeToFront;
                // y = LIGHT_SPEED(t) + lightPoint
                // y = point
                //  -lightPoint + point = LIGHT_SPEED(t)
                // (-lightPoint + point)/ (LIGHT_SPEED) = t
}
//End page Awareness
//Begin page Item Selection
short closestPP() {
    return closestInRange(3,6);
}

short closestEnergy() {
    return closestInRange(0,2);
}
short closestMirror() {
    return closestInRange(7,8);
}
short closestInRange(short start, short stop) {
    short closest = start;
    while (game.hasItem(closest)!=-1 && closest<=stop) closest++;
    for (short i=closest+1; i<=stop; i++) {
        DEBUG(("item:%d hasItem:%d closest:%f i:%f",i,game.hasItem(i),distanceBetween(ItemLoc[closest],myPos),distanceBetween(ItemLoc[i],myPos)));
        if (game.hasItem(i)==-1 &&
        distanceBetween(ItemLoc[i],myPos)<distanceBetween(ItemLoc[closest],myPos))
            closest = i;
    }
    return game.hasItem(closest)==-1 ? closest : -1;
}
//End page Item Selection
//Begin page Movement
void move(float *destination, float yVel)
{
    float ydirection[3]={0,1,0};
    if (distanceBetween(myPos, destination)<0.3 || mathVecInner(ydirection,destination,3)<0.6)
    {//making sure we don't overshoot
        api.setPositionTarget(destination);
        DEBUG(("Movement using setPositionTarget"));
    }
    else
    {//
        float vel[3];
        mathVecSubtract(vel, destination, myPos, 3);
        float temp= yVel * 1.0f/vel[1];
        DEBUG(("TEMP %f", temp));
        vel[0]*=temp;
        vel[1]=yVel;
        vel[2]*=temp;
        api.setVelocityTarget(vel);
    }
}
void moveToPoint(float targetPos[3]) {
    float velocityTarget[3];
    mathVecSubtract(velocityTarget, targetPos, myPos, 3);
    float dist = mathVecNormalize(velocityTarget, 3); // returns magnitude pre-normalization
    scale(velocityTarget, (dist*dist < 0.02f) ? dist*dist : 0.02f);
    // vecManip(velocityTarget, (dist*dist < 0.02f) ? dist*dist : 0.02f, velocityTarget);
    //if (realTimeToLat(chosenPOILoc, chosenPOIID) - timeToPoint < 10 || running) {
	api.setVelocityTarget(velocityTarget);
	//}
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

float* clone(float inVec[3]) {
    float returnVec[3];
    for (short i=0;i<3;i++) returnVec[i]=inVec[i];
    return returnVec;
}

void scale(float vector[], float scalar) {//2%  does edit the passed array
    for (int i = 0; i < 3; i++) {
        vector[i] *= scalar;
    }
}
void vecManip(float outVec[3], float c, float inVec[3])
{   
    /*This function sets a vec that you task in (outVec) to a scalar multiple of another vec*/
	outVec[0] = c * inVec[0];
	outVec[1] = c * inVec[1];
	outVec[2] = c * inVec[2];
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
float *enAtt;
float enEnergy;

short myPosInArea, enPosInArea;

float zeroVec[3];
float uploadVec[3];
float lightInterfacePos;

float ItemLoc[NUM_ITEMS][3];
float futurePos[3];
short gameTime;
bool enemyHasMirror;
short mirrorTimerEnd;
bool enemyMirrorActivated;

float lightTail, lightFront;

short closeMirror, closePP, closeEnergy;
bool firstLightPassWithMirror, abandoningPPs;
void init()
{
	memset(zeroVec, 0.0f, 12); //sizeof(float) is 4
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
	enAtt=&enState[6];

	//Filling ItemLoc
	for(int i=0;i<NUM_ITEMS;i++)
	{
	    game.getItemLoc(ItemLoc[i],i);
	}
	api.getMyZRState(myState);
    closePP = closestPP();
    firstLightPassWithMirror=true;  
    abandoningPPs=false;
    DEBUG(("LOL! We are TheMachKeppleriansCorElevenWhiteHole2 and we think we're soooo cool"));
}


void loop()
{
    update();
    int memoryFilled = game.getMemoryFilled();

	if (game.hasItem(closePP)!=-1) {
    	closePP = closestPP();
	}
	
	if ((gameTime<45 || myPosInArea!=1)&&closePP!=-1&&game.getScore()<3.0&&!abandoningPPs) {
    	DEBUG(("going for point-packs (closePP: %d) (first pass:%d) (close mirror: %d)", closePP,firstLightPassWithMirror,closeMirror));
	    moveToPoint(ItemLoc[closePP]);
        closeMirror = closestMirror();
    }
    /**/else if(game.hasItem(closeMirror)==-1||firstLightPassWithMirror) {
        DEBUG(("going for mirror (first light pass:%d)",firstLightPassWithMirror));
        api.setPositionTarget(ItemLoc[closeMirror]);
        closeEnergy=closestEnergy();
        abandoningPPs=true;
        if (game.hasItem(closeMirror)==0&&myPosInArea==-1) firstLightPassWithMirror=false;
        
    }
    else if (game.getNumMirrorsHeld()>0||game.getMirrorTimeRemaining()>0){
        DEBUG(("going for energy waypoint and deploying mirror"));
        if (!firstLightPassWithMirror&&game.getNumMirrorsHeld()>0&&!game.posInDark(futurePos)&&myPosInArea!=-1)
            game.useMirror();
        float target[3];
        target[0]=ItemLoc[closeMirror][0];
        target[1]=-0.35; //I don't know how they determine this coordinate
        target[2]=ItemLoc[closeEnergy][2];
        if (myPosInArea==-1&&memoryFilled==0) game.takePic();
        api.setPositionTarget(target);
    }
    else /*if (game.hasItem(closeEnergy)==-1)*/{
        DEBUG(("going for energy"));
        api.setPositionTarget(ItemLoc[closeEnergy]);
        if (myPosInArea==-1&&memoryFilled==0) game.takePic();
        
    }
	
	// pic logic (37%)
	if (game.isFacingOther() and distanceBetween(myPos,enPos)>PHOTO_MIN_DISTANCE 
        and (myEnergy>1.2f or myPosInArea==1) and enPosInArea!=-1
        and game.getMemoryFilled()<2 && game.getMirrorTimeRemaining() == 0){
        if (!enemyHasMirror or (!enemyMirrorActivated && (game.getPicPoints() > 0.0f))) {
            game.takePic();
        }
        else if (!enemyMirrorActivated&&game.getPicPoints() < 0.0f) {
            enemyMirrorActivated = true;
            mirrorTimerEnd = gameTime + 24;
        }
    }
    
	if (( ((memoryFilled == 2 and myEnergy>1.0f) 
            or (gameTime > 165)) 
            or (myPosInArea==-1 and enPosInArea==-1 and myEnergy > 2.5 ))and memoryFilled>0){
        api.setAttitudeTarget(uploadVec);
        // DEBUG(("rotation: %f angle: %f", mathVecMagnitude(myRot,3),angleBetween(myAtt, uploadVec)));
        if (angleBetween(myAtt, uploadVec) <.25f && mathVecMagnitude(myRot, 3) < UPLOAD_ANG_VEL) {
            game.uploadPics();
        }
    }
    else if ((myEnergy > 1.2 || myPosInArea==1)&& (enPosInArea != -1
              || enSoonInLight())
    ) {
        float vecToTarget[3];
        mathVecSubtract(vecToTarget, enPos, myPos, 3);
        mathVecAdd(vecToTarget, vecToTarget, enVel, 3);
        mathVecAdd(vecToTarget, vecToTarget, myVel, 3);
        mathVecNormalize(vecToTarget,3);
        api.setAttitudeTarget(vecToTarget);
        DEBUG(("Rotating to face (dist between: %f)", distanceBetween(myPos,enPos)));
    }
}

void update() {
    gameTime++;
// 	DEBUG(("energy usage: us:%f them:%f", game.getEnergy()-myEnergy, game.getOtherEnergy()-enEnergy));
	enEnergy=game.getOtherEnergy();
    myEnergy=game.getEnergy();
    api.getMyZRState(myState);
	api.getOtherZRState(enState);
	futurePos[1]==myVel[1]+myPos[1]-.025;
    myPosInArea=game.posInArea(myPos);
    enPosInArea=game.posInArea(enPos);
	
    lightInterfacePos=game.getLightInterfacePosition();
    lightTail = lightInterfacePos-0.05f; // 0.05 accounts for half of grey zone
    lightFront = lightTail+ 0.9; // 0.7 for dark and 0.2 for both grey
    if(lightFront>0.8) lightFront-=1.6; // if it's out of bounds, correct it
    enemyHasMirror = hasEnemyPickedUpAMirror();
}

//End page main

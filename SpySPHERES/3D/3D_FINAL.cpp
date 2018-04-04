//Begin page Enemy Awareness

int enemyItemTargeted() {
    int targetItem = 0;
    float minAngle = PI/6;
    float itemLoc[3];
    float enemyVecToItem[3];
    for (int i = 0; i < 9; i++) {
        game.getItemLoc(itemLoc, i);
        mathVecSubtract(enemyVecToItem, itemLoc, enemyPos, 3);
        float angle = angleCalc(enemyVelo, enemyVecToItem);
        if (angle < minAngle) {
            minAngle = angle;
            targetItem = i;
        }
    }
    return (minAngle < PI/6 && game.hasItem(targetItem) == -1) ? targetItem : -1;   
}

bool plankChecker() {
    return ((game.hasItem(7) == 1 || game.hasItem(8) == 1)
        && (enemyPos[1] < 0.2f)
        && (enemyVelo[1] < -0.015f)
        && switchCount == 1);
}

bool hasEnemyPickedUpAMirror() {
    return (gameTime >= mirrorTimerEnd) ? false : (game.hasItem(7) == 1 || game.hasItem(8) == 1);
    
}
//End page Enemy Awareness
//Begin page Utilities
float angleCalc(float vec1[3], float vec2[3])
{
	return acosf(mathVecInner(vec1, vec2, 3)/(mathVecMagnitude(vec1,3) * mathVecMagnitude(vec2,3)));
}

float distanceBetween(float pos1[3],float pos2[3]){//2%
    float distance[3];
    mathVecSubtract(distance,pos1,pos2,3);
    return mathVecMagnitude(distance,3);
}


void vecManip(float outVec[3], float c, float inVec[3])
{   
    /*This function sets a vec that you task in (outVec) to a scalar multiple of another vec*/
	outVec[0] = c * inVec[0];
	outVec[1] = c * inVec[1];
	outVec[2] = c * inVec[2];
}
//End page Utilities
//Begin page main
//Declare any variables shared between functions here
float myState[12];
float enemyState[12];
float *myPos;
float *myVelo;
float *myAtt;
float *myRot;
float *enemyPos;
float *enemyVelo;
int gameTime;
int timeToLightSwitch;
float energy;
bool blue;
float uploadVec[3];
float zeroVec[3];
int pointPackTarget;
int switchCount;
bool enemyPlanking;
float highSpot[3];
bool enemyDarkStrat;
bool enemyHasMirror;
int mirrorTimerEnd;
bool enemyMirrorActivated;
int closeMirror;
int closeEnergy;

void init(){
    setPointers();
    closeMirror=closeEnergy=-1;
    api.getMyZRState(myState);
    blue = (myState[0] > 0);
    gameTime = -1;
    zeroVec[0] = zeroVec[1] = zeroVec[2] = 0.0f; // use memset?
    uploadVec[0]= 0.0f;
    uploadVec[1] = 0.0f;
    uploadVec[2] = 1.0f;
    pointPackTarget = 6;
    switchCount = 0;
    enemyPlanking = false;
    enemyDarkStrat = false;
    enemyMirrorActivated = false;
    mirrorTimerEnd = 240;
    DEBUG(("Greetings to all teams from BACON!"));
    DEBUG(("Especially Zanneio, great to see you guys back!"));
}

void loop(){
    update();
    float pointPackLoc[3];
    float closeMirrorLoc[3];
    float transitionSpot[3];
    float closeEnergyLoc[3];
    float waitSpot[3];

    if ((mathVecMagnitude(enemyVelo, 3) > 0.02f && enemyItemTargeted() == pointPackTarget && gameTime >= 10 && gameTime < 22) 
        || (game.hasItem(pointPackTarget) != -1 && (timeToLightSwitch > ((enemyDarkStrat) ? 20 : 5)) && switchCount == 0)) {
        pointPackTarget = (enemyPos[0]>myPos[0]) ? 5 : 4;
        //DEBUG(("2 POINT PACK"));
    }
    game.getItemLoc(pointPackLoc, pointPackTarget);
    game.getItemLoc(closeMirrorLoc, closeMirror);
    game.getItemLoc(closeEnergyLoc, closeEnergy);
    transitionSpot[0] = waitSpot[0] = closeMirrorLoc[0];
    transitionSpot[1] = waitSpot[1] = -0.2f;
    transitionSpot[2] = -0.1f;
    waitSpot[2] = 0.25f;
    if (!enemyPlanking)
        enemyPlanking = plankChecker();
    
    if (game.hasItem(pointPackTarget) == -1){
        api.setPositionTarget(pointPackLoc);
        closeMirror = (enemyPos[0]<myPos[0]) ? 8 : 7;
        closeEnergy = (enemyPos[0]<myPos[0]) ? 0 : 1;
    }
    else if (game.hasItem(closeMirror) == -1)
        api.setPositionTarget(closeMirrorLoc);
    else if (!enemyPlanking) {
        if (switchCount == 1 && timeToLightSwitch < 28 && myPos[1] > -0.05f) {
            api.setPositionTarget(transitionSpot);
            game.useMirror();
        }
        else if (switchCount == 1 && timeToLightSwitch < 25)
            api.setPositionTarget(waitSpot);
        else if (switchCount == 2 && ((game.getMemoryFilled() == 0 && energy < 3.0f) 
            || (energy < 2.0f)
            || (game.posInArea(enemyPos) == -1))) {
            game.useMirror();
            api.setPositionTarget(closeEnergyLoc);
            }
        else
            api.setVelocityTarget(zeroVec);
    }
    else {
        api.setPositionTarget(highSpot);
    }
    if (((timeToLightSwitch < 24 && switchCount == 2) || switchCount > 2) && game.getMemoryFilled() < 2)
        api.setPositionTarget(highSpot);
    if (game.isFacingOther() and distanceBetween(myPos,enemyPos)>.5f 
        and (energy>1.2f or game.posInLight(myPos)) and game.posInArea(enemyPos)!=-1
        and game.getMemoryFilled()<2){
        if (((!enemyHasMirror) or (!enemyMirrorActivated && (game.getPicPoints() > 0.0f))) && game.getMirrorTimeRemaining() == 0) {
            game.takePic();
        }
        else if (!enemyMirrorActivated) {
            if (game.getPicPoints() < 0.0f) {
                enemyMirrorActivated = true;
                mirrorTimerEnd = gameTime + 24;
            }
        }
    }
    int memoryFilled = game.getMemoryFilled();
	if (( ( ((memoryFilled == 2 and energy>1.0f) 
            or (memoryFilled > 0 and game.getCurrentTime() > 170))
            && !(enemyDarkStrat && game.posInDark(myPos) && switchCount == 1)) 
            or (game.posInDark(myPos)and game.posInDark(enemyPos) and energy > 2.5 and timeToLightSwitch>15))and memoryFilled>0){
        api.setAttitudeTarget(uploadVec);
        DEBUG(("rotation: %f angle: %f", mathVecMagnitude(myRot,3),angleCalc(myAtt, uploadVec)));
        if (angleCalc(myAtt, uploadVec) <.25f && mathVecMagnitude(myRot, 3) < 0.05f) {
            game.uploadPics();
        }
    }
    else if (energy > 1. && ((game.posInArea(enemyPos) != -1) 
            || ((gameTime < 30 || timeToLightSwitch < 17))))
            {
        float vecToTarget[3];
        mathVecSubtract(vecToTarget, enemyPos, myPos, 3);
        mathVecAdd(vecToTarget, vecToTarget, enemyVelo, 3);
        mathVecAdd(vecToTarget, vecToTarget, myVelo, 3);
        mathVecNormalize(vecToTarget,3);
        api.setAttitudeTarget(vecToTarget);
    }
    if (mathVecMagnitude(myVelo, 3) > 0.02f && energy < 0.5f)
        api.setVelocityTarget(zeroVec);
    if (game.getFuelRemaining() == 0)
        game.takePic();
}




void update() {
    gameTime++;
    energy = game.getEnergy();
    timeToLightSwitch = game.getLightSwitchTime();
    if (timeToLightSwitch == 60) 
        switchCount ++;
    api.getMyZRState(myState);
    api.getOtherZRState(enemyState);

    highSpot[0] = myPos[0];
    highSpot[1] = -0.2f;
    highSpot[2] = -0.5f;
    if (gameTime == 25 && game.posInArea(enemyPos) == -1)
        enemyDarkStrat = true;
    enemyHasMirror = hasEnemyPickedUpAMirror();
}

inline void setPointers() { //3%   Talis-Pointer mumbo-jumbo
    myPos=myState;
    enemyPos=enemyState;
    myVelo=myState+3;
    enemyVelo=enemyState+3;
    myAtt=myState+6;
    myRot=myState+9;
}
//End page main

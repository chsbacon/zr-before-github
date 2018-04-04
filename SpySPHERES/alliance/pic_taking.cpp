//Begin page Awareness
bool hasEnemyPickedUpAMirror() {
    return (gameTime >= mirrorTimerEnd) ? false : (game.hasItem(7) == 1 || game.hasItem(8) == 1);
    
}

int enemyItemTargeted() {
    int targetItem = 0;
    float minAngle = PI/6.0f;
    float itemLoc[3];
    float enemyVecToItem[3];
    for (int i = 0; i < 9; i++) {
        game.getItemLoc(itemLoc, i);
        mathVecSubtract(enemyVecToItem, itemLoc, enemyPos, 3);
        float angle = angleBetween(enemyVel, enemyVecToItem);
        if (angle < minAngle) {
            minAngle = angle;
            targetItem = i;
        }
    }
    return (minAngle < PI/6.0f && game.hasItem(targetItem) == -1) ? targetItem : -1;   
}

int enTimeToLight() {
    float lightTail = game.getLightInterfacePosition()-0.05f; // 0.05 accounts for half of grey zone
    float lightFront = (lightTail + 0.9); // 0.7 for dark and 0.2 for both grey
    lightFront-= lightFront>0.8 ? 1.6: 0; // if it's out of bounds, correct it
    int timeToTail = (int) ((lightTail - enemyPos[1])/ (enemyVel[1] - LIGHT_SPEED));
    int timeToFront = (int) ((lightFront - enemyPos[1])/ (enemyVel[1] - LIGHT_SPEED));
    timeToFront=timeToFront<0?500:timeToFront;
    timeToTail=timeToTail<0?500:timeToTail;
    return (timeToTail<timeToFront? timeToTail : timeToFront==500?-1:timeToFront);

                // y = LIGHT_SPEED(t) + lightPoint
                // y = enemyVel[i](t) + enemyPos[1]
                //  lightPoint - enemyPos[1] = enemyVel[i](t) - LIGHT_SPEED(t)
                // (lightPoint - enemyPos[1])/ (enemyVel[1] - LIGHT_SPEED) = t
}
//End page Awareness
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

//End page Vectors
//Begin page main

float myState[12];
float enemyState[12];
float *myPos;
float *myVel;
float *myAtt;
float *myRot;
float *enemyPos;
float *enemyVel;
float *enemyAtt;
int gameTime;
float energy;
bool blue;
float zeroVec[3];
float uploadVec[3];
float lightInterfacePos[3];
int pointPackTarget;
bool enemyHasMirror;
int mirrorTimerEnd;
bool enemyMirrorActivated;
int timeToLightEntry;

void init(){
    setPointers();
    blue = myPos[0] > 0;
    memset(zeroVec, 0.0f, sizeof(float)*3);
    memcpy(uploadVec, EARTH, sizeof(float)*3);
    mirrorTimerEnd = 240;
    gameTime = -1;
    lightInterfacePos[0]=0.0;lightInterfacePos[2]=0.0;
}

void loop(){
    update();
    DEBUG(("time to light cross: %d", timeToLightEntry));
    if (game.isFacingOther() and distanceBetween(myPos,enemyPos)>PHOTO_MIN_DISTANCE 
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
	if (( ((memoryFilled == 2 and energy>1.0f) 
            or (game.getCurrentTime() > 170)) 
            or (game.posInDark(myPos)and game.posInDark(enemyPos) and energy > 2.5 /*and timeToLightSwitch>15*/))and memoryFilled>0){
        api.setAttitudeTarget(uploadVec);
        DEBUG(("rotation: %f angle: %f", mathVecMagnitude(myRot,3),angleBetween(myAtt, uploadVec)));
        if (angleBetween(myAtt, uploadVec) <.25f && mathVecMagnitude(myRot, 3) < UPLOAD_ANG_VEL) {
            game.uploadPics();
        }
    }
    else if (energy > ENERGY_COST_TAKE_PICTURE && (game.posInArea(enemyPos) != -1
              || (timeToLightEntry<4 and timeToLightEntry>=0))
    ) {
        float vecToTarget[3];
        mathVecSubtract(vecToTarget, enemyPos, myPos, 3);
        mathVecAdd(vecToTarget, vecToTarget, enemyVel, 3);
        mathVecAdd(vecToTarget, vecToTarget, myVel, 3);
        mathVecNormalize(vecToTarget,3);
        api.setAttitudeTarget(vecToTarget);
    }
//     if (mathVecMagnitude(myVel, 3) > 0.02f && energy < 0.5f)
//         api.setVelocityTarget(zeroVec);
//     if (game.getFuelRemaining() == 0)
//         game.takePic();
}

void update() {
    gameTime++;
    energy = game.getEnergy();
    api.getMyZRState(myState);
    api.getOtherZRState(enemyState);
    // highSpot[0] = myPos[0];
    // highSpot[1] = -0.2f;
    // highSpot[2] = -0.5f;
    lightInterfacePos[1]=game.getLightInterfacePosition();
    enemyHasMirror = hasEnemyPickedUpAMirror();
    timeToLightEntry = enTimeToLight();
}
void setPointers() { // Talis-Pointer mumbo-jumbo (rude! t. Talis)
    myPos=myState;
    enemyPos=enemyState;
    myVel=myState+3;
    enemyVel=enemyState+3;
    myAtt=myState+6;
    enemyAtt=enemyState+6;
    myRot=myState+9;
}
//End page main

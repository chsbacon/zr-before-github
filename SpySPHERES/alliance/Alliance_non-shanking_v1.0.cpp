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
    // DEBUG(("predicted: %f tail: %f front: %f",predictedPos, soonlightTail, soonlightFront));
    return (predictedPos>soonlightTail || predictedPos<soonlightFront)? true : false;
}

int timeToLight(float point[3]) {
    int timeToFront = (int) ((point[1]-lightFront)/ (LIGHT_SPEED));
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
//Begin page Item Targeting
void getDistances(){
    float dist[3];
    for (int i=0;i<9;i++){
        if (game.hasItem(i)==-1){
            mathVecSubtract(dist,&ItemLoc[i][0],myPos,3);
            distToItems[i]=mathVecMagnitude(dist,3);
        }  
        else{
            distToItems[i]=255.0;
        }
    }
}

// short selectEnergy(){
//     short shorter;
//     shorter = (distToItems[0]<distToItems[1] and distToItems[0]<distToItems[2])?0:((distToItems[1]>distToItems[2])?1:2);
//     return (distToItems[shorter]<10)?shorter:-1;
// }

// short selectMirror(){
//     short shorter = (distToItems[7]<distToItems[8])?7:8;
//     return (distToItems[shorter]<10)?shorter:-1;
// }

// inline bool pointPacksClumpedX(){
//     float posOf3[3];
//     float posOf4[3];
//     float posOf5[3];
//     mathVecSubtract(posOf4,posOf3,posOf4,3);
//     mathVecSubtract(posOf5,posOf3,posOf5,3);
//     return (mathVecMagnitude(posOf4,3)<mathVecMagnitude(posOf5,3))?true:false;
// }

// short selectEnergy(){
//     short shortest;
//     bool validPairs[2];
//     if (pointPacksClumpedX()){
//         validPairs[0]=(distToItems[3]<10 and distToItems[4]<10)?true:false;
//         validPairs[1]=(distToItems[5]<10 and distToItems[6]<10)?true:false;
//         if (validPairs[0]){
//             shortest = (distToItems[3]<distToItems[4])?distToItems[3]:distToItems[4];
//         }
//         if (validPairs[1]){
//             shortest = (distToItems[5]<distToItems[6])?distToItems[5]:distToItems[6];
//         }
//         shortest =  closestPointPack();
//     }
//     else{
//         validPairs[0]=(distToItems[3]<10 and distToItems[5]<10)?true:false;
//         validPairs[1]=(distToItems[4]<10 and distToItems[6]<10)?true:false;
//         if (validPairs[0]){
//             shortest =  (distToItems[3]<distToItems[5])?distToItems[3]:distToItems[5];
//         }
//         if (validPairs[1]){
//             shortest =  (distToItems[4]<distToItems[6]) ? distToItems[4] : distToItems[6];
//         }
//         shortest =  closestPointPack();
//     }
//     return (distToItems[shortest]<10)?shortest:-1;
// }

short closestPointPack(){
    if (distToItems[3] < distToItems[4] and distToItems[3] < distToItems[5] and distToItems[3] < distToItems[6])
        return 3;
    else if (distToItems[4] < distToItems[5] and distToItems[4] < distToItems[6])
        return 4;
    else if (distToItems[5] < distToItems[6])
        return 5;
    else if (distToItems[6]<10)
        return 6;
    else
        return -1;
}
//End page Item Targeting
//Begin page Movement
void getToInTime(float position[], int time) {//6%
    float usToIt[3];
    mathVecSubtract(usToIt, position, myPos, 3);
    float distance = mathVecMagnitude(usToIt, 3);
    if (distance > .1) {
        // DEBUG(("VELOCITY CONTROL"));
        float speed = distance/time;
        mathVecNormalize(usToIt, 3);
        scale(usToIt, speed);
        api.setVelocityTarget(usToIt);
    }
    else {
        //DEBUG(("POSITION CONTROL"));
        api.setPositionTarget(position);
    }
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

// bool gainEnergy;//Have they gained energy?
//if they have we should back off
// bool goingOut;//are they going out of bounds

float distToItems[9];
float target[3];//for shanking
float zeroVec[3];
float uploadVec[3];
float lightInterfacePos[3]; //we only ever access [1], I think - remove and replace with float?

short energyChoice;//item ID or -1 for not taking energy

float ItemLoc[NUM_ITEMS][3];//ITEM_LOC didn't seem to be working in alliance

bool blue, energyTaken;

short gameTime;
bool enemyHasMirror;
short mirrorTimerEnd;
bool enemyMirrorActivated;

float lightTail, lightFront;

void init()
{
	memset(zeroVec, 0.0f, 12); //sizeof(float) is 4
    memcpy(uploadVec, EARTH, 12);
    // energyTaken=false;
    // goingOut=false;
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
// 	enEnergy=5.0f;
// 	gainEnergy=false;
	
	//Filling ItemLoc
	for(int i=0;i<NUM_ITEMS;i++)
	{
	    game.getItemLoc(ItemLoc[i],i);
	}
	
	//Are we blue?
	api.getMyZRState(myState);
	blue = myPos[0]>0.0f;
	
// 	//Which energy pack will we take?
// 	energyChoice = blue ? 0 : 1;//going for our side
// 	if(ItemLoc[energyChoice][2]>0.15f&&ItemLoc[2][2]>0.15f)
// 	{//They're too low
// 	    energyChoice = -1;
// 	}
// 	else if  (ItemLoc[energyChoice][2]>ItemLoc[2][2])
// 	{//Get the higher one
// 	    energyChoice = 2;
// 	}
}


void loop()
{
	update();
	
	//GameLogic
	    #define SPEEDUP 2
	    short mirrorChoice=blue?7:8;
        if (game.hasItem(mirrorChoice)==-1) {
            getToInTime(ItemLoc[mirrorChoice], timeToLight(ItemLoc[mirrorChoice])-SPEEDUP);
            
        }
        else  {
            if (fabsf(lightFront-myPos[1])<0.035f && (mirrorTimerEnd==240||gameTime>mirrorTimerEnd)) { 
                game.useMirror();
            }
            short closestPP = closestPointPack();
            if (closestPP!=-1 && !(distanceBetween(myPos,ItemLoc[closestPP])>0.5&&enemyItemTargeted()==closestPP)) api.setPositionTarget(ItemLoc[closestPP]);
            else if (myEnergy>1) {
                float high[3];
                high[0]=myPos[0];high[1]=myPos[1];
                if (distanceBetween(myPos,enPos)<PHOTO_MIN_DISTANCE) {
                    mathVecSubtract(high,myPos, enPos, 3);
                }
                high[2]=-0.5;
                api.setPositionTarget(high);
            }
            else api.setVelocityTarget(zeroVec);
        }

	// pic logic
	if (game.isFacingOther() and distanceBetween(myPos,enPos)>PHOTO_MIN_DISTANCE 
        and (myEnergy>1.2f or game.posInLight(myPos)) and game.posInArea(enPos)!=-1
        and game.getMemoryFilled()<2 && game.getMirrorTimeRemaining() == 0){
        // DEBUG(("enemyHasMirror:%d enemyMirrorActivated:%d",enemyHasMirror,enemyMirrorActivated));
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
    int memoryFilled = game.getMemoryFilled();
	if (( ((memoryFilled == 2 and myEnergy>1.0f) 
            or (game.getCurrentTime() > 170)) 
            or (game.posInDark(myPos)and game.posInDark(enPos) and myEnergy > 2.5 ))and memoryFilled>0){
        api.setAttitudeTarget(uploadVec);
        DEBUG(("rotation: %f angle: %f", mathVecMagnitude(myRot,3),angleBetween(myAtt, uploadVec)));
        if (angleBetween(myAtt, uploadVec) <.25f && mathVecMagnitude(myRot, 3) < UPLOAD_ANG_VEL) {
            game.uploadPics();
        }
    }
    else if (myEnergy > ENERGY_COST_TAKE_PICTURE && (game.posInArea(enPos) != -1
              || enSoonInLight())
    ) {
        float vecToTarget[3];
        mathVecSubtract(vecToTarget, enPos, myPos, 3);
        mathVecAdd(vecToTarget, vecToTarget, enVel, 3);
        mathVecAdd(vecToTarget, vecToTarget, myVel, 3);
        mathVecNormalize(vecToTarget,3);
        api.setAttitudeTarget(vecToTarget);
        DEBUG(("Rotating to face"));
    }
}

void update() {
    gameTime++;
	enEnergy=game.getOtherEnergy();
    myEnergy=game.getEnergy();
    api.getMyZRState(myState);
	api.getOtherZRState(enState);
	getDistances();
	
// 	goingOut= enEnergy<0.1f&&(enVel[1]>LIGHT_SPEED
//     ||enPos[1]>0.7f
//     ||enPos[0]>0.54f || enPos[0]<-0.54f
//     ||enPos[2]>0.54f || enPos[2]<-0.54f);

// 	gainEnergy=myEnergy>enEnergy;


    lightInterfacePos[1]=game.getLightInterfacePosition();
    lightTail = lightInterfacePos[1]-0.05f; // 0.05 accounts for half of grey zone
    lightFront = lightTail+ 0.9; // 0.7 for dark and 0.2 for both grey
    if(lightFront>0.8) lightFront-=1.6; // if it's out of bounds, correct it
    enemyHasMirror = hasEnemyPickedUpAMirror();
}

//End page main

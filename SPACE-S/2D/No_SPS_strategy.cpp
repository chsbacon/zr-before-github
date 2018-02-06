//Begin page main
//objctives:

//Google doc with strategy and other stuff:
//We should try to have this open while editing the program
//https://docs.google.com/document/d/10w-H8W0UfstahHsAHaU2zgSZeyglRemLZVez4ZFFrww/edit?usp=sharing


float myState[12];
float enState[12];
#define myPos (&myState[0])
#define myVel (&myState[3])
#define myAtt (&myState[6])
#define myRot (&myState[9])
#define enPos (&enState[0])
#define enVel (&enState[3])
#define enAtt (&enState[6])
#define enRot (&enState[9])
float stealLoc[3];
float center[3];
bool isBlue;
int lastItem;
int tItem;
bool pOne;
bool pTwo;
int enScoreOneTick;
int enScoreTwoTick;
float myZone[3];
float firstSPS[3];
float secondSPS[3];
float thirdSPS[3];

#define SPEEDCONST .2

void init(){
    lastItem = 0;
    update();
    pOne = true;
    lastItem = -1;
    game.dropSPS();
    firstSPS[0] = 0;
    firstSPS[1]= .15;
    firstSPS[2] = 0;
    isBlue = 0;
    center[0] = .375;
    center[1] = 0;
    center[2] = 0;
    enScoreOneTick = 0;
    api.setPosGains(SPEEDCONST,0.1,10 * SPEEDCONST);
}

void loop(){
    //updates basic information
	update();
	//finding how much they've scored
	enScoreTwoTick = enScoreOneTick;
	enScoreOneTick = game.getOtherScore() - enScoreOneTick;
	//this will be subsituted with our ideal item algorithm
	tItem = 3;
	
	//determines which stage of the strategy we're in
	if(pOne)
	    phaseOne(tItem);
	else if(pTwo)
	    phaseTwo();
}

//The first stage, we got and get their item
void phaseOne(int tItem){
    if(game.getCurrentTime() < 2)
	    isBlue = tellIfBlue();
	if(!isBlue)
	    symm(center);
	    symm(firstSPS);
	  
    if(pickUpItemJamie(tItem)){
        game.dropSPS();
        for(int i = 0; i < 3; i ++)
            secondSPS[i] = myPos[i];
        pOne = false;
        pTwo = true;
    }
}

//the second stage, we go back and wait for them to score points
void phaseTwo(){
    //if they've placed all their satellites and are carrying an item, mirror them
    //if they've placed all their satellites and have no item go back
    //if they haven't placed all their satellites go back and start herding
    float distOne[3];
    float distTwo[3];
    float distanceOne;
    float distanceTwo;
    
    if(enScoreOneTick - enScoreTwoTick >= 10){
        game.getItemLoc(myZone, lastItem);
        symm(myZone);
        api.setPositionTarget(myZone);
    }
    else
        api.setPositionTarget(center);
        
    //when I'm equidistant from both SPS drop the third
    DEBUG(("Dist One %f Dist Two %f", distOne, distTwo));
    mathVecSubtract(distOne, firstSPS, myPos, 3);
    mathVecSubtract(distTwo, secondSPS, myPos, 3);
    distanceOne = mathVecMagnitude(distOne, 3);
    distanceTwo = mathVecMagnitude(distTwo, 3);
    DEBUG(("Distance difference %f", fabsf(distanceOne - distanceTwo)));
    if(fabsf(distanceOne - distanceTwo) < .1){
        game.dropSPS();
        for(int i = 0; i < 3; i ++)
            thirdSPS[i] = myPos[i];
    }
}

int enLastItem(int myItem){
    for(int i = 0; i < 6; i ++)
        if(i != myItem)
            if(game.hasItem(i) == 2)
                return i;
    
    return -1;
}

/*
int getPreferredItem(){
    //this function tells us which item to go for
    //add fuction for detecting if we can get to item first
    int itemIndex[6]
    if (tellIfBlue == 1{
        itemIndex[0] = 1;
        itemIndex[1] = 5;
        itemIndex[2] = 2;
        itemIndex[3] = 4;
        itemIndex[4] = 3;
        itemIndex[5] = 0;
    }
    else{
        itemIndex[0] = 0;
        itemIndex[1] = 4;
        itemIndex[2] = 3;
        itemIndex[3] = 5;
        itemIndex[4] = 2;
        itemIndex[5] = 1;
    }
    for (int i = 0; i < 6; i++){
        if (!itemHasBeenPickedUp(itemIndex[i]) &&  !enTarg(itemIndex[i])){
            return itemList[i];
        }
    }
    
}
*/
/*

bool enTarg(int itemId){
    if (getAngBetween(enAtt, enVel) < .01 && getAngBetween(enAtt, enVel) > -.01){
        mathVecSubract(float targ[3], getItemLoc(float itemPos[3], itemId), enPos);
        if(getAngBetween(targ, enAtt) < .01 && getAngBetween(targ, enAtt) > -.01){
            return true
        }
        else{
            return false
        }
    }
    else{
        return false
    }
}
*/

//finds the distance between two

bool pickUpItemJamie(int item){
    float dist = 0;
    float itemLoc[3];
    float tVector[3];
    float pickDist;
    float midPos[3];
    bool gotIt = false;
    //finding the vector between our pos and the target item
    game.getItemLoc(itemLoc, item);
    mathVecSubtract(tVector, itemLoc, myPos, 3);
    //finding the distance to the item
    dist = mathVecMagnitude(tVector, 3);
    //normalize the vector then face it
    mathVecNormalize(tVector, 3);
    api.setAttitudeTarget(tVector);
    //if I'm close enough pick up the item
    if(item < 2)
        pickDist = .162;
    else if(item > 3)
        pickDist = .135;
    else
        pickDist = .149;
    if(dist < pickDist){
            //DEBUG(("Docking %f", mathVecMagnitude(myVel, 3)));
            if(game.dockItem())
                gotIt = true;
            //trying to reverse
            scale(tVector, (dist - pickDist));
            //mathVecMagnitude(tVector, 3)));
            mathVecAdd(midPos, myPos, tVector, 3);
            api.setPositionTarget(midPos);
            }
        else
            api.setPositionTarget(itemLoc);
    return gotIt;
}

void scale(float array[], float scalar){
    //scales an array by a scalar
    for(int i = 0; i < 3; i ++)
       array[i] *= scalar;
}

void update(){
    //updates general information abou the spheres
    api.getMyZRState(myState);
	api.getOtherZRState(enState);
}

bool tellIfBlue(){
    //tells which side we're on
    bool isBlue = 1;
	if(myPos[1] > 0)
        isBlue = 1;
    else
        isBlue = 0;
    DEBUG(("Am I blue %i", isBlue));
    return isBlue;
}

void symm(float arr[3]){
    for(int i = 0; i < 3; i ++)
        arr[i] *= -1;        
}

float getAngleBetween(float vecOne[3], float vecTwo[3]){
    //finds the angle between two 3d vectors
    float dotProd = ((vecOne[0] * vecTwo[0]) + (vecOne[1] * vecTwo[1]) + (vecOne[2] * vecTwo[2]));
    dotProd = dotProd / (mathVecMagnitude(vecOne, 3) * mathVecMagnitude(vecTwo, 3));
    return acosf(dotProd);
}
//End page main

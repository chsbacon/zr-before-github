//Begin page main
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

int targetItem;
float pickUpDist;

float SPSLocs[3][3];

float myZone[4];
float newZoneInfo[4];
int numZoneDataPoints;

int time;
#define ROUND_LENGTH 180

float myScore;
float enScore;
float myDeltaScore;
float enDeltaScore;
bool newItemInZone;

bool carryingItem;
bool pickNewTargetItem;

int mode;
#define SPS 0
#define ITEM 1
#define DEFEND 2
#define ATTACK 3

#define PRINTVEC(vec) DEBUG(("%f %f %f", vec[0], vec[1], vec[2]))

float zeroVec[3];

void init(){
    #define SPEEDCONST .2
    api.setPosGains(SPEEDCONST,0.1,10 * SPEEDCONST);
    // fast is nice but we can't let this happen:
    // http://zerorobotics.mit.edu/ide/simulation/1163130/
    game.dropSPS();
    api.getMyZRState(myState);
    memcpy(SPSLocs[0], myPos, 12);
	mode = SPS;
	carryingItem=false;
	pickNewTargetItem=true;
	newItemInZone=false;
	memset(zeroVec, 0.0f, 12);
	memset(myZone, 0.0f, 12);
	numZoneDataPoints=0;
}

void loop(){
    api.getMyZRState(myState);
    api.getOtherZRState(enState);

    newItemInZone = game.getScore()-myScore > myDeltaScore;
    myDeltaScore = game.getScore()-myScore;
    myScore = game.getScore();
    enDeltaScore = game.getOtherScore()-enScore;
    enScore = game.getOtherScore();

    time = game.getCurrentTime();
    
    if(game.getNumSPSHeld()>0) mode = SPS;
    else {
        game.getZone(newZoneInfo);
        scale(myZone, numZoneDataPoints);
        mathVecAdd(myZone, myZone, newZoneInfo, 3);
        scale(myZone, 1.0f/(++numZoneDataPoints));
        DEBUG(("%d zone: %f %f %f %f", numZoneDataPoints, myZone[0],myZone[1],myZone[2],myZone[3]));
        // game.getZone(myZone);
        if(!carryingItem &&
        (myScore+(myDeltaScore*(ROUND_LENGTH-time)))>
        (1+enScore+(enDeltaScore*(ROUND_LENGTH-time)))
        && compareDiff(zeroVec, enVel, enPos, myZone))
            mode = DEFEND;
        else if (true) mode = ITEM;
    }
    if (pickNewTargetItem)
        targetItem = bestItem();
    float itemLoc[3];
    game.getItemLoc(itemLoc, targetItem);
    DEBUG(("item dist to zone: %f", dist(itemLoc, myZone)));
    DEBUG(("mode:%d item:%d", mode, targetItem));
    switch(mode) {
        case SPS:
            {
                // memcpy(SPSLocs[1], itemLoc, 12);
                pickNewTargetItem=false;
                mathVecSubtract(SPSLocs[1],itemLoc, myPos,3);
                float itemDistance = mathVecNormalize(SPSLocs[1],3);
                scale(SPSLocs[1], itemDistance-itemDockDistOuter(targetItem));
                mathVecAdd(SPSLocs[1],SPSLocs[1], myPos, 3);
                PRINTVEC(SPSLocs[1]);
                mathVecSubtract(SPSLocs[2], SPSLocs[1], SPSLocs[0], 3);
                scale(SPSLocs[2], 0.5f);
                float altitude[3];
                float zaxis[3]; zaxis[0]=0; zaxis[1]=0; zaxis[2]=1;
                mathVecCross(altitude, SPSLocs[2], zaxis);
                mathVecNormalize(altitude, 3);
                scale(altitude, sqrtf(3)*mathVecMagnitude(SPSLocs[2],3));
                mathVecAdd(SPSLocs[2],SPSLocs[2],SPSLocs[0],3);
                mathVecAdd(SPSLocs[2], altitude, SPSLocs[2], 3);
    
                if (game.getNumSPSHeld()==2) {
                    api.setPositionTarget(SPSLocs[2]);
                    if (dist(SPSLocs[2],myPos)<0.05f) game.dropSPS();
                }
                else {
                    api.setPositionTarget(SPSLocs[1]);
                    if (dist(SPSLocs[1],myPos)<0.05f) game.dropSPS();
                    //pickNewTargetItem=true;
                }
            }
            break;
        case ITEM:
            pickNewTargetItem=false;
            if(!carryingItem) {
                if(getItem(targetItem)) {
                    DEBUG(("picked up item"));
                    carryingItem = true;
                }
            }
            else {
                DEBUG(("taking the item back to the zone"));
                float toZone[3];
                mathVecSubtract(toZone, myZone, myPos, 3);
                api.setAttitudeTarget(toZone);
                float distToZone = mathVecNormalize(toZone,3);
                scale(toZone, distToZone - 0.15);
                mathVecAdd(toZone, toZone, myPos,3);
                api.setPositionTarget(toZone);
                float heldItem[3];
                memcpy(heldItem, myAtt, 12);
                scale(heldItem, pickUpDist);
                mathVecAdd(heldItem, heldItem, myPos,3);
                if (dist(heldItem, myZone) < 0.1) {
                    game.dropItem();
                    carryingItem=false;
                    pickNewTargetItem=true;
                }
            }
            break;
        case DEFEND:
            api.setPositionTarget(myZone);
            break;
        
    }
}

// does not work if you start too close to item
bool getItem(int id) {
    //DEBUG(("getting %d", id));
    float toItem[3], itemLoc[3], targetPos[3];
    float itemPickupDist = itemDockDistOuter(id);
    game.getItemLoc(itemLoc, id);
    mathVecSubtract(toItem, itemLoc, myPos, 3);
    float itemDist = mathVecNormalize(toItem, 3);
    api.setAttitudeTarget(toItem);
    scale(toItem, itemDist-itemPickupDist+0.01f);
    DEBUG(("dist:%f (%f) speed:%f angle:%f", itemDist, itemDist-itemPickupDist+0.01f, mathVecMagnitude(myVel,3),angle(myAtt,toItem)));
    mathVecAdd(targetPos, toItem, myPos, 3);
    api.setPositionTarget(targetPos);
    //DEBUG(("getItem target:"));
    PRINTVEC(toItem);
    if(mathVecMagnitude(myVel,3)<0.01f && itemDist<itemPickupDist && angle(myAtt,toItem)<0.25f) {
        DEBUG(("attmempting to dock"));
        pickUpDist = itemDist;
        return game.dockItem();
    }
    return false;
}

int bestItem() {
    #define NUM_ITEMS 6
    #define INV_DIST_WEIGHT 1.0
    #define VEL_WEIGHT 1.0
    #define EN_VEL_WEIGHT -1.0
    #define LARGE_ITEM_WEIGHT -2.5
    #define MYZONE_WEIGHT -10
    float itemWeights[NUM_ITEMS];
    memset(itemWeights, 0.0f, NUM_ITEMS*4);
    for (int i=0; i<NUM_ITEMS; i++) {
        float loc[3];
        game.getItemLoc(loc, i);
        float diff[3];
        mathVecSubtract(diff, loc, myPos, 3);
        itemWeights[i]+= INV_DIST_WEIGHT * (1/mathVecNormalize(diff, 3));
        itemWeights[i]+= VEL_WEIGHT * mathVecInner(myVel, diff,3);
        float enDiff[3];
        mathVecSubtract(enDiff, loc, enPos, 3);
        itemWeights[i]+= EN_VEL_WEIGHT * mathVecInner(enVel, diff,3);
        int type = game.getItemType(i);
        itemWeights[i]+= LARGE_ITEM_WEIGHT * (type==ITEM_TYPE_LARGE);
        itemWeights[i]+= MYZONE_WEIGHT * (dist(loc, myZone)<0.12);
    }
    return maxArray(itemWeights, NUM_ITEMS);
}

float itemDockDistOuter(int id) {
    int itemType = game.getItemType(id);
    return (itemType==ITEM_TYPE_LARGE) ? 
            0.173f :(itemType==ITEM_TYPE_MEDIUM ? 0.16f : 0.146f);
}
int maxArray(float* arr, int len) {
    float max = arr[0];
    float maxIndex = 0;
    for (int i=1;i<len; i++) {
        if (arr[i]>max) {
            max = arr[i];
            maxIndex=i;
        }
    }
    return maxIndex;
}
void scale (float* vec, float scale) {
    for (int i=0; i<3; i++) {
        vec[i] *= scale;
        // a = a*b
        // a *= b
    }
}

float dist(float* vec1, float* vec2) {
    float diff[3];
    mathVecSubtract(diff, vec1, vec2, 3);
    return mathVecMagnitude(diff,3);
}

float angle(float* vec1, float* vec2) {
    return acosf(mathVecInner(vec1,vec2,3)/(mathVecMagnitude(vec1,3)*mathVecMagnitude(vec2,3)));
}

bool compareDiff(float* a1, float* a2, float* b1, float*b2) {
    #define tol 0.15f
    float aDiff[3], bDiff[3];
    mathVecSubtract(aDiff, a2, a1, 3);
    mathVecSubtract(bDiff, b2, b1, 3);
    return (angle(aDiff, bDiff)<tol);
}
//End page main

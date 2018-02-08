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

int targetID;

float SPSLocs[3][3];

float zone[4];

bool carryingItem;

int mode;
#define SPS 0
#define ITEM 1
#define DEFEND 2
#define ATTACK 3

#define PRINTVEC(vec) DEBUG(("%f %f %f", vec[0], vec[1], vec[2]))

void init(){
    game.dropSPS();
    api.getMyZRState(myState);
    memcpy(SPSLocs[0], myPos, 12);
	targetID=5;
	mode = SPS;
	carryingItem=false;
}

void loop(){
    api.getMyZRState(myState);
    api.getOtherZRState(enState);

    if(game.getNumSPSHeld()>0) mode = SPS;
    else {
        game.getZone(zone);
        //DEBUG(("%f %f %f %f",zone[0],zone[1],zone[2],zone[3]));
        if (true) mode = ITEM;
    }
    int targetItem = bestItem();
    float itemLoc[3];
    game.getItemLoc(itemLoc, targetItem);
    DEBUG(("mode:%d item:%d", mode, targetItem));
    switch(mode) {
        case SPS:
            {
                // memcpy(SPSLocs[1], itemLoc, 12);
                if (game.getCurrentTime()<10) {
                    mathVecSubtract(SPSLocs[1],itemLoc, myPos,3);
                    float itemDistance = mathVecNormalize(SPSLocs[1],3);
                    scale(SPSLocs[1], itemDistance-itemDockDistOuter(targetItem));
                    mathVecAdd(SPSLocs[1],SPSLocs[1], myPos, 3);
                    PRINTVEC(SPSLocs[1]);
                }
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
                }
            }
            break;
        case ITEM:
            DEBUG(("%scarrying item", (carryingItem?"":"not ")));
            if(!carryingItem) {
                if(getItem(targetItem)) {
                    DEBUG(("picked up item"));
                    carryingItem = true;
                }
            }
            else {
                DEBUG(("taking the item back to the zone"));
                float toZone[3];
                mathVecSubtract(toZone, zone, myPos, 3);
                api.setAttitudeTarget(toZone);
                api.setPositionTarget(zone);
                if (dist(myPos, zone)< 0.2) {
                    game.dropItem();
                    carryingItem=false;
                }
            }
            break;
            
    }
}

// does not work if you start too close to item
bool getItem(int id) {
    DEBUG(("getting %d", id));
    float toItem[3], itemLoc[3];
    float itemPickupDist = itemDockDistOuter(id);
    game.getItemLoc(itemLoc, id);
    mathVecSubtract(toItem, itemLoc, myPos, 3);
    PRINTVEC(toItem);
    float itemDist = mathVecNormalize(toItem, 3);
    api.setAttitudeTarget(toItem);
    scale(toItem, itemDist-itemPickupDist+0.15f);
    //DEBUG(("dist:%f (%f) speed:%f", itemDist, itemPickupDist, mathVecMagnitude(myVel,3)));
    mathVecAdd(toItem, toItem, myPos, 3);
    api.setPositionTarget(toItem);
    DEBUG(("getItem target:"));
    PRINTVEC(toItem);
    if(mathVecMagnitude(myVel,3)<0.01f && itemDist<itemPickupDist && angle(myAtt,toItem)<0.25f) {
        DEBUG(("attmempting to dock"));
        return game.dockItem();
    }
    return false;
}

int bestItem() {
    #define NUM_ITEMS 6
    #define DIST_WEIGHT -1.0
    #define VEL_WEIGHT 1.0
    #define EN_VEL_WEIGHT -1.0
    #define LARGE_ITEM_WEIGHT -3.0
    float itemWeights[NUM_ITEMS];
    memset(itemWeights, 0.0f, NUM_ITEMS*4);
    for (int i=0; i<NUM_ITEMS; i++) {
        float loc[3];
        game.getItemLoc(loc, i);
        float diff[3];
        mathVecSubtract(diff, loc, myPos, 3);
        itemWeights[i]+= DIST_WEIGHT * mathVecNormalize(diff, 3);
        itemWeights[i]+= VEL_WEIGHT * mathVecInner(myVel, diff,3);
        float enDiff[3];
        mathVecSubtract(enDiff, loc, enPos, 3);
        itemWeights[i]+= EN_VEL_WEIGHT * mathVecInner(enVel, diff,3);
        int type = game.getItemType(i);
        itemWeights[i]+= LARGE_ITEM_WEIGHT*(type==ITEM_TYPE_LARGE);
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
//End page main

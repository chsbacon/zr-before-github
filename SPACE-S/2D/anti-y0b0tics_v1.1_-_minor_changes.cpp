//Begin page main
float myState[12];
float enState[12];
// State variables
#define myPos (&myState[0])
#define myVel (&myState[3])
#define myAtt (&myState[6])
#define myRot (&myState[9])
#define enPos (&enState[0])
#define enVel (&enState[3])
#define enAtt (&enState[6])
#define enRot (&enState[9])

int time;
#define ROUND_LENGTH 180

// Zone variables
float myZone[4];
float newZoneInfo[4];
int numZoneDataPoints;
float zoneShift[3];

int targetItem;

float pickUpDist;

// Score variables
float myScore;
float enScore;
float myDeltaScore;
float enDeltaScore;
bool newItemInZone;

// Item variables
bool carryingItem;
bool pickNewTargetItem;

#define PRINTVEC(vec) DEBUG(("%f %f %f", vec[0], vec[1], vec[2]))

float zeroVec[3];

void init(){
    #define SPEEDCONST .4
    api.setPosGains(SPEEDCONST,0.1,7 * SPEEDCONST);
    carryingItem=false;
	pickNewTargetItem=true;
	newItemInZone=false;
	memset(zeroVec, 0.0f, 12);
	memset(myZone, 0.0f, 12);
	numZoneDataPoints=0;
	game.dropSPS();game.dropSPS();game.dropSPS();
}

void loop(){
    api.getMyZRState(myState);
    api.getOtherZRState(enState);
    

    // newItemInZone = game.getScore()-myScore > myDeltaScore;
    // Getting the average score
    myDeltaScore = game.getScore()-myScore;
    myScore = game.getScore();
    enDeltaScore = game.getOtherScore()-enScore;
    enScore = game.getOtherScore();

    time = game.getCurrentTime();
    
    game.getZone(newZoneInfo);
    PRINTVEC(newZoneInfo);
    //throw out garbage values
    // Finding the zone through averaging points
    if(mathVecMagnitude(newZoneInfo,3)<5.0f) {
        //calculate new average
        //it would be nice to make this more accurate
        // Transferring data
        memcpy(zoneShift, myZone, 12);
        scale(myZone, numZoneDataPoints);
        mathVecAdd(myZone, myZone, newZoneInfo, 3);
        // Averaging points and scaling
        scale(myZone, 1.0f/(++numZoneDataPoints)); // ++x adds one and returns new value
        mathVecSubtract(zoneShift, zoneShift, myZone,3);//x++ returns then adds
        PRINTVEC(myZone);
        DEBUG(("zoneShift: %f", mathVecMagnitude(zoneShift,3)));
    }
    //DEBUG(("%d zone: %f %f %f %f", numZoneDataPoints, myZone[0],myZone[1],myZone[2],myZone[3]));
    // game.getZone(myZone);
    if (pickNewTargetItem)
            targetItem = bestItem();
    // If getting a certain amount of points, protect out zone to keep getting points        
    if(myDeltaScore>=0.45) {
        DEBUG(("protecting zone"));
        float defendPoint[3];
        // Move to the zone, get between zone and enemy
        mathVecSubtract(defendPoint, enPos, myZone, 3);
        scale(defendPoint, 0.15/mathVecMagnitude(defendPoint,3));
        mathVecAdd(defendPoint, defendPoint, myZone,3);
        api.setPositionTarget(defendPoint);
        
    }
    // If there is a close item, go get it
    else if (targetItem != -1){
        float itemLoc[3];
        game.getItemLoc(itemLoc, targetItem);
        pickNewTargetItem=false;
        // Grab it
        if(!carryingItem) {
            if(getItem(targetItem)) {
                DEBUG(("picked up item"));
                carryingItem = true;
            }
        }
        
        else {
            // Getting distance to zone
            DEBUG(("taking the item back to the zone"));
            float toZone[3];
            mathVecSubtract(toZone, myZone, myPos, 3);
            float distToZone = mathVecNormalize(toZone,3);
            DEBUG(("angle to zone:%f",angle(myAtt,toZone)));
            // Getting to zone
            api.setAttitudeTarget(toZone);
            /*scale(toZone, distToZone - pickUpDist);
            // toZone becomes our position target (not actually vec to zone)
            mathVecAdd(toZone, toZone, myPos,3);
            api.setPositionTarget(toZone);*/
            // we go to the point between us and the center of the zone
            // that is pickUpDist from the center
            
            // Alternative point based on attitude
            float zoneDropOffPoint[3];
            memcpy(zoneDropOffPoint, myAtt, 12);
            scale(zoneDropOffPoint, -pickUpDist);
            mathVecAdd(zoneDropOffPoint, zoneDropOffPoint, myZone,3);
            api.setPositionTarget(zoneDropOffPoint);
            
            float heldItem[3]; //position of item
            memcpy(heldItem, itemLoc, 12);
            // memcpy(heldItem, myAtt, 12);
            // scale(heldItem, pickUpDist);
            
            
            /*float error[3];
            mathVecSubtract(error, itemLoc, heldItem, 3);
            DEBUG(("item loc error: %f", mathVecMagnitude(error, 3)));
            PRINTVEC(itemLoc);*/
            
            // mathVecAdd(heldItem, heldItem, myPos,3);
            DEBUG(("held item dist to zone: %f", dist(heldItem, myZone)));
            if (dist(heldItem, myZone) < 0.06
            && mathVecMagnitude(zoneShift,3)<0.1) {
                // Put down the item  
                game.dropItem();
                carryingItem=false;
                pickNewTargetItem=true;
            }
        }
    }
}

bool getItem(int id) {
    //DEBUG(("getting %d", id));
    // Finding the vector to the item
    float toItem[3], itemLoc[3], targetPos[3];
    float itemPickupDist = itemDockDistOuter(id);
    game.getItemLoc(itemLoc, id);
    mathVecSubtract(toItem, itemLoc, myPos, 3);
    float itemDist = mathVecNormalize(toItem, 3);
    api.setAttitudeTarget(toItem);
    // Going to the item
    scale(toItem, itemDist-itemPickupDist+0.01f);
    DEBUG(("dist:%f (%f) speed:%f angle:%f", itemDist, itemDist-itemPickupDist+0.01f, mathVecMagnitude(myVel,3),angle(myAtt,toItem)));
    mathVecAdd(targetPos, toItem, myPos, 3);
    api.setPositionTarget(targetPos);
    //DEBUG(("getItem target:"));
    // Picking up the item 
    if(mathVecMagnitude(myVel,3)<0.01f && itemDist<itemPickupDist && angle(myAtt,toItem)<0.25f) {
        DEBUG(("attmempting to dock"));
        //try to correct for jump when dropping items
        pickUpDist = itemDist - 0.04f;
        return game.dockItem();
    }
    return false;
}

// Calculating the best point:distance ratio
int bestItem() {
    float maxPoints = 0;
    int bestItem = -1;
    for (int i=0; i<NUM_ITEMS; i++) {
        float loc[3];
        game.getItemLoc(loc, i);
        float points = 0;
        points+=(1/dist(myPos,loc));
        if (points>5) points=5;
        int type = game.getItemType(i);
        //points += 5*(type==ITEM_TYPE_LARGE);
        points += -10 * (dist(loc, myZone)<0.1);
    //we need a good way to determine if something is actually in our zone
        if (points>maxPoints) {
            maxPoints=points;
            bestItem=i;
        }
    }
    return bestItem;
}

float itemDockDistOuter(int id) {
    int itemType = game.getItemType(id);
    return (itemType==ITEM_TYPE_LARGE) ? 
            0.173f :(itemType==ITEM_TYPE_MEDIUM ? 0.16f : 0.146f);
}

// Scalar!!!
void scale (float* vec, float scale) {
    for (int i=0; i<3; i++) {
        vec[i] *= scale;
        // a = a*b
        // a *= b
    }
}

// Finding the resultant
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

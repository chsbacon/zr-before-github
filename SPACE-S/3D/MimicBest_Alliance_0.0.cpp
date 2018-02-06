//Begin page main
//to fix:
//More stable guarding e.g. not getting forced out


//State variables
float myState[12];
float enState[12];
// macros that point to parts of state array
// ( "&" returns the pointer)
#define myPos (&myState[0])
#define myVel (&myState[3])
#define myAtt (&myState[6])
#define myRot (&myState[9])
#define enPos (&enState[0])
#define enVel (&enState[3])
#define enAtt (&enState[6])
#define enRot (&enState[9])

#define ROUND_LENGTH 180

float myZone[4];
float enZone[3];

int targetItem; // if we're carrying an item, targetItem is that one

float myScore;
float enScore;
float myDeltaScore;
float enDeltaScore;
float positionTarget[3];
float itemPositions[9][12];
bool carryingItem;
#define PRINTVEC(str, vec) DEBUG(("%s %f %f %f", str, vec[0], vec[1], vec[2]))
//Above allows printing a vector to debug w/o large code size

float pickUpDist;
//                                   large            small    medium
#define DOCK_OUTER_DIST(id) ( id<2 ? 0.173f : (id>3 ? 0.146f : 0.16f))
#define DOCK_INNER_DIST(id) ( id<2 ? 0.151f : (id>3 ? 0.124f : 0.138f))

float zeroVec[3];
int currentTime;
float SPSLocs[2][3];
// float twoToThree[3];
float vcoef;
bool justPickedUp;
bool enAttacking;
float itemLoc[3];  
float themToUs[3]; // them to our zone


void init(){
    // adjust positionTarget PID gains
    DEBUG(("BACON 2.0.6"));
    #define SPEEDCONST .45f//.45 or .55 is 37 secs
    #define DERIVCONST 2.8f//2.8 or 3.3 is 37 secs
    api.setPosGains(SPEEDCONST,0.1f,DERIVCONST);
    //#define ATTCONST .4
    //api.setAttGains(ATTCONST,.1,8*ATTCONST);
    
    api.getMyZRState(myState);
    
    SPSLocs[0][0] = 0.7f;
    SPSLocs[0][1] = 0.f;
    SPSLocs[0][2] = 0.25f;
    
    SPSLocs[1][0] = 0.3f;   
    SPSLocs[1][1] = -0.7f;
    SPSLocs[1][2] = -0.15f;
    
    // twoToThree[0] = 0.0f;
    // twoToThree[1] = -1.0f;
    // twoToThree[2] = 0.0f;
    
    if (myPos[1]<0) {
        scale(SPSLocs[0], -1.0f);
        scale(SPSLocs[1], -1.0f);
        // scale(twoToThree, -1.0f);
    }
    
    //carryingItem=false;
	memset(zeroVec, 0.0f, 12);
	//myScore = 0.0f;
	//enScore = 0.0f;
	game.dropSPS();
	vcoef=2.2f; //2.5
	//justPickedUp = false;
	enAttacking=false;
	DEBUG(("Submitted 10/30/16"));
}

void loop(){
    for (int i=0;i<9;i++){
        game.getItemLoc(itemPositions[i],i);
    }
    justPickedUp = false;
    currentTime=api.getTime();
    myDeltaScore = (myScore<.01f)?0.0f:(game.getScore()-myScore);
    enDeltaScore = game.getOtherScore()-enScore;
    //Finds current score increase
    myScore = game.getScore();
    enScore = game.getOtherScore();
	api.getMyZRState(myState);
    api.getOtherZRState(enState);
    if (game.hasItem(targetItem)!=1) targetItem = bestItem();
    //refresh bestItem if we're not carrying an item
    
  
    game.getItemLoc(itemLoc, targetItem);
    mathVecSubtract(themToUs,myZone,enPos,3);
    int numSPSHeld = game.getNumSPSHeld();
    
    //Placing our SPS
    if (numSPSHeld>0) {
        // Place SPS
        if (currentTime>45 || dist(myPos, SPSLocs[2-numSPSHeld]) < 0.25f
        || (getItem(myPos[1]<0?1:0) && !carryingItem)) {
            //||(dist(myPos, zeroVec) < 0.5f && dist(myPos, SPSLocs[3-numSPSHeld]) > 0.05f);
            game.dropSPS();
            DEBUG(("DROPPEDHERE"));
            
        }
        if (carryingItem){
            memcpy(positionTarget, SPSLocs[2-numSPSHeld], 12);
        }
    }
    else {
        game.getZone(myZone);
        if ((dist(myZone, enPos) > .75f) or carryingItem or myDeltaScore<.01f)
            enAttacking = false;
        if(myDeltaScore>=0.39f || enAttacking
        || ( (dist(myZone,enPos)<.5f || angle(themToUs,enVel)<.7f)
        && !carryingItem && dist(myPos,myZone)<dist(enPos,myZone)+.25f
        && mathVecMagnitude(enVel,3)>.01f && willWin())) {
            //If score being gained is sufficiently high, will defend zone
            //DEBUG(("protecting zone"));
            enAttacking = true;
            guard(myZone, 0.04f);
            //memcpy(positionTarget,myZone,12);
            DEBUG(("GUARD"));
        }
        else if (dist(itemLoc,myZone)>.4f and enDeltaScore<.01f and myDeltaScore>.01f and !carryingItem){
            memcpy(positionTarget,myZone,12);
            DEBUG(("Your move"));
        }
        else if (/*targetItem != -1 and */  !carryingItem && game.getFuelRemaining()>3){

            getItem(targetItem);

        }
        else if (carryingItem){
            float toZone[3];
            mathVecSubtract(toZone, myZone, myPos, 3);
            float distToZone = mathVecNormalize(toZone,3);
            api.setAttitudeTarget(toZone);
            scale(toZone, distToZone - dist(myPos, itemPositions[targetItem]));
            mathVecAdd(toZone, toZone, myPos, 3);
            memcpy(positionTarget, toZone, 12);
            mathVecSubtract(toZone, myZone, itemPositions[targetItem], 3);
            PRINTVEC("itemLoc", itemPositions[targetItem]);
            if (dist(itemPositions[targetItem], myZone) < 0.03f
            || (game.getFuelRemaining()<1 && angle(myVel,toZone) > 1.52f)
            /*or (dist(itemLoc,enZone)>0.1f && dist(myPos,enZone)<.35f
            && enDeltaScore>0.0f && !justPickedUp && currentTime>60)*/) {
                //Drops item in zone when close enough
                game.dropItem();
                carryingItem=false;
                vcoef = 0.62f;
                //api.setPosGains(SPEEDCONST*.85,0.1,7 * SPEEDCONST*.85);
            }
        }
        else {
            api.setVelocityTarget(zeroVec);
        }
        
    }
    carryingItem = (game.hasItem(targetItem)==1);
    
    #define destination positionTarget
    float fvector[3],distance,flocal;
    float accel = .0175f;
    mathVecSubtract(fvector, destination, myPos, 3);
    distance = mathVecNormalize(fvector, 3);
    if (distance > 0.05f) {
        flocal = vcoef*0.07f;
        //flocal = 10.f;
        //flocal=velocity+accel*6.f;
        //if ( ( fabs(flocal - velocity) > 0.008f)){
            //DEBUG(("VELOCITY CONTROL : vel = %f  des_vel = %f", velocity, flocal));
            if (flocal*flocal/accel>distance-.02f){
                flocal = sqrtf(distance*accel)-.02f;
                //DEBUG(("Slower"));
            }
            scale(fvector, flocal);
            api.setVelocityTarget(fvector);
            PRINTVEC("positionTarget", positionTarget);
        //}
    }
    else{
        api.setPositionTarget(destination);
    }
    
}
bool willWin(){
    return (myScore+(180-currentTime)*myDeltaScore>enScore+(180-currentTime)*enDeltaScore);
}
bool getItem(int id) {
    //DEBUG(("getting %d", id));
    float toItem[3],itemDist;
    mathVecSubtract(toItem,itemPositions[id],myPos,3);
    itemDist=mathVecMagnitude(toItem,3);
    float targetPos[3];
    float itemDirectionScalable[3];
    memcpy(itemDirectionScalable,&itemPositions[id][6],12);
    float itemPickupDist = DOCK_INNER_DIST(id)+0.011f;
    scale(itemDirectionScalable,itemPickupDist);
    mathVecAdd(targetPos,itemDirectionScalable,itemPositions[id],3);
    memcpy(positionTarget, targetPos, 12);
    scale(itemDirectionScalable,-1.f);
    api.setAttitudeTarget(itemDirectionScalable);
    if(mathVecMagnitude(myVel,3)<0.01f && itemDist<DOCK_OUTER_DIST(id)
    && itemDist>DOCK_INNER_DIST(id) && angle(myAtt,toItem)<0.25f
    && game.hasItem(id)==0) {
        //DEBUG(("attempting to dock"));
        carryingItem = game.dockItem(id);
        pickUpDist = itemDist;
        //game.getItemLoc(itemLoc, targetItem);
        justPickedUp = true;
        return carryingItem;
    }
    
    return false;
}


int bestItem() {
    float maxPoints = -1000000;
    int bestItem = -1;
    
    
    for (int i=0; i<NUM_ITEMS; i++) {

        double points = 0;
        points+=(1/dist(myPos,itemPositions[i]));
        if (points>5) points=5;
        bool large = game.getItemType(i)==ITEM_TYPE_LARGE;
        points += 50*(large)
        + 75*(dist(itemPositions[i],enZone)<.13f/* and dist(myPos,enZone)<.35f*/)
        + 250*(dist(myPos,myZone)<.4f and dist(itemPositions[i],myZone)<.4f
            and (myScore>enScore or angle(themToUs,enVel)<.7f
            or dist(myZone,enPos)<.5f))
            //This is changed so that if they have a slight timing advantage
            //we will go for a steal, as opposed to just mantaining
            //their advantage.
        + -1000*(game.hasItem(i)==2 && (!large || currentTime>120))
        //+ 1000 * (dist(loc,myZone)<.4f && dist(myPos,myZone)<.4f)
        + -10000 * (dist(itemPositions[i], myZone)<0.06f);
        if (points>maxPoints) {
            maxPoints=points;
            bestItem=i;
        }
        //DEBUG(("Points %f Best: %i", points, i));
    }
    
    
    return bestItem;
}

// takes 2 paramters:
// pos, a 3d vector of a position to guard
// dist, the guarding radius
void guard(float pos[3], float dist) {
    float defendPoint[3];
    mathVecSubtract(defendPoint, enPos, pos, 3);
    scale(defendPoint, dist/mathVecMagnitude(defendPoint,3));
    mathVecAdd(defendPoint, defendPoint, pos, 3);
    memcpy(positionTarget, defendPoint, 12);
    vcoef = .7f;
}


/*
        Vector-Related Functions
*/
// scalar mutliplication.
// modifies vector argument
void scale (float* vec, float scale) {
    for (int i=0; i<3; i++) {
        vec[i] *= scale;
    }
}

// magnitude of difference between two vectors.
// with position vectors, it is the distance between the points
float dist(float* vec1, float* vec2) {
    float diff[3];
    mathVecSubtract(diff, vec1, vec2, 3);
    return mathVecMagnitude(diff,3);
}

// use dot product to compute the angle between 
// two vectors in radians
float angle(float* vec1, float* vec2) {
    return acosf(mathVecInner(vec1,vec2,3)/(mathVecMagnitude(vec1,3)*mathVecMagnitude(vec2,3)));
}

//End page main

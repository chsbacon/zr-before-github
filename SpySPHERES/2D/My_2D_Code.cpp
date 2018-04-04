//Begin page main
#include <ZRGame.h>
float myState[12];
float* myPos;
float* myVel;
float* myAtt;
float enState[12];
float* enemyPos;
float* enemyVel;
float* enemyAtt;
float timeToSwitch;
float differenceVector[3];
float safetyZone;
float energy;
float packDistances[4];
float itemLoc[3];
float darkZoneCoordinate;
float lowestDistance[2];
float target[3];
short targetMirror;
float mirror1[3];
float mirror2[3];
void init(){
    api.getMyZRState(myState);
    darkZoneCoordinate = -.1;
    safetyZone = .5;
    myPos=myState;
    enemyPos=enState;
    myVel=myState+3;
    enemyVel=enState+3;
    myAtt=myState+6;
    enemyAtt=enState+6;
    api.setPosGains(.10,0.,.94);
    for (float i=1.;i<3.;i++){
        mirror1[int(i)]=mirror2[int(i)]=(i-1)*.7-.7;
    }
    mirror1[0]=.6;
    mirror2[0]=-.6;
    //unless there is a good reason for the above code we should use this:
    //game.getItemLoc(mirror1, 7);
    //game.getItemLoc(mirror2, 8)
    if (distanceBetween(myPos,mirror1)<distanceBetween(myPos,mirror2)){
        targetMirror = 7;
    }
    else{
        targetMirror = 8;
    }
}

void loop(){
    update();
    //energy update has been moved to update()
    /**api.getMyZRState(state);
    for (int i=0;i<3;i++){
        myPos[i] = state[i];
        myVel[i] = state[i+3];
    }
    api.getOtherZRState(enState);
    for (int i=0;i<3;i++){
        enemyPos[i] = enState[i];
    }**/
    if (game.getLightSwitchTime() == 1){
        darkZoneCoordinate *= -1;
    }
    ///Picture Pointing And Taking
	if ((game.posInLight(enemyPos) or (fabsf(enemyPos[1]) < safetyZone) or (-1<game.getLightSwitchTime() and game.getLightSwitchTime()<10)) and energy > 2.5){
	    differenceVector[0] = enemyPos[0]-myPos[0];
	    differenceVector[1] = enemyPos[1]-myPos[1];
	    differenceVector[2] = enemyPos[2]-myPos[2];
	    api.setAttitudeTarget(differenceVector);
	    DEBUG(("TurningToFace"));
	}
	if ((game.getMemoryFilled() == 2 and (energy>2. or game.posInLight(myPos))) or game.getMemoryFilled() > 0 and game.getCurrentTime() > 178){
	    game.uploadPics();
	}
	else if (game.isFacingOther() and distanceBetween(myPos,enemyPos)>.5 and (energy>1.6 or game.posInLight(myPos))){
	    if (game.getPicPoints()>0.){
	        game.takePic();
	    }
	}
	
	///Using the Mirror
	mathVecSubtract(target,myPos,enemyPos,3);
	if ((not game.posInDark(myPos)) and angleBetween(enemyAtt,target)<.45 and fabsf(myPos[1])>.07){
	    game.useMirror();
    }
	///State Machine Movement
	if (game.hasItem(targetMirror) == -1){
	    game.getItemLoc(target,targetMirror);
	    api.setPositionTarget(target);
	}
	else if (game.getCurrentTime() > 165 and game.getScore() > .5 + game.getOtherScore() and game.posInLight(myPos)){
	    api.setPositionTarget(enemyPos);
	}
	else if (energy < .35 and not game.posInLight(myPos)){
	    target[2]=target[0]=0.;
	    target[1] = darkZoneCoordinate*-.15;
        api.setVelocityTarget(target);
	}
	else if (game.posInLight(myPos) and energy > 3){
	    target[2]=target[0]=0;
	    target[1] = darkZoneCoordinate*.3;
	    api.setVelocityTarget(target);
	}
	else if ((energy < 3.7 or game.getCurrentTime()<30)){
	    for (int i=0;i<4;i++){
	        game.getItemLoc(itemLoc,i);
	        if (game.hasItem(i) == -1 and game.posInDark(itemLoc)){
	            packDistances[i] = .01;
	        }
	        else if (game.hasItem(i) == -1){

	            packDistances[i] = distanceBetween(myPos,itemLoc);
	        }
	        else {
	            packDistances[i] = 100.;
	        }
	    }
	    lowestDistance[0] = -1.;
	    lowestDistance[1] = 100.;
        for (int i=0;i<4;i++){
            if (packDistances[i] < lowestDistance[1]){
                lowestDistance[1] = packDistances[i];
                lowestDistance[0] = i;
            }
        }
        ///Plan to add scaling comparison in here (as in "if can reach pack before switch or zero energy, do so")
        if (lowestDistance[1] < 1.75){
            game.getItemLoc(itemLoc,lowestDistance[0]);
            api.setPositionTarget(itemLoc);
        }
        /**else if (-1<game.getLightSwitchTime() and game.getLightSwitchTime()<10){
            target[0] = game.getDarkGreyBoundary()*2.;
            api.setPositionTarget(target);
        }**/
        else{
            target[2]=target[0]=0.;
            target[1] = darkZoneCoordinate*-.15;
            api.setVelocityTarget(target);
        }
        
	}
	else if (not game.posInDark(myPos) and energy > 3.5){
	    target[2]=target[0]=0;
	    target[1] = darkZoneCoordinate*.3;
	    api.setVelocityTarget(target);
	}
	else if (energy > 3 and (-1<game.getLightSwitchTime() and game.getLightSwitchTime()<15) and game.posInDark(myPos)){
	    target[2]=target[0]=0;
	    target[1] = darkZoneCoordinate*myPos[1] * 15./float(game.getLightSwitchTime());
	    api.setVelocityTarget(target);
	}
	else if (distanceBetween(myPos,enemyPos)<.6 and not game.posInDark(enemyPos)){
	    for (int i=0;i<3;i++){
	        target[i] = enemyPos[i] + (myPos[i]-enemyPos[i])/distanceBetween(myPos,enemyPos)*.65;
	    }
	    api.setPositionTarget(target);
	}
	else{
	    target[2]=target[1]=target[0]=0;
	    api.setVelocityTarget(target);
	}
}


float distanceBetween(float pos1[3],float pos2[3]){
    return (powf((powf((pos1[0]-pos2[0]),2)+powf((pos1[1]-pos2[1]),2)+powf((pos1[2]-pos2[2]),2)),.5));
    //alternately there's the readable code below:
    //float distance[3];
    //mathVecSubtract(distance,pos1,pos2,3);
    //return mathVecMagnitude(distance,3);
}

float angleBetween(float vector1[3],float vector2[3]){
    mathVecNormalize(vector1,3);
    mathVecNormalize(vector2,3);
    return (acosf(vector1[0]*vector2[0]+vector1[1]*vector2[1]+vector2[2]*vector2[2]));
    //because we don't need the exact angle, just comparing the dot product without the arccos should be fine
}

inline void update(){
    api.getMyZRState(myState);
	api.getOtherZRState(enState);
	energy = game.getEnergy();
}

//End page main

//Begin page main
//Declare any variables shared between functions here
float myState[12];
float enState[12];
float myZone[4];
#define myPos (&myState[0])
#define myVel (&myState[3])
#define myAtt (&myState[6])
#define myRot (&myState[9])
#define enPos (&enState[0])
#define enVel (&enState[3])
#define enAtt (&enState[6])
#define enRot (&enState[9])
float enScoreChange;
float lastEnScore;
void init(){
	//This function is called once when your code is first loaded.

	//IMPORTANT: make sure to set any variables that need an initial value.
	//Do not assume variables will be set to 0 automatically!
}

void loop(){
	enScoreChange = game.getOtherScore()-lastEnScore;
	lastEnScore = game.getOtherScore();
	api.getMyZRState(myState);
	api.getOtherZRState(enState);
}

bool* possItemsInEnZone(){
    float enZone[4];
    float zeroVec[3];
    zeroVec[0] = 0;
    zeroVec[1] = 0;
    zeroVec[2] = 0;
    mathVecSubtract(enZone, zeroVec, myZone, 3);
    bool itemsIn[6];
    for(int i = 0; i < 6; i++){
        float testItemPos[3];
        game.getItemLoc(testItemPos, i);
        //float distanceVec[3];
        mathVecSubtract(distanceVec, testItemPos, enZone, 3);
        if(mathVecMagnitude(distanceVec, 3) < enZone[3]){
            itemsIn[i] = true;
        }
        else{
            itemsIn[i] = false;
        }
    }
    return itemsIn;
    
    
}
int itemsInEnZone(){
    int currentAnswer = -1;
    bool* possItemsList;
    possItemsList = possItemsInEnZone();
    for(int possStates = 0; possStates < 64;possStates++){
        bool works = true;
        float sum = 0.0;
        for(int i = 0; i < 6; i++){
            if((2**i) & possStates > 0){
                if(possItemsList[i] == false){
                   works = false;
                }
                else{
                    if(i<2){
                        sum += 0.2;
                    }
                    else if(i<4){
                        sum += 0.15;
                    }
                    else{
                        sum+= 0.1;
                    }
                }
            }
        }
        if(sum != enScoreChange){
            works = false;
        }
        if((works = true) && (possStates > currentAnswer)){
            currentAnswer = possStates;
        }
    }
    return currentAnswer;

}
//End page main

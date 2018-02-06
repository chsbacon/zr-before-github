//Begin page main
#define  DEBUG_MESSAGES
//#define  DECISION_MESSAGES
#define UPLOAD_BUG_01
//#define  EXTRA_CONDITIONS


//ZANNEIO VERSION A:003
//Guitar Hero!.
//Move to top and get outer pictures
//Take pictures and upload
//If flare hits go to shadow immediately!
//Stabilize when the satellite goes out of bounds or when it is in the shadow zone.

//Declare any variables shared between functions here
unsigned char   step,TIME, photos[3], old_step, upload, ids[3], sd_counter;
char        t,flares;
bool        i0;
float       tol, velocity;
float       dest[3], cpos[3], cvel[3], catt[3], R, Rd, 
            *poiG, poiA[3], poiB[3], *pois[2];


void init(){
	//This function is called once when your code is first loaded.

	//IMPORTANT: make sure to set any variables that need an initial value.
	//Do not assume variables will be set to 0 automatically!
	
	step = 0;
	old_step = 0;
	TIME = -1;
    upload = 0;
	tol = 0.05f;
	t=20;
	flares = 0;
	sd_counter = 0;
}

void loop(){
	//This function is called once per second.  Use it to control the satellite.
	float   poi_t[3], O[3], npos[3], temp1[3];
	float   State[12], opos[3], ovel[3], v[3];
	float   d,od,ovelocity,L,theta_tol,angle,angle2;
	unsigned char   time_to_top[2], time_to_POI[2];
	
	unsigned char   TS, ms, MEM, i, flare_step;
	char            tf, ret;
	bool            outside_zone;
	
	
	TIME++;
	api.getMyZRState(State);
	cpos[0] = State[0];
	cpos[1] = State[1];
	cpos[2] = State[2];
	cvel[0] = State[3];
	cvel[1] = State[4];
	cvel[2] = State[5];
	catt[0] = State[6];
	catt[1] = State[7];
	catt[2] = State[8];

	
	api.getOtherZRState(State);
	opos[0] = State[0];
	opos[1] = State[1];
	opos[2] = State[2];
	//ovel[0] = State[3];
	//ovel[1] = State[4];
	//ovel[2] = State[5];
	
	
	//d = mathVecMagnitude(cpos,3);
	velocity = mathVecMagnitude(cvel,3);
	//od = mathVecNormalize(opos,3);
	//ovelocity = mathVecMagnitude(ovel,3);
	tf = game.getNextFlare();
	if (tf<0)
	    tf = 31;
	ms = game.getMemoryFilled();
	MEM =  game.getMemorySize();
	
	npos[0] = cpos[0];npos[1] = cpos[1];npos[2] = cpos[2];
	d=mathVecNormalize(npos,3);
	
	
	
	

	//Time needed to reach shadow zone
	TS=22;
	if (cpos[0]>0.35)
	    TS=30;
	
	//New idea...
	//If two flares have already occured, then you can sacrifice a memory slot...
	flare_step = 10;  //shadow zone
	/*if ((flares>=2)&&(TIME+tf<200)&&(tf<=30)&&(MEM>1)){
    	    TS = 26;
    	    if ((upload==1)||((ms>0)&&(tf<=TS)))
    	        flare_step = 8;//upload
    	    else
    	        flare_step = 2;//do nothing
	}
    */
	outside_zone = (cpos[1]*cpos[1]+cpos[2]*cpos[2]>0.04f)||(cpos[0]<0);
	if (tf <= 30){
        #ifdef DEBUG_MESSAGES
        if ( (tf%10==0) )
            DEBUG(("Next flare in %d secs:  ", tf));
        #endif
        if (tf<=TS){
            if ( ((step!=2)||(old_step!=flare_step))&&(step!=12) ){
                step = flare_step;
            }
        }
        /*if (tf==0){
            flares++;
            if (ms==0)
                upload=0;
        }*/
    }
    
    //new stuff   
	if ((tf<=8)&&(old_step==10)&&(!outside_zone)&&(velocity>0.01))
    	  step=6;
    	        
	angle = game.getFuelRemaining();
	DEBUG(("TIME: %d FUEL: %f.\n", TIME, angle));
	

	
	if (old_step!=10){
    	if ((TIME>216)&&(ms>0)&&(step!=8))
    	    step = 8;
    	if (TIME%60<=1){
    	    if (old_step<6)
    	        step = 0;
    	}
	}
	
	if ((angle<=14)){//If fuel running low,  go to Shadow Zone or upload and turn off
        step = flare_step;
    }
	
	
	//If we are not inside the shadow zone and a flare closes  3% code
	//simple
	if ((tf<2)&&(flare_step==10))
        if (outside_zone)
            step = 12;
	/* //for flare steps
    if ( ((tf<2)&&(flare_step==10))||((tf<5)&&(flare_step!=10)) )
        if ((cpos[1]*cpos[1]+cpos[2]*cpos[2]>0.04f)||(cpos[0]<0))
            step = 12;
    */
	
	if (TIME%60==0){
	    photos[0]=0;
	    photos[1]=0;
	    photos[2]=0;
	}
	
	//if (TIME<=1){
        //2% codesize---------------------------------
        ids[1] = 2;
        
        //Uncomment this at final version (both colors)
        if (cpos[1]>=0){
            ids[0]=0;
            ids[2]=1;
        }
        else{
            ids[0]=1;
            ids[2]=0;
        }
        //-----------------------------------------------
	//}

    game.getPOILoc(poiA, ids[0]);
    game.getPOILoc(poiB, ids[1]);
    pois[0] = poiA;
    pois[1] = poiB;
    
    
    
    
    
    
	
    //GO to outer zone ONLY!
    L = 2.3f;  
    R = 0.53f;
    Rd = 0.42f;
    theta_tol = 0.95f;  //cos of 0.4f is smaller than 0.93

	switch(step){
	    
	    //MODE 00: Set POI destination
	    case 0:
	        old_step = 0;

            //HEAVY CODE HERE!!! MORE than 10% in codesize
            
            //original
    	    //angle2 = sqrtf(poiA[0]*poiA[0] + poiA[2]*poiA[2]);
    	    //reduced code
    	    angle2 = sqrtf(0.04f - poiA[1]*poiA[1]);
            time_to_top[0] = (unsigned char) (acosf(-poiA[2]/angle2)*10);//time to top

            //original
            //angle2 = sqrtf(poiB[0]*poiB[0] + poiB[2]*poiB[2]);
            //time_to_top[0] = (unsigned char) (acosf(-poiB[2]/angle2)*10);//time to top
            //reduced code (assuming y=0, then angle2=0.2f)
            time_to_top[1] = (unsigned char) (acosf(-poiB[2]/0.2f)*10);//time to top
            
            //original
            //v[0] = 0; v[1] = poiA[1]; v[2] = -sqrtf(0.04f - poiA[1]*poiA[1]);
            //mathVecNormalize(v,3);
            //time_to_POI[0] = 0; //(unsigned char) (acosf(mathVecInner(v,npos,3))*10 + fabs(d-0.46f)*40 + 6);
            //reduced code
            ret = (unsigned char)(fabs(d-0.46f)*50);
            time_to_POI[0] = (unsigned char) (acosf(5*(poiA[1]*npos[1]-angle2*npos[2]))*14) 
                                + ret;
            
            //original
            //v[0] = 0; v[1] = poiB[1]; v[2] = -sqrtf(0.04 - poiB[1]*poiB[1]);
            //mathVecNormalize(v,3);
            //time_to_POI[1] = 0;//(unsigned char) (acosf(mathVecInner(v,npos,3))*10 + fabs(d-0.46f)*40 + 6);
            //reduced code (assuming y=0, then angle2=0.2f)
            time_to_POI[1] = (unsigned char) ( acosf(-npos[2])*14   ) 
                                + ret;
            
            DEBUG(("A. Time to top for POI %d %d, time to reach %d\n", ids[0], time_to_top[0], time_to_POI[0]));
            DEBUG(("B. Time to top for POI %d %d, time to reach %d\n", ids[1], time_to_top[1], time_to_POI[1]));
        
            //CHOOSE best suited POI
            if (time_to_top[0] < time_to_top[1]){
                if ((time_to_top[0] < time_to_POI[0])&&(time_to_top[1] >= time_to_POI[1])){
                    i0 = 1;
                }
                else{
                    i0=0;
                }
            }
            else{
                if ((time_to_top[1] < time_to_POI[1])&&(time_to_top[0] >= time_to_POI[0])){
                    i0 = 0;
                }
                else{
                    i0 = 1;
                }
            }
            
            
            if (photos[i0]>0){
                DEBUG(("SWAP POINTS\n"));
                i0 = !i0;
            }
            
            t = time_to_top[i0];
            poiG =  pois[i0];
            
            
            //New STUFF
            /*ret = t - time_to_POI[i0] - 4;
            if (ret>0){
                if (ret>10)
                    ret = 10;
                t = t - (ret>>1);
            }*/
            /*if (time_to_POI[i0] > t)
                t = time_to_POI[i0] + 15 - (time_to_POI[i0]-t)/2;*/
            
            
            #ifdef DEBUG_MESSAGES
    	    DEBUG(("TIME: %d. Choosing POI: %d... Reaching after %d secs\n ",TIME, ids[i0], t));
    	    #endif    

    	    poi_predict(poi_t, poiG, t);
        	dest[0]=L*poi_t[0];dest[1]=L*poi_t[1];dest[2]=L*poi_t[2];
    	    
            //TEST CODE
        	//dest[0]=L*poi_t[0];dest[1]=L*poi_t[1];dest[2]=L*poi_t[2];
        	//DEBUG(("TIME: %d. POI now: %f %f %f", TIME, poiG[0], poiG[1], poiG[2]));
        	//DEBUG(("TIME: %d. POI after %d secs: %f %f %f", TIME, t, poi_t[0], poi_t[1], poi_t[2]));
        	//DEBUG(("Set Destination: %f %f %f", dest[0], dest[1], dest[2]));
        	step = 2;
        	t++;
        	goto MOVE;
	    break;
	    
	    
	    //MODE 02: Moving and stuff
    	case 2:
    	    MOVE:
    	    #ifdef DEBUG_MESSAGES
    	    DEBUG(("TIME: %d. Moving to.. old_step=%d, upload = %d.  \n",TIME,old_step,upload));
    	    #endif
    	    
    	    if ((t>7)&&(upload==0)&&(TIME%60<55)){
                game.takePic(ids[i0]);
                #ifdef DEBUG_MESSAGES
    	        DEBUG(("TIME: %d. Dummy Picture.  \n",TIME));
    	        #endif
                /*if (game.getMemoryFilled()>ms){
                    photos[i0]+=1;
                    if (ms+1 == MEM)
                        upload = 1;
                }*/
    	    }
    	    
    	    if ((upload!=1)&&(old_step==0)){
    	        t--;
    	        #ifdef DEBUG_MESSAGES
    	        DEBUG(("TIME: %d. POI reaching TOP in %d secs...  \n",TIME, t));
    	        #endif
    	        if (t<0){
    	            step = 0;
    	            #ifdef DEBUG_MESSAGES
    	            DEBUG(("TIME: %d. POI lost :(  ....  \n",TIME));
    	            #endif
    	        }
    	    }

    	    temp1[0] = dest[0];temp1[1] = dest[1];temp1[2] = dest[2];
            mathVecNormalize(temp1,3);
            
            //if you can't move in a straight line... 0.7*0.53 = 0.371
            angle = mathVecInner(temp1,npos,3);
    	    if ((angle<0.4f)||(d<0.35f)){
                //GO to an intermediate point (O) first
                //if angle is near to 2pi then
                /*if (angle<-0.95){
                    #ifdef DEBUG_MESSAGES
                    DEBUG(("Large Angle.."));
                    #endif
                    if (cpos[0]<0) O[0]=-0.3f; else O[0]=0.3; 
                    O[1]=0; O[2] = 0.5f;
                }
                else*/
                    mathVecAdd(O,temp1,npos,3);
                mathVecNormalize(O,3);
                for (i=0;i<3;i++)
                    O[i]*=0.45f;
                gotopos2(poiG, O, 2.0f);
                #ifdef DEBUG_MESSAGES
                DEBUG(("TIME: %d. Intermediate point: %f %f %f.\n", TIME, O[0], O[1], O[2]));
                #endif
            }
            else{
    	        ret = gotopos2(poi_t, dest, 2.0f);
            }
    	    
    	    //if you are heading for a poi take pictures...
    	    if(upload!=1){
    	        //3% codesize
    	        #ifdef EXTRA_CONDITIONS
                mathVecSubtract(temp1, cpos, poiG, 3); //desired attitude
                mathVecNormalize(temp1,3);
                angle = mathVecInner(poiG, temp1, 3)/0.2f;
                #else
                angle=1;
                #endif
                //--------------------------------------------------
                //Check if you can take the photo.
                if ( /*(d>Rd)&&(d<R)&&*/(game.alignLine(ids[i0]))&&(angle>theta_tol)&&(t<=2)&&(t>=0) ){
                    game.takePic(ids[i0]);
                    ms += 1;
                    photos[i0]+=1;
                    #ifdef DEBUG_MESSAGES
                    DEBUG(("TIME %d. TAKING PICTURE of POI #%d (L=%f) memory = %d.\n",TIME,ids[i0],L,ms));
                    #endif
                    if (ms==MEM)
                        step = 8;
                    else
                        step = 0;
                }
    	    }
    	    
    	    
    	    //if you are heading towards the Shadow zone
    	    if (old_step==10){
    	        if ((tf>30)&&(ms==0)){
    	            step = 0;
    	        }
    	        
    	        if ((outside_zone)&&(cpos[2]>0.32f)&&(cvel[1]>0)){
                    //temp1[0]=-cvel[0]; temp1[1]=-cvel[1];temp1[2]=1-cvel[2];
                    temp1[0] = 0.32f-cpos[0]-cvel[0];temp1[1] = -cpos[1]-cvel[1];
                        temp1[2] = -cpos[2]-cvel[2];
                    api.setForces(temp1);
    	        }
    	    }
    	    
    	    
    	    
    	    //if you are heading towards upload point
    	    if (old_step==8){
    	        if (ms==0){
    	            step = 0;
    	            upload=0;
    	        }
    	        else
    	            upload=1;
    	    }
    	    
    	    
    	    //NEW STUFF
    	    //if you are not heading for the shadow zone or the upload point
    	    //if ((old_step!=8)&&(old_step!=10)){
    	        angle = mathVecInner(npos,cvel,3)/velocity;
    	        if (velocity>0.01){
        	        if ((d>0.54)&&(angle>0))
        	            step = 6;
        	        if ((d<0.38)&&(angle<-0.2f))
        	            step = 6;
    	        }
    	    //}
    	break;
    	  
    	
    	
    	//MODE 06: Stabilize the sphere. Then go to MODE 00 or 2?
        case 6:
    	    //stabilize
    	    #ifdef UPLOAD_BUG_01
    	    DEBUG(("TIME: %d. HARD STABILIZE", TIME));
    	    #endif
    	    if (velocity>0.01)
    	        hard_stabilize();
    	    else
    	        step = 0;
    	 break;
    	    
    	    
    	//MODE 08: Change heading for uploading (T)
        case 8:
    	    old_step = 8;
    	    //check if earth is in line of sight
    	    dest[0]=cpos[0];dest[1]=cpos[1];dest[2]=cpos[2];
    	    #ifdef UPLOAD_BUG_01
    	    if (dest[0]<0)
    	        dest[0] = 0.1;
    	    #endif
    	    upload = 1;
    	    step = 2;
    	    #ifdef DEBUG_MESSAGES
    	    DEBUG(("TIME %d. Going to Transmit point (z=%f).", TIME, dest[2]));
    	    #endif
    	    goto MOVE;
    	 break;
    	     
    	     
    	//MODE 10: Set Heading for Shadow Zone. Protection from flare
    	case 10:
    	    old_step = 10;
    	    #ifdef DEBUG_MESSAGES
    	       DEBUG(("FLARE!!!!! Set heading to Shadow Zone..\n"));
    	    #endif
            dest[0] = 0.41f; dest[1]=0; dest[2]=-0.10f;
            step = 2;
            if (ms>0){
                upload = 1;
                #ifdef DEBUG_MESSAGES
                DEBUG(("Make upload = 1."));
                #endif
            }
            goto MOVE;
    	    break;
    	    
    	    
    	//Turnoff - On    
    	case 12: 
    	    #ifdef DEBUG_MESSAGES
            DEBUG(("TIME %d. Stand By to turn Off - On.\n",TIME));
            #endif
    	    old_step = 12;
    	    sd_counter++;
    	    //hard_stabilize();
    	    if (tf==1){
    	        #ifdef DEBUG_MESSAGES
                DEBUG(("TIME %d. Turn Off.\n",TIME));
                #endif
    	        game.turnOff();
    	        sd_counter = 0;
    	    }
    	    if ((sd_counter>1)&&(tf>25)){
    	        #ifdef DEBUG_MESSAGES
                DEBUG(("TIME %d. Turn On.\n",TIME));
                #endif
    	        game.turnOn();
    	        step = 0;
    	        sd_counter = 0;
    	    }
    	    break;
	}
}




short gotopos2(float poi[3], float dest[3], float coef){
    short r;
    float att[3], v[3], d, vel =0.02f, temp[3];
    r=0;

    mathVecSubtract(v,dest,cpos,3);
	d = mathVecMagnitude(v,3);
    
    if (upload == 0){
        //compute attitude facing the POI when at final destination
        att[0] = -dest[0];att[1]=-dest[1];att[2]=-dest[2];
    }
    else{
        //Facing earth?
        temp[0]=0.64f;
        temp[1]=0;
        temp[2]=0;
        mathVecSubtract(att,temp,cpos,3);
        mathVecNormalize(att,3);
        #ifdef DEBUG_MESSAGES
        DEBUG(("ANGLE = %f\n", acosf(mathVecInner(att,catt,3))));
        #endif
        if (acosf(mathVecInner(att,catt,3))<0.05f){
            game.uploadPic();
            r=game.getMemoryFilled();
            DEBUG(("MEMORY: %d",r));
            if (r==0){//no need? only for bug
                upload = 0;
                DEBUG(("STOP UPLOADING... catt =\n"));
            }
        }
    }
    
	if (d>=0.15f){
	    mathVecNormalize(v,3);
	    v[0]*=vel;v[1]*=vel;v[2]*=vel;
	}
	else{
	    v[0]*=coef*d;v[1]*=coef*d;v[2]*=coef*d;
	}
    
    api.setPositionTarget(dest);
	if (d>0.08f){
	    api.setVelocityTarget(v);
	}
  
    //consider removing for codesize
    //mathVecNormalize(att,3);
    //------------------------
	api.setAttitudeTarget(att);

	r = 0;
    if (d<tol){
        v[0]=0;v[1]=0;v[2]=0;
        api.setVelocityTarget(v);
        r=1;
    }
	return r;
}


//Stabilization function
void  hard_stabilize(){
    float    force[3];
    
    memcpy(force, cvel, 3*sizeof(float));
    force[0]*=-4.3f;force[1]*=-4.3f;force[2]*=-4.3f;
    api.setForces(force);
}


/*void  bounce(){
    float    force[3];
    
    force[0]= 
}*/

void    poi_predict(float poi_t[3], float poi[3], unsigned char t){
    float theta,a1,a2;
    
    theta = 0.1f*t;
    a1 = cosf(theta);
    a2 = -sinf(theta);
    
    poi_t[0] = poi[0]*a1 + poi[2]*a2;
    poi_t[1] = poi[1];
    poi_t[2] = -poi[0]*a2 + poi[2]*a1;
    
    //If the point is predicted to be on the back...
    //This is not totally correct, but its close and small..
    if (poi_t[0]>0){
        poi_t[0]=-poi_t[0];
        poi_t[2]=-poi_t[2];
    }
}
//End page main

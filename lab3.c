
/*****************************************************************************\
* Laboratory Exercises COMP 3510                                              *
* Author: Saad Biaz, Andrew Bradley                                                           *
* Date  : April 1, 2014, April 10, 2019                                       *
\*****************************************************************************/

/*****************************************************************************\
*                             Global system headers                           *
\*****************************************************************************/


#include "common3.h"

/*****************************************************************************\
*                             Global data types                               *
\*****************************************************************************/



/*****************************************************************************\
*                             Global definitions                              *
\*****************************************************************************/
#define MAX_QUEUE_SIZE 88888888 /*Max Queue Size 8*/
#define MAX_NUM_DEV 31 /*Max Device Size is 31*/





/*****************************************************************************\
*                            Global data structures                           *
\*****************************************************************************/




/*****************************************************************************\
*                                  Global data                                *
\*****************************************************************************/
Event* eventQue[MAX_NUMBER_DEVICES][MAX_QUEUE_SIZE]; //Array of Queues
int storePos = MAX_QUEUE_SIZE - 1; 
//"Tail" of each device queue       
int nextToStore[MAX_NUMBER_DEVICES] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
//"Head" of each device queue
int nextToProcess[MAX_NUMBER_DEVICES] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
//Number of processes waiting to be serviced in each queue
int numPWaiting[MAX_NUMBER_DEVICES] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};


//Total of missed events for each device
int totMisEventsArray[MAX_NUMBER_DEVICES] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
//Total events served for each device
int totSerEventsArray[MAX_NUMBER_DEVICES] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
//Total response time for each device
float totRespEventsArray[MAX_NUMBER_DEVICES] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
//Total turnaround time for events on each device
float totTurnTimeEventsArray[MAX_NUMBER_DEVICES] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
//Events seen but not processed due to full que
int seenQFull[MAX_NUMBER_DEVICES] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,};
//Cumulative data for all devices
int totMisEvents = 0;
int totSerEvents = 0;
int totEvents = 0;
float totSerTime = 0;
float totRespTime = 0;
float totTurnaroundTime = 0;




/*****************************************************************************\
*                               Function prototypes                           *
\*****************************************************************************/

void Control(void);
void InterruptRoutineHandlerDevice(void);
void BookKeeping();
void AddEvent(Event* event);
Event* Deque(int numDev);


/*****************************************************************************\
* function: main()                                                            *
* usage:    Create an artificial environment for embedded systems. The parent *
*           process is the "control" process while children process will gene-*
*           generate events on devices                                        *
*******************************************************************************
* Inputs: ANSI flat C command line parameters                                 *
* Output: None                                                                *
*                                                                             *
* INITIALIZE PROGRAM ENVIRONMENT                                              *
* START CONTROL ROUTINE                                                       *
\*****************************************************************************/

int main (int argc, char **argv) {

   if (Initialization(argc,argv)){
      Control();
   } 
} /* end of main function */

/***********************************************************************\
 * Input : none                                                          *
 * Output: None                                                          *
 * Function: Monitor Devices and process events (written by students)    *
 \***********************************************************************/
void Control(void){
   Event* e;
   int count = 0;
   int eventNum = 0;
   int deviceNum = 0;
 
   float turnaroundTime = 0;
   float eventTime = 0;
   float startSerTime = 0;
   float endSerTime = 0;
 
   int nextserEvent[MAX_NUMBER_DEVICES] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
 

   while (1) {
   
      count &= MAX_NUMBER_DEVICES - 1;
      if(numPWaiting[count] > 0) {
      //Gets event and its data from the queue
         e = Deque(count);  
         deviceNum = e->DeviceID;
         eventNum = e->EventID;
         eventTime = e->When;
      
      //Calculates missed events
         if (nextserEvent[deviceNum] != eventNum) {
            totMisEvents += e->EventID - nextserEvent[deviceNum];
            totMisEventsArray[deviceNum] += (eventNum - nextserEvent[deviceNum]);
         }
      
         Server(e);
      
      //Calculates turnAroundTime data
         endSerTime = Now();
         turnaroundTime = endSerTime - eventTime;
         totTurnTimeEventsArray[deviceNum] += turnaroundTime;
         totTurnaroundTime += turnaroundTime;
      //Events serviced data
         totSerEvents++;
         totSerEventsArray[deviceNum]++;
         nextserEvent[deviceNum] = eventNum + 1;
      //Resets count to 0 after servicing event.
      //Maintains priority based upon device ID number.
         count = -1;
      }
     
      count++;
   }
   

}


/***********************************************************************\
* Input : None                                                          *
* Output: None                                                          *
* Function: This routine is run whenever an event occurs on a device    *
*           The id of the device is encoded in the variable flag        *
\***********************************************************************/
void InterruptRoutineHandlerDevice(void){
   printf("An event occured at %f  Flags = %d \n", Now(), Flags);
	// Put Here the most urgent steps that cannot wait
   Event *currentEvent;
   int deviceNum = 0;
   float respTime = 0; 
   int temp = Flags;
   Flags = 0;
   
   while(temp){
   
      if(temp & 1) {
         
         //Gets event from buffer
         currentEvent = &BufferLastEvent[deviceNum];
         //Response time data
         respTime = Now() - currentEvent->When;
         totRespEventsArray[deviceNum] += respTime;
         totRespTime += respTime;
         //Display event and add to queue
         DisplayEvent('A' + deviceNum, currentEvent);
         AddEvent(currentEvent);
         
      }
         
      temp = temp >> 1;
      deviceNum++;
   }
   
   

}


/***********************************************************************\
* Input : None                                                          *
* Output: None                                                          *
* Function: This must print out the number of Events buffered not yet   *
*           not yet processed (Server() function not yet called)        *
\***********************************************************************/
void BookKeeping(void){
  // For EACH device, print out the following metrics :
  // 1) the percentage of missed events, 2) the average response time, and 
  // 3) the average turnaround time.
  // Print the overall averages of the three metrics 1-3 above
   int n = 0;
   float avgTurnaround;
   float avgResponse;
   int totalEvents = totSerEvents + totMisEvents;
  
  
   printf("\n >>>>>> Done\n");
   //For each device
   
   /////////////////////////////////////////////
   while(n < MAX_NUMBER_DEVICES) {
      
      //Need to calculate averages for each device using arrays & totals
      printf("The percentage of missed events for device %i is %10.2f\n", n,
         				 ((float)totMisEventsArray[n]/100)*100);
      printf("The average response time for device %i is %10.6f\n", n, 
         				totRespEventsArray[n]/totSerEventsArray[n]);
      printf("The average turnaround time for device %i is %10.6f\n", n,
         				totTurnTimeEventsArray[n]/totSerEventsArray[n]);
      n++; 
   }
   
   
   ///////////////////////////////////////////////////////// MAY NEED TO RECALCULATE
   //For all devices
   printf("The percentage of missed events: %10.2f\n", ((float)totMisEvents/totalEvents)*100);
   printf("The average response time: %10.6f\n",totRespTime/totSerEvents);
   printf("The average turn around time: %10.6f\n",totTurnaroundTime/totSerEvents);
}


//Adds event to queue
void AddEvent(Event* e){

   int devNum;
  
   devNum = e->DeviceID;
   //IF queue is full, does not add event to queue
   if (numPWaiting[devNum] == MAX_QUEUE_SIZE){
      seenQFull[devNum]++;
      return;
   }
   //Stores event at "tail"  
   eventQue[devNum][nextToStore[devNum]] = e;
   //Moves "tail"
   nextToStore[devNum] = (nextToStore[devNum] + 1) & storePos;
   
   //Adds to total number of events waiting to be serviced for each device
   
   numPWaiting[devNum]++;
   
	
}



Event* Deque(int devNum){

   Event* e;
 
   e = eventQue[devNum][nextToProcess[devNum]];
   
   nextToProcess[devNum] = (nextToProcess[devNum] + 1) & storePos;
   numPWaiting[devNum]--;
  
 
   return e;
      
 
   

}













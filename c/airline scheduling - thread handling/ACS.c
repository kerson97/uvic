#include <stdio.h>
#include <pthread.h>
#include <sys/types.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>



//forward declarations
void storeLines();
void store_the_chunks(char *line);
void * customer_entry(void * cus_info);
void *clerk_entry(void * clerkNum);
void * createClerk(void * clerkNum);
int startTime();
double getCurrentSimulationTime();


typedef struct customer_info{ /// use this struct to record the customer information read from customers.txt
    int user_id;
	int class_type;
	int service_time;
	int arrival_time;
}customer_info;

typedef struct clerk_info{ /// use this struct to record the clerk info
	int clerkNum;
}clerk_info;

//Queue Implementation (partial credit to geeksforgeeks)

typedef struct QNode {
    struct customer_info customer;
    struct QNode* next;
} QNode;

struct Queue {
    struct QNode *front, *rear;
};

struct QNode* newNode(struct customer_info customer){
    struct QNode* temp = (struct QNode*)malloc(sizeof(struct QNode));
    temp->customer = customer;
    temp->next = NULL;
    return temp;
}

struct Queue* createQueue(){
    struct Queue* q = (struct Queue*)malloc(sizeof(struct Queue));
    q->front = q->rear = NULL;
    return q;
}

void enQueue(struct Queue* q, struct customer_info* customer){
    struct QNode* temp = newNode(*customer);

    if(q->rear == NULL){
        q->front = q->rear = temp;
        return;
    }
    q->rear->next = temp;
    q->rear = temp;
}
void deQueue(struct Queue* q){
    if(q->front == NULL)return;
    
    struct QNode* temp = q->front;
    q->front = q->front->next;
    if(q->front == NULL)q->rear=NULL;
    free(temp);   
}
int isEmpty(struct Queue* q){
    if(q->front == NULL)return 1;
    else return 0;
  
}

//global variables
int numCustomers;
char buffer[10];
char storedLines[100][100];
FILE *customers;
char chunks[100][100];

int queueLength[2];
int queueStatus[2] = {0, 0};
int winnerselected [2] = {0, 0};
struct Queue* Queues[2];

pthread_mutex_t queue0lock;
pthread_mutex_t queue1lock;
pthread_mutex_t waitLock;

pthread_cond_t queueConds[2];
pthread_cond_t clerkconds[5];
pthread_cond_t clerkWaitingCond;

struct timeval init_time;
double overall_waiting_time=0;
static struct timeval start_time;


int main(int argc, char *argv[]) {

    //initialize locks and cond vars

    pthread_mutex_init(&queue0lock, NULL);
    pthread_mutex_init(&queue1lock, NULL);
    pthread_mutex_init(&waitLock, NULL);

    pthread_cond_init(&queueConds[0], NULL);
    pthread_cond_init(&queueConds[1], NULL);
    pthread_cond_init(&clerkWaitingCond, NULL);

    for(int i=0; i<5; i++){
        pthread_cond_init(&clerkconds[i], NULL);
    }

    //create Queue 0(Economy Class) and Queue 1 (Business Class)
    Queues[0] = createQueue();
    Queues[1] = createQueue();

    //pass customers from txt file into struct
    customers = fopen(argv[1], "r");
    fgets(buffer, 100, customers);
    numCustomers = atoi(buffer);
    storeLines();  

    int i;
    struct customer_info customerArray[numCustomers];
        for(i=0; i<numCustomers; i++){
            store_the_chunks(storedLines[i]);

            customerArray[i].user_id = atoi(chunks[0]);
            customerArray[i].class_type = atoi(chunks[1]);
            customerArray[i].arrival_time = atoi(chunks[2]);
            customerArray[i].service_time = atoi(chunks[3]);
        }


        //create and initialize clerks 
        int j=0;
        struct clerk_info clerkArray[5];
        for(j = 0; j<5; j++){
        clerkArray[j].clerkNum = j;
        }
    
        pthread_t clerkID[5];
	    for(i = 0; i < 5; i++){ // number of clerks
		pthread_create(&clerkID[i], NULL, createClerk, (void *)&clerkArray[i]); // pass clerk info to clerk thread
	    }

        //start simulating time
        startTime();
    
        //create customer threads
        pthread_t customID[numCustomers];
    	for(i = 0; i < numCustomers; i++){ // number of customers
		pthread_create(&customID[i], NULL, customer_entry, (void *)&customerArray[i]); //custom_info: passing the customer information (e.g., customer ID, arrival time, service time, etc.) to customer thread
        }

        //join customer threads
        for(i=0; i<numCustomers; i++){
		pthread_join(customID[i], NULL);
	    }

        //print results
        printf("All jobs done!\n");
        printf("The average waiting time for all customers was: %.2f seconds\n",overall_waiting_time/numCustomers);
        



}

void storeLines(){
int i;
for(i=0; i<numCustomers; i++){
    fgets(storedLines[i],100,customers);
    storedLines[i][strlen(storedLines[i])-1] = '\0';
    }
}

void store_the_chunks(char *line){
    char buffer[100];
    char *t;
    int num_chunks=0;

    strncpy(buffer,line, 100);
    t = strtok(buffer, ":");

    while (t != NULL) {

        strncpy(chunks[num_chunks],t, 100);

        num_chunks++;
        t = strtok(NULL, ",");
    }
    }

void * customer_entry(void * cus_info){
	struct customer_info * p_myInfo = (struct info_node *) cus_info;
	
    int queueNum = p_myInfo->class_type;
    int wakingClerk; 
    pthread_mutex_t chosenLock;
    double queueEnterTime;

    //sleep for arrival time
    usleep(p_myInfo->arrival_time*100000);

    fprintf(stdout, "A customer arrives: customer ID %2d. \n", p_myInfo->user_id);
	queueEnterTime = getCurrentSimulationTime();
	
    //Enqueue operation: get into either business queue or economy queue 

    if(queueNum ==0)chosenLock = queue0lock;
    if(queueNum ==1)chosenLock = queue1lock;
	
	pthread_mutex_lock(&chosenLock); 
	{
		fprintf(stdout, "A customer enters a queue: the queue ID %1d, and length of the queue %2d. \n", queueNum, queueLength[queueNum]+1);

		enQueue(Queues[queueNum], p_myInfo);
		queueLength[queueNum]++;
        
        //try to wake a sleeping clerk if there are any
        pthread_mutex_lock(&waitLock);
        pthread_cond_signal(&clerkWaitingCond);
        pthread_mutex_unlock(&waitLock);

		while (true) {
 			pthread_cond_wait(&queueConds[queueNum], &chosenLock);
			if (Queues[queueNum]->front->customer.user_id ==p_myInfo->user_id && !winnerselected[queueNum]) { //if customer is front of queue && winner isn't selected, dequeue
				deQueue(Queues[queueNum]);
				queueLength[queueNum]--;
				winnerselected[queueNum] = true; // update the winner_selected variable to indicate that the first customer has been selected from the queue
				break;
				
			}
		}
			
	}
	pthread_mutex_unlock(&chosenLock); //unlock mutex_lock such that other customers can enter into the queue
	
	// Try to figure out which clerk awoken me
	usleep(10);
	wakingClerk = queueStatus[queueNum];
	queueStatus[queueNum] = 0;
	
    double waitingTime = getCurrentSimulationTime()-queueEnterTime;
    overall_waiting_time+=waitingTime;  //add waiting time to overall waiting time
    
	fprintf(stdout, "A clerk starts serving a customer: start time %.2f, the customer ID %2d, the clerk ID %1d.\n", getCurrentSimulationTime(), p_myInfo->user_id, wakingClerk);
	
	usleep(p_myInfo->service_time*100000);
	
	fprintf(stdout, "-->>> A clerk finishes serving a customer: end time %.2f, the customer ID %2d, the clerk ID %1d. \n",getCurrentSimulationTime(), p_myInfo->user_id, wakingClerk);
	
	pthread_cond_signal(&clerkconds[wakingClerk]); // Notify the clerk that service is finished, it can serve another customer
	
	pthread_exit(NULL);
	
	return NULL;
    

	}

void * createClerk(void * clerkNum){

    struct clerk_info * clerk_myInfo = (struct info_node *) clerkNum;
    int serving;
    int clerkNumber = clerk_myInfo->clerkNum;

    pthread_mutex_t chosenLock;

    while(true){

        if(queueLength[0]!=0 || queueLength[1] !=0){ //if queues aren't empty
            if(queueLength[1]!=0){                       //determine who to serve by customer type in queues
                chosenLock = queue1lock;
                serving=1;}
                else if(queueLength[0]!=0){
                    chosenLock = queue0lock;
                    serving=0;
                }
                pthread_mutex_lock(&chosenLock);    
                queueStatus[serving] = clerkNumber;     //set queueStatus of queue to clerk ID
                pthread_cond_broadcast(&queueConds[serving]);   //wake up customer in that queue
                winnerselected[serving] = 0;
                pthread_mutex_unlock(&chosenLock);
                pthread_cond_wait(&clerkconds[clerkNumber], &chosenLock);   //wait for customer to finish service
        }
        else {
            pthread_mutex_lock(&waitLock);
            pthread_cond_wait(&clerkWaitingCond, &waitLock);  //if no customers are waiting for service clerk waits to be woken
            pthread_mutex_unlock(&waitLock); //if no customers are waiting for service, wait
            continue;}
        
    }
       
}

double getCurrentSimulationTime(){
	
	struct timeval cur_time;
	double cur_secs, init_secs;
	
	//pthread_mutex_lock(&start_time_mtex); you may need a lock here
	init_secs = (start_time.tv_sec + (double) start_time.tv_usec / 1000000);
	//pthread_mutex_unlock(&start_time_mtex);
	
	gettimeofday(&cur_time, NULL);
	cur_secs = (cur_time.tv_sec + (double) cur_time.tv_usec / 1000000);
	
	return cur_secs - init_secs;
}


int startTime(){
	
	gettimeofday(&start_time, NULL); // record simulation start time

	return 0;
}

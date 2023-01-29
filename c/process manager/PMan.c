//CSC 360 Assignment 1, Carson Seidel (V00933347)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <readline/history.h>
#include <readline/readline.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <stdbool.h>

char *commandsArray[50];
int num_commands=0;
int process_count = 0;

//forward declarations of functions

void store_the_commands(char *lines);
void bg_entry(char **argv);
void check_zombieProcess(void);
void bglist_entry();
void bgsig_entry(pid_t pid, char *command_type);
void pstat_entry(pid_t pid);
void exit_routine();

//linked list implementation (inspiration from CodeVault on YouTube)

typedef struct Node {
	int x;
	char *name;
	struct Node* next;
} Node;
void insert_node(Node** head, int pid, char *name){
	Node* new_node = malloc(sizeof(Node));
	if(new_node == NULL){printf("Error adding node to list");}
	new_node->next = NULL;
	new_node->x = pid;
	new_node->name = name;
	if(*head == NULL){
		*head = new_node;
		process_count++;
		return;
	}

	Node* curr = *head;
	while(curr->next !=NULL){
		curr = curr->next;
	} 
	curr->next = new_node;
	process_count++;
}
	
void remove_node(Node** head, int pid){
	if(*head == NULL) return;

	if((*head)->x == pid){
		Node* to_remove = *head;
		*head = (*head)->next;
		free(to_remove);
		process_count--;
		return;
	}
	for(Node* curr = *head; curr->next != NULL; curr= curr->next){
		if(curr->next->x == pid){
			Node* to_remove = curr->next;
			curr->next = curr->next->next;
			free(to_remove);
			process_count--;
		}
	}
}

void deallocate(Node** head){
	Node* curr = *head;
	while(curr!= NULL){
		Node* aux = curr;
		curr = curr->next;
		free(aux);
	}
	*head = NULL;
	}

bool node_search(Node** head, int pid){

Node* current = *head;
while(current != NULL){
	if(current->x == pid){
		return true;
	}
	current = current->next;
}
	return false;
}
 Node* headPnode = NULL;


int main(){


	while(1){	
		
	num_commands=0;

	char *cmd;
	cmd = readline("PMan: > ");
	

	if(strlen(cmd)!=0) store_the_commands(cmd);	//if command isn't empty, parse commands
	else continue;

	char *cmd_type = commandsArray[0];		//set command type

	if (strcmp(cmd_type, "bg") == 0){		//attempt to launch program in bg
		if(commandsArray[1]!=NULL){			//simple error check
			bg_entry(commandsArray);
		}
		else printf("Invalid input, correct usage is: bg <cmd> \n");	
	}
		
		else if(strcmp(cmd_type, "bglist")==0){ //list background programs
				bglist_entry();
		}
		
		else if(strcmp(cmd_type, "bgkill")==0 || strcmp(cmd_type, "bgstop")==0 || strcmp(cmd_type, "bgstart")==0){

			if(commandsArray[1]!=NULL){	 //error check for invalid input
				pid_t victim = atoi(commandsArray[1]);
				bgsig_entry(victim, cmd_type);
			}
			else printf("Invalid input, correct usage is: <cmd> <pid> \n");
		}

		else if(strcmp(cmd_type, "pstat")==0){
			if(commandsArray[1]!=NULL){
				pstat_entry(atoi(commandsArray[1]));
			}
			else printf("Invalid input, correct usage is: pstat <pid> \n");
		}

		else if(strcmp(cmd_type, "exit")==0){
			exit_routine();
		}

	else {
		printf("Command not found \n");
	}
	check_zombieProcess();
	}

	return 0;
	
}

void check_zombieProcess(void){
	int status;
	int retVal = 0;
	
	while(1) {
		usleep(1000);
		if(headPnode == NULL){
			return ;
		}
		retVal = waitpid(-1, &status, WNOHANG);
		if(retVal > 0) {
			remove_node(&headPnode, retVal); //remove zombie from list
		}
		else if(retVal == 0){
			break;
		}
		else{
			perror("waitpid failed");
			exit(EXIT_FAILURE);
		}
	}
}
		
void store_the_commands(char *cmd){		//store commands in string in a char * (borrowed from my SENG 265 A1)
	int j=0;
	commandsArray[j] = strtok(cmd, " ");
	while(commandsArray[j]!=NULL){
		j++;
		commandsArray[j] = strtok(NULL, " ");
		num_commands++;
	}
	
}

void proc_stat_search(pid_t pid){

		char buffer[100];
		char *parsedInput[100];

	if(node_search(&headPnode, pid)==true){

		char procFile[1000];
		sprintf(procFile, "/proc/%d/stat", pid);
		FILE *open = fopen(procFile, "r");

		int j=0;
		while(!feof(open)){
			fgets(buffer, 500, open);		
		}
		fclose(open);
		j=0;
		parsedInput[j] = strtok(buffer, " ");	//store stat segments in char **
		while(parsedInput[j]!=NULL){
			j++;
			parsedInput[j] = strtok(NULL, " ");
		}
	}    
	else {
		printf("STAT file for %d not found \n", pid);
		return;
		}

	printf("file name: %s\n", parsedInput[1]);		//print elements in char ** at corresponding positions
	printf("state: %s\n", parsedInput[2]);

	ulong utime = atoi(parsedInput[13])/sysconf(_SC_CLK_TCK);
	ulong stime = atoi(parsedInput[14])/sysconf(_SC_CLK_TCK);
	printf("utime: %lu\n", utime);
	printf("stime: %lu\n", stime);
	printf("rss: %s\n", parsedInput[23]);


}
void proc_status_search(pid_t pid){

	if(node_search(&headPnode, pid)==true){
			char lastlines[2][50];
			char procFile[1000];
			char buffer[500];
			
			sprintf(procFile, "/proc/%d/status", pid);
			FILE *open = fopen(procFile, "r");

			int j=0;
			while(!feof(open) && j<55){			//read lines until voluntary/unvoluntary ctxt switches, then store/print them
				j++;
				fgets(buffer, 500, open);
				if(j==54){
				strcpy(lastlines[0], buffer);
				printf("%s", lastlines[0]);
				}
				if(j==55){
				strcpy(lastlines[1], buffer);
				printf("%s", lastlines[1]);
				}
			}	
		}    
		else {
			printf("STATUS file for %d not found \n", pid);
			return;
			}

}

void bg_entry(char **argv){

	int j;
	for(j=0; j<num_commands; j++){	//restructure array to remove redundant "bg" command
		argv[j]=argv[j+1];
	}

		int status = 0;
		pid_t pid;
		pid = fork();
		
		if(pid == 0){
			if(execvp(argv[0], argv) < 0){		//launch argv[0] in child process
				perror("Error on execvp");

		}
		else return;
		exit(EXIT_SUCCESS);
	}
			else if(pid > 0) {
				waitpid(pid, &status, WNOHANG);			//don't wait for child to finish
				insert_node(&headPnode, pid, argv[0]); //add node to list
			}
		else {
			perror("fork failed");
			exit(EXIT_FAILURE);
		}
}

void bglist_entry(){ //print elements (path/pid) of nodes in linked list
	
	if(headPnode!=NULL){
	for(Node* curr = headPnode; curr!= NULL; curr = curr->next){
		printf("PID: %d PATH: %s\n", curr->x, curr->name);
		}
		printf("\nTotal background processes: %d\n",process_count);
	}
	else printf("Nothing currently running in the background \n");
}

void bgsig_entry(pid_t pid, char *command_type){ //send signals with kill(), simple error handling

if(strcmp(command_type, "bgkill")==0){
	if(node_search(&headPnode, pid) == true){
		int status = kill(pid, SIGTERM);

		if(status == 0){
		printf("Process Killed Succesfully! \n");
		remove_node(&headPnode, pid);
		}
		else if(status == -1){
			printf("Failed to kill process with PID: %d", pid);
		}
	}
	else printf("Process does not exist \n");
}

if(strcmp(command_type, "bgstop")==0){
	if(node_search(&headPnode, pid) == true){
		int status = kill(pid, SIGSTOP);

		if(status == 0){
		printf("Process Stopped Succesfully! \n");
		}
		else if(status == -1){
			printf("Failed to stop process with PID: %d", pid);
		}
	}
	else printf("Process does not exist \n");
}

if(strcmp(command_type, "bgstart")==0){
	if(node_search(&headPnode, pid) == true){
		int status = kill(pid, SIGCONT);

		if(status == 0){
		printf("Process Started Succesfully! \n");
		}
		else if(status == -1){
			printf("Failed to start process with PID: %d", pid);
		}
	}
	else printf("Process does not exist \n");
}

}

void pstat_entry(pid_t pid){
proc_stat_search(pid);
proc_status_search(pid);
}

void exit_routine(){
	char kill;
	int killed =0;
	printf("Terminate all children? (y/n)\n");
	scanf("%c", &kill);

	if(kill=='y'){
		if(headPnode!=NULL){
			for(Node* curr = headPnode; curr!= NULL; curr = curr->next){	//kill remaining processes
				printf("Killing PID: %d PATH: %s\n", curr->x, curr->name);
				bgsig_entry(curr->x, "bgkill");
				remove_node(&headPnode, curr->x);
				killed++;
			}	
		printf("\nTotal Processes Killed: %d\n",killed);
		deallocate(&headPnode);
		exit(0);
		}
		else printf("Nothing to kill \n");
		exit(0);

	}
	else exit(0);

}

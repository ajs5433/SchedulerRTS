#include <cstdlib>
#include <iostream>
#include <time.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <iostream>
#include <vector>
#include <string>
#include "Task.h"
#include <sstream>
using namespace std;

#define PRINT_ALL true				// created to print extra information in the program for verification of the correct program flow.
#define EARLIEST_DEADLINE_FIRST 	0
#define SHORTEST_COMP_TIME 			1
#define LEAST_SLACK_TIME			2
#define SLEEP_TIME					1000

/* Global variables: 	*/
int total_elapsed_time 	= 0;
int program_exec_time 	= 0;
bool program_running 	= false;
int scheduling_algorithm = -1;
vector<Task*> allTasks;
vector<Task*> activeTasks; // Ordered by priority, low to high

/* Thread created to keep a real-time count of the time elapsed
 * thread first waits for program to load input values from user and initialize threads
 * then starts running using timespec as a time counter
 * */
void *timeElapsedThread(void *ptr){
	struct timespec start, end;
	while(!program_running);
	while(program_running){
		clock_gettime(CLOCK_REALTIME,&start);
		clock_gettime(CLOCK_REALTIME,&end);
		while(end.tv_sec - start.tv_sec < 1)
			clock_gettime(CLOCK_REALTIME,&end);
		clock_gettime(CLOCK_REALTIME,&start);
		if(!program_running)
			break;
		total_elapsed_time++;

		//Printing for verification purposes
		if(PRINT_ALL)
			printf("\n-----------------------------------------\nAFTER %d SECONDS:", total_elapsed_time);
	}
	return NULL;
}


/*
 * This function represents the behavior of all the threads in the program.
 * Each thread has a task assigned that constains all the information about the task
 * that is currently running.
 * */
void* taskThread(void* t) {
	int current_time;
	Task *task = (Task *) t;
	while(!program_running);
	if(PRINT_ALL)
		printf("\n%s started! (%d,%d,%d)", task->name.c_str(), task->execution_time, task->period, task->deadline);
	while(program_running){
		//Update Values
		task->seconds_in_period++;
		if (task->status == running){
			task->remaining_time--;
			if (task->remaining_time == 0){
				task->completedExecution(true);
				printf("\n--> %s: Completed!");
			}
		}else{ // if task is not running
			if(task->seconds_in_period >= task->deadline){
				printf("\n*** %s: Missed the deadline");
			}
		}
		if(task->seconds_in_period == task->period){
			if(!task->completed)
				printf("\n*** %s: Task did not complete in the given period ");
			task->remaining_time = task->execution_time;
			task->seconds_in_period = 0;
			task->completedExecution(false);

			if(PRINT_ALL)
				printf("\n%s Starting new period", task->name.c_str());
		}
		if(PRINT_ALL){
			if(task->status == running){
				printf("\nthread %s active! cycle: %d, remaining:  %d, completion status: %s",task->name.c_str(),task->seconds_in_period, task->remaining_time, (task->completed)? "true" : "false");
			}else{
				printf("\nthread %s not running! cycle: %d, remaining:  %d, completion status: %s",task->name.c_str(),task->seconds_in_period, task->remaining_time, (task->completed)? "true" : "false");
			}
		}

		current_time = total_elapsed_time;
		while(current_time==total_elapsed_time && program_running)
			usleep(SLEEP_TIME);	//small sleep to avoid processor overload by threads
	}

	//printing for verification purposes
	if(PRINT_ALL)
		printf("\nSimulation over, %s is no longer active", task->name.c_str());
	return NULL;
}


/* User selection of scheduling algorithm type*/
void getSchedulingAlgorithm(){
	printf("Choose a scheduling algorithm! Scheduling algorithm choices:\n(0) Earliest Deadline First\n(1) Shortest Completion Time\n(2) Least Slack Time\n");
	printf("\nEnter the number of your choice: ");
	while (!(scheduling_algorithm>=0 && scheduling_algorithm <=2)) {
		cin >> scheduling_algorithm;
	}
	switch(scheduling_algorithm) {
		case EARLIEST_DEADLINE_FIRST:
			printf("\tYou selected Earliest Deadline First!!\n");
			break;
		case SHORTEST_COMP_TIME:
			printf("\tYou selected Shortest Completion Time!!\n");
			break;
		default:
			printf("\tYou selected Least Slack Time!!\n");
			break;
	}
}

/*
 * Creating Threads and setting the default thread priority to 1
 * to guarantee the main thread (scheduler) will have higher priority
 * than all threads
 * */
void createThread(pthread_t* thread, Task* task) {
	pthread_attr_t attributes;
	pthread_attr_init(&attributes);
	int priority = 1; // Default starting priority for all tasks
	struct sched_param param;
	pthread_attr_getschedparam(&attributes, &param);
	param.sched_priority = priority;
	pthread_attr_setschedparam(&attributes, &param);
	pthread_create(thread, &attributes, taskThread, (void*)task);
}

/* User inputs the tasks in this method*/
void getTasksFromUser(){
	string input;
	string name;
	int c,p,d;
	int tokenCount = 0;
	int priority = 1;
	size_t pos = 0;

	printf("\nEnter tasks in the format \'<name> <execution time> <deadline> <period>\' and follow each with a newline.\nTo run tasks, enter \'start\' followed by a newline.\n");
	getline(cin, input); // Ignore newline from previous println
	while (true) {
		getline(cin, input);
		if (input == "start")
			break;

		while (((pos = input.find(" ")) <= string::npos) && tokenCount < 4) {
			switch(tokenCount) {
			case 0:
				name = input.substr(0, pos);
				tokenCount++;
				break;
			case 1:
				c = atoi(input.substr(0, pos).c_str());
				tokenCount++;
				break;
			case 2:
				p = atoi(input.substr(0, pos).c_str());
				tokenCount++;
				break;
			case 3:
				d = atoi(input.substr(0, pos).c_str());
				tokenCount++;
				break;
			default:
				tokenCount++;
				break;
			}
			input.erase(0, pos+1);
		}
		if (tokenCount != 4) {
			printf("Invalid task format.\n");
		} else {
			pthread_t* myThread = NULL;
			Task* task = new Task(name, c, p, d, priority);
			createThread(myThread, task);
			pthread_mutex_init(&(task->runLock), NULL);
			allTasks.push_back(task);
			tokenCount = 0;

			if(PRINT_ALL)
				printf("\nTask \"%s\" created!! \tallTasks.size = %d\n", task->name.c_str(), allTasks.size());
		}
	}
}

void getSimulationTime(){
	string input;
	int number;
	while (true) {
		cout << "Please enter the simulation time in seconds, for 3 secs insert 3: ";
		getline(cin, input);
		stringstream myStream(input);
		if (myStream >> number){
			program_exec_time = number;
			break;
		}else
			cout << "Please enter a number, try again" << endl;
	}
}

Task* getHighestPriorityTask(){
	Task* priorityTask = NULL;
	Task* thisTask = NULL;
	activeTasks.clear();
	unsigned int t;

	switch(scheduling_algorithm){
		case EARLIEST_DEADLINE_FIRST:
			for (t=0;t<allTasks.size();t++){
				thisTask = allTasks.at(t);
				if(!thisTask->completed)
					if (priorityTask == NULL || (priorityTask->deadline > thisTask->deadline))
						priorityTask = thisTask;
			}
			break;
		case SHORTEST_COMP_TIME:
			for (t=0;t<allTasks.size();t++){
				thisTask = allTasks.at(t);
				if(!thisTask->completed)
					if (priorityTask == NULL || (priorityTask->remaining_time > thisTask->remaining_time))
						priorityTask = thisTask;
			}
			break;
		case LEAST_SLACK_TIME:
			for (t=0;t<allTasks.size();t++){
				thisTask = allTasks.at(t);
				if(!thisTask->completed){
					if (priorityTask == NULL || (priorityTask->deadline - priorityTask->remaining_time > thisTask->deadline - thisTask->remaining_time)) {
						priorityTask = thisTask;
					}
				}
			}
			break;
		default:
			printf("ERROR: There are only three scheduling algorithms! verify code!");
			break;
	}
	return priorityTask;
}

void initializeClock(){
	//creating thread to count time
	pthread_t time_elapsed_thread;
	pthread_create(&time_elapsed_thread, NULL, timeElapsedThread, NULL);
}

int main(int argc, char *argv[]) {
	Task * current_task = NULL;
	Task * previous_task = NULL;
	int current_time;

	//Initialization
	initializeClock();
	getSchedulingAlgorithm();
	getTasksFromUser();
	getSimulationTime();

	/* main scheduler process:
	 * wait for the priority of the tasks to change, then
	 * wait for the current execution (1 second) to complete
	 * after one second the next task will run and previous will stop
	 * */
	current_task = getHighestPriorityTask();
	if(PRINT_ALL && current_task!=NULL)
		printf("\n-->Highest priority task: %s",current_task->name.c_str());
	current_task->start();
	program_running = true;
	while(total_elapsed_time < program_exec_time){
		previous_task = current_task;
		while(current_task == previous_task && total_elapsed_time < program_exec_time){
			current_task = getHighestPriorityTask();
			usleep(100);
		}
		if(PRINT_ALL && current_task!=NULL)
				printf("\n-->Highest priority task: %s",current_task->name.c_str());
		current_time = total_elapsed_time;
		while(current_time == total_elapsed_time && total_elapsed_time < program_exec_time);
		previous_task->stop();
		if(current_task==NULL){
			printf("\nAll tasks completed!!");
		}else{
			current_task->start();
			if(previous_task->completed)
				printf("\n%s completed!", previous_task->name.c_str());
			else if (previous_task == NULL)
				printf("A new task is available, %s is now running",current_task->name.c_str());
			else
				printf("\nPriority change! %s interrupted by %s", previous_task->name.c_str(), current_task->name.c_str());
		}
	}
	program_running = false;
	sleep(1);

	return EXIT_SUCCESS;
}

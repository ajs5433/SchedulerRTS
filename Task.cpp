/*
 * Task.cpp
 *
 *  Created on: Sep 30, 2017
 *      Author: ajs5433
 */

#include "Task.h"

Task::Task(string task_name, int task_exec_time, int task_period, int task_deadline, int task_priority) {
	name 			= task_name;
	execution_time 	= task_exec_time;
	remaining_time	= task_exec_time;
	period			= task_period;
	deadline 		= task_deadline;
	priority		= task_priority;
	seconds_in_period = 0;

	pthread_mutex_init(&completeLock, NULL);
	pthread_mutex_init(&runLock, NULL);
	completed 	= false;
	status	 	= unscheduled;
}

Task::~Task() {
	// TODO Auto-generated destructor stub
}

void Task::start(){
	//pthread_mutex_lock(&runLock);
	status = running;
	//pthread_mutex_unlock(&runLock);
}

void Task::stop(){
	//pthread_mutex_lock(&runLock);
	status = idle;
	//pthread_mutex_unlock(&runLock);
}

void Task::completedExecution(bool c){
	pthread_mutex_lock(&completeLock);
	completed = c;
	pthread_mutex_unlock(&completeLock);
}

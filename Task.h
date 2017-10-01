/*
 * Task.h
 *
 *  Created on: Sep 30, 2017
 *      Author: ajs5433
 */

#ifndef TASK_H_
#define TASK_H_

#include <iostream>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <cstdlib>
#include <pthread.h>
using namespace std;

enum ThreadStatus {unscheduled, running, idle, completed};

class Task {
public:
	string name;
	int execution_time;
	int remaining_time;
	int period;
	int deadline;
	int priority;
	int seconds_in_period;			//Number of seconds elapsed in the current period, when this variable is equal to zero everything is restarted

	pthread_mutex_t runLock;
	pthread_mutex_t completeLock;
	bool completed;
	ThreadStatus status;

public:
	Task(string task_name, int task_exec_time, int task_period, int task_deadline, int task_priority);
	virtual ~Task();
	void start();
	void stop();
	void completedExecution(bool c);
};

#endif /* TASK_H_ */

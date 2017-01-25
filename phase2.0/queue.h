#pragma once
#include <iostream>
//#include <cstdio>

//#include <cstdlib>
//#include <string>
#include "priority_queue.h"


class Regular_Queue
{
private:
	node *front;
	int size = 0;  //used to indicate the max size for jobs on disk
	int current_size = 0;  //holds a counter to show if/when size has been reached.

public:
	Regular_Queue(int init)
	{
		size = init;
		front = NULL;
	}
	Regular_Queue()
	{
		size = 5000;  //set as a default for queues that are implemented not as a disk.
		front = NULL;
	}

	/*
	what the routine does_______________________________________ -
	when called, it inserts a node onto a FIFO queue, carrying over PCB information along with it. also makes sure if disk is full
	to not insert onto disk


	_________________________________________________________
	critique : _____________________________________________
	no critique needed


	___________________________________
	*/
	void insert( struct PCB* Y)
	{
		int priority = 0;
		node *tmp, *q;
		tmp = new node;
		struct PCB*temp;  //creates a temporary register to store PCB, then loads it into queue,
		temp = new PCB;
		priority = Y->priority;
		temp->job_number = Y->job_number;
		temp->maximum_run_time = Y->maximum_run_time;
		temp->main_memory_needed = Y->main_memory_needed;
		temp->job_class = Y->job_class;
		temp->priority = Y->priority;
		temp->arrival_time = Y->arrival_time;
		temp->memory_index = Y->memory_index;
		temp->priority = priority;
		temp->waiting_time = Y->waiting_time;
		temp->turnaround_time = Y->turnaround_time;
		temp->cpu_burst_time = Y->cpu_burst_time;
		temp->loading_time = Y->loading_time;
		temp->temp = Y->temp;
		temp->traffic_count = Y->traffic_count;
		tmp->T = temp;
		current_size++;
		if (current_size <= size) {
			tmp->priority = priority;
			if (front == NULL)
			{
				tmp->link = front;
				front = tmp;
			}
			else
			{
				q = front;
				while (q->link != NULL)
					q = q->link;
				tmp->link = q->link;
				q->link = tmp;
			}
		}
		else {
			std::cout << "AAAAAAAAAAAAAAAAAAAAAAAAAAA \n";
		}
	}

	/*
	what the routine does_______________________________________-
	deletes the front of the queue.


	_________________________________________________________
	critique:_____________________________________________
	simple sub-program, I have no critique of it.


	___________________________________
	*/
	void del()
	{
		node *tmp;
		if (front == NULL)
			std::cout << "Queue Underflow\n";
		else
		{
			current_size--;
			tmp = front;

			front = front->link;
			free(tmp);
		}
	}

	/*
	what the routine does_______________________________________-
	checks to see if the queue is empty or full.


	_________________________________________________________
	critique:_____________________________________________
	no critique, not part of original specifications


	___________________________________
	*/
	bool disk_check() {
		if (front == NULL) {
			return 0;
			std::cout << "Queue is empty\n";
		}
		else {
			return  1;
		}
	}
	/*
	what the routine does_______________________________________-
	transfers data from the top queue element to the PCB


	_________________________________________________________
	critique:_____________________________________________
	works for this setup.


	___________________________________
	*/
	struct PCB* transfertwo(struct PCB*Q) {

		node *ptr;
		ptr = front;
		if (front == NULL) {
			//disk_check = 0;
			std::cout << "Queue is empty\n";
		}
		else
		{
			Q = ptr->T;               //pass in attributes of PCB register and assign them values from this queue 
			Q->job_number = ptr->T->job_number;
			Q->job_class = ptr->T->job_class;
			Q->main_memory_needed = ptr->T->main_memory_needed;
			Q->maximum_run_time = ptr->T->maximum_run_time;
			Q->waiting_time = ptr->T->waiting_time;
			Q->priority = ptr->T->priority;

			ptr = ptr->link;

		}
		return Q;

	}
	/*
	what the routine does_______________________________________-
	prints a display of statistics. This is not used in Phase 2, In phase I it was used to print rejected jobs.
	The function has been left in, in case any last minute debugging is needed.


	_________________________________________________________
	critique:_____________________________________________
	no critique needed.


	___________________________________
	*/
	void display(std::ofstream &output)
	{
		node *ptr;
		ptr = front;
		if (front == NULL) {
			//disk_check = 0;

		}
		else
		{
			while (ptr != NULL)
			{
				std::cout << "priority "<<"job id  " << " job class   main memory needed   maximum run time " << std::endl;
				std::cout << ptr->T->priority<<"          "<<ptr->T->job_number << "         " << ptr->T->job_class <<
					"             " << ptr->T->main_memory_needed << "                 " << ptr->T->maximum_run_time << std::endl;
				ptr = ptr->link;
			}
		}
	}
	/*
	what the routine does_______________________________________-
	This is used in the disk. This checks each element in the node and increments the time associated eith the parameter
	which is a time quantum or the leftover cpu burst from a finishing job//note that this is the same function from 
	priority queue but since it is not loaded into a 	subqueue yet there is no need for the kflag element to detect
	if the element is running
	_________________________________________________________
	critique:_____________________________________________
	simple algorithm, no critique needed



	___________________________________
	*/
	void assign(int x) {
		node*ptr;
		ptr = front;
		while (ptr != NULL) {
			if (ptr->T->kflag == 0) {
				ptr->T->temp += x;
			}
			ptr = ptr->link;
		}
	}
	struct node*n() {
		return front;

	}
};
#pragma once
#include <iostream>

struct node
{
	int priority;

	struct PCB *T;
	struct node *link;
};


class Priority_Queue
{
private:
	node *front;
	

public:
	Priority_Queue()
	{

		front = NULL;

	}


	/*
	what the routine does_______________________________________-
	checks to see if priority queue is empty

	_________________________________________________________
	critique:_____________________________________________
	no critique.


	___________________________________
	*/
	bool queue_check() {
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
	inserts a node into the queue. it also takes PCB attributes and loads
	the queue with information.


	_________________________________________________________
	critique:_____________________________________________
	no critique.


	___________________________________
	*/
	void insert(int priority, struct PCB* Y)
	{

		node *tmp, *q;
		tmp = new node;
		struct PCB*temp;
		temp = new PCB;  //temp creates a PCB register for each node on the queue and stores current PCB information
						 //passed in from the parameters
		
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
		tmp->priority = priority;
		if (front == NULL || priority > front->priority)
		{
			tmp->link = front;
			front = tmp;

		}
		else
		{
			q = front;
			while (q->link != NULL && q->link->priority >= priority)
				q = q->link;
			tmp->link = q->link;
			q->link = tmp;
		
		}
	}
	/*
	what the routine does_______________________________________-
	checks the current time units of the balanced and cpu elements to see if it has reached 400 (or 600).
	If it has it updates the priority and resets the time units. See the PCB Block for more info 


	_________________________________________________________
	critique:_____________________________________________
	no critique.


	___________________________________
	*/
	int checktime(int kflag) {
		node*ptr;
		ptr = front;
		while (ptr != NULL) {
			
			if (ptr->T->temp >= 400&&kflag==2) {
				
				ptr->T->priority++;
				ptr->T->temp = 0;
			
				
			}
			else if (ptr->T->temp >= 600&&kflag==3) {
				ptr->T->priority++;
				ptr->T->temp = 0;
				
			}
			if (ptr->T->priority >= 5&&kflag==2) {
				return 1;
			}
			if (ptr->T->priority >= 5&&kflag==3) {
				return 2;
			}
			ptr = ptr->link;
		}
		return 0;
	}
	/*
	what the routine does_______________________________________-
	This is used in every queue. This checks each element in the node (which is specified in which queue at the subprogram call)
	and sees if a switch(kflag) has been set. if it has that means the current element is running and shouldnt increase in time.
	If it hasnt, it increases based on the time given. the time is either the time quantum or the left over cpu burst time
	if it is less than time quantum
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
	/*
	what the routine does_______________________________________-
deletes front of queue.
	_________________________________________________________
	critique:_____________________________________________
	the specifications are met here, nothing that does not work or needs fixed, or can be changed.



	___________________________________
	*/
	void del()
	{
		node *tmp;
		if (front == NULL)
			std::cout << "Queue Underflow\n";
		else
		{
			tmp = front;
			//std::cout << "Deleted item is: " << tmp->info << std::endl;
			front = front->link;
			free(tmp);
		}
	}
	
	
	/*
	what the routine does_______________________________________-
	displays queue. not used in final process .
	The function has been left in, in case any last minute debugging is needed.
	_________________________________________________________
	critique:_____________________________________________
	the specifications are met here, nothing that does not work or needs fixed, or can be changed.



	___________________________________
	*/
	void display(std::ofstream&output) {
		node *ptr;
		ptr = front;
		while (ptr != NULL) {
			std::cout << "priority " << "job id  " << " job class   main memory needed   maximum run time " << std::endl;
			std::cout << ptr->T->priority << "          " << ptr->T->job_number << "         " << ptr->T->job_class <<
				"             " << ptr->T->main_memory_needed << "                 " << ptr->T->maximum_run_time << "  " << ptr->T->temp<<
				std::endl;
			output<<  "priority " << "job id  " << " job class   main memory needed   maximum run time " << std::endl;
				output<< ptr->T->priority << "          " << ptr->T->job_number << "         " << ptr->T->job_class <<
					"             " << ptr->T->main_memory_needed << "                 " << ptr->T->maximum_run_time << "  " << ptr->T->temp <<
					std::endl;
			ptr = ptr->link;
		}
	
	
	}
	/*
	what the routine does_______________________________________-
	used rarely, only when a job has its priority raised to 5. This takes all elements from the queue and puts them
	into a temporary queue, deletes the main queue, then recreates a main queue, reinserts all elements, and returns
	the main queue back. This reorganizes the elements based on their priority, since the queue can only reprioritize
	typically on an insert, this method is used when that can't be done.
	_________________________________________________________
	critique:_____________________________________________
	the specifications are met here, nothing that does not work or needs fixed, or can be changed.



	___________________________________
	*/
	Priority_Queue re_prioritize(Priority_Queue pq) {
		node *tmp;
		tmp = pq.n();
		Priority_Queue temp;
		
		while (tmp != NULL)
		{
			temp.insert(tmp->T->priority,tmp->T);
			tmp = tmp->link;

		}
		while (pq.queue_check() != 0) {
			pq.del();
		}
		return temp;
	}
	/*
	what the routine does_______________________________________-
	returns first queue element.


	_________________________________________________________
	critique:_____________________________________________
	works pretty simply, no critique.


	___________________________________
	*/
	struct node*n() {
		return front;

	}

};
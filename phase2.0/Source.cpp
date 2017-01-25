struct PCB
{
	int priority;
	int job_number;  //job id
	int arrival_time = 0;
	int maximum_run_time;//processing time
	int main_memory_needed;   //requested memory needed
	int job_class; //class
	int memory_index;
	int turnaround_time = 0;
	int waiting_time = 0;
	int cpu_burst_time = 0;  //starts with same number as maximum run time, but decremented each time quantum
	int loading_time = 0;
	int temp = 0;  //stores the time the job has been waiting since it last ran. resets every 400 or 600 units depending on job
	int kflag = 0;    //used as a switch variavle to detect if current job is running or waiting
	int traffic_count = 1;
};

#include <iostream>
#include <vector>
#include <iomanip>
#include <sstream> 
#include <fstream>
#include "priority_queue.h"
#include "queue.h"
#include "Memory_manager.h"
#include <math.h>
/*
name: Bryan Reynolds
Course number: CS4323
Assignment title: Phase II
Date: 11/27/16

___________________________
Global Variables:
int counter    --counts all I/O Jobs. This controls the RR priority scheme, and  is decremented during the RR scheme
int balcounter  --counts all balanced jobs. This controls the RR priority scheme, and  is decremented during the RR scheme
int cpucounter  --counts all CPU-driven jobs. This controls the RR priority scheme, and  is decremented during the RR scheme
int countertwo  -- This is a copy of counter but does not decrement. used for final stats.
int balcountertwo --  This is a copy of counter but does not decrement. used for final stats.
int cpucountertwo  --This is a copy of counter but does not decrement. used for final stats.
int sumcounter  --This is a sum of the counts, used for averages and for total number of jobs
unsigned short rejectcounter   -----------------------------------------------------------------------------------
This stores the count of all rejected jobs. Due to a clearer understanding of what to show in termination and the need
for a reduced file size, the rejected_jobs queue and the display of all the rejected jobs in final calculations from
Phase I has been removed. The count of rejected jobs is stored instead and shown in final stats

---------------------------------------------------------------------------------------------------------------------
unsigned short job_counter  --this controls the termination and allows it to print a job for every 15 jobs
int CPU_overall_time -- Stores the CPU clock
_________________________________________________________________
description of main routine
main acts as the system in general, using a series of loops which in sequence do this:
1. check for file errors
2. as long as an entry 0 is not found, or end of file hasnt occured, call jsched to load jobs into
ready queue (the three priority queues). additional error detection can be found here in case the file is not found.
3.dispatch and terminate all jobs in the ready queue. After this, memory statistics are gathered and various job counters are
incremented.
4. load jobs from disk  into ready queue. if memory cannot be found, the jobs are dispatched and terminated, and more jobs are
loaded from disk. the scheduler's first priority is clearing the disk before getting more jobs.
this also keeps the limited size of the disk from becoming an issue.
After this second dispatch sequence, memory statistics are gathered again along with various job counters.
5. steps 1 through 4 are looped continously until end of file is reached
6. The cumulative results are printed


____________________________________
critique of the routine
Since this implementation is strictly adhered to guidelines as opposed to Phase I, there is not much room for error. Something I
would have liked is for better external fragmentation, although due to restrictions that the disk cannot starve and must get
priority before more jobs are loaded from the system there is not much room to adjust. This is exampled below--

The method of flushing the disk out after the input file can cause this, for example consider the disk queue showing this:
job 1 needs 73 KB
Job 2 needs 81 KB
job 3 needs 66 KB

Since all three jobs have to use the largest parition, each job runs individually, causing a larger amount of external fragmentation.
Fortunately this does not happed as often as worst case would prove,  and quite often the memory
is actually fully used and there is no externalfragmentation on some dispatches. This affects the average statistically,
but worst case complexity only occurs during disk loading into the scheduler and when it consists of solely large jobs.


Internal fragmentation is below 12(rounded to 11 in testing). This is quite good in terms of fragmentation as far as coding in my opinion.
A first-fit algorithm was used rather than best fit, and has proven successful in this regard.


*/


int trim(int x);

void inputsub(std::ifstream& myfile, struct PCB* Q, Priority_Queue & pq, int&error, int&errortwo, int&errorthree);

void RR_priority(Priority_Queue&pq, Priority_Queue&balanced, Priority_Queue&CPU,Regular_Queue&lq, int & time,
	Memory & memory, double & average_turnaround,double & average_waiting, std::ofstream&output, int &remainone);

void RR_balanced(Priority_Queue&pq, Priority_Queue&balanced, Priority_Queue&CPU,Regular_Queue&lq, int & time,
	Memory & memory, double & average_turnaround, double & average_waiting,
	std::ofstream&output, int &remainone, int&remaintwo, int&kflag);

void RR_cpu(Priority_Queue&pq, Priority_Queue&balanced, Priority_Queue&CPU, Regular_Queue&lq, int & time, Memory & memory,
	double & average_turnaround, double & average_waiting, std::ofstream&output, int &remainone, int&remaintwo,
	int&remainthree, int &kflag);

void J_SCHED(std::ifstream& myfile, struct PCB*Q, Priority_Queue & pq, Priority_Queue&bq,
	Priority_Queue&cq, Memory & memory, Regular_Queue & lq, short kflag, std::ofstream & output, int&joberror,
	int&queue_error, int&error, int&errortwo, int&errorthree);

void J_DISPATCH(struct PCB*Q, Priority_Queue & pq,
	Priority_Queue&balanced, Priority_Queue&CPU, Regular_Queue & lq,
	double &average_turnaround, double & average_waiting, Memory &memory, std::ofstream & output);

void J_TERM(struct PCB*Q, Priority_Queue& pq, Memory &mem_manager, double& average_turnaround,
	double& average_waiting, std::ofstream&output);

void CPU_subprogram(struct PCB*Q, short kflag, int time_quantum);

void print_Results(double internal_fragmentation, double external_fragmentation, Memory &memory, double average_turnaround,
	double average_waiting, std::ofstream&output);

int counter = 0;
int balcounter = 0;
int cpucounter = 0;
int countertwo = 0;
int balcountertwo = 0;
int cpucountertwo = 0;
int sumcounter = 0;
unsigned short rejectcounter = 0;
unsigned short job_counter = 0;
int CPU_overall_time = 0;


int main()
{
	int queue_error = 0; //both queue error and joberror prevent duplicate jobs from running. this can also be caused by newline
						 //characters in the input file after the data. this prevents either issue. more information is 
						 //shown in J_SCHED
	int joberror = 0;
	double internal_fragmentation = 0;
	double external_fragmentation = 0;
	int error = 0;     //error, errortwo, and errorthree are used to detect 0 entries, a disk that finished loading, or input errors
	int errortwo = 0;
	int errorthree = 0;
	Regular_Queue lq(300);  //This represents the Disk 

	Memory memory(26);
	CPU_overall_time = 0;
	memory.setup();
	std::string text = "Project.txt";
	std::string text2 = "Project2.txt";
	std::string text3 = "sample.txt";
	std::string text4 = "/home/opsys/OS-I/16F-jobs";
	std::string text5 = "sim.txt";
	std::string custom = "";

	
	std::ofstream output("logfile.txt", std::ofstream::out);
	struct PCB * pcb;
	//___________________________________________________
	std::cout << "README: \n This simulator simulates job processing and scheduling using Round Robin algorithms and" <<
		"multi-level priority feedback queues along with simulated memory (800K)" <<
		"using best-fit algorithm and a partition table, along with a secondary memory that will house 300 jobs. This " <<
		"simulates job scheduling in an operating system with optimized results" << std::endl;
		
	std::cout << "make sure data file has the following format\n";
	std::cout << "1 2 320 2\n2 5 10 2\n";
	std::cout << "First entry is the job number, second entry is either 1,2, or 3 (CPU, balanced, or mixed job)\n";
	std::cout << "third entry is main memory needed, fourth entry is maximum run time\n\n";
	std::cout << "enter name of data file along with extension (make sure it is in Phase 2 Folder,otherwise type location of file as well\n";
	std::cin >> custom;
	std::ifstream myfile(custom);
	std::cout << "Individual job statistics are now in logfile.txt in folder\n";



	//______________________________________________
	pcb = (struct PCB *)malloc(sizeof(struct PCB));

	Priority_Queue pq;        //I/O priority queue
	Priority_Queue balanced; //balanced
	Priority_Queue CPU;      //cpu priority queue
	double average_waiting = 0;
	double average_turnaround = 0;
	while (!myfile.eof()) {
		error = 0;
		while (!myfile.eof() && errortwo != 1) {
			J_SCHED(myfile, pcb, pq, balanced, CPU, memory, lq, 0, output, joberror, queue_error, error, errortwo, errorthree);
			if (pcb->job_number < 0) {
				std::cout << "\nerror detected!!!\n file corruption/ read error\n";
				return 0;
			}
		}
		
		balcountertwo += balcounter;
		countertwo += counter;
		cpucountertwo += cpucounter;
		sumcounter += (balcounter + counter + cpucounter);
		//---------------------- Dispatch jobs from input file
		J_DISPATCH(pcb, pq, balanced, CPU, lq, average_turnaround, average_waiting, memory, output);
		//---------------------- gather fragmentation statistics
		memory.count_of_runs++;
		memory.stats(internal_fragmentation, external_fragmentation, output);
		memory.kflag = 1;
		memory.stats(internal_fragmentation, external_fragmentation, output);
		errorthree = 0;
		//______________________________loads disk onto queue, if memory is full, dispatch and terminate until disk is clear

		while (lq.disk_check() == true && errorthree == 0) {
			J_SCHED(myfile, pcb, pq, balanced, CPU, memory, lq, 1, output, joberror, queue_error, error, errortwo, errorthree);
			if (errorthree == 1 || lq.disk_check() == false) {
				balcountertwo += balcounter;
				countertwo += counter;
				cpucountertwo += cpucounter;
				sumcounter += (balcounter + counter + cpucounter);
				//---------------------gather fragmentation statistics
				memory.count_of_runs++;
				memory.stats(internal_fragmentation, external_fragmentation, output);
				memory.kflag = 1;  //runs a different set of code for the stats function. 
				memory.stats(internal_fragmentation, external_fragmentation, output);
				J_DISPATCH(pcb, pq, balanced, CPU, lq, average_turnaround, average_waiting, memory, output);
				errorthree = 0;
			}
		}
		//_________________________________________________________________
		errortwo = 0;
		errorthree = 0;       //reset errors to zero
	}
	print_Results(internal_fragmentation, external_fragmentation, memory, average_turnaround, average_waiting, output);

}
/*
what the routine does_______________________________________ -
Prints final statistics


_________________________________________________________
critique : _____________________________________________
the specifications are met here, nothing that does not work or needs fixed, or can be changed.


___________________________________
*/
void print_Results(double internal_fragmentation, double external_fragmentation, Memory &memory, double average_turnaround,
	double average_waiting, std::ofstream&output) {


	std::cout << "\n\nfinal statistics: \n";
	output << "\n\nfinal statistics: \n";


	std::cout << "\nsimulation complete \n";
	std::cout << "total number of jobs processed " << sumcounter << "\n";
	std::cout << "number of I/O jobs " << countertwo << "\n";
	std::cout << "number of balanced jobs " << balcountertwo << "\n";
	std::cout << "number of CPU jobs: " << cpucountertwo << '\n';
	std::cout << "average turnaround time: " << round(average_turnaround / sumcounter) << '\n';
	std::cout << "average waiting time: " << round(average_waiting / sumcounter) << '\n';
	std::cout << "Clock value at termination: " << CPU_overall_time << '\n';
	output << "total number of jobs processed " << sumcounter << "\n";
	output << "number of I/O jobs " << countertwo << "\n";
	output << "number of balanced jobs " << balcountertwo << "\n";
	output << "number of CPU jobs: " << cpucountertwo << '\n';
	output << "average turnaround time: " << round(average_turnaround / sumcounter) << '\n';
	output << "average waiting time: " << round(average_waiting / sumcounter) << '\n';
	output << "Clock value at termination: " << CPU_overall_time << '\n';
	std::cout << "Average external fragmentation: " <<
		round(external_fragmentation / memory.count_of_runs) << "KB" <<
		"\nAverage Internal Fragmentation: " << round(internal_fragmentation / sumcounter) << "KB" << '\n';
	output << "Number of rejected jobs: " << rejectcounter << '\n';
	std::cout << "Number of rejected jobs: " << rejectcounter << '\n';
	output << "Average external fragmentation: " << round(external_fragmentation / memory.count_of_runs) << "KB" <<
		"\nAverage Internal Fragmentation: " << round(internal_fragmentation / sumcounter) << "KB" << '\n';

}
/*
what the routine does_______________________________________-
terminates a job, releases it form memory, and prints it(every one out of 15 is printed only).

The "setw(number) function is used to align and organize the columns and numbers. Due to limits of the
setw function, a function I created (trim) is used within this function to control the spacing based on the
length of the numbers.

Differing from Phase I is the more simplistic coding, and the recreation of the table of statistics for readability.
Because of the large amount of information, each job termination is now broken down into 3 lines of information, for example:

id                    class              submit                        load
5                     1                  865                           865
termination           processing         turnaround                    waiting
965                   40                 100                           60
traffic count         priority
0                     5

id                    class              submit                        load
9                     3                  865                           1877
termination           processing         turnaround                    waiting
2024                  147                1159                          1012
traffic count         priority
0                     0


each job will be seperated with two newline characters
_________________________________________________________
critique:_____________________________________________
the specifications are met here, nothing that does not work or needs fixed, or can be changed.
Much better organization compared to Phase I.



___________________________________
*/
void J_TERM(struct PCB*Q, Priority_Queue& pq, Memory &mem_manager, double& average_turnaround,
	double& average_waiting, std::ofstream&output) {

	job_counter++;

	mem_manager.memrelease(Q->memory_index);
	average_turnaround += Q->turnaround_time;
	average_waiting += Q->waiting_time;

	if (job_counter >= 15) {
		output << "id" << std::setw(20) << "class" << std::setw(15) << "submit" << std::setw(23) << "load" << '\n';
		output << Q->job_number << std::setw(17 - trim(Q->job_number) + trim(Q->job_class)) << Q->job_class <<
			std::setw(15 + trim(Q->arrival_time))
			<< Q->arrival_time << std::setw(30 - trim(Q->arrival_time) + trim(Q->loading_time)) << Q->loading_time << '\n';


		output << "termination" << std::setw(16) << "processing" << std::setw(14) << "turnaround" << std::setw(22) << "waiting" << '\n';
		output << CPU_overall_time << std::setw(17 - trim(CPU_overall_time) + trim(Q->maximum_run_time)) << Q->maximum_run_time <<
			std::setw(14 - trim(Q->maximum_run_time) + trim(Q->waiting_time)) << Q->turnaround_time
			<< std::setw(26 - trim(Q->turnaround_time) + trim(Q->waiting_time)) << Q->waiting_time << std::endl ;
		output << "traffic count" << std::setw(12) << "priority" << '\n';
		output << Q->traffic_count << std::setw(17) << Q->priority << "\n\n";
		job_counter = 0;
	}
	pq.del();


}
/*
what the routine does_______________________________________ -
Described briedly above in J_TERM. This controls the spacing between characters to help align columns and data

_________________________________________________________
critique : _____________________________________________
Works perfectly. Incredibly increased the readability  and fixes the issue posed in phase I. Phase I had
an issue where the numbers would grow and misalign with the columns, along with returning on multiple lines.



___________________________________
*/
int trim(int x) {
	x = x / 10;
	if (x < 1) {}
	else if (x >= 1 && x < 10) {
		return 0;
	}
	else if (x >= 10 && x < 100) {
		return 1;
	}
	else if (x >= 100 && x < 1000) {

		return 2;
	}
	else if (x >= 1000 && x < 10000) {
		return 3;
	}
	else if (x >= 10000 && x < 100000) {
		return 4;
	}
	return 0;
}
/*
what the routine does_______________________________________ -
The J_Dispatch Routine contained a lot of code so in order to make it modular it has been broken down into code
that works individually for each subqueue of the ready queue. This is for the priority queue

_________________________________________________________
critique : _____________________________________________
Works perfectly. J_TERM is called when a job has finished dispatching. the jobs reprioritize with every new job that
runs. This keeps higher priority jobs running first



___________________________________
*/
void RR_priority(Priority_Queue&pq, Priority_Queue&balanced, Priority_Queue&CPU, Regular_Queue&lq,int & time, Memory & memory, 
	double & average_turnaround,double & average_waiting,std::ofstream&output, int &remainone) {
	int time_quantum = 20;
	int flag = 0; //switch used to terminate job rather than decrease time quantum continously
	node*ptr;        //pointer that points to front of queue
	ptr = pq.n();
	int n = counter;          //n is the number of jobs in queue so the loop can decide which direction to go with
							  // the current job.  this is used along with count to decide when the end has been reached.
	remainone = n;    //this is equal to n, but is not used with count to decide which section of the loop to
					  //process, rather it is used to end the loop when reached
	int count = 0;
	int kflag = 0;
	while (remainone != 0&&kflag!=1)
	{
		
		if (ptr->T->cpu_burst_time <= time_quantum && ptr->T->cpu_burst_time > 0)
		{
			lq.assign(ptr->T->cpu_burst_time);
			balanced.assign(ptr->T->cpu_burst_time);
			CPU.assign(ptr->T->cpu_burst_time);
			time += ptr->T->cpu_burst_time;
			CPU_subprogram(ptr->T, 0, 0);
			ptr->T->cpu_burst_time = 0;
			flag = 1;        //if a job has <= time quantum it is processed for termination
		}
		else if (ptr->T->cpu_burst_time > 0)
		{
			lq.assign(time_quantum);
			balanced.assign(time_quantum);
			CPU.assign(time_quantum);
			ptr->T->cpu_burst_time -= time_quantum;
			time += time_quantum;
			CPU_subprogram(ptr->T, 1, time_quantum); //decrements time quantum from burst time
		}
		if (flag == 1)          //termination loop
		{
			remainone--;
			counter--;
			ptr->T->turnaround_time = time - ptr->T->arrival_time;
			ptr->T->waiting_time = time - ptr->T->arrival_time - ptr->T->maximum_run_time;
			J_TERM(ptr->T, pq, memory, average_turnaround, average_waiting, output);
			ptr = pq.n();
			n--;
			if (count > n) {     //count can increase greater than n depending on process. this prevents it.
				count = n;
			}
		}
		if (count == n) {     //reinsert queue item to put it at the end and prepare for new job. This reprioritizes jobs as well
							  //moving them in the appropriate spots
			count = 0;
			if (flag == 0) {
				pq.insert(ptr->T->priority, ptr->T);
				pq.del();
			}
			ptr = pq.n();
			flag = 0;
		}
		else {
			count++;
			if (flag == 0) {
				pq.insert(ptr->T->priority, ptr->T);
				pq.del();
			}
			ptr = pq.n();
			flag = 0;
		}
	
		
		if (balanced.checktime(2)==1) {  //if a job reaches priority 5 it is reprioritized so it moves to the front of the
									  //queue, then is popped off and pushed into I/O queue
		
			balanced = balanced.re_prioritize(balanced);
			ptr = balanced.n();

			balcounter--;
			n++;
			remainone++;
			ptr->T->traffic_count++;
			pq.insert(5, ptr->T);
			ptr = pq.n();
			balanced.del();
			kflag = 1;
		
		}
		if (CPU.checktime(3) == 2) {  //if a job reaches priority 5 it is reprioritized so it moves to the front of the
										   //queue, then is popped off and pushed into I/O queue
			
			CPU = CPU.re_prioritize(CPU);
			ptr = CPU.n();

			cpucounter--;
			n++;
			remainone++;
			ptr->T->traffic_count++;
			pq.insert(5, ptr->T);
			ptr = pq.n();
			balanced.del();
			kflag = 1;
		
		}
	}
}
/*
what the routine does_______________________________________ -
The J_Dispatch Routine contained a lot of code so in order to make it modular it has been broken down into code
that works individually for each subqueue of the ready queue. This is for the balanced queue. Works differently since
It changes priorities unlike the IO queue

_________________________________________________________
critique : _____________________________________________
Works perfectly. J_TERM is called when a job has finished dispatching. the jobs reprioritize with every new job that
runs. This keeps higher priority jobs running first



___________________________________
*/
void RR_balanced(Priority_Queue&pq, Priority_Queue&balanced, Priority_Queue&CPU,Regular_Queue&lq, int & time, Memory & memory,
	double & average_turnaround, double & average_waiting,std::ofstream&output, int &remainone, int&remaintwo, int&kflag) {
	int time_quantum = 40;
	int flag = 0;
	node*ptr;
	int n;
	int count = 0;
	int temp = 0; //holds a temporary clock to deteck when 400 units has been reached to raise priority
	ptr = balanced.n();
	remaintwo = n = balcounter;

	while (remaintwo != 0 && remainone == 0 && kflag != 1)
	{

		while (ptr != NULL) {
			if (ptr->T->priority != 5) {
				if (ptr->T->temp >= 400) {
					ptr->T->priority++;
					ptr->T->temp = 0;
				}
			
			}
			if (ptr->T->priority == 5) {  //if a job reaches priority 5 it is reprioritized so it moves to the front of the
										  //queue, then is popped off and pushed into I/O queue
				balanced = balanced.re_prioritize(balanced);

				ptr = balanced.n();
				remaintwo--;
				balcounter--;
				n--;
				remainone++;
				ptr->T->traffic_count++;
				pq.insert(5, ptr->T);
				ptr = ptr->link;
				balanced.del();
				kflag = 1;
			}
			ptr = ptr->link;
		}
	
		ptr = balanced.n();
		balanced = balanced.re_prioritize(balanced);
		ptr = balanced.n();

		
		if (ptr != NULL&&kflag != 1) {
			if (ptr->T->cpu_burst_time <= time_quantum && ptr->T->cpu_burst_time > 0)
			{
				ptr->T->temp = 0;
				ptr->T->kflag = 1;
				lq.assign(ptr->T->cpu_burst_time);
				balanced.assign(ptr->T->cpu_burst_time); //adds time to temp variable. check PCB block at top of code for more info
				CPU.assign(ptr->T->cpu_burst_time);
				ptr->T->kflag = 0;
				time += ptr->T->cpu_burst_time;
				temp += ptr->T->cpu_burst_time;
				ptr->T->temp += ptr->T->cpu_burst_time;
				CPU_subprogram(ptr->T, 0, 0);
				ptr->T->cpu_burst_time = 0;
				flag = 1;
			}
			else if (ptr->T->cpu_burst_time > 0)
			{
				ptr->T->temp = 0;
				ptr->T->kflag = 1;
				lq.assign(time_quantum);
				balanced.assign(time_quantum);
				CPU.assign(time_quantum);
				ptr->T->kflag = 0;
				ptr->T->cpu_burst_time -= time_quantum;
				time += time_quantum;
				temp += time_quantum;
				ptr->T->temp += ptr->T->cpu_burst_time;
				CPU_subprogram(ptr->T, 1, time_quantum);
				if (ptr->T->priority > 1) {
					ptr->T->priority -= 1;  //every time a job runs but not terminates its priority decreases per instructions
				}
			}
			if (flag == 1)
			{
				remaintwo--;
				balcounter--;
				ptr->T->turnaround_time = time - ptr->T->arrival_time;
				ptr->T->waiting_time = time - ptr->T->arrival_time - ptr->T->maximum_run_time;
				J_TERM(ptr->T, balanced, memory, average_turnaround, average_waiting, output);
				ptr = balanced.n();
				n--;
				if (count > n) { //count can increase greater than n depending on process. this prevents it.
					count = n;
				}
			}
			if (count == n) {
				count = 0;
				//reinsert queue item to put it at the end and prepare for new job. This reprioritizes jobs as well
				//moving them in the appropriate spots
				if (flag == 0) {
					balanced.insert(ptr->T->priority, ptr->T);
					balanced.del();
				}
				ptr = balanced.n();
				flag = 0;
			}

			else {
				count++;
				if (flag == 0) {
					balanced.insert(ptr->T->priority, ptr->T);
					balanced.del();
				}
				ptr = balanced.n();
				flag = 0;
			}
		}
	}

}

/*
what the routine does_______________________________________ -
The J_Dispatch Routine contained a lot of code so in order to make it modular it has been broken down into code
that works individually for each subqueue of the ready queue. This is for the CPU queue. Works differently since
It changes priorities unlike the IO queue

_________________________________________________________
critique : _____________________________________________
Works perfectly. J_TERM is called when a job has finished dispatching. the jobs reprioritize with every new job that
runs. This keeps higher priority jobs running first



___________________________________
*/
void RR_cpu(Priority_Queue&pq, Priority_Queue&balanced, Priority_Queue&CPU, Regular_Queue&lq,int & time, Memory & memory,
	double & average_turnaround,double & average_waiting, std::ofstream&output, int &remainone, int&remaintwo, 
	int&remainthree, int &kflag) {
	int time_quantum = 75;
	int flag = 0;
	node*ptr;
	int n;
	int count = 0;
	int temp = 0;
	ptr = CPU.n();

	remainthree = n = cpucounter;
	while (remainthree != 0 && remainone == 0 && remaintwo == 0 && kflag != 1)
	{
		//every 600 units priority increases for each job
		if (kflag != 1) {
			while (ptr != NULL) {
				if (ptr->T->priority != 5) {
					if (ptr->T->temp >= 600) {
						ptr->T->priority++;
						ptr->T->temp = 0;
					}

				}
				if (ptr->T->priority == 5) {
					
					CPU = CPU.re_prioritize(CPU);  //move to IO queue if priority = 5
					ptr = CPU.n();
					remainthree--;
					n--;
					cpucounter--;
					ptr->T->traffic_count++;
					pq.insert(5, ptr->T);
					remainone++;
					ptr = ptr->link;
					CPU.del();
					kflag = 1;


				}
				ptr = ptr->link;
			}
		}
		ptr = CPU.n();


		
		if (ptr != NULL && kflag != 1) {
			if (ptr->T->cpu_burst_time <= time_quantum && ptr->T->cpu_burst_time > 0)
			{
				ptr->T->temp = 0;
				ptr->T->kflag = 1;
				balanced.assign(ptr->T->cpu_burst_time);
				CPU.assign(ptr->T->cpu_burst_time);
				ptr->T->kflag = 0;
				time += ptr->T->cpu_burst_time;
				temp += ptr->T->cpu_burst_time;
				ptr->T->temp += ptr->T->cpu_burst_time;
				CPU_subprogram(ptr->T, 0, 0);
				ptr->T->cpu_burst_time = 0;
				flag = 1;
			}
			else if (ptr->T->cpu_burst_time > 0)
			{
				ptr->T->temp = 0;
				ptr->T->kflag = 1;
				balanced.assign(time_quantum);
				CPU.assign(time_quantum);
				ptr->T->kflag = 0;
				ptr->T->cpu_burst_time -= time_quantum;
				time += time_quantum;
				temp += time_quantum;
				ptr->T->temp += ptr->T->cpu_burst_time;
				CPU_subprogram(ptr->T, 1, time_quantum);
				if (ptr->T->priority > 0) {        //decrease priority if job gets a chance to run but does not terminate
					ptr->T->priority -= 1;
				}
			}
			if (flag == 1)
			{
				ptr->T->turnaround_time = time - ptr->T->arrival_time;
				ptr->T->waiting_time = time - ptr->T->arrival_time - ptr->T->maximum_run_time;
				cpucounter--;
				remainthree--;
				J_TERM(ptr->T, CPU, memory, average_turnaround, average_waiting, output);
				ptr = CPU.n();
				n--;
				if (count > n) {
					count = n;
				}
			}
			if (count == n) {
				count = 0;
				ptr = CPU.n();
				flag = 0;
			}
			else {
				count++;
				if (flag == 0) {
					CPU.insert(ptr->T->priority, ptr->T);
					CPU.del();
				}
				ptr = CPU.n();
				flag = 0;
			}
		}
	}
}
/*
what the routine does_______________________________________ -
The J_Dispatch Routine runs all three RR routines from above inside a loop. This loop runs multiple times, as the priority
of some jobs reach 5, they are pushed into the IO queue. Since the IO queue has to be dispatched first the loops repeats.
A switch(kflag) skips over the other functions if a high priority 5 job is formed. The priority queue  has its own
kflag which helps detect balanced and cpu jobs increasing priority, but since it is the main priority queue and the highest
it does not need to be skipped. Jobs stay within their respective queues unless a priority of 5 is reached. . A job with a 
higher priority, such as "3" in the balanced subqueue, will be dispatched over jobs with priority 2 or lower. The queue is resorted 
based on priority each dispatch . Because of higher jobs being ran first, it is hard for a job to reach a priority of 5 to change subqueues
(see critique below)

_________________________________________________________
critique : _____________________________________________
contains heavy code ifyou include the subprograms, but does everything it needs to. The critique is this:
After a priority reaches 5, it is supposed to change queues. The problem is, with the limited memory
and the way that the disk is depleted before more input jobs occur, this does not happen.
When the priority reaches for example, 2 in the balanced queue, it is given higher priority among other jobs and is treated
within its respective queue first among other priority 2 jobs (unless a priority 5 is reached)
. Because of this, in order to reach a priority of 5, it not only has to wait for 1600 time units before
being ran (in balanced queue) despite the decrementing every time a job runs but doesnt terminate along with multiple jobs of similar 
rising priority (run alongside another 2 or 3 priority, etc.)  The round robin scheme has been tested with
 fake priorities to test if the jobs switch queues correctly and they do. if very time-heavy jobs and a lot of them consumed
 the priority queue  long enough for balanced jobs or CPU jobs to reach a high priority without decrementing, this could be acheived.
 The highest priority jobs reach as far as a test runs suggest is a 4, although a 2 is more common, and 3 is seen more often.



___________________________________
*/
void J_DISPATCH(struct PCB*Q, Priority_Queue & pq,
	Priority_Queue&balanced, Priority_Queue&CPU, Regular_Queue & lq,
	double &average_turnaround, double & average_waiting, Memory &memory, std::ofstream & output) {
	int kflag = 0;
	int  time = 0;
	int remainone = 0, remaintwo = 0, remainthree = 0;
	time = CPU_overall_time;

	while (pq.queue_check() != 0 || balanced.queue_check() != 0 || CPU.queue_check() != 0) {

		RR_priority(pq,balanced,CPU,lq, time, memory, average_turnaround, average_waiting, output, remainone);
		kflag = 0;
		RR_balanced(pq, balanced,CPU,lq, time, memory, average_turnaround, average_waiting, output, remainone, remaintwo, kflag);
		RR_cpu(pq, balanced, CPU,lq, time, memory, average_turnaround, average_waiting, output, remainone, remaintwo, remainthree, kflag);
	}
}
/*
what the routine does_______________________________________-
Quite a simple routine. This is the CPU from Phase I. It is called CPU_subprogram to create less confusion with the CPU clock
and the CPU queue, but it is the cpu. Just like the instructions from Phase I, carries address of PCB and increments clock
by runtime (or dispatched time since RR is used).
A switch(kflag) is used to show whether the full time quantum is being added to the CPU or a number below the time_quantum
_________________________________________________________
critique:_____________________________________________
the specifications are met here, nothing that does not work or needs fixed, or can be changed.



___________________________________
*/
void CPU_subprogram(struct PCB*Q, short kflag, int time_quantum) {

	//points to address of PCB passed in.
	if (kflag == 0) {
		CPU_overall_time += Q->cpu_burst_time;
	}
	else
		CPU_overall_time += time_quantum;	//the job may have dispatched partially in RR scheme but not terminated, 
											//so CPU increments by time quantum instead
}
/*
what the routine does_______________________________________-
Uses a switch (kflag) to dictate whether the scheduler is scheduling input jobs or jobs from the disk.
Loads jobs into their appropriate queues to be dispatched later. This also assigns memory locations to each job.
various error detection methods are used.
_________________________________________________________
critique:_____________________________________________
the specifications are met here, nothing that does not work or needs fixed, or can be changed.



___________________________________
*/
void J_SCHED(std::ifstream& myfile, struct PCB*Q, Priority_Queue & pq, Priority_Queue&bq,
	Priority_Queue&cq, Memory & memory, Regular_Queue & lq, short kflag, std::ofstream & output, int&joberror,
	int&queue_error, int&error, int&errortwo, int&errorthree) {

	short memory_index = 0;

	if (kflag == 0) { //kflag chooses whether the data is coming from an input file or disk
		inputsub(myfile, Q, pq, error, errortwo, errorthree);

		if (error != 1) {
			if (errortwo == 0) {  //errortwo is a 0 or memory error job. when a job is a 0 entry or requires more memory than present  
								  //in system, it is tossed 
				if (Q->job_number == joberror) {  //checks for duplicate entries of jobs. This can sometimes occur if a newline character is
												  //entered after the data on the input file. This prevents it and tosses it.
					queue_error = 1;
				}
				joberror = Q->job_number;
				if (queue_error == 0) {   //check to make sure a duplicate entry does not exist.
					if (memory.checkmem(Q->main_memory_needed, memory_index) == true) {
						memory.aquiremem(Q->main_memory_needed, memory_index, output);
						Q->memory_index = memory_index;
						Q->loading_time = CPU_overall_time;
						if (Q->job_class == 1) {
							Q->priority = 5;
							pq.insert(5, Q);
						}
						else if (Q->job_class == 2) {
							Q->priority = 1;
							bq.insert(1, Q);
						}
						else {
							Q->priority = 0;
							cq.insert(0, Q);
						}
					}
					else {
						if (Q->job_class == 1) {
							counter--;
						}
						else if (Q->job_class == 2) {
							balcounter--;
						}
						else {
							cpucounter--;
						}
						lq.insert(Q);
						//disk insert. notice above how the individual counters decrement since they are not being dispatched
					}
				}
			}
			else {
				errortwo = 0;
			}
		}
	}
	else {
		//______________________________-This part of the scheduler pulls from the disk rather than an input file
		if (lq.disk_check() == true) {
			Q = lq.transfertwo(Q); //transfers data of current job into the current PCB

								   //error checking is not needed here like it is above, mainly since it has already been checked before disk is unloaded.
			if (memory.checkmem(Q->main_memory_needed, memory_index) == true) {  //check is memory is free (a false means free)
				memory.aquiremem(Q->main_memory_needed, memory_index, output);  //aquires memory, returning true in the process. memory index stores memory location
				Q->memory_index = memory_index;
				Q->loading_time = CPU_overall_time; //stores loading time
				if (Q->job_class == 1) {
					Q->priority = 5;
					pq.insert(5, Q);
					counter++;
				}
				else if (Q->job_class == 2) {
					Q->priority = 1;
					bq.insert(1, Q);
					balcounter++;
				}
				else {
					Q->priority = 0;
					cq.insert(0, Q);
					cpucounter++;
				}
				lq.del();    //delete front of disk
			}
			else {
				errorthree = 1;  //switches to show that memory is full, and a dispatch and term needed
			}
		}
	}
}
/*
what the routine does_______________________________________-
Reorganized differently from Phase I, because it had trouble reading data with the newly formed J_SCHED fuction.
It reads the data from the input file, detects errors with the file and rejects them, and loads them
through J_SCHED(J_SCHED called inputsub)
critique:_____________________________________________
the specifications are met here, nothing that does not work or needs fixed, or can be changed.



___________________________________
*/
void inputsub(std::ifstream& myfile, struct PCB* Q, Priority_Queue & pq, int&error, int&errortwo, int&errorthree) {
	std::string line;
	std::string token;

	if (!myfile) {  //check to make sure a file exists
		std::cout << "cannot find files \n";
	}
	if (myfile.eof()) {

		errortwo = 1;
	}
	else {
		try {
			while (std::getline(myfile, line, '\n'))
			{
				std::istringstream iss(line);
				int first_on_line;  //job id
				int second_on_line;  //job class
				int third_on_line; //main memory needed
				int fourth_on_line;    //processing time
				if (!myfile.eof()) {
					if (iss >> first_on_line >> second_on_line >> third_on_line >> fourth_on_line) {
						if (second_on_line != 0 && third_on_line <= 128) {  //check for a 0 entry or an odd memory request
							Q->job_number = first_on_line;
							Q->job_class = second_on_line;
							Q->main_memory_needed = third_on_line;
							Q->maximum_run_time = fourth_on_line;
							Q->arrival_time = CPU_overall_time;
							Q->cpu_burst_time = fourth_on_line;
							Q->turnaround_time = 0;
							Q->waiting_time = 0;
							Q->temp = 0;
							Q->kflag = 0;
							Q->traffic_count = 1;
							if (Q->job_class == 1) {
								counter++;
							}
							else if (Q->job_class == 2) {
								balcounter++;
							}
							else {
								cpucounter++;
							}
						}
						else {
							rejectcounter++;
							errortwo = 1;
							error = 1;
						}
					}
				}
				else {
					if (iss >> first_on_line >> second_on_line >> third_on_line >> fourth_on_line) {

						if (second_on_line != 0 && third_on_line <= 128) {  //check for a 0 entry or an odd memory request
							Q->job_number = first_on_line;
							Q->job_class = second_on_line;
							Q->main_memory_needed = third_on_line;
							Q->maximum_run_time = fourth_on_line;
							Q->arrival_time = CPU_overall_time;
							Q->cpu_burst_time = fourth_on_line;
							if (Q->job_class == 1) {
								counter++;
							}
							else if (Q->job_class == 2) {
								balcounter++;
							}
							else {
								cpucounter++;
							}
						}
						else {
							error = 1;
							errortwo = 1;
						}
					}
					else {
						errortwo = 0;
						rejectcounter++;
					}
				}
				throw 1;
			}
		}
		catch (...) {

		}
	}
}
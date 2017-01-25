
class Dispatcher {
private:
	

public:



	void priority_RR(int time, int remainone,Priority_Queue&pq,std::ofstream & output,
		Memory &memory, double &average_turnaround, double &average_waiting, int counter,
		int CPU_overall_time,void CPU_subprogram(struct PCB*Q, short kflag, int time_quantum), void J_TERM(struct PCB*Q, Priority_Queue& pq, Memory &mem_manager, double& average_turnaround,
			double& average_waiting, std::ofstream&output)) {
		node*ptr;
		int count = 0;
		int time_quantum = 20;
		int kflag = 0;
		int flag = 0;
		int n = counter;

		int temp;
		int kkflag = 0;

		time = CPU_overall_time;
		temp = 0;
		count = 0;
		remainone = n;
		//n = 3;
		flag = 0;
		
		n = remainone;
		ptr = pq.n();

		//pq.display(output);
		count = 0;
		//	std::cout << "job number " << ptr->T->job_number << "  n " << n << "count: " << count << " " << remainone << '\n';
		printf("\n\nPoooorocess\t|Turnaround Time|Waiting Time\n\n");
		output << "\n\nPoooorocess\t|Turnaround Time|Waiting Time\n\n";
		while (remainone != 0)
		{
			std::cout << "L" << '\n';
			std::cout << ptr->T->maximum_run_time;
			if (ptr->T->cpu_burst_time <= time_quantum && ptr->T->cpu_burst_time > 0)
			{
				//std::cout << "A" << '\n';
				time += ptr->T->cpu_burst_time;
				CPU_subprogram(ptr->T, 0, 0);
				ptr->T->cpu_burst_time = 0;
				flag = 1;
			}
			else if (ptr->T->cpu_burst_time > 0)
			{
				//std::cout << "B" << ptr->T->cpu_burst_time << '\n';
				ptr->T->cpu_burst_time -= time_quantum;
				time += time_quantum;
				CPU_subprogram(ptr->T, 1, time_quantum);
				std::cout << "B" << ptr->T->cpu_burst_time << '\n';

			}
			if (flag == 1)
			{
				remainone--;
				counter--;
				//std::cout << "EEE" << '\n';
				ptr->T->turnaround_time = time - ptr->T->arrival_time;
				ptr->T->waiting_time = time - ptr->T->arrival_time - ptr->T->maximum_run_time;
				printf("P[%d]\t|\t%d\t|\t%d\n", ptr->T->job_number, time - ptr->T->arrival_time, time - ptr->T->arrival_time - ptr->T->maximum_run_time);
				std::cout << "time :" << time << '\n';
				pq.display(output);
				output << "about to terminate " << '\n';
				J_TERM(ptr->T, pq, memory, average_turnaround, average_waiting, output);
				ptr = pq.n();
				std::cout << "DD" << '\n';
				output << "DD" << '\n';;
				//std::cout << "Dordon " << ptr->T->job_number << " end \n";
				pq.display(output);
				std::cout << "WWWWWWW" << '\n';
				output << "WWWWWWW" << '\n';
				n--;
				if (count > n) {
					count = n;
				}
				std::cout << "EE " << n << " " << count << " ";
				output << "EE " << n << " " << count << " ";
			}
			if (count == n) {
				std::cout << " G " << '\n';
				output << " G " << '\n';
				count = 0;

				if (flag == 0) {
					pq.display(output);
					pq.insert(ptr->T->priority, ptr->T);
					std::cout << "3 h ";
					output << "4h" << '\n';
					pq.del();
					pq.display(output);
					output << "44444hhhhh" << '\n';
				}
				ptr = pq.n();
				flag = 0;
			}

			else {
				count++;
				if (flag == 0) {
					pq.display(output);
					pq.insert(ptr->T->priority, ptr->T);
					std::cout << "3 h ";
					output << "3h" << '\n';
					pq.del();
					pq.display(output);
					output << "3333hhhhh" << '\n';
				}

				ptr = pq.n();
				flag = 0;
			}

		}
	}
	void balanced_RR() {
	
	}
	void CPU_RR() {
	
	}
};
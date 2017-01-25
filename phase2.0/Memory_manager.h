


#include "table.h"
class Memory {
private:
	short k;  //holds memory location, which can be used later to release the memory
	bool *arrayl;//this virtual memory is used as an array
	std::vector<table> partition_table;  //the partition table per instructions. works as a vector holding the 
	//table class. Each table class index of the partition_table acts as a row in the table
	double internal_fragmentation = 0;
	double external_fragmentation = 0;
	int counter = 0;  // sum of memory spots used. used for external fragmentation

public:
	bool kflag = 0; //used as a switch variable
	int count_of_runs = 0;  //counts each time a dispatch is ran. used for external fragmentation
	
	Memory(short number)
	{     //this is an array of memory. per instructions a partition table points to each memory location
		k = number;
		arrayl = new bool[number];
		for (short i = 0; i <number; i++) {
			arrayl[i] = false;
		}

	}
	~Memory() { delete[] arrayl; }
	/*
	what the routine does_______________________________________-
	the different indexes represent different regions of size in fixed memory
	arraly[0] to arrayl[3] =4 regions of size 8k
	arrayl[4] to arrayl[7]=4 regions of size 12k
	arrayl[8] to arrayl[13]=6 regions of size 18k
	arrayl[14] to arraryl[19]=6 regions of size 32k
	array[20]=1 region of size 52k
	array[21] to arrayl[24]=4 regions of size 60k
	arrayl[25]=1 region of size 128k

the partition table points to each element in memory. checkmem checks each pointer to memory to check for a boolean value.
	_________________________________________________________
	critique:_____________________________________________
	The memory meets specifications. The memory is virtual, using not actual memory, but simulated memory. A partition table
	points to the actual memory


	___________________________________
	*/
	bool checkmem(short kk, short& u) {

		for (int i = 0; i <= 25; i++) {

		//	std::cout << "i " << i << '\n';
			if (kk <= partition_table[i].region_size) {
			//	std::cout << "kk " << kk << " l\n";
				if (*partition_table[i].T == false) {
				//	std::cout << "?????????";
					u = i;
					return true;
				}
			}

		}
		return false;
	}

	/*
	what the routine does_______________________________________-
	This creates the partition table and assigns the addresses of each fixed memory partition to its corresponding place in
	the partition table


	_________________________________________________________
	critique:_____________________________________________
	works perfectly according to specifications.


	___________________________________
	*/
	void setup() {

		table tab(0, true, 0);
		tab.setregion(8);     //each "set region" signifies the partitions of different sizes. in this example 8 kB. 
		         //all unit sizes of 8 KB are set up with pointers and pushed into the partition table. For this reason, setup is only
		         //called once, since only one table is needed of course
		for (int i = 0; i <= 3; i++) {
			tab.T = &arrayl[i];

			partition_table.push_back(tab);
		}
		tab.setregion(12);
		for (int i = 4; i <= 7; i++) {
			tab.T = &arrayl[i];
			partition_table.push_back(tab);
		}
		tab.setregion(18);
		for (int i = 8; i <= 13; i++) {
			tab.T = &arrayl[i];
			partition_table.push_back(tab);
		}
		tab.setregion(32);
		for (int i = 14; i <= 19; i++) {
			tab.T = &arrayl[i];
			partition_table.push_back(tab);
		}
		tab.setregion(52);
		{
			tab.T = &arrayl[20];
			partition_table.push_back(tab);
		}
		tab.setregion(60);
		for (int i = 21; i <= 24; i++) {
			tab.T = &arrayl[i];
			partition_table.push_back(tab);
		}
		tab.setregion(128);
		{
			tab.T = &arrayl[25];
			partition_table.push_back(tab);
		}

	}
	/*
	what the routine does_______________________________________-
assigns a job to the partition table, which then points to its respective place in memory, and changes the memory to true or false.
A first-fit algorithm is used for assigning jobs
	_________________________________________________________
	critique:_____________________________________________
	the specifications are met here, nothing that does not work or needs fixed, or can be changed.
	The first-fit algorithm was a choice over Best fit based on a few test runs of the program.
	The first fit works great, some dispatches fully utilize every partition in memory



	___________________________________
	*/
	bool aquiremem(short kk, short&u,std::ofstream & output) {

		for (int i = 0; i <= 25; i++) {

			if (kk <= partition_table[i].region_size) {
			
				if (*partition_table[i].T == false) {
	
					*partition_table[i].T = true;
					partition_table[i].job_size = kk;
					counter = counter + partition_table[i].region_size;
		
					u = i; //contains memory index
					internal_fragmentation = internal_fragmentation + (partition_table[i].region_size - kk);

					return true;
				}
			}

		}
		return false;

	}
	/*
	what the routine does_______________________________________-
	uses the partition table to free the respective memory region. the parameter "kk" is the memory index being freed.
	This also resets the row for the partition table.
	_________________________________________________________
	critique:_____________________________________________
	the specifications are met here, nothing that does not work or needs fixed, or can be changed. simple 3 lines of code



	___________________________________
	*/
	bool memrelease(short kk) {

		*partition_table[kk].T = false;
		partition_table[kk].job_size = 0;
		
		return true;
	}
	/*
	what the routine does_______________________________________-
displays the partition table. The output statements have been removed. the function as a whole has been left in, inc
case any last minute debugging is needed to check for faults.
	_________________________________________________________
	critique:_____________________________________________
This is not a required function so no critique.



	___________________________________
	*/
	void display(std::ofstream&output) {
		for (int i = 0; i <= 25; i++) {
		
		
		}
	
	}
	/*
	what the routine does_______________________________________-
	Collects statistics for fragmentation. Uses a switch(kflag) to reset external fragmentation to 0, rather than gather 
	statistics. External fragmentation is used by counting all the used partitions through the public counter variable
	and substracts it from 800 KB(the size of memory). This is done on each Dispatch run, and added collectively with
	other Dispatched statistics in order to obtain an average. 
	_________________________________________________________
	critique:_____________________________________________
	Because external fragmentation is the sum of unused partitions
	it was easier to use the following algorithm rather than add up all unused regions and divide by the number of regions multipled
	by the number of runs. Both algorithms performed equally however. This function is called twice in order to reset the
	variables to zero, rather than add an additional reset function. It seems to work fine though, and did not 
	clutter the code at all.



	___________________________________
	*/
	void stats(double & internalf, double& externalf,std::ofstream&output) {
		if (kflag == 1) {
			external_fragmentation = 0;
			kflag = 0;
			counter = 0;
		}
		else {
			external_fragmentation = 800 - counter;
			externalf += external_fragmentation;
			internal_fragmentation=internal_fragmentation ;
			internalf = internal_fragmentation;
		}
	}
};

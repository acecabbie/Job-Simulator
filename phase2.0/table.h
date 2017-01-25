#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
class table {




public:
	//this makes up the partition table vector in memory. region size, if it is occupied or not, and the job size are included. 
	int region_size = 0;
	bool occupied;
	int job_size = 0;
	bool * T;      //this points to the memory array index (specifically an index), so the table can be used rather than accessing memory
	               //directly
	table(int init_regionsize, bool init_occupied,
		int init_jobsize) :

		region_size(init_regionsize),
		occupied(init_occupied),
		job_size(init_jobsize)

	{ }

	/*
	what the routine does_______________________________________ -
	This just changes the region size to the parameter specified. It it for readability mostly, as this is changed in setup
	each time a new memory size is needed to initialize

	_________________________________________________________
	critique : _____________________________________________
no critique needed
	___________________________________
	*/
	void setregion(int region) {
		region_size = region;

	}
	

};
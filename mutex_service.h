#ifndef MUTEX_SERVICE_H
#define MUTEX_SERVICE_H

#include <algorithm>
#include <iostream>
#include <vector>

class Mutex_Service
{
	public:
		int node_id;
		int num_nodes;
		int num_keys;
		// What is the best way to store the keys?
		std::vector<int> keys; // 1 at index means have key else 0
		bool requesting_cs; 

		Mutex_Service() = default;
		Mutex_Service(int node_id, int num_nodes);


		void Generate_Keys();
		void Add_Key(int value);
		void Remove_Key(int value);

		void Print_Keys();
};


#endif

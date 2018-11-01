#ifndef MUTEX_SERVICE_H
#define MUTEX_SERVICE_H

#include <iostream>
#include <vector>

class Mutex_Service
{
	public:
		int node_id;
		int num_nodes;
		// What is the best way to store the keys?
		std::vector<int> keys;

		Mutex_Service() = default;
		Mutex_Service(int node_id, int num_nodes);

		void Generate_Keys();
		void Add_Key(int value);
		void Remove_Key(int value);

		size_t Num_Keys();
		void Print_Keys();
};


#endif

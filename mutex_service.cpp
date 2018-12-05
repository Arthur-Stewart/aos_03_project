#include "mutex_service.h"

Mutex_Service::Mutex_Service(int node_id, int num_nodes) : node_id(node_id), num_nodes(num_nodes), num_keys(0), requesting_cs(false), request_timestamp(0)
{
	Generate_Keys();
}

void Mutex_Service::Generate_Keys()
{
	keys = std::vector<int>(num_nodes, 0);
	// Depends on if you have nodes starting at zero
	
	for (int i = node_id; i < num_nodes; ++i)
	{
		Add_Key(i);
	}
} 

void Mutex_Service::Add_Key(int value)
{
	keys[value] = 1;
	++num_keys;
}

void Mutex_Service::Remove_Key(int value)
{
	if (keys[value] == 0)
	{
		std::cout << "Could not remove key" << std::endl;
		return;
	}
	else
	{
		keys[value] = 0;
		--num_keys;
	}
}

void Mutex_Service::Print_Keys()
{
	std::cout << "KEYS: ";
	for (int i = 0; i < num_nodes; ++i)
	{
		if (keys[i])
		{
			std::cout << i << " ";
		}
	}
	std::cout << std::endl;
	std::cout << "Number of keys: " << num_keys << std::endl;
}


bool Mutex_Service::Can_Execute_CS()
{
	return num_keys == num_nodes;
}

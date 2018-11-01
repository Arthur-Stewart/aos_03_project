#include "mutex_service.h"

Mutex_Service::Mutex_Service(int node_id, int num_nodes) : node_id(node_id), num_nodes(num_nodes)
{
	Generate_Keys();
}

void Mutex_Service::Generate_Keys()
{
	// Depends on if you have nodes starting at zero
	for (int i = node_id + 1; i < num_nodes; ++i)
	{
		//std::cerr << "Adding key: " << i << std::endl;
		keys.emplace_back(i);
	}
} 

size_t Mutex_Service::Num_Keys()
{
	return keys.size();
}
	
		
void Mutex_Service::Print_Keys()
{
	std::cout << "KEYS: ";
	for (const auto& k: keys)
	{
		std::cout << k << " ";
	}
	std::cout << std::endl;
	std::cout << "Number of keys: " << Num_Keys() << std::endl;
}

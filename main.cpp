#include "node.h"
#include "client.h"
#include "message.h"
#include "parser.h"
#include "server.h"

#include <algorithm>
#include <cctype> 
#include <fstream>
#include <iostream>
#include <iterator>
#include <sstream>
#include <thread>
#include <vector>
	
// What are the next problems to solve?

// Generate the keys
// Mutual Exclusion Services
// Application layer 
// 		CS-enter
// 		CS-leave

// Design Test Mechanism

// Exerimental results
// Make plots


int main(int argc, char** argv)
{
	if(argc != 3)
	{
		std::cerr << "usage: ./main config node_id" << std::endl; 
	 	return -1;
	}

	Parser p1(argv[1]);
	p1.Parse_Config();

	Node process_node = p1.node_map[std::stoi(argv[2])];
	std::cout << process_node << std::endl;

	Server s1(process_node, p1);

	s1.mutex_service.Print_Keys();

	//std::thread t1(&Server::Listen, s1);

	//sleep(3); // To let all servers get setup

	//t1.join();
	
}


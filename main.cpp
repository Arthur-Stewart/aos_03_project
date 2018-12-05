#include "node.h"
#include "client.h"
#include "message.h"
#include "parser.h"
#include "server.h"
#include "mutex_service.h"

#include <algorithm>
#include <cctype> 
#include <fstream>
#include <iostream>
#include <iterator>
#include <sstream>
#include <thread>
#include <vector>

// old POSIX libs
#include <semaphore.h>
#include <pthread.h>
#include <fcntl.h>

using namespace std;
	
// What are the next problems to solve?

// Generate the keys
// Mutual Exclusion Services
// Application layer 
// 		CS-enter
// 		CS-leave

// Design Test Mechanism

// Exerimental results
// Make plots

int csApp(Server *s1);

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
	//s1.testing = true;

	//s1.mutex_service.Print_Keys();

	

	std::thread t1(csApp, &s1);

	s1.Listen();

	sleep(3); // To let all servers get setup


	// Have each server start application
	// s1.Start_Simulation();


	// the server threads should do the communication and take care of mutex
	// the main thread run the critical section
	// cs_enter() // the cs_enter can contain a simple POSIX semaphore func, sem_wait, which will wait for sem_post by server thread
	
	// crit section starts
	/*
	
	while(!s1.finished)
	{
		// if app hasn't request cs
		s1.CS_Enter();

		std::cout << "Executing CS" << std::endl;

		if(s1.testing)
		{
			s1.log.emplace_back(s1.v_clock);
		}

		unsigned long int t = std::round(s1.cs_execution_time(s1.gen) * 1e6);

		std::chrono::nanoseconds ns(t);
		//std::cout << "CS Exectuion Time" << t/1e6 << std::endl;
		std::this_thread::sleep_for(ns);
		// Timer with the exponential probability distribution

		// leaving cs
		s1.CS_Leave();
		
	}	
	// critical section ends
	
	*/
	
	t1.join();
	
}


// funct for cs app thread

int csApp(Server *s1)
{


	// crit section starts
	/*
	*/
	while(!(s1->finished))
	{
		// if app hasn't request cs
		if( !(s1->appRequestCs) )
		{
			s1->CS_Enter();

			std::cout << "Executing CS" << std::endl;

			if(s1->testing)
			{
				s1->log.emplace_back(s1->v_clock);
			}

			unsigned long int t = std::round(s1->cs_execution_time(s1->gen) * 1e6);

			std::chrono::nanoseconds ns(t);
			//std::cout << "CS Exectuion Time" << t/1e6 << std::endl;
			std::this_thread::sleep_for(ns);
			// Timer with the exponential probability distribution

			// leaving cs
			s1->CS_Leave();
		}
				
	}	
	// critical section ends
	/*
	*/

	return 0;
}

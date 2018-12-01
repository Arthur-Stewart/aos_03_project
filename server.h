#ifndef SERVER_H
#define SERVER_H 
#include "node.h"
#include "client.h"
#include "mutex_service.h"
#include "parser.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>

#include <algorithm>
#include <string>
#include <chrono>
#include <cstddef>
#include <fstream>
#include <map>
#include <numeric>
#include <unordered_map>
#include <iostream>
#include <iterator>
#include <queue>
#include <random>
#include <set>
#include <sstream>
#include <thread>
#include <vector>

#define BACKLOG 100000 // how many pending connections queue will hold

// APPLICATION SERVICE
// MUTUAL EXCLUSION SERVICE

class Server
{
	public:
		//Socket variables
		int sockfd, newsockfd;  // listen on sock_fd, new connection on newsockfd
		struct addrinfo hints, *servinfo, *p;
		struct sockaddr_storage their_addr; // connector's address information
		socklen_t sin_size;
		struct sigaction sa;
		int yes=1;
		char s[INET6_ADDRSTRLEN];
		int rv;
		int error_num;
		
		//Relavant to project 3
		Node serv;
		std::unordered_map<int, Node> node_map;

		int num_nodes; // number of nodes in the network
		int mean_inter_request_delay;
		int mean_cs_execution_time;
		int num_of_cs_requests;
		int lamport_clock;
		Mutex_Service mutex_service;

		//std::random_device rd;
		std::exponential_distribution<> inter_request_delay;
		std::exponential_distribution<> cs_execution_time;
		std::mt19937 gen;	
		
		//Constructor 
		Server(Node& serv, Parser& parser);
		
		void Start_Simulation();
		void CS_Request();
		void CS_Execute();
		void CS_Leave();

		void ProcessMessage(const char* buffer);
		void Message_Handler(std::string type, int destination, int timestamp);
		
		int Listen();
		
		void *get_in_addr(struct sockaddr *sa);
};

void sigchld_handler(int s);

#endif // SERVER_H

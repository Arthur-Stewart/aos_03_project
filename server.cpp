// old POSIX libs
#include <semaphore.h>
#include <pthread.h>
#include <fcntl.h>


#include "server.h"
#include "mutex_service.h"

// Help from Beej's Guide to Socket

using namespace std;
void Server::ProcessMessage(const char* buffer)
{


	std::string b(buffer);
	std::istringstream iss(b);
	std::vector<std::string> msg_tokens{std::istream_iterator<std::string>{iss},std::istream_iterator<std::string>{}};

	std::string kind = msg_tokens[0];
	
	int source = std::stoi(msg_tokens[1]);
	int timestamp = std::stoi(msg_tokens[2]); 

	
	lamport_clock = std::max(timestamp, lamport_clock) + 1;

	if (testing)
	{
		for (int i = 0; i < num_nodes; ++i)
		{
			v_clock[i] = std::max(v_clock[i], std::stoi(msg_tokens[i + 3]));
		}

		++v_clock[serv.node_id];
	}

	if (kind == "Request")
	{
		if (mutex_service.requesting_cs)
		{
			//std::cout << "Rcvd req ts: " << timestamp << " from " << source << std::endl;
			//std::cout << "Own req ts: " << mutex_service.request_timestamp << std::endl;
		}
		//std::cout << "Recieved Request " << source << " with timestamp " << timestamp << std::endl;
		// If requesting critical section 
		if (mutex_service.requesting_cs)
		{
			// If own timestamp of request is higher priority (lower value)
			if (mutex_service.request_timestamp < timestamp)
			{
				// Defer
				//std::cout << "Defer" << std::endl;
				mutex_service.deferred.emplace(source);
			}
			else if (mutex_service.request_timestamp == timestamp)
			{
				// Break ties with lowest process id
				if (serv.node_id < source)
				{
					// Defer
					//std::cout << "Defer" << std::endl;
					mutex_service.deferred.emplace(source);
				}
				else
				{
					mutex_service.Remove_Key(source);
					Message_Handler("Forward", source, lamport_clock);
				}
			}
			else
			{
				// Reply
				mutex_service.Remove_Key(source);
				Message_Handler("Forward", source, lamport_clock);
			}
		}
		// Reply
		else
		{
			mutex_service.Remove_Key(source);
			Message_Handler("Reply", source, lamport_clock);
		}

	}
	if (kind == "Reply"  || kind == "Forward")
	{
		//lamport_clock = std::max(timestamp, lamport_clock) + 1;
		//std::cout << "Received Reply " << source << std::endl;
		mutex_service.Add_Key(source);
		if (kind == "Forward")
		{
			mutex_service.deferred.emplace(source);	
		}
		// If you received a reply message then it was because you have an outstanding critical section request
		// You should check this property for debug
		if (!mutex_service.requesting_cs)
		{
			std::cout << "Program logic error: Requesting_CS should be true" << std::endl;
		}
		if (mutex_service.Can_Execute_CS() && (!appInCs))
		{	
			CS_Execute();
		}
	}
	if (kind == "Finished")
	{
		++num_finished;
		std::cout << "Finished msg: " << num_finished << std::endl;
		//mutex_service.Print_Keys();
		if (num_finished == num_nodes)
		{
			std::cout << "All finished" << std::endl;
			all_finished = true;
		}
	}
}

void Server::Start_Simulation()
{
	CS_Request();
}

void Server::CS_Request()
{
	
	mutex_service.request_timestamp = ++lamport_clock;
	mutex_service.requesting_cs = true;
	
	//std::cout << "Req CS : " << mutex_service.request_timestamp << std::endl;
	//mutex_service.Print_Keys();
	//std::cout << "Completed requests: " << cs_requests_completed << std::endl;
	
	if (testing)
	{
		++v_clock[serv.node_id];
	}

	if ((mutex_service.num_keys == mutex_service.num_nodes) && (!appInCs))
	{
		CS_Execute();
	}
	else
	{
		// Request keys from all processes
		for (int i = 0; i < num_nodes; ++i)
		{
			if (mutex_service.keys[i] == 0)
			{
				Message_Handler("Request", i, mutex_service.request_timestamp);
			}
		}
	}

}

// actually should be a signal giving permission to enter
// due to design issue (misunderstanding project specs), it is containing cs code

void Server::CS_Execute()
{
	// change the request flag, as this request is complete
	// change inCS flag to true
	// then post the semaphore to let other thread execute the cs
	appRequestCs = false;
	appInCs = true;
	sem_post(&csEnterSem);

/* 	// cs code
	// pasted this section to main.cpp

	std::cout << "Executing CS" << std::endl;

	if(testing)
	{
		log.emplace_back(v_clock);
	}

	unsigned long int t = std::round(cs_execution_time(gen) * 1e6);

	std::chrono::nanoseconds ns(t);
	//std::cout << "CS Exectuion Time" << t/1e6 << std::endl;
	std::this_thread::sleep_for(ns);
	// Timer with the exponential probability distribution
	CS_Leave();
*/
}

// Funct to ask for permission to enter CS
// due to design issue, not used
void Server::CS_Enter()
{
	// change the request flag
	appRequestCs = true;
		
	sem_wait(&csEnterSem);

}


// Leave CS
void Server::CS_Leave()
{

	if(testing)
	{
		++v_clock[serv.node_id];
		log.emplace_back(v_clock);
	}

	++lamport_clock;
	++cs_requests_completed;
	std::cout << "Leaving CS: " << cs_requests_completed << std::endl; 
	//mutex_service.Print_Keys();
	mutex_service.requesting_cs = false;
	while (!mutex_service.deferred.empty())
	{
		//std::cout << "Sending deferred keys" << std::endl;
		int source = mutex_service.deferred.front(); 
		mutex_service.Remove_Key(source);
		Message_Handler("Reply", source, lamport_clock);
		mutex_service.deferred.pop();
	}

	//Need some kind of service that generates the CS request after completion
	//Start a timer based on the probability distribution
	// Gives delay in nano seconds
	delay = std::round(inter_request_delay(gen) * 1e6);
	timer = std::chrono::high_resolution_clock::now();

	// change inCS flag to false
	appInCs = false;
	
	if(cs_requests_completed == num_of_cs_requests && !finished)
	// finishing case for this node
	{
		std::cout << "Finished" << std::endl;
		finished = true;
		++num_finished;
		//mutex_service.Print_Keys();
		for (int i = 0; i < num_nodes; ++i)
		{
			if (i != serv.node_id)
			{
				Message_Handler("Finished", i, lamport_clock);
			}
		}
		if (num_finished == num_nodes)
		{
			all_finished = true;	
			std::cout << "All finished" << std::endl;
		}
	}

}

int Server::Listen()
{
	// cout << "listening on node: " << serv.node_id << " | " ;
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE; // use my IP
	//std::cout << "Listening port: " << serv.port << std::endl;

	if ((rv = getaddrinfo(NULL, serv.port.c_str(), &hints, &servinfo)) != 0) 
	{
		std::cout << "Server side error" << std::endl;
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		error_num = 1;
		exit(1);
	}

	// loop through all the results and bind to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) 
	{
		if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
			perror("server: socket");
			continue;
		}
		if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) 
		{
			perror("setsockopt");
			exit(1);
		}
		if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) 
		{
			close(sockfd);
			perror("server: bind");
			continue;
		}
		break;
	}

	freeaddrinfo(servinfo); // all done with this structure
	if (p == NULL)  
	{
		fprintf(stderr, "server: failed to bind\n");
		exit(1);
	}
	if (listen(sockfd, BACKLOG) == -1) 
	{
		perror("listen");
		exit(1);
	}
	sa.sa_handler = sigchld_handler; // reap all dead processes
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	if (sigaction(SIGCHLD, &sa, NULL) == -1) 
	{
		perror("sigaction");
		exit(1);
	}

	char buffer[2048];

	//sleep(3);
	//CS_Request();
	//Start_Simulation();

	//while(true) 
	
	// wait for first request
	while(!appRequestCs)
	{
	}

	while(!all_finished)
	{  
		//std::cout << "Clock: " << lamport_clock << std::endl;

		/*
		if(cs_requests_completed == num_of_cs_requests && !finished)
		// finishing case for this node
		{
			std::cout << "Finished" << std::endl;
			finished = true;
			++num_finished;
			//mutex_service.Print_Keys();
			for (int i = 0; i < num_nodes; ++i)
			{
				if (i != serv.node_id)
				{
					Message_Handler("Finished", i, lamport_clock);
				}
			}
			if (num_finished == num_nodes)
			{
				all_finished = true;	
				std::cout << "All finished" << std::endl;
				// break;
			}
		}*/
		
		// else 
		if ( (!appInCs) && appRequestCs && (!mutex_service.requesting_cs) && (cs_requests_completed < num_of_cs_requests) )
		// app has requested cs
		{
			std::chrono::high_resolution_clock::time_point t2 = std::chrono::high_resolution_clock::now();
			long unsigned int duration = std::chrono::duration_cast<std::chrono::nanoseconds>(t2-timer).count();
			if (duration > delay)
			{
				timer = t2; // update timer, else it will send multiple consecutive request.
				CS_Request();
			}
		}
		else
		// accept msg
		{
			sin_size = sizeof their_addr;
			newsockfd= accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
			if (newsockfd == -1) 
			{
				std::cout << "Accept error" << std::endl;
				perror("accept");
				continue;
			}

			inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr), s, sizeof s);
			memset(buffer, 0, 1024);
			int read_rtn = read(newsockfd, buffer, 1023);
			if (read_rtn >= 0)
			{
				ProcessMessage(buffer);
			} 
			else
			{
				std::cout << "Read error" << std::endl;
			}

			close(newsockfd);
		}

		//std::cout << "all_finished " << all_finished << std::endl;

	}  // end server
	sleep(2);

	std::ofstream of;
	std::string file_name = std::to_string(serv.node_id) + "n_fin.txt";
	of.open(file_name);

	of << num_messages << std::endl;

	if (testing)
	{
		std::ofstream out;
		std::string file_name = std::to_string(serv.node_id) + ".txt";
		out.open(file_name);
		for (const auto vector: log)
		{
			for (const auto entry: vector)	
			{
				out << entry << " ";
			}
			out << serv.node_id << std::endl;
		}
	}
	return 0;
}

void Server::Message_Handler(std::string type, int destination, int timestamp)
{
	//++timestamp;
	//++lamport_clock;
	//
	//mutex_service.Print_Keys();
	
	++num_messages;

	//std::cout << "Send: " << type << " to " << destination << std::endl;

	if (testing)
	{
		++v_clock[serv.node_id];
		Message pack(type, serv.node_id, timestamp, v_clock);
		Client c1(serv, node_map[destination]);
		c1.SendMessage(pack);
		c1.Close();
	}
	else
	{
		Message pack(type, serv.node_id, timestamp);
		Client c1(serv, node_map[destination]);
		c1.SendMessage(pack);
		c1.Close();
	}
}

//void Server::Message_Handler(std::string type, int destination, int timestamp, std::vector<int> vector_timestamp)
//{
//	++timestamp;
//	++v_clock[serv.node_id];
//	Message pack(type, serv.node_id, timestamp, v_clock);
//	Client c1(serv, node_map[destination]);
//	
//	c1.SendMessage(pack);
//	c1.Close();
//
//}

//Server Constructors/Destructor

Server::Server(Node& serv, Parser& parser) : serv(serv), node_map(parser.node_map), num_nodes(parser.num_nodes), 
		mean_inter_request_delay(parser.mean_inter_request_delay), 
		mean_cs_execution_time(parser.mean_cs_execution_time), 
		num_of_cs_requests(parser.num_of_cs_requests), 
		cs_requests_completed(0), lamport_clock(0), 
		num_finished(0), finished(false), all_finished(false), 
		v_clock(parser.num_nodes, 0), num_messages(0)
{
	mutex_service = Mutex_Service(serv.node_id, num_nodes);
	cs_execution_time =  std::exponential_distribution<>(parser.mean_cs_execution_time);
	inter_request_delay = std::exponential_distribution<>(parser.mean_inter_request_delay);
	gen = std::mt19937(std::random_device()());
	sem_init(&csEnterSem, 0, 0); // init semaphore to 0
	appRequestCs = false;
	appInCs = false;
}

// SOCKET HELPERS
void *Server::get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) 
	{
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

void sigchld_handler(int s)
{
	(void)s; // quiet unused variable warning
	int saved_errno = errno;

	while(waitpid(-1, NULL, WNOHANG) > 0);
	errno = saved_errno;
}


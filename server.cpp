#include "server.h"
// Help from Beej's Guide to Sockets

void Server::ProcessMessage(const char* buffer)
{
	std::string b(buffer);
	std::istringstream iss(b);
	std::vector<std::string> msg_tokens{std::istream_iterator<std::string>{iss},std::istream_iterator<std::string>{}};

	std::string kind = msg_tokens[0];
	
	int source = std::stoi(msg_tokens[1]);
	int timestamp = std::stoi(msg_tokens[2]); 

	if (kind == "Request")
	{
		// Logic
		std::cout << "Recieved Request" << std::endl;
		// If not requesting critical section send them key
		// Reply

		// If requesting critical section 
		// >> If own timestamp of request is higher
		// >> Defer 
		// else
		// Reply
	}
	if (kind == "Reply")
	{
		// If you receieved a reply message then it was because you have an outstanding critical section request
		// You should check this property
		// Check if you have enough keys to enter critical section
	}
}

void Server::Start_Simulation()
{
	// What's the logic of the simulation?
	// Make a CS Request
	CS_Request();
}

// I might want to have a separate function for CS execution
//Generate CS Request 

void Server::CS_Request()
{
	std::cout << "Requesting CS" << std::endl;
	mutex_service.requesting_cs = true;

	if (mutex_service.num_keys == num_nodes)
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
				// Send message
				Message_Handler("Request", i, lamport_clock);
			}
		}
	}

	++lamport_clock;
}

void Server::CS_Execute()
{
	std::cout << "Executing CS" << std::endl;
	unsigned long int t = std::round(cs_execution_time(gen) * 1e6);
	std::cout << "t nano seconds " << t << std::endl;

	std::chrono::nanoseconds ms(t);
	//std::cout << "CS Exectuion Time" << sec << std::endl;
	std::this_thread::sleep_for(ms);
	// Timer with the exponential probability distribution
	CS_Leave();
}

//Leave CS
void Server::CS_Leave()
{
	std::cout << "Leaving CS" << std::endl;
	mutex_service.requesting_cs = false;
	//Need some kind of service that generates the CS request after completion
}

int Server::Listen()
{
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

	char buffer[1024];

	while(true) 
	{  
		sin_size = sizeof their_addr;
		newsockfd= accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
		if (newsockfd == -1) 
		{
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

		close(newsockfd);

	}  // end server
	sleep(2);
}

void Server::Message_Handler(std::string type, int destination, int timestamp)
{
	Message pack(type, serv.node_id, timestamp);
	Client c1(serv, node_map[destination]);

	//Debug
	//std::cout << pack << std::endl;
	
	c1.SendMessage(pack);
	c1.Close();
}

//Server Constructors/Destructor

Server::Server(Node& serv, Parser& parser) : serv(serv), node_map(parser.node_map), num_nodes(parser.num_nodes), 
		mean_inter_request_delay(parser.mean_inter_request_delay), 
		mean_cs_execution_time(parser.mean_cs_execution_time), 
		num_of_cs_requests(parser.num_of_cs_requests), lamport_clock(0)
{
	mutex_service = Mutex_Service(serv.node_id, num_nodes);
	cs_execution_time =  std::exponential_distribution<>(parser.mean_cs_execution_time);
	inter_request_delay = std::exponential_distribution<>(parser.mean_inter_request_delay);
	gen = std::mt19937(std::random_device()());
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


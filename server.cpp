#include "server.h"
// Help from Beej's Guide to Sockets

int Server::Listen()
{
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE; // use my IP
	//std::cerr << "Listening port: " << serv.port << std::endl;

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

void Server::ProcessMessage(const char* buffer)
{
	std::string b(buffer);
	std::istringstream iss(b);
	std::vector<std::string> msg_tokens{std::istream_iterator<std::string>{iss},std::istream_iterator<std::string>{}};

	std::string kind = msg_tokens[0];
	
	int source = std::stoi(msg_tokens[1]);
	int origin = std::stoi(msg_tokens[2]); // Broadcast's origin
	
	std::string contents;

	if((kind == "Broadcast") || (kind == "Convergecast"))
	{
		contents = msg_tokens[3]; 
	}

	if (kind == "Discovery")
	{
		if (serv.parent.empty())
		{
			serv.parent.emplace_back(node_map[source]);
			Message_Handler("Parent", source);
					
			for (const auto& one_hop: serv.one_hop_neighbors)
			{
				if (serv.parent[0].node_id != one_hop.node_id)
				{
					++num_discovery_message;
				}
			}

			if (num_discovery_message == 0)
			{
				Message_Handler("Done", serv.parent[0]);
			}

			//Sends a Discovery message to each of it's neighbors
			for (const auto& one_hop: serv.one_hop_neighbors)
			{
				if (serv.parent[0].node_id != one_hop.node_id)
				{
					Message_Handler("Discovery", one_hop);
				}
			}
		}
		//PARENT field already set send no
		else
		{
			Message_Handler("No", source);
		}
	}
	
	else if (kind == "Parent")
	{
		serv.children.emplace_back(node_map[source]);
	}

	else if (kind == "No")
	{
		if (++num_no_message == num_discovery_message)
		{
			Message_Handler("Done", serv.parent[0]);
		}
	}
	
	else if (kind == "Done")
	{
		if(++num_done_message == serv.children.size())
		{
			// parent is empty,i.e. no parent => is root
			if (serv.parent.empty())
			{
				// Add tree_neighbors (unrooted tree)
				serv.tree_neighbors = serv.children;
				serv.PrintTreeNeighbors();

				for (const auto&n: serv.children)
				{
					Message_Handler("Finished", n);
				}
				discovered = true;
			}
			else //If node has recived done from all of its children send done to parent
			{
				Message_Handler("Done", serv.parent[0]);
			}
		}
	}

	else if (kind == "Finished")
	{
		// Add tree_neighbors (unrooted tree)
		for (auto &v: serv.parent)
		{
			serv.tree_neighbors.emplace_back(v);
		}
		for (auto &v: serv.children)
		{
			serv.tree_neighbors.emplace_back(v);
		}

		serv.PrintTreeNeighbors();
		
		for (const auto&n: serv.children)
		{
			Message_Handler("Finished", n);
		}
		discovered = true;
	}

	else if (kind == "Broadcast")
	{
		std::cout << "Received Broadcast: " << contents << std::endl;

		//For testing
		parent_map[origin] = source; //Set parent_map

		if (IsLeaf(origin))  // Send convergcast to parent
		{
			Message_Handler("Convergecast", node_map[parent_map[origin]], contents, origin);
		}
		else // Continue Broadcast
		{
			Broadcast(contents, origin);
		}
	}

	else if (kind == "Convergecast")
	{
		// Need to do similar logic to building the tree
		// Once a node receives convergecast from all of its childre 
		if (serv.node_id == origin)
		{
			// Convergecast reached original broadcaster
			if(++converge_count_map[origin] == Num_Children(origin))
			{
				converge_count_map[origin] = 0;
				//std::cerr << "Convergecast success: " << contents << std::endl;
				//For testing
				// Send next message is the queue if it exists
				if (!message_queue.empty())
				{
					// Pop message off message queue 
					Broadcast(message_queue.front(), serv);
					message_queue.pop();
				}
			}
		}
		else // Send message to logical parent
		{
			if(++converge_count_map[origin] == Num_Children(origin))
			{
				// Reset convergecount
				converge_count_map[origin] = 0;
				Message_Handler("Convergecast", node_map[parent_map[origin]], contents, origin);
			}
		}
	}
}

//Broadcast operation for origin node
void Server::Broadcast(std::string contents, Node ignore)
{
	std::cout << "Broadcasting: " << contents << std::endl;
	// Send a message to all neighbors
	for (const auto& n: serv.tree_neighbors)
	{
		Message_Handler("Broadcast", n, contents, ignore.node_id);
	}
}

void Server::Broadcast(std::string contents, int origin)
{
	// Send a message to all neighbors except logical parent
	for (const auto &dest: serv.tree_neighbors)
	{
		if (parent_map[origin] != dest.node_id)
		{
			Message_Handler("Broadcast", dest, contents, origin);
		}
	}
}

void Server::Message_Handler(std::string type, std::string destination)
{
	Message_Handler(type, node_map[std::stoi(destination)]);
}

void Server::Message_Handler(std::string type, int destination)
{
	Message_Handler(type, node_map[destination]);
}

void Server::Message_Handler(std::string type, Node destination)
{
	Message pack(type);
	pack.source = serv.node_id;
	Client c1(serv, destination);

	//Debug
	//std::cerr << pack << std::endl;

	c1.SendMessage(pack);
	c1.Close();
}

void Server::Message_Handler(std::string type, Node destination, std::string contents)
{
	Message pack(type, contents);
	pack.source = serv.node_id;
	Client c1(serv, destination);
	c1.SendMessage(pack);
	c1.Close();
}

// KEY MESSAGE_HANDLER OPERATION
void Server::Message_Handler(std::string type, Node destination, std::string contents, int origin)
{
	Message pack(type, contents);
	pack.source = serv.node_id;
	pack.origin = origin;
	Client c1(serv, destination);
	c1.SendMessage(pack);
	c1.Close();
}

bool Server::IsLeaf(int origin)
{
	if (serv.node_id == origin)
	{
		return false;
	}
	return serv.tree_neighbors.size() == 1;
}

// Number of child nodes with respect to origin node
size_t Server::Num_Children(int origin)
{
	if (serv.node_id == origin)
	{
		return serv.tree_neighbors.size();
	}

	return  serv.tree_neighbors.size() - 1;
}

//Server Constructors/Destructor

//Server::Server(Node& serv) : serv(serv), discovered(false)
//{}

//Server::Server(Node& serv, std::unordered_map<int, Node> node_map) : num_nodes(node_map.size()), serv(serv), node_map(node_map), num_discovery_message(0), num_no_message(0), num_done_message(0), discovered(false), sum(0) 
//{}

Server::Server(Node& serv, Parser& parser) : serv(serv), node_map(parser.node_map), num_nodes(parser.num_nodes), 
		mean_inter_request_delay(parser.mean_inter_request_delay), 
		mean_cs_execution_time(parser.mean_inter_request_delay), 
		num_of_cs_requests(parser.num_of_cs_requests) 
{
	mutex_service = Mutex_Service(serv.node_id, num_nodes);
}

// SOCKET HELPERS

void *Server::get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

void sigchld_handler(int s)
{
	(void)s; // quiet unused variable warning

	//waitpid() might overwrite errno, so we save and restore it:
	int saved_errno = errno;

	while(waitpid(-1, NULL, WNOHANG) > 0);

	errno = saved_errno;
}


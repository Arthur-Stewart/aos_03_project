#ifndef MESSAGE_H
#define MESSAGE_H

#include <iostream>
#include <vector>

class Message
{
	public:
		std::string kind;
		int source; // sending node
		int timestamp; // Lamport style timestamp
		//int destination; // receiving node
		
		std::vector<int> vector_timestamp;
		
		Message(std::string kind, int source,  int timestamp);

		Message(std::string kind, int source,  int timestamp, std::vector<int> vector_timestamp);

		std::string To_String();
		
		friend std::ostream &operator<<(std::ostream &os, Message const &m); 
};

#endif // MESSAGE_H

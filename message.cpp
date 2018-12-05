#include "message.h"
		
Message::Message(std::string kind, int source,  int timestamp) : kind(kind), source(source), timestamp(timestamp)
{}

Message::Message(std::string kind, int source,  int timestamp, std::vector<int> v_timestamp) : kind(kind), source(source), timestamp(timestamp)
{
	vector_timestamp = v_timestamp;
}

std::string Message::To_String()
{
	std::string s = kind + " " + std::to_string(source) + " " + std::to_string(timestamp);
	
	if (!vector_timestamp.empty())
	{
		for (const auto n: vector_timestamp)
		{
			s =  s + " " + std::to_string(n);
		}
		return s;
	}

	else
	{
		return s; 
	}
}

// To print message contents for debugging 
std::ostream &operator<<(std::ostream &os, Message const &m)
{
	os << "KIND:" << m.kind << std::endl;
	os << "SOURCE:" << m.source << std::endl;
	os << "TIMESTAMP:" << m.timestamp << std::endl;
	
	return os;
}

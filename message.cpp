#include "message.h"
		
Message::Message(std::string kind, int source,  int timestamp) : kind(kind), source(source), timestamp(timestamp)
{}

std::string Message::To_String()
{
	return kind + " " + std::to_string(source) + " " + std::to_string(timestamp);
}

// To print message contents for debugging 
std::ostream &operator<<(std::ostream &os, Message const &m)
{
	os << "KIND:" << m.kind << std::endl;
	os << "SOURCE:" << m.source << std::endl;
	os << "TIMESTAMP:" << m.timestamp << std::endl;
	
	return os;
}

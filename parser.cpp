#include "parser.h"

Parser::Parser(const std::string config) : config(config), line_num(0), num_nodes(0)
{}

void Parser::Parse_Config()
{
	std::ifstream in(config);
	while (std::getline(in, line))
	{
		if (Is_Valid_Line(line))
		{
			// FirstLine
			if (line_num == 0)
			{
				std::istringstream iss(line);
				std::vector<std::string> tokens{std::istream_iterator<std::string>{iss},std::istream_iterator<std::string>{}};
				num_nodes = std::stoi(tokens[0]);
				mean_inter_request_delay = std::stoi(tokens[1]);
				mean_cs_execution_time = std::stoi(tokens[2]);
				num_of_cs_requests = std::stoi(tokens[3]);

			}
			// First n
			else if (line_num < num_nodes + 1)
			{
				std::istringstream iss(line);
				std::vector<std::string> tokens{std::istream_iterator<std::string>{iss},std::istream_iterator<std::string>{}};
				int node_id = std::stoi(tokens[0]);
				std::string host = tokens[1] + ".utdallas.edu";
				std::string port = tokens[2];
				node_map[node_id] = Node(node_id, host, port);
			}
			
			line_num++;
		}
	}
}
bool Parser::Is_Valid_Line(std::string line)
{
	std::istringstream iss(line);
	std::string first;
	iss >> first;
	if (first.empty())
	{
		return false;
	}
	for (auto it: first)
	{
		if (it == '#')
		{
			return false;
		}
	}
	return true;
}

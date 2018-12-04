#include <algorithm>
#include <iostream>
#include <iterator>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

void PrintVector(const std::vector<int>& v);

bool Compare_Vec (const std::vector<int>& first, const std::vector<int>& second);

int main(int argc, char **argv)
{
	if(argc != 3)
	{
		std::cerr << "usage: ./main test num_nodes" << std::endl; 
	 	return -1;
	}

	std::string file_name = argv[1];
	int num_nodes = std::stoi(argv[2]);

	std::ifstream in(file_name);

	std::vector<std::vector<int>> vec;
	std::string line;
	
	// Read the file into a vector of vector of ints
	while (std::getline(in, line))
	{
			std::istringstream iss(line);
			std::vector<std::string> tokens{std::istream_iterator<std::string>{iss},std::istream_iterator<std::string>{}};

			std::vector<int> t;

			for (int i = 0; i < num_nodes + 1; ++i)
			{
				t.emplace_back(std::stoi(tokens[i]));
			}
			vec.emplace_back(t);
	}


	//Sort the 
	std::sort(vec.begin(), vec.end(), Compare_Vec);
	
	for (const auto v: vec)
	{
		PrintVector(v);	
	}

	for (size_t i = 0; i < vec.size() ; i+=2)
	{
		if (vec[i][num_nodes] != vec[i+1][num_nodes])
		{
			std::cout << "Overlapping critical sections" << std::endl;
		}
	}
}

bool Compare_Vec (const std::vector<int>& first, const std::vector<int>& second)
{
	for (size_t i = 0; i < first.size() - 1UL; ++i)
	{
		if (first[i] != second[i])
		{
			return first[i] < second[i];
		}
	}

	//std::cout << "Vectors not comparable";

	return false;
}

void PrintVector(const std::vector<int>& v)
{
	for (const auto elem: v)
	{
		std::cout << elem << " ";	
	}
	std::cout << std::endl;


}

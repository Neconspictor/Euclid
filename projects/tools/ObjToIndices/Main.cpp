#include <iostream>
#include <fstream>
#include <string>
#include <regex>
#include <list>

using namespace std;

struct Vertex
{
	float x;
	float y;
	float z;
};

struct Face
{
	int index1;
	int index2;
	int index3;
};

ostream& operator<<(ostream& out, Vertex& vertex) {
	out << vertex.x << ", " << vertex.y << ", " << vertex.z;
	return out;
}

ostream& operator<<(ostream& out, Face& face) {
	out << face.index1 << ", " << face.index2 << ", " << face.index3;
	return out;
}

vector<string> tokenize(const string& source, const string& pattern)
{
	regex re(pattern);
	sregex_token_iterator
		first{ source.begin(), source.end(), re, -1 },
		last;
	return{ first, last };
}

string trim(const string& str,
	const string& whitespace = " \t")
{
	const auto strBegin = str.find_first_not_of(whitespace);
	if (strBegin == string::npos)
		return ""; // no content

	const auto strEnd = str.find_last_not_of(whitespace);
	const auto strRange = strEnd - strBegin + 1;

	return str.substr(strBegin, strRange);
}

string removeFileExtension(const string& str)
{
	const string& ext = ".";
	const auto strEnd = str.find_last_of(ext);
	if (strEnd == string::npos)
		return str; // no file extension!
	return str.substr(0, strEnd);
}


void processFile(ifstream* file, list<Vertex>& vertices, list<Face>& faces)
{
	//skip all lines until we have the first face statement
	bool abort = false;
	bool collectingVertices = false;
	bool collectingIndices = false;
	string current;
	int line = 1;

	// at first collect all vertices
	// a vertex description begins with "v "
	regex vertexDescription("v .*");

	while (!abort)
	{
		getline(*file, current);
		current = trim(current); // no leading and trailing whitespaces
		if (regex_match(current, vertexDescription))
		{
			collectingVertices = true;
			vector<string> tokens = tokenize(current, "( |\t)+");
			if (tokens.size() != 4) // three vertex coordinates and one for 'v'
			{
				throw runtime_error("malformed file at line " + to_string(line));
			}

			vertices.push_back({ stof(tokens[1]), stof(tokens[2]), stof(tokens[3])});
		} else if (collectingVertices)
		{
			abort = true;
		}
		if (file->eof())
		{
			abort = true;
		}

		++line;
	}

	cout << "Collected vertices: " << endl;
	for (auto it = vertices.begin(); it != vertices.end(); )
	{
		cout << *it;
		++it;
		if (it != vertices.end())
		{
			cout << "," << endl;
		} else
		{
			cout << endl;
		}
	}

	// collect vertex indices
	abort = false;
	regex indexDescription("f .*");
	current = "";

	while (!abort)
	{
		getline(*file, current);
		current = trim(current); // no leading and trailing whitespaces
		if (regex_match(current, indexDescription))
		{
			collectingIndices = true;
			vector<string> tokens = tokenize(current, "( |\t)+");
			if (tokens.size() != 4) // three index lists and one for 'f'
			{
				throw runtime_error("malformed file at line " + to_string(line));
			}


			//the index is always the first number of the resulting token
			vector<string> indexTokens1 = tokenize(tokens[1], "/");
			vector<string> indexTokens2 = tokenize(tokens[2], "/");
			vector<string> indexTokens3 = tokenize(tokens[3], "/");

			Face face;
			// face indices begin with 1, but we need 0 as the beginning
			face.index1 = stoi(indexTokens1[0]) - 1;
			face.index2 = stoi(indexTokens2[0]) - 1;
			face.index3 = stoi(indexTokens3[0]) - 1;
			faces.push_back(face);

			if (face.index1 < 0
			|| face.index2 < 0
			|| face.index3 < 0
				)
			{
				throw runtime_error("malformed file at line " + to_string(line));
			}

		} else if (collectingIndices)
		{
			abort = true;
		}
		if (file->eof())
		{
			abort = true;
		}

		++line;
	}

	cout << "Collected face indices: " << endl;
	for (auto it = faces.begin(); it != faces.end(); )
	{
		cout << *it;
		++it;
		if (it != faces.end())
		{
			cout << "," << endl;
		}
		else
		{
			cout << endl;
		}
	}
}


void writeFile(ofstream& out, list<Vertex>& vertices, list<Face>& faces, string inputFile)
{
	out << "/* File Created by ObjToIndices" << endl
		<< " * Contains converted vertices and face indices from file " << inputFile << endl
		<< " */" << endl << endl;
	out << "/* VERTICES */" << endl;
	out << "const static const float vertices[] = {" << endl;
	for (auto it = vertices.begin(); it != vertices.end(); )
	{
		out << "  " << *it;
		++it;
		if (it != vertices.end())
		{
			out << "," << endl;
		}
		else
		{
			out << endl;
		}
	}

	out << "};" << endl << endl;
	out << "/* INDICES */" << endl;
	out << "static const uint32_t indices[] = {" << endl;
	for (auto it = faces.begin(); it != faces.end(); )
	{
		out << "  " << *it;
		++it;
		if (it != faces.end())
		{
			out << "," << endl;
		}
		else
		{
			out << endl;
		}
	}
	out << "};";
};

int main(int argc, char** argv)
{
	if (argc != 2)
	{
		cout << "Please specify exact one file!" << endl;
		return EXIT_FAILURE;
	}

	ifstream input;
	ofstream out;
	string inputFile = argv[1];
	string outputFile = removeFileExtension(inputFile) + ".myMesh";
	list<Vertex> vertices;
	list<Face> faces;

	bool abort = false;
	input.exceptions(ifstream::badbit);
	input.open(inputFile);

	if (!input.is_open())
	{
		cout << "Couldn't open file " << inputFile << endl;
		return EXIT_FAILURE;
	}
	cout << "Processing file " << inputFile << " ..."<< endl;

	try
	{
		processFile(&input, vertices, faces);
	} catch(exception& e)
	{
		cout << "An error occurred." << endl;
		cout << "Error description: " << e.what() << endl;
		abort = true;
	}

	input.close();

	if (abort)
	{
		cout << "Aborting..." << endl;
		return EXIT_FAILURE;
	}

	cout << "Writing output file..." << endl;
	out.open(outputFile);
	if (!out.is_open())
	{
		cout << "Couldn't open output file " << outputFile << endl;
		return EXIT_FAILURE;
	}

	try
	{
		writeFile(out, vertices, faces, inputFile);
	}
	catch (exception& e)
	{
		cout << "An error occurred." << endl;
		cout << "Error description: " << e.what() << endl;
		abort = true;
	}

	out.close();

	if (abort)
	{
		cout << "Aborting..." << endl;
		return EXIT_FAILURE;
	}

	cout << "Finished." << endl;

	return EXIT_SUCCESS;
}
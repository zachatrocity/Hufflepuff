#include <iostream>
#include <fstream>
#include <string>
#include "huff.h"

using namespace std;

const int MAXSIZE = 256;

//utility functions
string createNewHuffFile(string fn);
void printFreqTable(int[]);

void main(){
	fstream inputFile;
	fstream outputFile;
	string fn = "";
	int freqtable[MAXSIZE] = { 0 };
	int readSize = 1;
	unsigned char buffer[1];
	cout << "please enter a file name:" << endl;
	getline(cin, fn);

	inputFile.open(fn, ios::in | ios::binary);
	outputFile.open(createNewHuffFile(fn), ios::out | ios::binary);
	
	inputFile.read((char*)buffer, 1);

	while (buffer[0] != inputFile.eofbit && !inputFile.eof())
	{
		freqtable[buffer[0]]++;
		inputFile.read((char*)buffer, readSize);
	}
	
	printFreqTable(freqtable);

	system("pause");
}

//1) Header metadata
//	Length of source filename(integer)
//	Source filename(string) - it is not null terminated
//	Number of huffman table entries(integer)
//	Huffman table
//		Each table entry is structured as follows :
//			glyph(integer) - value of glyph(0 - 256), or - 1 (frequency node)
//			(Remember the eof glyph should be 256.)
//			left pointer(integer)
//			right pointer(integer)
//	NOTE : The maximum number of entries in a huffman table would be 513 (257 possible glyphs + 256
//	   merge(frequency) nodes.)
//2) Compressed data

string createNewHuffFile(string inputPath)
{
	size_t found = inputPath.find_last_of("/\\");
	string outputPath = inputPath.substr(0, found) + "\\";  //append a back slash to the end of the path
	string outputName = inputPath.substr(found + 1);

	if (outputName.find(".")) // file name has extension
	{
		//strip off the extension and add .dmp
		size_t period = outputName.find_last_of(".");
		outputName = outputName.substr(0, period) + ".huf";
	}
	else {
		//no file extension so just add .dmp
		outputName += ".huf";
	}

	return outputPath + outputName;
}

void printFreqTable(int freqtable[])
{
	for (int i = 0; i < MAXSIZE; i++)
	{
		if (freqtable[i] > 0)
		{
			cout << '\'' << (char)i << "' | " << freqtable[i] << endl;
		}
	}
}


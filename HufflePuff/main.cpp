#include <iostream>
#include <ctime>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include "huff.h"
#include <iomanip>
#include <algorithm>


using namespace std;


struct huffnode{
	int glyph;
	int freq;
	int left;
	int right;
};

const static int MAXSIZE = 256; 
const static int HUFMAXSIZE = 513;
static int freqtable[MAXSIZE] = { 0 };
static huffnode hufftable[HUFMAXSIZE] = { -1 };

//utility functions
string createNewHuffFile(string fn);
void buildHuffTree();
void printFreqTable();
void printHuffTable(int size);
void reheap(int start, int end);
void log(string l);

void main(){
	string fn = "";
	clock_t start, end;

	cout << "please enter a file name:" << '\n';
	getline(cin, fn);

	//START CLOCK
	start = clock();

	FILE* file;
	errno_t errorCode = fopen_s(&file, fn.c_str(), "rb");
	// obtain file size:
	fseek(file, 0, SEEK_END);
	long lSize = ftell(file);
	rewind(file);

	// allocate memory to contain the whole file:
	char *inputFileBuffer = (char*)malloc(sizeof(char)*lSize);
	if (inputFileBuffer == NULL) { fputs("Memory error", stderr); exit(2); }
	
	// copy the file into the buffer:
	size_t result = fread(inputFileBuffer, 1, lSize, file);
	if (result != lSize) { fputs("Reading error", stderr); exit(3); }
	/* the whole file is now loaded in the memory buffer. */

	//get frequencies
	for (int i = 0; i < result; i++)
	{
		freqtable[inputFileBuffer[i]]++;
	}
	
	//printFreqTable();

	//begin huffman algorithm
	buildHuffTree();

	//generate codes

	//
	
	//END CLOCK
	end = clock();
	cout << "The time was " << (double(end - start) / CLOCKS_PER_SEC) << '\n';

	//printHuffTable(0);

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

void buildHuffTree() {

	//build sorted ascending array
	int loc = 0;
	for (int i = 0; i < MAXSIZE; i++)
	{
		if (freqtable[i] > 0)
		{
			huffnode node;
			node.glyph = (char)i;
			node.freq = freqtable[i];
			node.left = -1;
			node.right = -1;
			hufftable[loc] = node;
			loc++;
		}
	}

	std::sort(hufftable, hufftable + loc, [](const huffnode &x, const huffnode &y){
		return x.freq < y.freq;
	});

	//printHuffTable(loc);

	int f = loc;
	int h = loc - 1;

	//repeat until glyph - 1 merge
	for (int merge = 0; merge < loc; merge++){
		//log("Merge");

		//mark m lower of slots 1 and 2
		int m = (hufftable[1].freq < hufftable[2].freq) ? 1 : 2;
		//move m to next free slot
		hufftable[f] = hufftable[m];
		//if m < the end of the heap (h) move h to m
		if (m < h) {
			hufftable[m] = hufftable[h];
			reheap(m, h);
		}
		//move lowest freq node to h
		hufftable[h] = hufftable[0];
		//create freq node at slot 0;
		huffnode fqnode; 
		fqnode.glyph = -1; 
		fqnode.freq = (hufftable[f].freq + hufftable[h].freq);
		fqnode.left = h;
		fqnode.right = f;
		hufftable[0] = fqnode;
		//reheap 
		reheap(0, h);
		//move h and f
		h--;
		f++;
	}

}

void reheap(int s, int e){
	int left = (s << 1) + 1;
	int right = (s << 1) + 2;
	if (s < e){

		if (hufftable[left].freq < hufftable[s].freq || hufftable[right].freq < hufftable[s].freq){
			//http://www.geeksforgeeks.org/compute-the-minimum-or-maximum-max-of-two-integers-without-branching/
			int minIndex = (hufftable[left].freq < hufftable[right].freq) ? left : right;
			//int minIndex = hufftable[left].freq ^ ((hufftable[right].freq ^ hufftable[left].freq) & -(hufftable[right].freq < hufftable[left].freq)); //min
			huffnode temp = hufftable[minIndex];
			hufftable[minIndex] = hufftable[s];
			hufftable[s] = temp;
			reheap(minIndex, e);
		}
	}
}

void log(string l){
	cout << "DEBUG: " << l << '\n';
}

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

void printFreqTable()
{
	for (int i = 0; i < MAXSIZE; i++)
	{
		if (freqtable[i] > 0)
		{
			cout << '\'' << (char)i << "' | " << freqtable[i] << '\n';
		}
	}

	cout << '\n';
}

void printHuffTable(int size)
{
	for (int i = 0; i < size; i++)
	{
		if (hufftable[i].freq > 0)
		{
			cout << "G: " << (char)hufftable[i].glyph << '\n';
			cout << "F: " << hufftable[i].freq << '\n';
			cout << "L: " << hufftable[i].left << '\n';
			cout << "R: " << hufftable[i].right << '\n' << '\n';
		}
	}
}


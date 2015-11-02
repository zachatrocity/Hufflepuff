//Zach Russell & Stuart Spradling --- Huff

#include <iostream>
#include <fstream>
#include <ctime>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include "huff.h"
#include <iomanip>
#include <bitset>
#include <algorithm>
#include <vector>
using namespace std;

struct huffnode{
	int glyph;
	int left;
	int right;
	int freq;
};

typedef string huffCode;

const static unsigned long long size = 8ULL * 1024ULL * 1024ULL;
static unsigned long long outBuffer[size];
const static int MAXSIZE = 257; 
const static int HUFMAXSIZE = 513;
static int freqtable[MAXSIZE] = { 0 };
static huffnode hufftable[HUFMAXSIZE] = { -1 };
static huffCode huffMap[MAXSIZE] = { "" };
static FILE* oFile;
static FILE* file;
static char fn[100];
static ofstream outputFile;

//utility functions
string createNewHuffFile(string fn);
int buildHuffTree();
int buildHuffTreeFromFile(char[]);
void generateBitCodes(int start, int end, huffCode bitCode);
void printFreqTable();
void printBitCodes();
void printHuffTable(int size);
void reheap(int start, int end);
void log(string l);


void main(){
	clock_t start, end;
	
	cout << "please enter a file name:" << '\n';
	cin >> fn;

	//START CLOCK
	start = clock();
	
	errno_t errorCode = fopen_s(&file, fn, "rb");
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
	
	//eof 
	freqtable[256] = 1;

	//printFreqTable();

	//begin huffman algorithm
	int last = buildHuffTree();

	//printHuffTable(0);

	//generate codes
	generateBitCodes(0, last, "");
	//
	//printBitCodes();

	//create file header and write it out
	outputFile.open(createNewHuffFile(fn), ios::binary);
	int filesize = strlen(fn);
	int numEntries = last + 1;
	outputFile.write((char*)&filesize, sizeof(int));
	outputFile.write((char*)&fn, filesize);
	outputFile.write((char*)&numEntries, sizeof last);

	for (int i = 0; i <= last; i++){
		outputFile.write((char*)&hufftable[i], sizeof(int) * 3);
	}

	string encodedData = "";
	
	//output the compressed data
	for (int x = lSize; x > 0; x--)
	{
		//encodedData += huffMap[(unsigned char)inputFileBuffer[x]];
		string code = huffMap[(unsigned char)inputFileBuffer[x]];
		
		
			int counter = 0;
			encodedData += code;
			while ((encodedData.length() / 8) >= 1)
			{
				bitset<8> hexValue(encodedData.substr(counter, counter + 8));
				unsigned long byte = hexValue.to_ulong();
				outputFile.write(reinterpret_cast<char*>(&byte), 1);
				encodedData.erase(encodedData.begin(), encodedData.begin()+7);
				counter += 8;
			}
		
		//outputFile << hex << uppercase << int(byte);
		//outputFile.write(reinterpret_cast<char*>(&byte), sizeof byte);
	}


	int remainder = encodedData.length() % 8;
	for (int i = 0; i < remainder; i++)
	{
		encodedData = "0" + encodedData;
	}

	bitset<8> hexValue(encodedData);
	unsigned long byte = hexValue.to_ulong();
	outputFile.write(reinterpret_cast<char*>(&byte), 1);

	//for (int i = 0; i < encodedData.length(); i += 8)
	//{
	//	bitset <8> hexValue(encodedData.substr(i, i + 8));
	//	//cout << uppercase << hex << hexValue.to_ulong();
	//	unsigned long byte = hexValue.to_ulong();
	//	outputFile.write(reinterpret_cast<char*>(&byte), 1);
	//}


	
	outputFile.close();
	//END CLOCK
	end = clock();
	cout << "The time to encode the file was " << (double(end - start) / CLOCKS_PER_SEC) << '\n';

	cout << "please enter a .huf file name:" << '\n';
	cin >> fn;

	//START CLOCK
	start = clock();

	errorCode = fopen_s(&file, fn, "rb");
	// obtain file size:
	fseek(file, 0, SEEK_END);
	lSize = ftell(file);
	rewind(file);

	// allocate memory to contain the whole file:
	inputFileBuffer = (char*)malloc(sizeof(char)*lSize);
	if (inputFileBuffer == NULL) { fputs("Memory error", stderr); exit(2); }

	// copy the file into the buffer:
	result = fread(inputFileBuffer, 1, lSize, file);
	if (result != lSize) { fputs("Reading error", stderr); exit(3); }
	/* the whole file is now loaded in the memory buffer. */

	last = buildHuffTreeFromFile(inputFileBuffer);
	generateBitCodes(0, last, "");
	printBitCodes();

	cout << "Placeholder";

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

int buildHuffTree() {

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

	//repeat until h is zero
	while (h > 0){
		//mark m lower of slots 1 and 2
		int m = (hufftable[1].freq < hufftable[2].freq) ? 1 : 2;

		if (h == 1)
			m = 1;

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

	return --f;
}

int buildHuffTreeFromFile(char inputFileBuffer[])
{
	int filenameSize = inputFileBuffer[0];
	int numOfEntries = inputFileBuffer[filenameSize + 4];

	int index = filenameSize + 8;
	for (int i = 0; i < numOfEntries; i++)
	{
		hufftable[i].glyph = inputFileBuffer[index];
		index += 4;
		hufftable[i].left = inputFileBuffer[index];
		index += 4;
		hufftable[i].right = inputFileBuffer[index];
		index += 4;
		hufftable[i].freq = 1;
	}

	return numOfEntries;
}

void reheap(int s, int e){
  	int left = (s << 1) + 1;
	int right = (s << 1) + 2;
	bool leftIn = left < e;
	bool rightIn = right < e;

	if (s < e){

		if (hufftable[left].freq < hufftable[s].freq || hufftable[right].freq < hufftable[s].freq){
			if (leftIn && rightIn){
				int minIndex = (hufftable[left].freq > hufftable[right].freq) ? right : left;
				huffnode temp = hufftable[minIndex];
				hufftable[minIndex] = hufftable[s];
				hufftable[s] = temp;
				reheap(minIndex, e);
			}
			else if (leftIn){
				huffnode temp = hufftable[left];
				hufftable[left] = hufftable[s];
				hufftable[s] = temp;
				reheap(left, e);
			}
		}
	}
}

void generateBitCodes(int start, int end, huffCode bitCode){


	if (hufftable[start].left == -1 && hufftable[start].right == -1){
		//leaf 
		huffMap[hufftable[start].glyph] = bitCode;
		bitCode = "";
		return;
	}

	huffCode leftPr = bitCode + "0";
	generateBitCodes(hufftable[start].left, end, leftPr);

	huffCode rightPr = bitCode + "1";
	generateBitCodes(hufftable[start].right, end, rightPr);
}

void log(string l){
	cout << "DEBUG: " << l << '\n';
}

string createNewHuffFile(string inputPath)
{
	int lastindex = inputPath.find_last_of(".");

	string rawname = inputPath.substr(0, inputPath.find_last_of(".")) + ".huf";
	return rawname;
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

void printBitCodes()
{
	for (int i = 0; i < MAXSIZE; i++)
	{
		if (huffMap[i] != "")
		{
			cout << (char)i << ":" << huffMap[i] << '\n';
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
			cout << setw(4) << i << " G= " << setw(2) << ((hufftable[i].glyph > 0) ? (char)hufftable[i].glyph : -1) <<
				" F= " << setw(4) << hufftable[i].freq <<
				" L= " << setw(2) << hufftable[i].left <<
				" R= " << setw(2) << hufftable[i].right << '\n';
		}
	}

	cout << '\n';
}


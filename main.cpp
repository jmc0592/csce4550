/*
 * Jacob Cole
 * main.cpp
 * This program is an implementation of a simplified AES encryption method.
 * It will read in a plain text file, a keyfile, and output the encrypted text of the plain file.
 */
#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>
#include <cctype>
using namespace std;

bool checkFileExists(string );
string preProcess(string );
string stripString(string );
string substitution(string );
void getKeyFromFile(string );
string padding(string );
string shiftRows(string );
void printBlocks(string , string );
unsigned char * parity(string );
void printHex(unsigned char *, string , int );
void mixColumns(unsigned char * );
unsigned char rgfMul(unsigned char , int );

string key, outputFileName;
int g_textLength;

//check if file exists. returns true if it does, false if not.
bool checkFileExists(string fileName) 
{
    ifstream file(fileName.c_str());

    if(file.good()) {
        file.close();
        return true;
    } else {
        file.close();
        return false;
    }
}

//remove punctuation and white spaces
string stripString(string s)
{
    string result;
    //remove puntuation
    remove_copy_if(s.begin(), s.end(), back_inserter(result), ptr_fun<int, int>(&ispunct));
    //remove whitespace
    result.erase(remove(result.begin(), result.end(), ' '), result.end());

    return result;
}

void printBlocks(string text, string step)
{    
    ofstream outputFile;    
    
    outputFile.open(outputFileName.c_str(), ofstream::app | ofstream::out);

    //send to std out and output file
    cout << step << endl;
    outputFile << step << endl;
    int j = 0;
    int k = 0;
    for(int i=0; i < text.length(); i++)
    {
        //end of 4x4
        if (k == 15) {
            cout << text[i] << endl << endl;
            outputFile << text[i] << endl << endl;
            j = k = 0;
        }
        //end of row 
        else if(j == 3) {
            cout << text[i] << endl;
            outputFile << text[i] << endl;
            j = 0;
            k++;
        }
        //rest of characters 
        else {
            cout << text[i];
            outputFile << text[i];
            j++;
            k++;
        }
    }
    outputFile.close();
}

void printHex(unsigned char *text, string step, int length)
{
    ofstream outputFile;    
    
    outputFile.open(outputFileName.c_str(), ofstream::app | ofstream::out);

    //send to std out and output file
    cout << step << endl;
    outputFile << step << endl;
    int j = 0;
    for(int i=0; i < length; i++)
    {
        //end of row
    	if(j == 3) {
            printf("%x\n", text[i]);
            outputFile << hex << int(text[i]) << endl;
            j = 0;
        }
        //rest of characters 
        else {
            printf("%x ", text[i]);
            outputFile << hex << int(text[i]) << " ";
            j++;
        }
    }
    outputFile << endl;
    cout << endl;
    if(step == "MixColumns: ") {
        //write the ending to std out and the output file
        cout << endl << "---End of AES---" << endl;
        outputFile << endl << "--End of AES---" << endl;
    }
    outputFile.close();
}

string preProcess(string pFileName)
{
    ifstream pTextFile(pFileName.c_str());
    ofstream outputFile;
    string tmp, line, cleanFileText;
    //clean string each line of file
    while(getline(pTextFile, line))
    {
        tmp = stripString(line);
        cleanFileText.append(tmp);
    }
    tmp.clear();
    pTextFile.close();
    
    //Send cleanfile to std out
    cout << "---Start of AES---" << endl << endl;
    cout << "Preprocessing:" << endl;
    cout << cleanFileText << endl << endl;

    //send to output file
    outputFile.open(outputFileName.c_str(), ofstream::app | ofstream::out);
    outputFile << "---Start of AES---\n\n";
    outputFile << "Preprocessing:" << endl;
    outputFile << cleanFileText << endl << endl;
    outputFile.close();

    return cleanFileText;
}

void getKeyFromFile(string keyFileName)
{
    ifstream file(keyFileName.c_str());
    getline(file, key);
}

//substitution
string substitution(string s)
{    
    ofstream outputFile;

    //vigenere algorithm
    int j=0;
    for(int i = 0; i < s.length(); i++)
    {   
        s[i] = 'A' + (((s[i] - 'A') + (key[j] - 'A')) % 26);
        //reset the key counter if it's at the last index of key
        if(j == key.length() - 1)
            j = 0;
        else 
            j++;
    }

    //send result to std out
    cout << "Substitution:" << endl;
    cout << s << endl << endl;

    //send results to output file
    outputFile.open(outputFileName.c_str(), ofstream::app | ofstream::out);
    outputFile << "Substitution:" << endl;
    outputFile << s << endl << endl;
    outputFile.close();
    
    return s;
}

string padding(string text)
{    
    char padChar = 'A';
	int padNum;
	ofstream outputFile;
	
	//get amount of padding characters to add
	padNum = 16 - (text.length() % 16);
	
	//add pad characters if a block is started but not finished
	if(padNum != 16)
	{
		for(int i=0; i < padNum; i++)
		{
			text.push_back(padChar);
		}
	}
	
    printBlocks(text, "Padding: ");

	return text;
}

string shiftRows(string text)
{	
	int rowCount = 1;
    int j = 0;
	string temp;
	int originalLen = text.length();

	for(int i=0; i < originalLen; i++)
	{
        //end of row
        if(j == 3){
            if (rowCount == 1) {
                //no shift
                rowCount++;
            }
            else if(rowCount == 2) {
                //shift left 1
                temp = text[i-3];
                text.erase(i-3, 1);
                text.insert(i, temp);
                rowCount++;

            } else if(rowCount == 3) {
                //shift left 2
                temp = text.substr(i-3, 2);
                text.erase(i-3, 2);
                text.insert(i-1, temp);
                rowCount++;
            } else if(rowCount == 4) {
                //shift left 3                
                temp = text[i];
                text.erase(i, 1);
                text.insert(i-3, temp);
                rowCount = 1;
            }
            j = 0;
        } else {
	        j++;
		}
	} 

    printBlocks(text, "ShiftRows:");
	
	return text;
}

unsigned char * parity(string text)
{
	unsigned char *charText = (unsigned char*) text.c_str();
	int textLength = text.length();
    g_textLength = textLength;//store in global to use later.
	int mask = 0x01;
	int countBits;

	for(int i=0; i < text.length(); i++)
	{
		countBits = 0;
		//loop through each bit of character increment bit counter if found
		for (int j = 0; j < 8; j++) {
			int k = (charText[i] >> j) & 1;//obtain bit
			if(k == 1)
				countBits++;
		}

		//if number of 1's is odd, set most sig bit to 1
		if(countBits % 2 == 1) {
			charText[i] = charText[i] ^ 10000000;//flip the most sig bit to 1
		}
	}
	printHex(charText, "Parity Bit: ", textLength);

	return charText;
}

unsigned char rgfMul(unsigned char c, int mult)
{
    unsigned char d;

    //multiplication by 3
    if(mult == 3) {
        d = c << 1;
        d = (d ^ c);

    }
    //multiplication by 2 
    else {
        d = c << 1;
    }

    //check if most sig bit of c is 1
    if(((c >> 7) & 1) == 1) {
        d = d ^ 27;
    }

    return d;
}

void mixColumns(unsigned char *text)
{
    unsigned char *mixedCharText;
    mixedCharText = new unsigned char[g_textLength];//initialize new char array to same size as previous
    int countBits, index;
    int numOfBlocks = g_textLength / 16;

    //loop through number of blocks
    for(int i=0; i < numOfBlocks; i++)
    {
        //get index of first character in 4x4 block
        index = 16 * i;

        //loop through each column in 4x4
        for(int j=0; j < 4; j++)
        {
            mixedCharText[index] = rgfMul(text[index],2) ^ rgfMul(text[index+4],3) ^ text[index+8] ^ text[index+12];//a0
            mixedCharText[index+4] = text[index] ^ rgfMul(text[index+4],2) ^ rgfMul(text[index+8],3) ^ text[index+12];//a1
            mixedCharText[index+8] = text[index] ^ text[index+4] ^ rgfMul(text[index+8],2) ^ rgfMul(text[index+12],3);//a2
            mixedCharText[index+12] = rgfMul(text[index],3) ^ text[index+4] ^ text[index+8] ^ rgfMul(text[index+12],2);//a3

            index++;
        }
    }
    printHex(mixedCharText, "MixColumns: ", g_textLength);
}

int main()
{

    string plaintTextFileName, keyFileName, substitutionText, paddedText, shiftedText, cleanFileText;
    unsigned char *parityText;
    bool fileExistsFlag;

    //prompt user to input plaintext file until they input file name that exists
    fileExistsFlag = false;
    while(fileExistsFlag == false) {
        cout << "Enter the name of the plaintext file: ";
        cin >> plaintTextFileName;
        
        fileExistsFlag = checkFileExists(plaintTextFileName);
        if(fileExistsFlag == false) 
	    cout << "Error: File does not exist." << endl;
    }

    //prompt user to input key file until they input file name that exists
    fileExistsFlag = false;
    while(fileExistsFlag == false) {
        cout << "Enter the name of the key file: ";
        cin >> keyFileName;
        
        fileExistsFlag = checkFileExists(keyFileName);
        if(fileExistsFlag == false) 
            cout << "Error: File does not exist." << endl;
    }
    getKeyFromFile(keyFileName);//store key to global variable

    //prompt user for output file. will create it later if it doesn't exist.
    cout << "Enter the name of the output file: ";
    cin >> outputFileName;

	/*AES Logic*/
    cleanFileText = preProcess(plaintTextFileName);

    substitutionText = substitution(cleanFileText);

    paddedText = padding(substitutionText);
	
	shiftedText = shiftRows(paddedText);

	parityText = parity(shiftedText);

	mixColumns(parityText);
	/*End AES Logic*/

    return 0;
    
}

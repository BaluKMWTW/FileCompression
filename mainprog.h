//
// This file is the driver for the huffman encoding application.
//

#pragma once

#include <iostream>
#include <map>
#include <unordered_map>
#include <fstream>
#include <queue>
#include <vector>
#include <functional>
#include <ctype.h>
#include <math.h>
#include "hashmap.h"
#include "bitstream.h"
#include "util.h"
using namespace std;

// Function prototypes
string menu();
bool is123456(string choice);
void do123456(string choice, string &filename, bool &isFile,
             hashmapF &frequencyMap,
             HuffmanNode* &encodingTree,
             hashmapE &encodingMap);
string printChar(int val);
void printMap(hashmapE &map);
void printMap(hashmapF &map);
void printTree(HuffmanNode* node, string str);
void printTextFile(string filename);
void printBinaryFile(string filename);

int go() {
    
    hashmapF frequencyMap;
    HuffmanNode* encodingTree = nullptr;
    hashmapE encodingMap;
    string filename;
    bool isFile = true;
    
    
    string choice = "wee";
    while (choice != "Q") {
        choice = menu();
        if (is123456(choice)){
            do123456(choice, filename, isFile, frequencyMap,
                    encodingTree, encodingMap);
        } else if (choice == "C") {
            cout << "Enter filename: ";
            cin >> filename;
            compress(filename);
        } else if (choice == "D") {
            cout << "Enter filename: ";
            cin >> filename;
            decompress(filename);
        } else if (choice == "B") {
            cout << "Enter filename: ";
            cin >> filename;
            printBinaryFile(filename);
        } else if (choice == "T") {
            cout << "Enter filename: ";
            cin >> filename;
            printTextFile(filename);
        }
    }

    return 0;
}

//
// menu
// Prints message to screen and gets response from keyboard.
//
string menu() {
    cout << "Welcome to the file compression app!" << endl;
    cout << "1.  Build character frequency map" << endl;
    cout << "2.  Build encoding tree" << endl;
    cout << "3.  Build encoding map" << endl;
    cout << "4.  Encode data" << endl;
    cout << "5.  Decode data" << endl;
    cout << "6.  Free tree memory" << endl;
    cout << endl;
    cout << "C.  Compress file" << endl;
    cout << "D.  Decompress file" << endl;
    cout << endl;
    cout << "B.  Binary file viewer" << endl;
    cout << "T.  Text file viewer" << endl;
    cout << "Q.  Quit" << endl;
    cout << endl;
    
    cout << "Enter choice: ";
    string choice;
    cin >> choice;
    return choice;
}

//
// is123456
// This function checks if choice is 1, 2, 3, 4, 5, or 6.
//
bool is123456(string choice) {
    if (choice == "1" || choice == "2" ||choice == "3" ||
        choice == "4" ||choice == "5" || choice == "6") {
        return true;
    } else {
        return false;
    }
}

//
// do123456
// This function runs code for choice equal to 1, 2, 3, 4, 5, and 6.
// Correct, this is not properly decomposed.  
//
void do123456(string choice, string &filename, bool &isFile,
             hashmapF &frequencyMap,
             HuffmanNode* &encodingTree,
             hashmapE &encodingMap) {
    // gets file/string and filename.
    if (choice == "1") {
        cout << "[F]ilename or [S]tring? ";
        string fORs;
        cin >> fORs;
        isFile = (fORs == "F" ? true : false);
        if (isFile) {
            cout << "Enter file name: ";
        } else {
            cout << "Enter string: ";
        }
        cin >> filename;
    }
    // Build Frequency Map
    if (choice == "1") {
        buildFrequencyMap(filename, isFile, frequencyMap);
        cout << endl;
        cout << "Building frequency map..." << endl;
        printMap(frequencyMap);
        cout << endl;
    // Build Encoding Tree
    } else if (choice == "2") {
        encodingTree = buildEncodingTree(frequencyMap);
        cout << endl;
        cout << "Building encoding tree..." << endl;
        printTree(encodingTree, "");
        cout << endl;
    // Build Encoding Map
    } else if (choice == "3") {
        encodingMap = buildEncodingMap(encodingTree);
        cout << endl;
        cout << "Building encoding map..." << endl;
        printMap(encodingMap);
        cout << endl;
    // Encode text
    } else if (choice == "4") {
        // this step is only valid for files
        if (!isFile) {
            cout << endl;
            cout << "********************************" << endl;
            cout << "Must provide file to run encode." << endl;
            cout << "Enter Q to start over and try again." << endl;
            cout << "********************************" << endl;
            cout << endl;
            return;
        }
        cout << endl;
        cout << "Encoding..." << endl;
        
        string fn = (isFile) ? filename : ("file_" + filename + ".txt");
        
        ofbitstream output(filename + ".huf");
        ifstream input(filename);
        
        stringstream ss;
        // note: << is overloaded for the hashmap class.  super nice!
        ss << frequencyMap;
        output << frequencyMap;  // add the frequency map to the file
        int size = 0;
        string codeStr = encode(input, encodingMap, output, size, true);
        // count bytes in frequency map header
        size = ss.str().length() + ceil((double)size / 8);
        cout << "Compressed file size: " << size << endl;
        cout << codeStr << endl;
        cout << endl;
        output.close();  // must close file so autograder can open for testing
    // Decode text
    } else if (choice == "5") {
        // this step is only valid for files
        if (!isFile) {
            cout << endl;
            cout << "********************************" << endl;
            cout << "Must provide file to run encode." << endl;
            cout << "Enter Q to start over and try again." << endl;
            cout << "********************************" << endl;
            cout << endl;
            return;
        }
        cout << endl;
        cout << "Decoding..." << endl;
        size_t pos = filename.find(".huf");
        if ((int)pos >= 0) {
            filename = filename.substr(0, pos);
        }
        pos = filename.find(".");
        string ext = filename.substr(pos, filename.length() - pos);
        filename = filename.substr(0, pos);
        ifbitstream input(filename + ext + ".huf");
        ofstream output(filename + "_unc" + ext);
        
        hashmapF dump;
        input >> dump;  // get rid of frequency map at top of file
        
        string decodeStr  = decode(input, encodingTree, output);
        cout << decodeStr << endl;
        cout << endl;
        output.close(); // must close file so autograder can open for testing
    // Free the Encoding Tree
    } else if (choice == "6") {
        cout << "Freeing encoding tree..." << endl;
        freeTree(encodingTree);
    }
}

//
// printChar
// This function takes in an integer value and prints the ASCII character with
// singles quotes.  e.g. val = 98, 'b' is returned.
//
string printChar(int ch){
    if (ch == '\n') {
        return "'\\n'";
    } else if (ch == '\t') {
        return "'\\t'";
    } else if (ch == '\r') {
        return "'\\r'";
    } else if (ch == '\f') {
        return "'\\f'";
    } else if (ch == '\b') {
        return "'\\b'";
    } else if (ch == '\0') {
        return "'\\0'";
    } else if (ch == ' ') {
        return "' '";
    } else if (ch == (int) PSEUDO_EOF) {
        return "EOF";
    } else if (ch == (int) NOT_A_CHAR) {
        return "N/A";
    } else {
        return string("'") + (char) ch + string("'");
    }
}

//
// printFrequencyMap
//
//
void printMap(hashmapF &map) {
    for (int key : map.keys()) {
        cout << key << ": " << '\t' << printChar(key);
        cout << '\t' << "-->" << '\t' << map.get(key) << endl;
    }
}

//
// printEncodingMap
//
//
void printMap(hashmapE &map) {
    for (auto e : map) {
        cout << e.first << ": " << '\t' << printChar(e.first);
        cout << '\t' << "-->" << '\t' << e.second << endl;
    }
}

//
// printTree
//
//
void printTree(HuffmanNode* node, string str) {
    if (node == nullptr) {
        return;
    } else {
        cout << str << "{" << printChar(node->character);
        if (node->character != NOT_A_CHAR)
            cout << "(" << node->character << ")";
        cout << ", count=" << node->count << "}" << endl;
        printTree(node->zero, str+" ");
        printTree(node->one, str+" ");
    }
}

//
// printTextFile
//
//
void printTextFile(string filename) {
    cout << filename << endl;
    ifstream inFile(filename);
    if (!inFile.is_open()) {
        cout << "File does not exist." << endl;
    }
    while (true) {
        char ch = inFile.get();
        if (ch == EOF) break;
        cout << ch;
    }
    cout << endl;
}

//
// printBinaryFile
//
//
void printBinaryFile(string filename) {
    cout << filename << endl;
    ifbitstream input(filename);
    if (!input.is_open()) {
        cout << "File does not exist." << endl;
    }
    int i = 0;
    while (true) {
        i++;
        int bit = input.readBit();
        if (input.fail()) break;
        cout << bit;
        if (i > 0 && i % 8 == 0) {
            cout << " ";
        }
        if (i > 0 && i % 64 == 0) {
            cout << endl;
        }
    }
    cout << endl;
}

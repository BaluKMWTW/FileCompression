// UIC CS 251 fall 2022
// Jan Komorowski
#pragma once
#include "bitstream.h"
#include "hashmap.h"
#include "hashmap.h"
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <queue>

using namespace std;

typedef hashmap hashmapF;
typedef unordered_map <int, string> hashmapE;

struct HuffmanNode {
    int character;
    int count;
    HuffmanNode* zero;
    HuffmanNode* one;
};
// class for priority queue
// compare lhs and rhs based on count
class prioritize {
public:
    bool operator()(HuffmanNode*& lhs, HuffmanNode*& rhs) {
        return lhs->count > rhs->count;
    }
};
struct compare
{
    bool operator()(const HuffmanNode *lhs,
        const HuffmanNode *rhs)
    {
        return lhs->count > rhs->count;
    }
};

//
// *This method frees the memory allocated for the Huffman tree.
//
void freeTree(HuffmanNode* node) {
    if(!node)
        return;
    freeTree(node->zero);
    freeTree(node->one);
    delete node;
}
void char_to_hash(char c, hashmap& h) {
    if(h.containsKey(int(c))) {// contains -> +1
        h.put(int(c), h.get(c) + 1);
    }else { // doesn't contain -> 1
        h.put(int(c), 1);
    }
}
//
// *This function build the frequency map.  If isFile is true, then it reads
// from filename.  If isFile is false, then it reads from a string filename.
//
void buildFrequencyMap(string filename, bool isFile, hashmapF &map) {
    if(isFile) { // build from file
        ifstream fs(filename);
        char c;
        while(fs.get(c))
            char_to_hash(c,map);
        fs.close();
    }else { // build from string
        for(char c: filename) {
            char_to_hash(c,map);
        }
    }
    // add EOF -> 1
    map.put(256, 1);
}
// helper function for buildEncodingTree
void build_htree(priority_queue<HuffmanNode*, vector<HuffmanNode*>, prioritize>& pq) {
    while (pq.size() > 1) {
        //cout << "hello" << endl;
        //cout << pq.size() << endl;
        HuffmanNode* first = pq.top();
        pq.pop();
        HuffmanNode* second = pq.top();
        pq.pop();
        HuffmanNode* link = new HuffmanNode();
        link->character = 257;
        link->count = first->count + second->count;
        link->zero = first;
        link->one = second;
        pq.push(link);
    }
}
//
// *This function builds an encoding tree from the frequency map.
//
HuffmanNode* buildEncodingTree(hashmapF &map) {
    // vector with keys
    vector<int> k = map.keys();
    priority_queue<HuffmanNode*, vector<HuffmanNode*>, prioritize> pq;
    for(auto e: k) { // create new hoffman node with giiven data and push to pq
        HuffmanNode* hnode = new HuffmanNode();
        hnode->zero = nullptr;
        hnode->one = nullptr;
        hnode->character = e;
        hnode->count = map.get(e);
        pq.push(hnode);
    }
    build_htree(pq);
    return pq.top();
}

//
// *Recursive helper function for building the encoding map.
//
void _buildEncodingMap(HuffmanNode* node, hashmapE &encodingMap, string str) { // HuffmanNode* prev is useless
    // traverse left
    if(node->zero)
        _buildEncodingMap(node->zero, encodingMap, str+"0");
    // traverse right
    if(node->one)
        _buildEncodingMap(node->one, encodingMap, str+"1");
    // reached leaf -> add to map
    if(node->character != 257)
        encodingMap.insert({node->character, str});
}

//
// *This function builds the encoding map from an encoding tree.
//
hashmapE buildEncodingMap(HuffmanNode* tree) {
    hashmapE encodingMap;
    string str;
    HuffmanNode* node = tree;
    _buildEncodingMap(node, encodingMap, str);
    
    return encodingMap;
}

//
// *This function encodes the data in the input stream into the output stream
// using the encodingMap.  This function calculates the number of bits
// written to the output stream and sets result to the size parameter, which is
// passed by reference.  This function also returns a string representation of
// the output file, which is particularly useful for testing.
//
string encode(ifstream& input, hashmapE &encodingMap, ofbitstream& output,
              int &size, bool makeFile) {
    string encoded;
    char c;
    // every time string is updated, update the size
    while(input.get(c)) {
        encoded += encodingMap[int(c)];
        size += encodingMap[int(c)].size();
    }
    // eof
    encoded += encodingMap[256];
    size += encodingMap[256].size();
    // make file?
    if(makeFile)
        for(char c: encoded) {
            if(c == '0') {
                output.writeBit(0);
            }else {
                output.writeBit(1);
            }
        }
    
    return encoded;
}


//
// *This function decodes the input stream and writes the result to the output
// stream using the encodingTree.  This function also returns a string
// representation of the output file, which is particularly useful for testing.
//
string decode(ifbitstream &input, HuffmanNode* encodingTree, ofstream &output) {
    string decoded;
    HuffmanNode* node = encodingTree;
    while(!input.eof()) {
        if(node->character != 257) { // leaf node
            if(node->character == 256){
                return decoded;
            }else {
                output.put(char(node->character));
                decoded += char(node->character);
                node = encodingTree;
            }
        }
        if(input.readBit() == 0) {
            node = node->zero;
        }else {
            node = node->one;
        }
    }
    return decoded;
}

//
// *This function completes the entire compression process.  Given a file,
// filename, this function (1) builds a frequency map; (2) builds an encoding
// tree; (3) builds an encoding map; (4) encodes the file (don't forget to
// include the frequency map in the header of the output file).  This function
// should create a compressed file named (filename + ".huf") and should also
// return a string version of the bit pattern.
//
string compress(string filename) {
    int size = 0;
    hashmap frequency;
    buildFrequencyMap(filename, true, frequency);
    HuffmanNode* encodingTree = buildEncodingTree(frequency);
    hashmapE encodingMap = buildEncodingMap(encodingTree);

    // create encoded file
    ofbitstream output(filename + ".huf");
    output << frequency;
    ifstream fs(filename);
    string encoded = encode(fs, encodingMap, output, size, true);
    
    // free the memory
    fs.close();
    output.close();
    freeTree(encodingTree);

    return encoded;
}

//
// *This function completes the entire decompression process.  Given the file,
// filename (which should end with ".huf"), (1) extract the header and build
// the frequency map; (2) build an encoding tree from the frequency map; (3)
// using the encoding tree to decode the file.  This function should create a
// compressed file using the following convention.
// If filename = "example.txt.huf", then the uncompressed file should be named
// "example_unc.txt".  The function should return a string version of the
// uncompressed file.  Note: this function should reverse what the compress
// function did.
//
string decompress(string filename) {
    hashmap frequency;
    ifbitstream bs(filename);
    filename = filename.substr(0,filename.size()-8) + "_unc.txt";
    ofstream os(filename);

    bs >> frequency;
    HuffmanNode* encodingTree = buildEncodingTree(frequency);
    string decoded = decode(bs, encodingTree, os);

    bs.close();
    os.close();
    freeTree(encodingTree);

    return decoded;
}

#include "hashmap.h"
#include <vector>
using namespace std;

//
// This constructor chooses number of buckets, initializes size of map to 0, and
// creates a bucket array with nBuckets number of pointers to linked lists.
//
hashmap::hashmap() {
    this->nBuckets=10;
    this->nElems=0;
    this->buckets=createBucketArray(nBuckets);
}

//
// This destructor frees memory for all elements of buckets & also all elements
// of linked lists that those bucket pointers may point to.
//
hashmap::~hashmap() {
    for (int i=0; i < nBuckets; i++){
        key_val_pair *front =  buckets[i];
        while (front != nullptr) {
            key_val_pair *temp = front->next;
            delete front;
            front = temp;
        }
    }
    delete[] buckets;
    
}

//
// This method puts key/value pair in the map.  It checks to see if key is
// already in map while traversing the list to find the end of it.
//
void hashmap::put(int key, int value) {
    int bucketChoice = hashFunction(key);
    if (buckets[bucketChoice % nBuckets] == nullptr) {
        nElems++;
        key_val_pair *newNode = new key_val_pair;
        newNode -> key = key;
        newNode -> value = value;
        newNode -> next = nullptr;
        // place new key/value pair in location of buckets
        buckets[bucketChoice % nBuckets] = newNode;
    } else {
        bool flag = false;
        // now find where the new key/value pair will go,
        // need to traverse linked list
        key_val_pair *node = buckets[bucketChoice % nBuckets];
        key_val_pair *prev = node;
        while (node != nullptr && !flag) {  //traverses linked list
            if (node -> key == key) {
                node -> value = value;
                flag = true;  
            }
            prev = node;
            node = node -> next;
        }

        //add the key/value pair to memory if it is new
        if (!flag) {
            nElems++;
            key_val_pair *newNode = new key_val_pair;
            newNode -> key = key;
            newNode -> value = value;
            newNode -> next = nullptr; 
            prev -> next = newNode;
        }
    }
}

//
// This method returns the value associated with key.
//
int hashmap::get(int key) const {
    int bucketChoice = hashFunction(key);
    key_val_pair *node = buckets[bucketChoice % nBuckets];
    int saveValue = 0;
    if (node == nullptr) {
        throw("Error: Key is not in map.");
    } else {
        while (node != nullptr) {
            if (node -> key == key) {
                saveValue = node -> value;
                return saveValue;
            }
            node = node -> next;
        }
    }
    // this should never be called
    return saveValue;
}

//
// This function checks if the key is already in the map.
//
bool hashmap::containsKey(int key) {
    int bucketChoice = hashFunction(key);
    key_val_pair *nodeCheck = buckets[bucketChoice % nBuckets];
    if (nodeCheck!=nullptr) {
        while (nodeCheck != nullptr) {  //traverses linked list
            if (nodeCheck -> key == key) { //check to see if key in linked list matches
                return true;
            }
            nodeCheck = nodeCheck -> next;  //go to next pointer
        }
        return false;
    } else {
        return false;
    }
}

//
// This method goes through all buckets and adds all keys to a vector.
//
vector<int> hashmap::keys() const {
     vector<int> keyVec;
     for (int i=0; i < nBuckets; i++){
         if ( buckets[i] != nullptr ){
             key_val_pair *node = buckets[i];
             while (node != nullptr) {
                 keyVec.push_back(node->key);
                 node = node -> next;
             }
         }
     }
     return keyVec;
}

//
// This function returns the number of elements in the hashmap.
//
int hashmap::size() {
    return nElems;
}

//
// Copy constructor
//
hashmap::hashmap(const hashmap &myMap) {
    // make a deep copy of the map
    nBuckets = myMap.nBuckets;

    buckets = createBucketArray(nBuckets);

    // walk through the old array and add all elements to this one
    vector<int> keys = myMap.keys();
    for (size_t i=0; i < keys.size(); i++) {
        int key = keys[i];
        int value = myMap.get(key);
        put(key,value);
    }

}

//
// Equals operator.
//
hashmap& hashmap::operator= (const hashmap &myMap) {
    // make a deep copy of the map

    // watch for self-assignment
    if (this == &myMap) {
        return *this;
    }

    // if data exists in the map, delete it
    for (int i=0; i < nBuckets; i++) {
        hashmap::key_val_pair* bucket = buckets[i];
        while (bucket != nullptr) {
            // walk the linked list and delete each node
            hashmap::key_val_pair* temp = bucket;
            bucket = bucket->next;
            delete temp;
        }
        buckets[i] = nullptr;
    }
    nElems = 0;
    // walk through the old array and add all elements to this one
    vector<int> keys = myMap.keys();
    for (size_t i=0; i < keys.size(); i++) {
        int key = keys[i];
        int value = myMap.get(key);
        put(key,value);
    }

    // return the existing object so we can chain this operator
    return *this;
}

//
// This function overloads the << operator, which allows for ease in printing
// to screen or inserting into a stream, in general.
//
ostream &operator<<(ostream &out, hashmap &myMap) {
    out << "{";
    vector<int> keys = myMap.keys();
    for (size_t i=0; i < keys.size(); i++) {
        int key = keys[i];
        int value = myMap.get(key);
        out << key << ":" << value;
        if (i < keys.size() - 1) { // no commas after the last one
            out << ", ";
        }
    }
    out << "}";
    return out;
}

//
// This function overloads the >> operator, which allows for ease at extraction
// from streams/files.
//
istream &operator>>(istream &in, hashmap &myMap) {
    // assume the format {1:2, 3:4}
    bool done = false;
    in.get(); // get the first char, {
    int nextChar = in.get(); // get the first real character
    while (!done) {
        string nextInput;
        while (nextChar != ',' and nextChar != '}') {
                nextInput += nextChar;
                nextChar = in.get();
        }
        if (nextChar == ',') {
            // read the space as well
            in.get(); // should be a space
            nextChar = in.get(); // get the next character
        } else {
            done = true; // we have reached }
        }
        // at this point, nextInput should be in the form 1:2
        // (we should have two integers separated by a colon)
        // BUT, we might have an empty map (special case)
        if (nextInput != "") {
            //vector<string> kvp;
            size_t pos = nextInput.find(":");
            myMap.put(stoi(nextInput.substr(0, pos)),
                      stoi(nextInput.substr(pos+1, nextInput.length() - 1)));
        }
    }
    return in;
}

//
// Given a number of buckets, creates a hashtable (array of linked list heads).
// @param nBuckets the number of buckets you want in the hashtable.
// return an array of heads of linked lists of key_val_pairs
//
hashmap::bucketArray hashmap::createBucketArray(int nBuckets) {
    bucketArray newBuckets = new key_val_pair*[nBuckets];
    for (int i = 0; i < nBuckets; i++) {
        newBuckets[i] = nullptr;
    }
    return newBuckets;
}

//
// The hash function for hashmap implementation.
// For an extension, you might want to improve this function.
//
// @param input - an integer to be hashed
// return the hashed integer
//
int hashmap::hashFunction(int input) const {
    // use unsigned integers for calculation
    // we are also using so-called "magic numbers"
    // see https://stackoverflow.com/a/12996028/561677 for details
    unsigned int temp = ((input >> 16) ^ input) * 0x45d9f3b;
    temp = ((temp >> 16) ^ temp) * 0x45d9f3b;
    temp = (temp >> 16) ^ temp;

    // convert back to positive signed int
    // (note: this ignores half the possible hashes!)
    int hash = (int) temp;
    if (hash < 0) {
        hash *= -1;
    }

    return hash;
}


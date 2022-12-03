/*
 * File: bitstream.h
 * -----------------
 * This file defines the ibitstream and obitstream classes which are basically
 * same as the ordinary istream and ostream classes, but add the
 * functionality to read and write one bit at a time.
 *
 * The idea is that you can substitute an ibitstream in place of an
 * istream and use the same operations (get, fail, >>, etc.)
 * along with added member functions of readBit, rewind, and size.
 *
 * Similarly, the obitstream can be used in place of ofstream, and has
 * same operations (put, fail, <<, etc.) along with additional
 * member functions writeBit and size.
 *
 * There are two subclasses of ibitstream: ifbitstream and istringbitstream,
 * which are similar to the ifstream and istringstream classes.  The
 * obitstream class similarly has ofbitstream and ostringbitstream as
 * subclasses.
 *
 
 * @author Keith Schwarz, Eric Roberts, Marty Stepp, Adam Koehler
 * @version 2022/11/12
 * - updated to use std::streampos to eliminate deprication warnings 
 * @version 2019/04/20
 * - added toPrintable(string)
 * @version 2018/09/25
 * - added doc comments for new documentation generation
 * @version 2016/11/12
 * - made toPrintable non-static and visible
 */
#ifndef _bitstream_h
#define _bitstream_h

#include <istream>
#include <ostream>
#include <fstream>
#include <sstream>

/**
 * Constant: PSEUDO_EOF
 * A constant representing the PSEUDO_EOF marker that you will
 * write at the end of your Huffman-encoded file.
 */
const int PSEUDO_EOF = 256;

/**
 * Constant: NOT_A_CHAR
 * A constant representing an extended character that does not
 * actually hold a value.  When you are constructing your Huffman
 * encoding tree, you should set the characters in each internal
 * node (non-leaf) to this value to explicitly mark that they are not
 * being used.
 */
const int NOT_A_CHAR = 257;

/**
 * Defines a class for reading files with all the functionality of istream
 * along with an added member function for reading a single bit and convenience
 * functions for rewinding the stream back to the beginning and getting the stream
 * size.
 *
 * You will probably not create instances of this class directly.  Instead, you
 * will create ifbitstreams or istringbitstreams to read from files or string buffers.
 */

static const int NUM_BITS_IN_BYTE = 8;

inline int GetNthBit(int n, int fromByte) {
    return ((fromByte & (1 << n)) != 0);
}

inline void SetNthBit(int n, int & inByte) {
    inByte |= (1 << n);
}

class ibitstream: public std::istream {
public:
    /* Constructor ibitstream::ibitstream
     * ----------------------------------
     * Each ibitstream tracks 3 integers as private data.
     * "lastTell" is streampos of the last byte that was read (this is used
     * to detect when other non-readBit activity has changed the tell)
     * "curByte" contains contents of byte currently being read
     * "pos" is the bit position within curByte that is next to read
     * We set initial state for lastTell and curByte to 0, then pos is
     * set at 8 so that next readBit will trigger a fresh read.
     */
    ibitstream() : std::istream(NULL), lastTell(0), curByte(0), pos(NUM_BITS_IN_BYTE) {
        this->fake = false;
    }
    /**
     * Initializes a new ibitstream that is not attached to any source.  You are
     * unlikely to use this function directly.
     */
    
    /* Member function ibitstream::readBit
     * -----------------------------------
     * If bits remain in curByte, retrieve next and increment pos
     * Else if end of curByte (or some other read happened), then read next byte
     * and start reading from bit position 0 of that byte.
     * If read byte from file at EOF, return EOF.
     */
    int readBit() {
        if (!is_open()) {
            //error("ibitstream::readBit: Cannot read a bit from a stream that is not open.");
        }
        
        if (this->fake) {
            int bit = get();
            if (bit == 0 || bit == '0') {
                return 0;
            } else {
                return 1;
            }
        } else {
            // if just finished bits from curByte or if data read from stream after last readBit()
            if (lastTell != tellg() || pos == NUM_BITS_IN_BYTE) {
                if ((curByte = get()) == EOF) {
                    // read next single byte from file
                    return EOF;
                }
                pos = 0; // start reading from first bit of new byte
                lastTell = tellg();
            }
            int result = GetNthBit(pos, curByte);
            pos++;   // advance bit position for next call to readBit
            return result;
        }
    }
    /**
     * Reads a single bit from the ibitstream and returns 0 or 1 depending on
     * the bit value.  If the stream is exhausted, EOF (-1) is returned.
     * Raises an error if this ibitstream has not been properly opened.
     */
    
    /* Member function ibitstream::rewind
     * ----------------------------------
     * Simply seeks back to beginning of file, so reading begins again
     * from start.
     */
    void rewind() {
        if (!is_open()) {
            //error("ibitstream::rewind: Cannot rewind stream that is not open.");
        }
        clear();
        seekg(0, std::ios::beg);
    }
    /**
     * Rewinds the ibitstream back to the beginning so that subsequent reads
     * start again from the beginning.  Raises an error if this ibitstream
     * has not been properly opened.
     */
    
    /**
     * Sets 'fake' mode, where it actually reads bytes when you say readBit.
     */
    void setFake(bool fake) {
        this->fake = fake;
    }
    
    
    /* Member function ibitstream::size
     * --------------------------------
     * Seek to file end and use tell to retrieve position.
     * In order to not disrupt reading, we also record cur streampos and
     * re-seek to there before returning.
     */
    long size() {
        if (!is_open()) {
            //error("ibitstream::size: Cannot get size of stream which is not open.");
        }
        clear();                    // clear any error state
        std::streampos cur = tellg();    // save current streampos
        seekg(0, std::ios::end);            // seek to end
        std::streampos end = tellg();    // get offset
        seekg(cur);                    // seek back to original pos
        return long(end);
    }
    /**
     * Returns the size in bytes of the data attached to this stream.
     * Raises an error if this ibitstream has not been properly opened.
     */
    
    
    /* Member function ibitstream::is_open
     * -----------------------------------
     * Default implementation of is_open has the stream always
     * open.    Subclasses can customize this if they'd like.
     */
    bool is_open() {
        return true;
    }
    /**
     * Returns whether or not this ibitstream is opened.  This only has
     * meaning if the ibitstream is a file stream; otherwise it always
     * returns true.
     */
    
private:
    std::streampos lastTell;
    int curByte;
    int pos;
    bool fake;
};


/**
 * Defines a class for writing files with all the functionality of ostream
 * along with an added member function for writing a single bit and a convenience
 * function for getting the stream size.
 *
 * You are unlikely to instantiate this class directly; instead, instantiate one
 * of the subclasses.
 */
class obitstream: public std::ostream {
public:
    /* Constructor obitstream::obitstream
     * ----------------------------------
     * Each obitstream tracks 3 integers as private data.
     * "lastTell" is streampos of the last byte that was written (this is used
     * to detect when other non-writeBit activity has changed the tell)
     * "curByte" contains contents of byte currently being written
     * "pos" is the bit position within curByte that is next to write
     * We set initial state for lastTell and curByte to 0, then pos is
     * set at 8 so that next writeBit will start a new byte.
     */
    obitstream() : std::ostream(NULL), lastTell(0), curByte(0), pos(NUM_BITS_IN_BYTE) {
        this->fake = false;
    }
    /**
     * Initializes a new obitstream that is not attached to any file.  Use the
     * open member function from ofstream to attach the stream to a file.
     */
    
    /* Member function obitstream::writeBit
     * ------------------------------------
     * If bits remain to be written in curByte, add bit into byte and increment pos
     * Else if end of curByte (or some other write happened), then start a fresh
     * byte at position 0.
     * We write the byte out for each bit (backing up to overwrite as needed), rather
     * than waiting for 8 bits.     This is because the client might make
     * 3 writeBit calls and then start using << so we can't wait til full-byte
     * boundary to flush any partial-byte bits.
     */
    void writeBit(int bit) {
        if (bit != 0 && bit != 1) {
            //error(std::string("obitstream::writeBit: must pass an integer argument of 0 or 1. You passed the integer ")
               //   + toPrintable(bit) + " (" + integerToString(bit) + ").");
        }
        //if (!is_open()) {
            //error("obitstream::writeBit: stream is not open");
        //}
        
        if (this->fake) {
            put(bit == 1 ? '1' : '0');
        } else {
            // if just filled curByte or if data written to stream after last writeBit()
            if (lastTell != tellp() || pos == NUM_BITS_IN_BYTE) {
                curByte = 0;   // zero out byte for next writes
                pos = 0;       // start writing to first bit of new byte
            }
            
            if (bit) {
                // only need to change if bit needs to be 1 (byte starts already zeroed)
                SetNthBit(pos, curByte);
            }
            
            if (pos == 0 || bit) {   // only write if first bit in byte or changing 0 to 1
                if (pos != 0) {
                    seekp(-1, std::ios::cur);   // back up to overwite if pos > 0
                }
                put(curByte);
            }
            
            pos++; // advance to next bit position for next write
            lastTell = tellp();
        }
    }
    /**
     * Writes a single bit to the obitstream.
     * Raises an error if this obitstream has not been properly opened.
     */
    
    
    /* Member function obitstream::size
     * --------------------------------
     * Seek to file end and use tell to retrieve position.
     * In order to not disrupt writing, we also record cur streampos and
     * re-seek to there before returning.
     */
    long size() {
        //if (!is_open()) {
            //error("obitstream::size: stream is not open");
        //}
        clear();                    // clear any error state
        std::streampos cur = tellp();    // save current streampos
        seekp(0, std::ios::end);            // seek to end
        std::streampos end = tellp();    // get offset
        seekp(cur);                    // seek back to original pos
        return long(end);
    }
    /**
     * Returns the size in bytes of the file attached to this stream.
     * Raises an error if this obitstream has not been properly opened.
     */
    
    
    void setFake(bool fake) {
        this->fake = fake;
    }
    /**
     * Sets 'fake' mode, where it actually writes bytes when you say writeBit.
     */
    
    /**
     * Returns whether or not this obitstream is opened.  This only has
     * meaning if the obitstream is a file stream; otherwise it always
     * returns true.
     */
    
private:
    std::streampos lastTell;
    int curByte;
    int pos;
    bool fake;
};

/**
 * A class for reading files in all of the usual ways, plus bit-by-bit.
 * You can treat this class like a normal ifstream, except that there is
 * extra support for bit-level operations.
 */
class ifbitstream: public ibitstream {
public:
    /* Constructor ifbitstream::ifbitstream
     * ------------------------------------
     * Wires up the stream class so that it knows to read data
     * from disk.
     */
    ifbitstream() {
        init(&fb);
    }
    /**
     * Constructs a new ifbitstream not attached to any file.  You can
     * open a file for reading using the .open() member functions.
     */
    
    
    /* Constructor ifbitstream::ifbitstream
     * ------------------------------------
     * Wires up the stream class so that it knows to read data
     * from disk, then opens the given file.
     */
    ifbitstream(const char* filename) {
        init(&fb);
        open(filename);
    }
    /**
     * Constructs a new ifbitstream that reads the file with the given name,
     * if it exists.  If not, the stream enters an error state.
     */
    
    /**
     * Constructs a new ifbitstream that reads the specified file, if
     * it exists.  If not, the stream enters an error state.
     */
    ifbitstream(const std::string& filename) {
        init(&fb);
        open(filename);
    }
    
    
    /* Member function ifbitstream::open
     * ---------------------------------
     * Attempts to open the specified file, failing if unable
     * to do so.
     */
    void open(const char* filename) {
        if (!fb.open(filename, std::ios::in | std::ios::binary)) {
            setstate(std::ios::failbit);
        }
    }
    /**
     * Opens the specified file for reading.  If an error occurs, the
     * stream enters a failure state, which can be detected by calling
     * ifb.fail().
     */
    
    /**
     * Opens the specified file for reading.  If an error occurs, the
     * stream enters a failure state, which can be detected by calling
     * ifb.fail().
     */
    void open(const std::string& filename) {
        open(filename.c_str());
    }
    
    /**
     * Returns whether or not this ifbitstream is connected to a file for
     * reading.
     */
    bool is_open() {
        return fb.is_open();
    }
    
    /**
     * Closes the currently-opened file, if the stream is open.  If the
     * stream is not open, puts the stream into a fail state.
     */
    void close() {
        if (!fb.close()) {
            setstate(std::ios::failbit);
        }
    }
    
private:
    // the actual file buffer which does reading and writing.
    std::filebuf fb;
};

/**
 * A class for writing files in all of the usual ways, plus bit-by-bit.
 * You can treat this class like a normal ofstream, except that there is
 * extra support for bit-level operations.
 *
 * As a safety feature, you cannot use ofbitstream to open files that end
 * in .h, .hh, .cpp, or .cc for writing, as this could very easily cause
 * you to destroy your source files.
 */
class ofbitstream: public obitstream {
public:
    
    /* Constructor ofbitstream::ofbitstream
     * ------------------------------------
     * Wires up the stream class so that it knows to write data
     * to disk.
     */
    ofbitstream() {
        init(&fb);
    }
    /**
     * Constructs a new ofbitstream not attached to any file.  You can
     * open a file for writing using the .open() member functions.
     */
    
    /* Constructor ofbitstream::ofbitstream
     * ------------------------------------
     * Wires up the stream class so that it knows to write data
     * to disk, then opens the given file.
     */
     ofbitstream(const char* filename) {
        init(&fb);
        open(filename);
    }
    /**
     * Constructs a new ofbitstream that writes the specified file, if
     * it exists.  If not, the stream enters an error state.  Read
     * the documentation on "open" for more details.
     */
    
    /**
     * Constructs a new ofbitstream that writes the specified file, if
     * it exists.  If not, the stream enters an error state.  Read
     * the documentation on "open" for more details.
     */
    ofbitstream(const std::string& filename) {
        init(&fb);
        open(filename);
    }

    /* Member function ofbitstream::open
     * ---------------------------------
     * Attempts to open the specified file, failing if unable
     * to do so.
     */
    void open(const char* filename) {
        // Confirm we aren't about to do something that could potentially be a
        // Very Bad Idea.
//        if (endsWith(filename, ".cpp") || endsWith(filename, ".h") ||
//            endsWith(filename, ".hh") || endsWith(filename, ".cc")) {
//            //error(std::string("ofbitstream::open: It is potentially dangerous to write to file ")
//                //  + filename + ", because that might be your own source code.  "
//                  //+ "We are explicitly disallowing this operation.  Please choose a "
//                  //+ "different filename.");
//            setstate(std::ios::failbit);
//        }
//        else {
            if (!fb.open(filename, std::ios::out | std::ios::binary)) {
                setstate(std::ios::failbit);
            }
        //}
    }
    /**
     * Opens the specified file for writing.  If an error occurs, the
     * stream enters a failure state, which can be detected by calling
     * ifb.fail().  If an invalid filename is specified (for example,
     * a source file), reports an error.
     */
    
    /**
     * Opens the specified file for writing.  If an error occurs, the
     * stream enters a failure state, which can be detected by calling
     * ifb.fail().  If an invalid filename is specified (for example,
     * a source file), reports an error.
     */
    void open(const std::string& filename) {
        open(filename.c_str());
    }

    
    /* Member function ofbitstream::is_open
     * ------------------------------------
     * Determines whether the file stream is open.
     */
    bool is_open() {
        return fb.is_open();
    }
    /**
     * Returns whether or not this ofbitstream is connected to a file for
     * reading.
     */
    
    /* Member function ofbitstream::close
     * ----------------------------------
     * Closes the given file.
     */
    void close() {
        if (!fb.close()) {
            setstate(std::ios::failbit);
        }
    }
    /**
     * Closes the currently-opened file, if the stream is open.  If the
     * stream is not open, puts the stream into a fail state.
     */
    
private:
    // the actual file buffer which does reading and writing.
    std::filebuf fb;
};

/**
 * A variant on C++'s istringstream class, which acts as a stream that
 * reads its data from a string.  This is mostly used by the testing
 * code to test your Huffman encoding without having to read or write
 * files on disk, but you can use it in your own testing if you would
 * like.
 */
class istringbitstream: public ibitstream {
public:
    
    /* Constructor istringbitstream::istringbitstream
     * ----------------------------------------------
     * Sets the stream to use the string buffer, then sets
     * the initial string to the specified value.
     */
    istringbitstream(const std::string& s) {
        init(&sb);
        sb.str(s);
    }
    /**
     * Constructs an istringbitstream reading the specified string.
     */
    
    
    /* Member function istringbitstream::str
     * -------------------------------------
     * Sets the underlying string in the buffer to the
     * specified string.
     */
    void str(const std::string& s) {
        sb.str(s);
    }
    /**
     * Sets the underlying string of the istringbitstream.
     */

private:
    // the actual string buffer that does character storage
    std::stringbuf sb;
};

/**
 * A variant on C++'s ostringstream class, which acts as a stream that
 * writes its data to a string.  This is mostly used by the testing
 * code to test your Huffman encoding without having to read or write
 * files on disk, but you can use it in your own testing if you would
 * like.
 */
class ostringbitstream: public obitstream {
public:
    /* Member function ostringbitstream::ostringbitstream
     * --------------------------------------------------
     * Sets the stream to use the string buffer.
     */
    ostringbitstream() {
        init(&sb);
    }
    /**
     * Constructs an ostringbitstream.
     */
    
    /* Member function ostringbitstream::str
     * -------------------------------------
     * Retrives the underlying string data.
     */
    std::string str() {
        return sb.str();
    }
    /**
     * Retrieves the underlying string of the istringbitstream.
     */
    
private:
    // the actual string buffer that does character storage
    std::stringbuf sb;
};

/**
 * Returns a printable string for the given character.
 * @example toPrintable('c') returns "c"
 * @example toPrintable('\n') returns "\\n"
 */
std::string toPrintable(int ch);

/**
 * Returns a string with each non-printable character in the given string
 * replaced by one that is printable.
 * Certain common escape characters are replaced by a backslash representation,
 * and non-printable ASCII characters are replaced by a backslash and their
 * ASCII numeric representation, such as \255.
 * @example toPrintable("hi \0 there\n') returns "hi \\0 there\\n"
 */
std::string toPrintable(const std::string& s);

#endif // _bitstream_h


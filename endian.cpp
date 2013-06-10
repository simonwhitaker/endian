#include <fstream>
#include <iostream>

#include <getopt.h>

using namespace std;

int main(int argc, char * argv[]) {

    // No idea what ch stands for, it was in `man 3 getopt` so I went with it ;)
    int ch;
    
    // By default read from STDIN...
    istream *instream = &cin;

    // ...and write to STDOUT
    ostream *outstream = &cout;
    
    unsigned long bytesPerWord = 0;

    // Use a lambda expression to form a closure around argv[0] for printing usage
    // info. Nicer, IMHO, than saving argv[0] to a global variable or passing it as
    // an argument to an external _usage function.
    auto _usage = [&] () {
        cerr << "Usage: "
             << argv[0]
             << " BYTES_PER_WORD [--infile INFILE] [--outfile OUTFILE]"
             << endl;
    };

    // Sanity check: make sure they've specified at least the minimum number of
    // command-line args
    if (argc < 2) {
        _usage();
        return 1;
    }
    
    // Get the number of bytes per word
    try {
        bytesPerWord = stoul(argv[1]);
    } catch (invalid_argument) {
        cerr << "Error: invalid BYTES_PER_WORD argument (" << argv[1] << ")" << endl;
        _usage();
        return 1;
    }

    // Parse the command-line options
    static struct option longopts[] = {
        { "infile", required_argument, NULL, 'i'},
        { "outfile", required_argument, NULL, 'o'},
        { NULL, 0, NULL, 0},
    };
        
    while ((ch = getopt_long(argc, argv, "i:o:", longopts, NULL)) != -1) {
        switch (ch) {
            case 'i': {
                instream = new ifstream(optarg, ios::binary);
                if (!((ifstream*)instream)->is_open()) {
                    cerr << "Error: failed to open " << optarg << " for reading." << endl;
                    return 1;
                }

                break;
            }
            case 'o': {
                outstream = new ofstream(optarg, ios::binary);
                if (!((ofstream*)outstream)->is_open()) {
                    cerr << "Error: failed to open " << optarg << " for writing." << endl;
                    return 1;
                }
                
                break;
            }
            default:
                _usage();
                return 1;
        }
    }
    
    // Create a temporary array to store the bytes from each word while we
    // reverse them
    char *buffer = new char[bytesPerWord];
    if (buffer == NULL) {
        cerr << "Error: couldn't allocate memory for the temporary byte array." << endl;
        return 1;
    }

    // OK, we're all set. Lets reverse some endianness.
    while (1) {
        instream->read(buffer, bytesPerWord);
        if (instream->eof()) {
            break;
        } else if (instream->fail()) {
            cerr << "Error: failure occured while reading from input stream." << endl;
            return 1;
        } else {
            // Now reverse the bytes in this word. Work through the first half of
            // the byte array, swapping each byte with the corresponding byte from
            // the far end of the array. So for example, with 64-bit words we'd
            // swap indices 0 and 7, then 1 and 6, then 2 and 5, then 3 and 4.
            for (unsigned char byteIndex = 0; byteIndex < bytesPerWord / 2; byteIndex++) {
                unsigned char byteToSwapIndex = bytesPerWord - 1 - byteIndex;
                
                // Swap the values at indices byteIndex and byteToSwapIndex. No need
                // to show off with XOR. :)
                char temp = buffer[byteIndex];
                buffer[byteIndex] = buffer[byteToSwapIndex];
                buffer[byteToSwapIndex] = temp;
            }
            
            // Finally, write the newly-reversed word to the output stream
            outstream->write(buffer, bytesPerWord);
            if (outstream->fail()) {
                cerr << "Error: write to output stream failed." << endl;
                return 1;
            }
        }
    }

    if (instream != &cin) {
        delete instream;
    }
    if (outstream != &cout) {
        delete  outstream;
    }

    delete [] buffer;

    return 0;
}


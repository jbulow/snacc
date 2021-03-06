// c++_examples/simple/example.C - an example of how to use  C++ ASN.1-BER
//             encoders and decoders generated by snacc
//
// AUTHOR: Mike Sample
// DATE:   Aug 92
//
// $Header: /usr/app/odstb/CVS/snacc/c++-examples/simple/example.C,v 1.5 1995/07/24 15:36:03 rj Exp $
// $Log: example.C,v $
// Revision 1.5  1995/07/24 15:36:03  rj
// check return value of new.
//
// changed `_' to `-' in file names.
//
// Revision 1.4  1995/02/18  13:54:18  rj
// added #define HAVE_VARIABLE_SIZED_AUTOMATIC_ARRAYS since not every C++ compiler provides them.
//
// Revision 1.3  1994/10/08  01:27:02  rj
// several \size_t'
//
// Revision 1.2  1994/08/31  08:56:32  rj
// first check-in. for a list of changes to the snacc-1.1 distribution please refer to the ChangeLog.
//

#include <stddef.h>
#include <stdlib.h>
#include <errno.h>
#include <fstream>
#include "asn-incl.h"
#include "p-rec.h"


main (int argc, char *argv[])
{
    AsnBuf  inputBuf;
    AsnBuf  outputBuf;
    size_t encodedLen;
    size_t decodedLen;
    size_t      dataSize;
    std::ifstream dataFile;
    PersonnelRecord pr;

    if (argc != 2)
    {
        std::cerr << "Usage: " << argv[0] << " <BER data file name>" << std::endl;
        std::cerr << "   Decodes the given PersonnelRecord BER data file" << std::endl;
        std::cerr << "   and re-encodes it to stdout" << std::endl;
        exit (1);
    }


    // open the data file
    dataFile.open (argv[1]);

    if (!dataFile)
    {
        perror ("ifstream::open");
        exit (1);
    }

    // get size of the data file file
    dataFile.seekg (0, std::ios::end);
    dataSize = dataFile.tellg();
    dataFile.seekg (0);

    // read data from file into contiguous block for a buffer
#if HAVE_VARIABLE_SIZED_AUTOMATIC_ARRAYS
    char data[dataSize];
#else
    char *data = new char[dataSize];
    if (!data)
	return 1;
#endif /* HAVE_VARIABLE_SIZED_AUTOMATIC_ARRAYS */
    dataFile.read (data, dataSize);
    dataFile.close();

    //
    // put the BER data read from the file
    // into buffer format, ready for reading from the
    // beginning
    //
    inputBuf.InstallData (data, dataSize);

    if (!pr.BDecPdu (inputBuf, decodedLen))
    {
        std::cerr << "--- ERROR - Decode routines failed, exiting..." << std::endl;
        exit (1);
    }

    std::cerr  << "decodedValue PersonnelRecord ::= " << pr << std::endl << std::endl;

    //
    // allocate a new buffer set up for writing to
    //
#if HAVE_VARIABLE_SIZED_AUTOMATIC_ARRAYS
    char outputData[dataSize + 512];
#else
    char *outputData = new char[dataSize + 512];
    if (!outputData)
	return 1;
#endif /* HAVE_VARIABLE_SIZED_AUTOMATIC_ARRAYS */
    outputBuf.Init (outputData, dataSize+512);
    outputBuf.ResetInWriteRvsMode();

    if (!pr.BEncPdu (outputBuf, encodedLen))
    {
        std::cerr << "--- ERROR - Encode routines failed" << std::endl;
    }

    // write the BER value to cout
    outputBuf.ResetInReadMode();
    for (; encodedLen > 0; encodedLen--)
        std::cout.put (outputBuf.GetByte());

    return 0;
}

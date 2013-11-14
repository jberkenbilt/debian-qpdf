
#include <qpdf/Pl_RC4.hh>
#include <qpdf/Pl_StdioFile.hh>

#include <stdio.h>
#include <string.h>
#include <iostream>
#include <stdlib.h>

#ifdef _WIN32
# include <io.h>
#else
# include <unistd.h>
#endif

int main(int argc, char* argv[])
{
    if (argc != 4)
    {
	std::cerr << "Usage: rc4 hex-key infile outfile" << std::endl;
	exit(2);
    }

    char* hexkey = argv[1];
    char* infilename = argv[2];
    char* outfilename = argv[3];
    int hexkeylen = strlen(hexkey);
    int keylen = hexkeylen / 2;
    unsigned char* key = new unsigned char[keylen + 1];
    key[keylen] = '\0';

    FILE* infile = fopen(infilename, "rb");
    if (infile == 0)
    {
	std::cerr << "can't open " << infilename << std::endl;
	exit(2);
    }

    for (unsigned int i = 0; i < strlen(hexkey); i += 2)
    {
	char t[3];
	t[0] = hexkey[i];
	t[1] = hexkey[i + 1];
	t[2] = '\0';

	long val = strtol(t, 0, 16);
	key[i/2] = (unsigned char) val;
    }

    FILE* outfile = fopen(outfilename, "wb");
    if (outfile == 0)
    {
	std::cerr << "can't open " << outfilename << std::endl;
	exit(2);
    }
    Pl_StdioFile* out = new Pl_StdioFile("stdout", outfile);
    // Use a small buffer size (64) for testing
    Pl_RC4* rc4 = new Pl_RC4("rc4", out, key, keylen, 64);
    delete [] key;

    // 64 < buffer size < 512, buffer_size is not a power of 2 for testing
    unsigned char buf[100];
    bool done = false;
    while (! done)
    {
	int len = fread(buf, 1, sizeof(buf), infile);
	if (len <= 0)
	{
	    done = true;
	}
	else
	{
	    rc4->write(buf, len);
	}
    }
    rc4->finish();
    delete rc4;
    delete out;
    fclose(infile);
    fclose(outfile);
    return 0;
}

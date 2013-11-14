
#include <qpdf/MD5.hh>
#include <qpdf/Pl_MD5.hh>
#include <qpdf/Pl_Discard.hh>
#include <iostream>

static void test_string(char const* str)
{
    MD5 a;
    a.encodeString(str);
    a.print();
}

int main(int, char*[])
{
    test_string("");
    test_string("a");
    test_string("abc");
    test_string("message digest");
    test_string("abcdefghijklmnopqrstuvwxyz");
    test_string("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghi"
		"jklmnopqrstuvwxyz0123456789");
    test_string("1234567890123456789012345678901234567890"
		"1234567890123456789012345678901234567890");
    MD5 a;
    a.encodeFile("md5.in");
    std::cout << a.unparse() << std::endl;
    MD5 b;
    b.encodeFile("md5.in", 100);
    std::cout << b.unparse() << std::endl;

    std::cout
	<< MD5::checkDataChecksum("900150983cd24fb0d6963f7d28e17f72", "abc", 3)
	<< std::endl
	<< MD5::checkFileChecksum("5f4b4321873433daae578f85c72f9e74", "md5.in")
	<< std::endl
	<< MD5::checkFileChecksum("6f4b4321873433daae578f85c72f9e74", "md5.in")
	<< std::endl
	<< MD5::checkDataChecksum("000150983cd24fb0d6963f7d28e17f72", "abc", 3)
	<< std::endl
	<< MD5::checkFileChecksum("6f4b4321873433daae578f85c72f9e74", "glerbl")
	<< std::endl;


    Pl_Discard d;
    Pl_MD5 p("MD5", &d);
    for (int i = 0; i < 2; ++i)
    {
	FILE* f = fopen("md5.in", "rb");
	if (f)
	{
	    // buffer size < size of md5.in
	    unsigned char buf[50];
	    bool done = false;
	    while (! done)
	    {
		int len = fread(buf, 1, sizeof(buf), f);
		if (len <= 0)
		{
		    done = true;
		}
		else
		{
		    p.write(buf, len);
		}
	    }
	    fclose(f);
	    p.finish();
	    std::cout << p.getHexDigest() << std::endl;
	}
    }

    return 0;
}

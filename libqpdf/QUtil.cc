
#include <qpdf/QUtil.hh>
#include <stdio.h>
#include <errno.h>
#include <ctype.h>
#include <stdlib.h>
#ifdef _WIN32
#include <Windows.h>
#include <direct.h>
#else
#include <unistd.h>
#endif

std::string
QUtil::int_to_string(int num, int fullpad)
{
    // This routine will need to be recompiled if an int can be longer than
    // 49 digits.
    char t[50];

    // -2 or -1 to leave space for the possible negative sign and for NUL...
    if (abs(fullpad) > (int)sizeof(t) - ((num < 0)?2:1))
    {
	throw QEXC::Internal("Util::int_to_string has been called with "
			     "a padding value greater than its internal "
			     "limit");
    }

    if (fullpad)
    {
	sprintf(t, "%0*d", fullpad, num);
    }
    else
    {
	sprintf(t, "%d", num);
    }

    return std::string(t);
}

std::string
QUtil::double_to_string(double num, int decimal_places)
{
    // This routine will need to be recompiled if a double can be longer than
    // 99 digits.
    char t[100];

    std::string lhs = int_to_string((int)num);

    // lhs.length() gives us the length of the part on the right hand
    // side of the dot + 1 for the dot + decimal_places: total size of
    // the required string.  -1 on the sizeof side to allow for NUL at
    // the end.
    //
    // If decimal_places <= 0, it is as if no precision was provided
    // so trust the buffer is big enough.  The following test will
    // always pass in those cases.
    if (decimal_places + 1 + (int)lhs.length() > (int)sizeof(t) - 1)
    {
	throw QEXC::Internal("Util::double_to_string has been called with "
			     "a number and a decimal places specification "
			     "that would break an internal limit");
    }

    if (decimal_places)
    {
	sprintf(t, "%.*f", decimal_places, num);
    }
    else
    {
	sprintf(t, "%f", num);
    }
    return std::string(t);
}

int
QUtil::os_wrapper(std::string const& description, int status) throw (QEXC::System)
{
    if (status == -1)
    {
	throw QEXC::System(description, errno);
    }
    return status;
}

FILE*
QUtil::fopen_wrapper(std::string const& description, FILE* f) throw (QEXC::System)
{
    if (f == 0)
    {
	throw QEXC::System(description, errno);
    }
    return f;
}

char*
QUtil::copy_string(std::string const& str)
{
    char* result = new char[str.length() + 1];
    // Use memcpy in case string contains nulls
    result[str.length()] = '\0';
    memcpy(result, str.c_str(), str.length());
    return result;
}

bool
QUtil::get_env(std::string const& var, std::string* value)
{
    // This was basically ripped out of wxWindows.
#ifdef _WIN32
    // first get the size of the buffer
    DWORD len = ::GetEnvironmentVariable(var.c_str(), NULL, 0);
    if (len == 0)
    {
        // this means that there is no such variable
        return false;
    }

    if (value)
    {
	char* t = new char[len + 1];
        ::GetEnvironmentVariable(var.c_str(), t, len);
	*value = t;
	delete [] t;
    }

    return true;
#else
    char* p = getenv(var.c_str());
    if (p == 0)
    {
        return false;
    }
    if (value)
    {
        *value = p;
    }

    return true;
#endif
}

std::string
QUtil::toUTF8(unsigned long uval)
{
    std::string result;

    // A UTF-8 encoding of a Unicode value is a single byte for
    // Unicode values <= 127.  For larger values, the first byte of
    // the UTF-8 encoding has '1' as each of its n highest bits and
    // '0' for its (n+1)th highest bit where n is the total number of
    // bytes required.  Subsequent bytes start with '10' and have the
    // remaining 6 bits free for encoding.  For example, an 11-bit
    // unicode value can be stored in two bytes where the first is
    // 110zzzzz, the second is 10zzzzzz, and the z's represent the
    // remaining bits.

    if (uval > 0x7fffffff)
    {
	throw QEXC::General("bounds error in QUtil::toUTF8");
    }
    else if (uval < 128)
    {
	result += (char)(uval);
    }
    else
    {
	unsigned char bytes[7];
	bytes[6] = '\0';
	unsigned char* cur_byte = &bytes[5];

	// maximum value that will fit in the current number of bytes
	unsigned char maxval = 0x3f; // six bits

	while (uval > maxval)
	{
	    // Assign low six bits plus 10000000 to lowest unused
	    // byte position, then shift
	    *cur_byte = (unsigned char) (0x80 + (uval & 0x3f));
	    uval >>= 6;
	    // Maximum that will fit in high byte now shrinks by one bit
	    maxval >>= 1;
	    // Slide to the left one byte
	    --cur_byte;
	    if (cur_byte < bytes)
	    {
		throw QEXC::Internal("QUtil::toUTF8: overflow error");
	    }
	}
	// If maxval is k bits long, the high (7 - k) bits of the
	// resulting byte must be high.
	*cur_byte = (unsigned char)((0xff - (1 + (maxval << 1))) + uval);

	result += (char*)cur_byte;
    }

    return result;
}

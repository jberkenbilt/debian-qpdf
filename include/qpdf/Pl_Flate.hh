// Copyright (c) 2005-2008 Jay Berkenbilt
//
// This file is part of qpdf.  This software may be distributed under
// the terms of version 2 of the Artistic License which may be found
// in the source distribution.  It is provided "as is" without express
// or implied warranty.

#ifndef __PL_FLATE_HH__
#define __PL_FLATE_HH__

#include <qpdf/Pipeline.hh>

#include <zlib.h>

class Pl_Flate: public Pipeline
{
  public:
    class Exception: public Pipeline::Exception
    {
      public:
	Exception(std::string const& message) :
	    Pipeline::Exception(message)
	{
	}

	virtual ~Exception() throw ()
	{
	}
    };

    static int const def_bufsize = 65536;

    enum action_e { a_inflate, a_deflate };

    Pl_Flate(char const* identifier, Pipeline* next,
	     action_e action, int out_bufsize = def_bufsize);
    virtual ~Pl_Flate();

    virtual void write(unsigned char* data, int len);
    virtual void finish();

  private:
    void handleData(unsigned char* data, int len, int flush);
    void checkError(char const* prefix, int error_code);

    unsigned char* outbuf;
    int out_bufsize;
    action_e action;
    bool initialized;
    z_stream zstream;
};

#endif // __PL_FLATE_HH__

#include <qpdf/Pl_AES_PDF.hh>
#include <qpdf/QUtil.hh>
#include <cstring>
#include <assert.h>
#include <stdexcept>
#include <qpdf/rijndael.h>
#include <string>
#include <stdlib.h>
#include <qpdf/qpdf-config.h>
#ifndef HAVE_RANDOM
# define random rand
# define srandom srand
#endif

bool Pl_AES_PDF::use_static_iv = false;

Pl_AES_PDF::Pl_AES_PDF(char const* identifier, Pipeline* next,
		       bool encrypt, unsigned char const key[key_size]) :
    Pipeline(identifier, next),
    encrypt(encrypt),
    cbc_mode(true),
    first(true),
    offset(0),
    nrounds(0)
{
    static int const keybits = 128;
    assert(key_size == KEYLENGTH(keybits));
    assert(sizeof(this->rk) / sizeof(uint32_t) == RKLENGTH(keybits));
    std::memcpy(this->key, key, key_size);
    std::memset(this->rk, 0, sizeof(this->rk));
    std::memset(this->inbuf, 0, this->buf_size);
    std::memset(this->outbuf, 0, this->buf_size);
    std::memset(this->cbc_block, 0, this->buf_size);
    if (encrypt)
    {
	this->nrounds = rijndaelSetupEncrypt(this->rk, this->key, keybits);
    }
    else
    {
	this->nrounds = rijndaelSetupDecrypt(this->rk, this->key, keybits);
    }
    assert(this->nrounds == NROUNDS(keybits));
}

Pl_AES_PDF::~Pl_AES_PDF()
{
    // nothing needed
}

void
Pl_AES_PDF::disableCBC()
{
    this->cbc_mode = false;
}

void
Pl_AES_PDF::useStaticIV()
{
    use_static_iv = true;
}

void
Pl_AES_PDF::write(unsigned char* data, size_t len)
{
    size_t bytes_left = len;
    unsigned char* p = data;

    while (bytes_left > 0)
    {
	if (this->offset == this->buf_size)
	{
	    flush(false);
	}

	size_t available = this->buf_size - this->offset;
	size_t bytes = (bytes_left < available ? bytes_left : available);
	bytes_left -= bytes;
	std::memcpy(this->inbuf + this->offset, p, bytes);
	this->offset += bytes;
	p += bytes;
    }
}

void
Pl_AES_PDF::finish()
{
    if (this->encrypt)
    {
	if (this->offset == this->buf_size)
	{
	    flush(false);
	}
	// Pad as described in section 3.5.1 of version 1.7 of the PDF
	// specification, including providing an entire block of padding
	// if the input was a multiple of 16 bytes.
	unsigned char pad = (unsigned char) (this->buf_size - this->offset);
	memset(this->inbuf + this->offset, pad, pad);
	this->offset = this->buf_size;
	flush(false);
    }
    else
    {
	if (this->offset != this->buf_size)
	{
	    // This is never supposed to happen as the output is
	    // always supposed to be padded.  However, we have
	    // encountered files for which the output is not a
	    // multiple of the block size.  In this case, pad with
	    // zeroes and hope for the best.
	    assert(this->buf_size > this->offset);
	    std::memset(this->inbuf + this->offset, 0,
			this->buf_size - this->offset);
	    this->offset = this->buf_size;
	}
	flush(true);
    }
    getNext()->finish();
}

void
Pl_AES_PDF::initializeVector()
{
    static bool seeded_random = false;
    if (! seeded_random)
    {
	// Seed the random number generator with something simple, but
	// just to be interesting, don't use the unmodified current
	// time....
	srandom((int)QUtil::get_current_time() ^ 0xcccc);
	seeded_random = true;
    }
    if (use_static_iv)
    {
	for (unsigned int i = 0; i < this->buf_size; ++i)
	{
	    this->cbc_block[i] = 14 * (1 + i);
	}
    }
    else
    {
	for (unsigned int i = 0; i < this->buf_size; ++i)
	{
	    this->cbc_block[i] = (unsigned char)((random() & 0xff0) >> 4);
	}
    }
}

void
Pl_AES_PDF::flush(bool strip_padding)
{
    assert(this->offset == this->buf_size);

    if (first)
    {
	first = false;
	if (this->cbc_mode)
	{
	    if (encrypt)
	    {
		// Set cbc_block to a random initialization vector and
		// write it to the output stream
		initializeVector();
		getNext()->write(this->cbc_block, this->buf_size);
	    }
	    else
	    {
		// Take the first block of input as the initialization
		// vector.  There's nothing to write at this time.
		memcpy(this->cbc_block, this->inbuf, this->buf_size);
		this->offset = 0;
		return;
	    }
	}
    }

    if (this->encrypt)
    {
	if (this->cbc_mode)
	{
	    for (unsigned int i = 0; i < this->buf_size; ++i)
	    {
		this->inbuf[i] ^= this->cbc_block[i];
	    }
	}
	rijndaelEncrypt(this->rk, this->nrounds, this->inbuf, this->outbuf);
	if (this->cbc_mode)
	{
	    memcpy(this->cbc_block, this->outbuf, this->buf_size);
	}
    }
    else
    {
	rijndaelDecrypt(this->rk, this->nrounds, this->inbuf, this->outbuf);
	if (this->cbc_mode)
	{
	    for (unsigned int i = 0; i < this->buf_size; ++i)
	    {
		this->outbuf[i] ^= this->cbc_block[i];
	    }
	    memcpy(this->cbc_block, this->inbuf, this->buf_size);
	}
    }
    unsigned int bytes = this->buf_size;
    if (strip_padding)
    {
	unsigned char last = this->outbuf[this->buf_size - 1];
	if (last <= this->buf_size)
	{
	    bool strip = true;
	    for (unsigned int i = 1; i <= last; ++i)
	    {
		if (this->outbuf[this->buf_size - i] != last)
		{
		    strip = false;
		    break;
		}
	    }
	    if (strip)
	    {
		bytes -= last;
	    }
	}
    }
    getNext()->write(this->outbuf, bytes);
    this->offset = 0;
}

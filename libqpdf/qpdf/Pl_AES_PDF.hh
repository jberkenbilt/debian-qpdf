#ifndef __PL_AES_PDF_HH__
#define __PL_AES_PDF_HH__

#include <qpdf/Pipeline.hh>
#include <qpdf/qpdf-config.h>

// This pipeline implements AES-128 with CBC and block padding as
// specified in the PDF specification.

class Pl_AES_PDF: public Pipeline
{
  public:
    // key_data should be a pointer to key_size bytes of data
    static unsigned int const key_size = 16;
    QPDF_DLL
    Pl_AES_PDF(char const* identifier, Pipeline* next,
	       bool encrypt, unsigned char const key[key_size]);
    QPDF_DLL
    virtual ~Pl_AES_PDF();

    QPDF_DLL
    virtual void write(unsigned char* data, int len);
    QPDF_DLL
    virtual void finish();

    // For testing only; PDF always uses CBC
    QPDF_DLL
    void disableCBC();
    // For testing only: use a fixed initialization vector for CBC
    QPDF_DLL
    static void useStaticIV();

  private:
    void flush(bool discard_padding);
    void initializeVector();

    static unsigned int const buf_size = 16;
    static bool use_static_iv;

    bool encrypt;
    bool cbc_mode;
    bool first;
    unsigned int offset;
    unsigned char key[key_size];
    uint32_t rk[key_size + 28];
    unsigned char inbuf[buf_size];
    unsigned char outbuf[buf_size];
    unsigned char cbc_block[buf_size];
    unsigned int nrounds;
};

#endif // __PL_AES_PDF_HH__

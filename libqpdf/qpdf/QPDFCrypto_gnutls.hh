#ifndef QPDFCRYPTO_GNUTLS_HH
#define QPDFCRYPTO_GNUTLS_HH

#include <qpdf/DLL.h>
#include <qpdf/QPDFCryptoImpl.hh>
#include <memory>
#include <gnutls/gnutls.h>
#include <gnutls/crypto.h>

class QPDFCrypto_gnutls: public QPDFCryptoImpl
{
  public:
    QPDFCrypto_gnutls();

    QPDF_DLL
    virtual ~QPDFCrypto_gnutls();

    virtual void MD5_init();
    virtual void MD5_update(unsigned char const* data, size_t len);
    virtual void MD5_finalize();
    virtual void MD5_digest(MD5_Digest);

    virtual void RC4_init(unsigned char const* key_data, int key_len = -1);
    virtual void RC4_process(unsigned char* in_data, size_t len,
                             unsigned char* out_data = 0);
    virtual void RC4_finalize();

    virtual void SHA2_init(int bits);
    virtual void SHA2_update(unsigned char const* data, size_t len);
    virtual void SHA2_finalize();
    virtual std::string SHA2_digest();

    virtual void rijndael_init(
        bool encrypt, unsigned char const* key_data, size_t key_len,
        bool cbc_mode, unsigned char* cbc_block);
    virtual void rijndael_process(
        unsigned char* in_data, unsigned char* out_data);
    virtual void rijndael_finalize();

  private:
    void badBits();

    gnutls_hash_hd_t hash_ctx;
    gnutls_cipher_hd_t cipher_ctx;
    int sha2_bits;
    bool encrypt;
    bool cbc_mode;
    char digest[64];
    unsigned char const* aes_key_data;
    size_t aes_key_len;
};

#endif // QPDFCRYPTO_GNUTLS_HH

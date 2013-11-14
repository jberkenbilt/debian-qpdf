
#ifndef __PL_QPDFTOKENIZER_HH__
#define __PL_QPDFTOKENIZER_HH__

#include <qpdf/Pipeline.hh>

#include <qpdf/QPDFTokenizer.hh>

//
// Treat incoming text as a stream consisting of valid PDF tokens, but
// output bad tokens just the same.  The idea here is to be able to
// use pipeline for content streams to normalize newlines without
// interfering with meaningful newlines such as those that occur
// inside of strings.
//

class Pl_QPDFTokenizer: public Pipeline
{
  public:
    Pl_QPDFTokenizer(char const* identifier, Pipeline* next);
    virtual ~Pl_QPDFTokenizer();
    virtual void write(unsigned char* buf, int len);
    virtual void finish();

  private:
    void processChar(char ch);
    void checkUnread();
    void writeNext(char const*, int len);
    void writeToken(QPDFTokenizer::Token&);

    QPDFTokenizer tokenizer;
    bool newline_after_next_token;
    bool just_wrote_nl;
    bool last_char_was_cr;
    bool unread_char;
    char char_to_unread;
    bool pass_through;
};

#endif // __PL_QPDFTOKENIZER_HH__

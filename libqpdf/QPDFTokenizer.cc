#include <qpdf/QPDFTokenizer_private.hh>

// DO NOT USE ctype -- it is locale dependent for some things, and it's not worth the risk of
// including it in case it may accidentally be used.

#include <qpdf/QIntC.hh>
#include <qpdf/QPDFExc.hh>
#include <qpdf/QPDFObjectHandle.hh>
#include <qpdf/QTC.hh>
#include <qpdf/QUtil.hh>
#include <qpdf/Util.hh>

#include <cstdlib>
#include <cstring>
#include <stdexcept>

using namespace qpdf;

using Token = QPDFTokenizer::Token;
using tt = QPDFTokenizer::token_type_e;

static inline bool
is_delimiter(char ch)
{
    return (
        ch == ' ' || ch == '\n' || ch == '/' || ch == '(' || ch == ')' || ch == '{' || ch == '}' ||
        ch == '<' || ch == '>' || ch == '[' || ch == ']' || ch == '%' || ch == '\t' || ch == '\r' ||
        ch == '\v' || ch == '\f' || ch == 0);
}

namespace
{
    class QPDFWordTokenFinder: public InputSource::Finder
    {
      public:
        QPDFWordTokenFinder(InputSource& is, std::string const& str) :
            is(is),
            str(str)
        {
        }
        ~QPDFWordTokenFinder() override = default;
        bool check() override;

      private:
        InputSource& is;
        std::string str;
    };
} // namespace

bool
QPDFWordTokenFinder::check()
{
    // Find a word token matching the given string, preceded by a delimiter, and followed by a
    // delimiter or EOF.
    Tokenizer tokenizer;
    tokenizer.nextToken(is, "finder", str.size() + 2);
    qpdf_offset_t pos = is.tell();
    if (tokenizer.getType() != tt::tt_word || tokenizer.getValue() != str) {
        QTC::TC("qpdf", "QPDFTokenizer finder found wrong word");
        return false;
    }
    qpdf_offset_t token_start = is.getLastOffset();
    char next;
    bool next_okay = false;
    if (is.read(&next, 1) == 0) {
        QTC::TC("qpdf", "QPDFTokenizer inline image at EOF");
        next_okay = true;
    } else {
        next_okay = is_delimiter(next);
    }
    is.seek(pos, SEEK_SET);
    if (!next_okay) {
        return false;
    }
    if (token_start == 0) {
        // Can't actually happen...we never start the search at the beginning of the input.
        return false;
    }
    return true;
}

void
Tokenizer::reset()
{
    state = st_before_token;
    type = tt::tt_bad;
    val.clear();
    raw_val.clear();
    error_message = "";
    before_token = true;
    in_token = false;
    char_to_unread = '\0';
    inline_image_bytes = 0;
    string_depth = 0;
    bad = false;
}

QPDFTokenizer::Token::Token(token_type_e type, std::string const& value) :
    type(type),
    value(value),
    raw_value(value)
{
    if (type == tt_string) {
        raw_value = QPDFObjectHandle::newString(value).unparse();
    } else if (type == tt_name) {
        raw_value = QPDFObjectHandle::newName(value).unparse();
    }
}

QPDFTokenizer::QPDFTokenizer() :
    m(std::make_unique<qpdf::Tokenizer>())
{
}

QPDFTokenizer::~QPDFTokenizer() = default;

Tokenizer::Tokenizer()
{
    reset();
}

void
QPDFTokenizer::allowEOF()
{
    m->allowEOF();
}

void
Tokenizer::allowEOF()
{
    allow_eof = true;
}

void
QPDFTokenizer::includeIgnorable()
{
    m->includeIgnorable();
}

void
Tokenizer::includeIgnorable()
{
    include_ignorable = true;
}

bool
Tokenizer::isSpace(char ch)
{
    return (ch == '\0' || util::is_space(ch));
}

bool
Tokenizer::isDelimiter(char ch)
{
    return is_delimiter(ch);
}

void
QPDFTokenizer::presentCharacter(char ch)
{
    m->presentCharacter(ch);
}

void
Tokenizer::presentCharacter(char ch)
{
    handleCharacter(ch);

    if (this->in_token) {
        this->raw_val += ch;
    }
}

void
Tokenizer::handleCharacter(char ch)
{
    // In some cases, functions called below may call a second handler. This happens whenever you
    // have to use a character from the next token to detect the end of the current token.

    switch (this->state) {
    case st_top:
        inTop(ch);
        return;

    case st_in_space:
        inSpace(ch);
        return;

    case st_in_comment:
        inComment(ch);
        return;

    case st_lt:
        inLt(ch);
        return;

    case st_gt:
        inGt(ch);
        return;

    case st_in_string:
        inString(ch);
        return;

    case st_name:
        inName(ch);
        return;

    case st_number:
        inNumber(ch);
        return;

    case st_real:
        inReal(ch);
        return;

    case st_string_after_cr:
        inStringAfterCR(ch);
        return;

    case st_string_escape:
        inStringEscape(ch);
        return;

    case st_char_code:
        inCharCode(ch);
        return;

    case st_literal:
        inLiteral(ch);
        return;

    case st_inline_image:
        inInlineImage(ch);
        return;

    case st_in_hexstring:
        inHexstring(ch);
        return;

    case st_in_hexstring_2nd:
        inHexstring2nd(ch);
        return;

    case st_name_hex1:
        inNameHex1(ch);
        return;

    case st_name_hex2:
        inNameHex2(ch);
        return;

    case st_sign:
        inSign(ch);
        return;

    case st_decimal:
        inDecimal(ch);
        return;

    case (st_before_token):
        inBeforeToken(ch);
        return;

    case (st_token_ready):
        inTokenReady(ch);
        return;

    default:
        throw std::logic_error("INTERNAL ERROR: invalid state while reading token");
    }
}

void
Tokenizer::inTokenReady(char ch)
{
    throw std::logic_error(
        "INTERNAL ERROR: QPDF tokenizer presented character while token is waiting");
}

void
Tokenizer::inBeforeToken(char ch)
{
    // Note: we specifically do not use ctype here.  It is locale-dependent.
    if (isSpace(ch)) {
        this->before_token = !this->include_ignorable;
        this->in_token = this->include_ignorable;
        if (this->include_ignorable) {
            this->state = st_in_space;
        }
    } else if (ch == '%') {
        this->before_token = !this->include_ignorable;
        this->in_token = this->include_ignorable;
        this->state = st_in_comment;
    } else {
        this->before_token = false;
        this->in_token = true;
        inTop(ch);
    }
}

void
Tokenizer::inTop(char ch)
{
    switch (ch) {
    case '(':
        this->string_depth = 1;
        this->state = st_in_string;
        return;

    case '<':
        this->state = st_lt;
        return;

    case '>':
        this->state = st_gt;
        return;

    case (')'):
        this->type = tt::tt_bad;
        QTC::TC("qpdf", "QPDFTokenizer bad )");
        this->error_message = "unexpected )";
        this->state = st_token_ready;
        return;

    case '[':
        this->type = tt::tt_array_open;
        this->state = st_token_ready;
        return;

    case ']':
        this->type = tt::tt_array_close;
        this->state = st_token_ready;
        return;

    case '{':
        this->type = tt::tt_brace_open;
        this->state = st_token_ready;
        return;

    case '}':
        this->type = tt::tt_brace_close;
        this->state = st_token_ready;
        return;

    case '/':
        this->state = st_name;
        this->val += ch;
        return;

    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
        this->state = st_number;
        return;

    case '+':
    case '-':
        this->state = st_sign;
        return;

    case '.':
        this->state = st_decimal;
        return;

    default:
        this->state = st_literal;
        return;
    }
}

void
Tokenizer::inSpace(char ch)
{
    // We only enter this state if include_ignorable is true.
    if (!isSpace(ch)) {
        this->type = tt::tt_space;
        this->in_token = false;
        this->char_to_unread = ch;
        this->state = st_token_ready;
    }
}

void
Tokenizer::inComment(char ch)
{
    if ((ch == '\r') || (ch == '\n')) {
        if (this->include_ignorable) {
            this->type = tt::tt_comment;
            this->in_token = false;
            this->char_to_unread = ch;
            this->state = st_token_ready;
        } else {
            this->state = st_before_token;
        }
    }
}

void
Tokenizer::inString(char ch)
{
    switch (ch) {
    case '\\':
        this->state = st_string_escape;
        return;

    case '(':
        this->val += ch;
        ++this->string_depth;
        return;

    case ')':
        if (--this->string_depth == 0) {
            this->type = tt::tt_string;
            this->state = st_token_ready;
            return;
        }

        this->val += ch;
        return;

    case '\r':
        // CR by itself is converted to LF
        this->val += '\n';
        this->state = st_string_after_cr;
        return;

    case '\n':
        this->val += ch;
        return;

    default:
        this->val += ch;
        return;
    }
}

void
Tokenizer::inName(char ch)
{
    if (isDelimiter(ch)) {
        // A C-locale whitespace character or delimiter terminates token.  It is important to unread
        // the whitespace character even though it is ignored since it may be the newline after a
        // stream keyword.  Removing it here could make the stream-reading code break on some files,
        // though not on any files in the test suite as of this
        // writing.

        this->type = this->bad ? tt::tt_bad : tt::tt_name;
        this->in_token = false;
        this->char_to_unread = ch;
        this->state = st_token_ready;
    } else if (ch == '#') {
        this->char_code = 0;
        this->state = st_name_hex1;
    } else {
        this->val += ch;
    }
}

void
Tokenizer::inNameHex1(char ch)
{
    this->hex_char = ch;

    if (char hval = util::hex_decode_char(ch); hval < '\20') {
        this->char_code = int(hval) << 4;
        this->state = st_name_hex2;
    } else {
        QTC::TC("qpdf", "QPDFTokenizer bad name 1");
        this->error_message = "name with stray # will not work with PDF >= 1.2";
        // Use null to encode a bad # -- this is reversed in QPDF_Name::normalizeName.
        this->val += '\0';
        this->state = st_name;
        inName(ch);
    }
}

void
Tokenizer::inNameHex2(char ch)
{
    if (char hval = util::hex_decode_char(ch); hval < '\20') {
        this->char_code |= int(hval);
    } else {
        QTC::TC("qpdf", "QPDFTokenizer bad name 2");
        this->error_message = "name with stray # will not work with PDF >= 1.2";
        // Use null to encode a bad # -- this is reversed in QPDF_Name::normalizeName.
        this->val += '\0';
        this->val += this->hex_char;
        this->state = st_name;
        inName(ch);
        return;
    }
    if (this->char_code == 0) {
        QTC::TC("qpdf", "QPDFTokenizer null in name");
        this->error_message = "null character not allowed in name token";
        this->val += "#00";
        this->state = st_name;
        this->bad = true;
    } else {
        this->val += char(this->char_code);
        this->state = st_name;
    }
}

void
Tokenizer::inSign(char ch)
{
    if (util::is_digit(ch)) {
        this->state = st_number;
    } else if (ch == '.') {
        this->state = st_decimal;
    } else {
        this->state = st_literal;
        inLiteral(ch);
    }
}

void
Tokenizer::inDecimal(char ch)
{
    if (util::is_digit(ch)) {
        this->state = st_real;
    } else {
        this->state = st_literal;
        inLiteral(ch);
    }
}

void
Tokenizer::inNumber(char ch)
{
    if (util::is_digit(ch)) {
    } else if (ch == '.') {
        this->state = st_real;
    } else if (isDelimiter(ch)) {
        this->type = tt::tt_integer;
        this->state = st_token_ready;
        this->in_token = false;
        this->char_to_unread = ch;
    } else {
        this->state = st_literal;
    }
}

void
Tokenizer::inReal(char ch)
{
    if (util::is_digit(ch)) {
    } else if (isDelimiter(ch)) {
        this->type = tt::tt_real;
        this->state = st_token_ready;
        this->in_token = false;
        this->char_to_unread = ch;
    } else {
        this->state = st_literal;
    }
}
void
Tokenizer::inStringEscape(char ch)
{
    this->state = st_in_string;
    switch (ch) {
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
        this->state = st_char_code;
        this->char_code = 0;
        this->digit_count = 0;
        inCharCode(ch);
        return;

    case 'n':
        this->val += '\n';
        return;

    case 'r':
        this->val += '\r';
        return;

    case 't':
        this->val += '\t';
        return;

    case 'b':
        this->val += '\b';
        return;

    case 'f':
        this->val += '\f';
        return;

    case '\n':
        return;

    case '\r':
        this->state = st_string_after_cr;
        return;

    default:
        // PDF spec says backslash is ignored before anything else
        this->val += ch;
        return;
    }
}

void
Tokenizer::inStringAfterCR(char ch)
{
    this->state = st_in_string;
    if (ch != '\n') {
        inString(ch);
    }
}

void
Tokenizer::inLt(char ch)
{
    if (ch == '<') {
        this->type = tt::tt_dict_open;
        this->state = st_token_ready;
        return;
    }

    this->state = st_in_hexstring;
    inHexstring(ch);
}

void
Tokenizer::inGt(char ch)
{
    if (ch == '>') {
        this->type = tt::tt_dict_close;
        this->state = st_token_ready;
    } else {
        this->type = tt::tt_bad;
        QTC::TC("qpdf", "QPDFTokenizer bad >");
        this->error_message = "unexpected >";
        this->in_token = false;
        this->char_to_unread = ch;
        this->state = st_token_ready;
    }
}

void
Tokenizer::inLiteral(char ch)
{
    if (isDelimiter(ch)) {
        // A C-locale whitespace character or delimiter terminates token.  It is important to unread
        // the whitespace character even though it is ignored since it may be the newline after a
        // stream keyword.  Removing it here could make the stream-reading code break on some files,
        // though not on any files in the test suite as of this writing.

        this->in_token = false;
        this->char_to_unread = ch;
        this->state = st_token_ready;
        this->type = (this->raw_val == "true") || (this->raw_val == "false")
            ? tt::tt_bool
            : (this->raw_val == "null" ? tt::tt_null : tt::tt_word);
    }
}

void
Tokenizer::inHexstring(char ch)
{
    if (char hval = util::hex_decode_char(ch); hval < '\20') {
        this->char_code = int(hval) << 4;
        this->state = st_in_hexstring_2nd;

    } else if (ch == '>') {
        this->type = tt::tt_string;
        this->state = st_token_ready;

    } else if (isSpace(ch)) {
        // ignore

    } else {
        this->type = tt::tt_bad;
        QTC::TC("qpdf", "QPDFTokenizer bad hexstring character");
        this->error_message = std::string("invalid character (") + ch + ") in hexstring";
        this->state = st_token_ready;
    }
}

void
Tokenizer::inHexstring2nd(char ch)
{
    if (char hval = util::hex_decode_char(ch); hval < '\20') {
        this->val += char(this->char_code) | hval;
        this->state = st_in_hexstring;

    } else if (ch == '>') {
        // PDF spec says odd hexstrings have implicit trailing 0.
        this->val += char(this->char_code);
        this->type = tt::tt_string;
        this->state = st_token_ready;

    } else if (isSpace(ch)) {
        // ignore

    } else {
        this->type = tt::tt_bad;
        QTC::TC("qpdf", "QPDFTokenizer bad hexstring 2nd character");
        this->error_message = std::string("invalid character (") + ch + ") in hexstring";
        this->state = st_token_ready;
    }
}

void
Tokenizer::inCharCode(char ch)
{
    bool handled = false;
    if (('0' <= ch) && (ch <= '7')) {
        this->char_code = 8 * this->char_code + (int(ch) - int('0'));
        if (++(this->digit_count) < 3) {
            return;
        }
        handled = true;
    }
    // We've accumulated \ddd or we have \d or \dd followed by other than an octal digit. The PDF
    // Spec says to ignore high-order overflow.
    this->val += char(this->char_code % 256);
    this->state = st_in_string;
    if (!handled) {
        inString(ch);
    }
}

void
Tokenizer::inInlineImage(char ch)
{
    if ((this->raw_val.length() + 1) == this->inline_image_bytes) {
        QTC::TC("qpdf", "QPDFTokenizer found EI by byte count");
        this->type = tt::tt_inline_image;
        this->inline_image_bytes = 0;
        this->state = st_token_ready;
    }
}

void
QPDFTokenizer::presentEOF()
{
    m->presentEOF();
}

void
Tokenizer::presentEOF()
{
    switch (this->state) {
    case st_name:
    case st_name_hex1:
    case st_name_hex2:
    case st_number:
    case st_real:
    case st_sign:
    case st_decimal:
    case st_literal:
        QTC::TC("qpdf", "QPDFTokenizer EOF reading appendable token");
        // Push any delimiter to the state machine to finish off the final token.
        presentCharacter('\f');
        this->in_token = true;
        break;

    case st_top:
    case st_before_token:
        this->type = tt::tt_eof;
        break;

    case st_in_space:
        this->type = this->include_ignorable ? tt::tt_space : tt::tt_eof;
        break;

    case st_in_comment:
        this->type = this->include_ignorable ? tt::tt_comment : tt::tt_bad;
        break;

    case st_token_ready:
        break;

    default:
        QTC::TC("qpdf", "QPDFTokenizer EOF reading token");
        this->type = tt::tt_bad;
        this->error_message = "EOF while reading token";
    }
    this->state = st_token_ready;
}

void
QPDFTokenizer::expectInlineImage(std::shared_ptr<InputSource> input)
{
    m->expectInlineImage(*input);
}

void
QPDFTokenizer::expectInlineImage(InputSource& input)
{
    m->expectInlineImage(input);
}

void
Tokenizer::expectInlineImage(InputSource& input)
{
    if (this->state == st_token_ready) {
        reset();
    } else if (this->state != st_before_token) {
        throw std::logic_error(
            "QPDFTokenizer::expectInlineImage called when tokenizer is in improper state");
    }
    findEI(input);
    this->before_token = false;
    this->in_token = true;
    this->state = st_inline_image;
}

void
Tokenizer::findEI(InputSource& input)
{
    qpdf_offset_t last_offset = input.getLastOffset();
    qpdf_offset_t pos = input.tell();

    // Use QPDFWordTokenFinder to find EI surrounded by delimiters. Then read the next several
    // tokens or up to EOF. If we find any suspicious-looking or tokens, this is probably still part
    // of the image data, so keep looking for EI. Stop at the first EI that passes. If we get to the
    // end without finding one, return the last EI we found. Store the number of bytes expected in
    // the inline image including the EI and use that to break out of inline image, falling back to
    // the old method if needed.

    bool okay = false;
    bool first_try = true;
    while (!okay) {
        QPDFWordTokenFinder f(input, "EI");
        if (!input.findFirst("EI", input.tell(), 0, f)) {
            break;
        }
        inline_image_bytes = QIntC::to_size(input.tell() - pos - 2);

        Tokenizer check;
        bool found_bad = false;
        // Look at the next 10 tokens or up to EOF. The next inline image's image data would look
        // like bad tokens, but there will always be at least 10 tokens between one inline image's
        // EI and the next valid one's ID since width, height, bits per pixel, and color space are
        // all required as well as a BI and ID. If we get 10 good tokens in a row or hit EOF, we can
        // be pretty sure we've found the actual EI.
        for (int i = 0; i < 10; ++i) {
            check.nextToken(input, "checker");
            auto typ = check.getType();
            if (typ == tt::tt_eof) {
                okay = true;
            } else if (typ == tt::tt_bad) {
                found_bad = true;
            } else if (typ == tt::tt_word) {
                // The qpdf tokenizer lumps alphabetic and otherwise uncategorized characters into
                // "words". We recognize strings of alphabetic characters as potential valid
                // operators for purposes of telling whether we're in valid content or not. It's not
                // perfect, but it should work more reliably than what we used to do, which was
                // already good enough for the vast majority of files.
                bool found_alpha = false;
                bool found_non_printable = false;
                bool found_other = false;
                for (char ch: check.getValue()) {
                    if ((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || (ch == '*')) {
                        // Treat '*' as alpha since there are valid PDF operators that contain *
                        // along with alphabetic characters.
                        found_alpha = true;
                    } else if (static_cast<signed char>(ch) < 32 && !isSpace(ch)) {
                        // Compare ch as a signed char so characters outside of 7-bit will be < 0.
                        found_non_printable = true;
                        break;
                    } else {
                        found_other = true;
                    }
                }
                if (found_non_printable || (found_alpha && found_other)) {
                    found_bad = true;
                }
            }
            if (okay || found_bad) {
                break;
            }
        }
        if (!found_bad) {
            okay = true;
        }
        if (!okay) {
            first_try = false;
        }
    }
    if (okay && (!first_try)) {
        QTC::TC("qpdf", "QPDFTokenizer found EI after more than one try");
    }

    input.seek(pos, SEEK_SET);
    input.setLastOffset(last_offset);
}

bool
QPDFTokenizer::getToken(Token& token, bool& unread_char, char& ch)
{
    return m->getToken(token, unread_char, ch);
}

bool
Tokenizer::getToken(Token& token, bool& unread_char, char& ch)
{
    bool ready = (this->state == st_token_ready);
    unread_char = !this->in_token && !this->before_token;
    ch = this->char_to_unread;
    if (ready) {
        token = (!(this->type == tt::tt_name || this->type == tt::tt_string))
            ? Token(this->type, this->raw_val, this->raw_val, this->error_message)
            : Token(this->type, this->val, this->raw_val, this->error_message);

        this->reset();
    }
    return ready;
}

bool
QPDFTokenizer::betweenTokens()
{
    return m->betweenTokens();
}

bool
Tokenizer::betweenTokens()
{
    return before_token;
}

QPDFTokenizer::Token
QPDFTokenizer::readToken(
    InputSource& input, std::string const& context, bool allow_bad, size_t max_len)
{
    return m->readToken(input, context, allow_bad, max_len);
}

QPDFTokenizer::Token
QPDFTokenizer::readToken(
    std::shared_ptr<InputSource> input, std::string const& context, bool allow_bad, size_t max_len)
{
    return m->readToken(*input, context, allow_bad, max_len);
}

QPDFTokenizer::Token
Tokenizer::readToken(InputSource& input, std::string const& context, bool allow_bad, size_t max_len)
{
    nextToken(input, context, max_len);

    Token token;
    bool unread_char;
    char char_to_unread;
    getToken(token, unread_char, char_to_unread);

    if (token.getType() == tt::tt_bad) {
        if (allow_bad) {
            QTC::TC("qpdf", "QPDFTokenizer allowing bad token");
        } else {
            throw QPDFExc(
                qpdf_e_damaged_pdf,
                input.getName(),
                context.empty() ? "offset " + std::to_string(input.getLastOffset()) : context,
                input.getLastOffset(),
                token.getErrorMessage());
        }
    }
    return token;
}

bool
Tokenizer::nextToken(InputSource& input, std::string const& context, size_t max_len)
{
    if (this->state != st_inline_image) {
        reset();
    }
    qpdf_offset_t offset = input.fastTell();

    while (this->state != st_token_ready) {
        char ch;
        if (!input.fastRead(ch)) {
            presentEOF();

            if ((this->type == tt::tt_eof) && (!this->allow_eof)) {
                // Nothing in the qpdf library calls readToken without allowEOF anymore, so this
                // case is not exercised.
                this->type = tt::tt_bad;
                this->error_message = "unexpected EOF";
                offset = input.getLastOffset();
            }
        } else {
            handleCharacter(ch);
            if (this->before_token) {
                ++offset;
            }
            if (this->in_token) {
                this->raw_val += ch;
            }
            if (max_len && (this->raw_val.length() >= max_len) && (this->state != st_token_ready)) {
                // terminate this token now
                QTC::TC("qpdf", "QPDFTokenizer block long token");
                this->type = tt::tt_bad;
                this->state = st_token_ready;
                this->error_message = "exceeded allowable length while reading token";
            }
        }
    }

    input.fastUnread(!this->in_token && !this->before_token);

    if (this->type != tt::tt_eof) {
        input.setLastOffset(offset);
    }

    return this->error_message.empty();
}

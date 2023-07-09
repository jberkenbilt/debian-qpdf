#ifndef QPDF_NAME_HH
#define QPDF_NAME_HH

#include <qpdf/QPDFValue.hh>

class QPDF_Name: public QPDFValue
{
  public:
    ~QPDF_Name() override = default;
    static std::shared_ptr<QPDFObject> create(std::string const& name);
    std::shared_ptr<QPDFObject> copy(bool shallow = false) override;
    std::string unparse() override;
    JSON getJSON(int json_version) override;

    // Put # into strings with characters unsuitable for name token
    static std::string normalizeName(std::string const& name);
    std::string
    getStringValue() const override
    {
        return name;
    }

  private:
    QPDF_Name(std::string const& name);
    std::string name;
};

#endif // QPDF_NAME_HH

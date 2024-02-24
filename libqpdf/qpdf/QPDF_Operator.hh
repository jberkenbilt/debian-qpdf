#ifndef QPDF_OPERATOR_HH
#define QPDF_OPERATOR_HH

#include <qpdf/QPDFValue.hh>

class QPDF_Operator: public QPDFValue
{
  public:
    ~QPDF_Operator() override = default;
    static std::shared_ptr<QPDFObject> create(std::string const& val);
    std::shared_ptr<QPDFObject> copy(bool shallow = false) override;
    std::string unparse() override;
    void writeJSON(int json_version, JSON::Writer& p) override;
    std::string
    getStringValue() const override
    {
        return val;
    }

  private:
    QPDF_Operator(std::string const& val);
    std::string val;
};

#endif // QPDF_OPERATOR_HH

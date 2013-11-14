#ifndef __QPDF_STREAM_HH__
#define __QPDF_STREAM_HH__

#include <qpdf/QPDFObject.hh>

#include <qpdf/QPDFObjectHandle.hh>

class Pipeline;
class QPDF;

class QPDF_Stream: public QPDFObject
{
  public:
    QPDF_Stream(QPDF*, int objid, int generation,
		QPDFObjectHandle stream_dict,
		off_t offset, int length);
    virtual ~QPDF_Stream();
    virtual std::string unparse();
    QPDFObjectHandle getDict() const;

    // See comments in QPDFObjectHandle.hh for these methods.
    bool pipeStreamData(Pipeline*, bool filter,
			bool normalize, bool compress);
    PointerHolder<Buffer> getStreamData();
    PointerHolder<Buffer> getRawStreamData();
    void replaceStreamData(PointerHolder<Buffer> data,
			   QPDFObjectHandle const& filter,
			   QPDFObjectHandle const& decode_parms);
    void replaceStreamData(
	PointerHolder<QPDFObjectHandle::StreamDataProvider> provider,
	QPDFObjectHandle const& filter,
	QPDFObjectHandle const& decode_parms,
	size_t length);

    // Replace object ID and generation.  This may only be called if
    // object ID and generation are 0.  It is used by QPDFObjectHandle
    // when adding streams to files.
    void setObjGen(int objid, int generation);

  private:
    void replaceFilterData(QPDFObjectHandle const& filter,
			   QPDFObjectHandle const& decode_parms,
			   size_t length);
    bool filterable(std::vector<std::string>& filters,
		    int& predictor, int& columns, bool& early_code_change);

    QPDF* qpdf;
    int objid;
    int generation;
    QPDFObjectHandle stream_dict;
    off_t offset;
    int length;
    PointerHolder<Buffer> stream_data;
    PointerHolder<QPDFObjectHandle::StreamDataProvider> stream_provider;
};

#endif // __QPDF_STREAM_HH__

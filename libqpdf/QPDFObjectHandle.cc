
#include <qpdf/QPDFObjectHandle.hh>

#include <qpdf/QPDF.hh>
#include <qpdf/QPDF_Bool.hh>
#include <qpdf/QPDF_Null.hh>
#include <qpdf/QPDF_Integer.hh>
#include <qpdf/QPDF_Real.hh>
#include <qpdf/QPDF_Name.hh>
#include <qpdf/QPDF_String.hh>
#include <qpdf/QPDF_Array.hh>
#include <qpdf/QPDF_Dictionary.hh>
#include <qpdf/QPDF_Stream.hh>

#include <qpdf/QTC.hh>
#include <qpdf/QEXC.hh>
#include <qpdf/QUtil.hh>

QPDFObjectHandle::QPDFObjectHandle() :
    initialized(false),
    objid(0),
    generation(0)
{
}

QPDFObjectHandle::QPDFObjectHandle(QPDF* qpdf, int objid, int generation) :
    initialized(true),
    qpdf(qpdf),
    objid(objid),
    generation(generation)
{
}

QPDFObjectHandle::QPDFObjectHandle(QPDFObject* data) :
    initialized(true),
    qpdf(0),
    objid(0),
    generation(0),
    obj(data)
{
}

bool
QPDFObjectHandle::isInitialized() const
{
    return this->initialized;
}

template <class T>
class QPDFObjectTypeAccessor
{
  public:
    static bool check(QPDFObject* o)
    {
	return (o && dynamic_cast<T*>(o));
    }
};

bool
QPDFObjectHandle::isBool()
{
    dereference();
    return QPDFObjectTypeAccessor<QPDF_Bool>::check(obj.getPointer());
}

bool
QPDFObjectHandle::isNull()
{
    dereference();
    return QPDFObjectTypeAccessor<QPDF_Null>::check(obj.getPointer());
}

bool
QPDFObjectHandle::isInteger()
{
    dereference();
    return QPDFObjectTypeAccessor<QPDF_Integer>::check(obj.getPointer());
}

bool
QPDFObjectHandle::isReal()
{
    dereference();
    return QPDFObjectTypeAccessor<QPDF_Real>::check(obj.getPointer());
}

bool
QPDFObjectHandle::isNumber()
{
    return (isInteger() || isReal());
}

double
QPDFObjectHandle::getNumericValue()
{
    double result = 0.0;
    if (isInteger())
    {
	result = getIntValue();
    }
    else if (isReal())
    {
	result = atof(getRealValue().c_str());
    }
    else
    {
	throw QEXC::Internal("getNumericValue called for non-numeric object");
    }
    return result;
}

bool
QPDFObjectHandle::isName()
{
    dereference();
    return QPDFObjectTypeAccessor<QPDF_Name>::check(obj.getPointer());
}

bool
QPDFObjectHandle::isString()
{
    dereference();
    return QPDFObjectTypeAccessor<QPDF_String>::check(obj.getPointer());
}

bool
QPDFObjectHandle::isArray()
{
    dereference();
    return QPDFObjectTypeAccessor<QPDF_Array>::check(obj.getPointer());
}

bool
QPDFObjectHandle::isDictionary()
{
    dereference();
    return QPDFObjectTypeAccessor<QPDF_Dictionary>::check(obj.getPointer());
}

bool
QPDFObjectHandle::isStream()
{
    dereference();
    return QPDFObjectTypeAccessor<QPDF_Stream>::check(obj.getPointer());
}

bool
QPDFObjectHandle::isIndirect()
{
    assertInitialized();
    return (this->objid != 0);
}

bool
QPDFObjectHandle::isScalar()
{
    return (! (isArray() || isDictionary() || isStream()));
}

// Bool accessors

bool
QPDFObjectHandle::getBoolValue()
{
    assertType("Boolean", isBool());
    return dynamic_cast<QPDF_Bool*>(obj.getPointer())->getVal();
}

// Integer accessors

int
QPDFObjectHandle::getIntValue()
{
    assertType("Integer", isInteger());
    return dynamic_cast<QPDF_Integer*>(obj.getPointer())->getVal();
}

// Real accessors

std::string
QPDFObjectHandle::getRealValue()
{
    assertType("Real", isReal());
    return dynamic_cast<QPDF_Real*>(obj.getPointer())->getVal();
}

// Name acessors

std::string
QPDFObjectHandle::getName()
{
    assertType("Name", isName());
    return dynamic_cast<QPDF_Name*>(obj.getPointer())->getName();
}

// String accessors

std::string
QPDFObjectHandle::getStringValue()
{
    assertType("String", isString());
    return dynamic_cast<QPDF_String*>(obj.getPointer())->getVal();
}

std::string
QPDFObjectHandle::getUTF8Value()
{
    assertType("String", isString());
    return dynamic_cast<QPDF_String*>(obj.getPointer())->getUTF8Val();
}

// Array acessors

int
QPDFObjectHandle::getArrayNItems()
{
    assertType("Array", isArray());
    return dynamic_cast<QPDF_Array*>(obj.getPointer())->getNItems();
}

QPDFObjectHandle
QPDFObjectHandle::getArrayItem(int n)
{
    assertType("Array", isArray());
    return dynamic_cast<QPDF_Array*>(obj.getPointer())->getItem(n);
}

// Array mutators

void
QPDFObjectHandle::setArrayItem(int n, QPDFObjectHandle const& item)
{
    assertType("Array", isArray());
    return dynamic_cast<QPDF_Array*>(obj.getPointer())->setItem(n, item);
}

// Dictionary accesors

bool
QPDFObjectHandle::hasKey(std::string const& key)
{
    assertType("Dictionary", isDictionary());
    return dynamic_cast<QPDF_Dictionary*>(obj.getPointer())->hasKey(key);
}

QPDFObjectHandle
QPDFObjectHandle::getKey(std::string const& key)
{
    assertType("Dictionary", isDictionary());
    return dynamic_cast<QPDF_Dictionary*>(obj.getPointer())->getKey(key);
}

std::set<std::string>
QPDFObjectHandle::getKeys()
{
    assertType("Dictionary", isDictionary());
    return dynamic_cast<QPDF_Dictionary*>(obj.getPointer())->getKeys();
}

// Dictionary mutators

void
QPDFObjectHandle::replaceKey(std::string const& key,
			    QPDFObjectHandle const& value)
{
    assertType("Dictionary", isDictionary());
    return dynamic_cast<QPDF_Dictionary*>(
	obj.getPointer())->replaceKey(key, value);
}

void
QPDFObjectHandle::removeKey(std::string const& key)
{
    assertType("Dictionary", isDictionary());
    return dynamic_cast<QPDF_Dictionary*>(obj.getPointer())->removeKey(key);
}

// Stream accessors
QPDFObjectHandle
QPDFObjectHandle::getDict()
{
    assertType("Stream", isStream());
    return dynamic_cast<QPDF_Stream*>(obj.getPointer())->getDict();
}

PointerHolder<Buffer>
QPDFObjectHandle::getStreamData()
{
    assertType("Stream", isStream());
    return dynamic_cast<QPDF_Stream*>(obj.getPointer())->getStreamData();
}

bool
QPDFObjectHandle::pipeStreamData(Pipeline* p, bool filter,
				 bool normalize, bool compress)
{
    assertType("Stream", isStream());
    return dynamic_cast<QPDF_Stream*>(obj.getPointer())->pipeStreamData(
	p, filter, normalize, compress);
}

int
QPDFObjectHandle::getObjectID() const
{
    return this->objid;
}

int
QPDFObjectHandle::getGeneration() const
{
    return this->generation;
}

std::map<std::string, QPDFObjectHandle>
QPDFObjectHandle::getPageImages()
{
    assertPageObject();

    // Note: this code doesn't handle inherited resources.  If this
    // page dictionary doesn't have a /Resources key or has one whose
    // value is null or an empty dictionary, you are supposed to walk
    // up the page tree until you find a /Resources dictionary.  As of
    // this writing, I don't have any test files that use inherited
    // resources, and hand-generating one won't be a good test beacuse
    // any mistakes in my understanding would be present in both the
    // code and the test file.

    // NOTE: If support of inherited resources (see above comment) is
    // implemented, edit comment in QPDFObjectHandle.hh for this
    // function.

    std::map<std::string, QPDFObjectHandle> result;
    if (this->hasKey("/Resources"))
    {
	QPDFObjectHandle resources = this->getKey("/Resources");
	if (resources.hasKey("/XObject"))
	{
	    QPDFObjectHandle xobject = resources.getKey("/XObject");
	    std::set<std::string> keys = xobject.getKeys();
	    for (std::set<std::string>::iterator iter = keys.begin();
		 iter != keys.end(); ++iter)
	    {
		std::string key = (*iter);
		QPDFObjectHandle value = xobject.getKey(key);
		if (value.isStream())
		{
		    QPDFObjectHandle dict = value.getDict();
		    if (dict.hasKey("/Subtype") &&
			(dict.getKey("/Subtype").getName() == "/Image") &&
			(! dict.hasKey("/ImageMask")))
		    {
			result[key] = value;
		    }
		}
	    }
	}
    }

    return result;
}

std::vector<QPDFObjectHandle>
QPDFObjectHandle::getPageContents()
{
    assertPageObject();

    std::vector<QPDFObjectHandle> result;
    QPDFObjectHandle contents = this->getKey("/Contents");
    if (contents.isArray())
    {
	int n_items = contents.getArrayNItems();
	for (int i = 0; i < n_items; ++i)
	{
	    QPDFObjectHandle item = contents.getArrayItem(i);
	    if (item.isStream())
	    {
		result.push_back(item);
	    }
	    else
	    {
		throw QEXC::General("unknown item type while inspecting "
				    "element of /Contents array in page "
				    "dictionary");
	    }
	}
    }
    else if (contents.isStream())
    {
	result.push_back(contents);
    }
    else
    {
	throw QEXC::General("unknown object type inspecting /Contents "
			    "key in page dictionary");
    }

    return result;
}

std::string
QPDFObjectHandle::unparse()
{
    std::string result;
    if (this->isIndirect())
    {
	result = QUtil::int_to_string(this->objid) + " " +
	    QUtil::int_to_string(this->generation) + " R";
    }
    else
    {
	result = unparseResolved();
    }
    return result;
}

std::string
QPDFObjectHandle::unparseResolved()
{
    dereference();
    return this->obj.getPointer()->unparse();
}

QPDFObjectHandle
QPDFObjectHandle::newIndirect(QPDF* qpdf, int objid, int generation)
{
    return QPDFObjectHandle(qpdf, objid, generation);
}

QPDFObjectHandle
QPDFObjectHandle::newBool(bool value)
{
    return QPDFObjectHandle(new QPDF_Bool(value));
}

QPDFObjectHandle
QPDFObjectHandle::newNull()
{
    return QPDFObjectHandle(new QPDF_Null());
}

QPDFObjectHandle
QPDFObjectHandle::newInteger(int value)
{
    return QPDFObjectHandle(new QPDF_Integer(value));
}

QPDFObjectHandle
QPDFObjectHandle::newReal(std::string const& value)
{
    return QPDFObjectHandle(new QPDF_Real(value));
}

QPDFObjectHandle
QPDFObjectHandle::newName(std::string const& name)
{
    return QPDFObjectHandle(new QPDF_Name(name));
}

QPDFObjectHandle
QPDFObjectHandle::newString(std::string const& str)
{
    return QPDFObjectHandle(new QPDF_String(str));
}

QPDFObjectHandle
QPDFObjectHandle::newArray(std::vector<QPDFObjectHandle> const& items)
{
    return QPDFObjectHandle(new QPDF_Array(items));
}

QPDFObjectHandle
QPDFObjectHandle::newDictionary(
    std::map<std::string, QPDFObjectHandle> const& items)
{
    return QPDFObjectHandle(new QPDF_Dictionary(items));
}


QPDFObjectHandle
QPDFObjectHandle::newStream(QPDF* qpdf, int objid, int generation,
			    QPDFObjectHandle stream_dict,
			    off_t offset, int length)
{
    return QPDFObjectHandle(new QPDF_Stream(
				qpdf, objid, generation,
				stream_dict, offset, length));
}

void
QPDFObjectHandle::makeDirectInternal(std::set<int>& visited)
{
    assertInitialized();

    if (isStream())
    {
	QTC::TC("qpdf", "QPDFObjectHandle ERR clone stream");
	throw QEXC::General("attempt to make a stream into a direct object");
    }

    int cur_objid = this->objid;
    if (cur_objid != 0)
    {
	if (visited.count(cur_objid))
	{
	    QTC::TC("qpdf", "QPDFObjectHandle makeDirect loop");
	    throw QEXC::General("loop detected while converting object from "
				"indirect to direct");
	}
	visited.insert(cur_objid);
    }

    dereference();
    this->objid = 0;
    this->generation = 0;

    QPDFObject* new_obj = 0;

    if (isBool())
    {
	QTC::TC("qpdf", "QPDFObjectHandle clone bool");
	new_obj = new QPDF_Bool(getBoolValue());
    }
    else if (isNull())
    {
	QTC::TC("qpdf", "QPDFObjectHandle clone null");
	new_obj = new QPDF_Null();
    }
    else if (isInteger())
    {
	QTC::TC("qpdf", "QPDFObjectHandle clone integer");
	new_obj = new QPDF_Integer(getIntValue());
    }
    else if (isReal())
    {
	QTC::TC("qpdf", "QPDFObjectHandle clone real");
	new_obj = new QPDF_Real(getRealValue());
    }
    else if (isName())
    {
	QTC::TC("qpdf", "QPDFObjectHandle clone name");
	new_obj = new QPDF_Name(getName());
    }
    else if (isString())
    {
	QTC::TC("qpdf", "QPDFObjectHandle clone string");
	new_obj = new QPDF_String(getStringValue());
    }
    else if (isArray())
    {
	QTC::TC("qpdf", "QPDFObjectHandle clone array");
	std::vector<QPDFObjectHandle> items;
	int n = getArrayNItems();
	for (int i = 0; i < n; ++i)
	{
	    items.push_back(getArrayItem(i));
	    items.back().makeDirectInternal(visited);
	}
	new_obj = new QPDF_Array(items);
    }
    else if (isDictionary())
    {
	QTC::TC("qpdf", "QPDFObjectHandle clone dictionary");
	std::set<std::string> keys = getKeys();
	std::map<std::string, QPDFObjectHandle> items;
	for (std::set<std::string>::iterator iter = keys.begin();
	     iter != keys.end(); ++iter)
	{
	    items[*iter] = getKey(*iter);
	    items[*iter].makeDirectInternal(visited);
	}
	new_obj = new QPDF_Dictionary(items);
    }
    else
    {
	throw QEXC::Internal("QPDFObjectHandle::makeIndirect: "
			     "unknown object type");
    }

    this->obj = new_obj;

    if (cur_objid)
    {
	visited.erase(cur_objid);
    }
}

void
QPDFObjectHandle::makeDirect()
{
    std::set<int> visited;
    makeDirectInternal(visited);
}

void
QPDFObjectHandle::assertInitialized() const
{
    if (! this->initialized)
    {
	throw QEXC::Internal("operation attempted on uninitialized "
			     "QPDFObjectHandle");
    }
}

void
QPDFObjectHandle::assertType(char const* type_name, bool istype)
{
    if (! istype)
    {
	throw QEXC::Internal(std::string("operation for ") + type_name +
			     " object attempted on object of wrong type");
    }
}

void
QPDFObjectHandle::assertPageObject()
{
    if (! (this->isDictionary() && this->hasKey("/Type") &&
	   (this->getKey("/Type").getName() == "/Page")))
    {
	throw QEXC::Internal("page operation called on non-Page object");
    }
}

void
QPDFObjectHandle::dereference()
{
    if (this->obj.getPointer() == 0)
    {
	this->obj = QPDF::Resolver::resolve(
	    this->qpdf, this->objid, this->generation);
	if (this->obj.getPointer() == 0)
	{
	    QTC::TC("qpdf", "QPDFObjectHandle indirect to unknown");
	    this->obj = new QPDF_Null();
	}
    }
}

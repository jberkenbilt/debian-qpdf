#include <qpdf/qpdf-c.h>

#include <qpdf/QPDF.hh>
#include <qpdf/QPDFWriter.hh>
#include <qpdf/QTC.hh>
#include <qpdf/QPDFExc.hh>

#include <list>
#include <string>
#include <stdexcept>

struct _qpdf_error
{
    PointerHolder<QPDFExc> exc;
};

struct _qpdf_data
{
    _qpdf_data();
    ~_qpdf_data();

    QPDF* qpdf;
    QPDFWriter* qpdf_writer;

    PointerHolder<QPDFExc> error;
    _qpdf_error tmp_error;
    std::list<QPDFExc> warnings;
    std::string tmp_string;

    // Parameters for functions we call
    char const* filename;
    char const* password;
};

_qpdf_data::_qpdf_data() :
    qpdf(0),
    qpdf_writer(0)
{
}

_qpdf_data::~_qpdf_data()
{
    delete qpdf_writer;
    delete qpdf;
}

// must set qpdf->filename and qpdf->password
static void call_read(qpdf_data qpdf)
{
    qpdf->qpdf->processFile(qpdf->filename, qpdf->password);
}

// must set qpdf->filename
static void call_init_write(qpdf_data qpdf)
{
    qpdf->qpdf_writer = new QPDFWriter(*(qpdf->qpdf), qpdf->filename);
}

static void call_write(qpdf_data qpdf)
{
    qpdf->qpdf_writer->write();
}

static QPDF_ERROR_CODE trap_errors(qpdf_data qpdf, void (*fn)(qpdf_data))
{
    QPDF_ERROR_CODE status = QPDF_SUCCESS;
    try
    {
	fn(qpdf);
    }
    catch (QPDFExc& e)
    {
	qpdf->error = new QPDFExc(e);
	status |= QPDF_ERRORS;
    }
    catch (std::runtime_error& e)
    {
	qpdf->error = new QPDFExc(qpdf_e_system, "", "", 0, e.what());
	status |= QPDF_ERRORS;
    }
    catch (std::exception& e)
    {
	qpdf->error = new QPDFExc(qpdf_e_internal, "", "", 0, e.what());
	status |= QPDF_ERRORS;
    }

    if (qpdf_more_warnings(qpdf))
    {
	status |= QPDF_WARNINGS;
    }
    return status;
}

char const* qpdf_get_qpdf_version()
{
    QTC::TC("qpdf", "qpdf-c called qpdf_get_qpdf_version");
    return QPDF::QPDFVersion().c_str();
}

qpdf_data qpdf_init()
{
    QTC::TC("qpdf", "qpdf-c called qpdf_init");
    qpdf_data qpdf = new _qpdf_data();
    qpdf->qpdf = new QPDF();
    return qpdf;
}

void qpdf_cleanup(qpdf_data* qpdf)
{
    QTC::TC("qpdf", "qpdf-c called qpdf_cleanup");
    delete *qpdf;
    *qpdf = 0;
}

QPDF_BOOL qpdf_more_warnings(qpdf_data qpdf)
{
    QTC::TC("qpdf", "qpdf-c called qpdf_more_warnings");

    if (qpdf->warnings.empty())
    {
	std::vector<QPDFExc> w = qpdf->qpdf->getWarnings();
	if (! w.empty())
	{
	    qpdf->warnings.assign(w.begin(), w.end());
	}
    }
    if (qpdf->warnings.empty())
    {
	return QPDF_FALSE;
    }
    else
    {
	return QPDF_TRUE;
    }
}

QPDF_BOOL qpdf_has_error(qpdf_data qpdf)
{
    QTC::TC("qpdf", "qpdf-c called qpdf_has_error");
    return (qpdf->error.getPointer() ? QPDF_TRUE : QPDF_FALSE);
}

qpdf_error qpdf_get_error(qpdf_data qpdf)
{
    if (qpdf->error.getPointer())
    {
	qpdf->tmp_error.exc = qpdf->error;
	qpdf->error = 0;
	QTC::TC("qpdf", "qpdf-c qpdf_get_error returned error");
	return &qpdf->tmp_error;
    }
    else
    {
	return 0;
    }
}

qpdf_error qpdf_next_warning(qpdf_data qpdf)
{
    if (qpdf_more_warnings(qpdf))
    {
	qpdf->tmp_error.exc = new QPDFExc(qpdf->warnings.front());
	qpdf->warnings.pop_front();
	QTC::TC("qpdf", "qpdf-c qpdf_next_warning returned warning");
	return &qpdf->tmp_error;
    }
    else
    {
	return 0;
    }
}

char const* qpdf_get_error_full_text(qpdf_data qpdf, qpdf_error e)
{
    if (e == 0)
    {
	return "";
    }
    return e->exc.getPointer()->what();
}

enum qpdf_error_code_e qpdf_get_error_code(qpdf_data qpdf, qpdf_error e)
{
    if (e == 0)
    {
	return qpdf_e_success;
    }
    return e->exc.getPointer()->getErrorCode();
}

char const* qpdf_get_error_filename(qpdf_data qpdf, qpdf_error e)
{
    if (e == 0)
    {
	return "";
    }
    return e->exc.getPointer()->getFilename().c_str();
}

unsigned long qpdf_get_error_file_position(qpdf_data qpdf, qpdf_error e)
{
    if (e == 0)
    {
	return 0;
    }
    return e->exc.getPointer()->getFilePosition();
}

char const* qpdf_get_error_message_detail(qpdf_data qpdf, qpdf_error e)
{
    if (e == 0)
    {
	return "";
    }
    return e->exc.getPointer()->getMessageDetail().c_str();
}

void qpdf_set_suppress_warnings(qpdf_data qpdf, QPDF_BOOL value)
{
    QTC::TC("qpdf", "qpdf-c called qpdf_set_suppress_warnings");
    qpdf->qpdf->setSuppressWarnings(value);
}

void qpdf_set_ignore_xref_streams(qpdf_data qpdf, QPDF_BOOL value)
{
    QTC::TC("qpdf", "qpdf-c called qpdf_set_ignore_xref_streams");
    qpdf->qpdf->setIgnoreXRefStreams(value);
}

void qpdf_set_attempt_recovery(qpdf_data qpdf, QPDF_BOOL value)
{
    QTC::TC("qpdf", "qpdf-c called qpdf_set_attempt_recovery");
    qpdf->qpdf->setAttemptRecovery(value);
}

QPDF_ERROR_CODE qpdf_read(qpdf_data qpdf, char const* filename,
			  char const* password)
{
    QPDF_ERROR_CODE status = QPDF_SUCCESS;
    qpdf->filename = filename;
    qpdf->password = password;
    status = trap_errors(qpdf, &call_read);
    QTC::TC("qpdf", "qpdf-c called qpdf_read", status);
    return status;
}

char const* qpdf_get_pdf_version(qpdf_data qpdf)
{
    QTC::TC("qpdf", "qpdf-c called qpdf_get_pdf_version");
    qpdf->tmp_string = qpdf->qpdf->getPDFVersion();
    return qpdf->tmp_string.c_str();
}

char const* qpdf_get_user_password(qpdf_data qpdf)
{
    QTC::TC("qpdf", "qpdf-c called qpdf_get_user_password");
    qpdf->tmp_string = qpdf->qpdf->getTrimmedUserPassword();
    return qpdf->tmp_string.c_str();
}

QPDF_BOOL qpdf_is_linearized(qpdf_data qpdf)
{
    QTC::TC("qpdf", "qpdf-c called qpdf_is_linearized");
    return (qpdf->qpdf->isLinearized() ? QPDF_TRUE : QPDF_FALSE);
}

QPDF_BOOL qpdf_is_encrypted(qpdf_data qpdf)
{
    QTC::TC("qpdf", "qpdf-c called qpdf_is_encrypted");
    return (qpdf->qpdf->isEncrypted() ? QPDF_TRUE : QPDF_FALSE);
}

QPDF_BOOL qpdf_allow_accessibility(qpdf_data qpdf)
{
    QTC::TC("qpdf", "qpdf-c called qpdf_allow_accessibility");
    return qpdf->qpdf->allowAccessibility();
}

QPDF_BOOL qpdf_allow_extract_all(qpdf_data qpdf)
{
    QTC::TC("qpdf", "qpdf-c called qpdf_allow_extract_all");
    return qpdf->qpdf->allowExtractAll();
}

QPDF_BOOL qpdf_allow_print_low_res(qpdf_data qpdf)
{
    QTC::TC("qpdf", "qpdf-c called qpdf_allow_print_low_res");
    return qpdf->qpdf->allowPrintLowRes();
}

QPDF_BOOL qpdf_allow_print_high_res(qpdf_data qpdf)
{
    QTC::TC("qpdf", "qpdf-c called qpdf_allow_print_high_res");
    return qpdf->qpdf->allowPrintHighRes();
}

QPDF_BOOL qpdf_allow_modify_assembly(qpdf_data qpdf)
{
    QTC::TC("qpdf", "qpdf-c called qpdf_allow_modify_assembly");
    return qpdf->qpdf->allowModifyAssembly();
}

QPDF_BOOL qpdf_allow_modify_form(qpdf_data qpdf)
{
    QTC::TC("qpdf", "qpdf-c called qpdf_allow_modify_form");
    return qpdf->qpdf->allowModifyForm();
}

QPDF_BOOL qpdf_allow_modify_annotation(qpdf_data qpdf)
{
    QTC::TC("qpdf", "qpdf-c called qpdf_allow_modify_annotation");
    return qpdf->qpdf->allowModifyAnnotation();
}

QPDF_BOOL qpdf_allow_modify_other(qpdf_data qpdf)
{
    QTC::TC("qpdf", "qpdf-c called qpdf_allow_modify_other");
    return qpdf->qpdf->allowModifyOther();
}

QPDF_BOOL qpdf_allow_modify_all(qpdf_data qpdf)
{
    QTC::TC("qpdf", "qpdf-c called qpdf_allow_modify_all");
    return qpdf->qpdf->allowModifyAll();
}

QPDF_ERROR_CODE qpdf_init_write(qpdf_data qpdf, char const* filename)
{
    QPDF_ERROR_CODE status = QPDF_SUCCESS;
    if (qpdf->qpdf_writer)
    {
	QTC::TC("qpdf", "qpdf-c called qpdf_init_write multiple times");
	delete qpdf->qpdf_writer;
	qpdf->qpdf_writer = 0;
    }
    qpdf->filename = filename;
    status = trap_errors(qpdf, &call_init_write);
    QTC::TC("qpdf", "qpdf-c called qpdf_init_write", status);
    return status;
}

void qpdf_set_object_stream_mode(qpdf_data qpdf, qpdf_object_stream_e mode)
{
    QTC::TC("qpdf", "qpdf-c called qpdf_set_object_stream_mode");
    qpdf->qpdf_writer->setObjectStreamMode(mode);
}

void qpdf_set_stream_data_mode(qpdf_data qpdf, qpdf_stream_data_e mode)
{
    QTC::TC("qpdf", "qpdf-c called qpdf_set_stream_data_mode");
    qpdf->qpdf_writer->setStreamDataMode(mode);
}

void qpdf_set_content_normalization(qpdf_data qpdf, QPDF_BOOL value)
{
    QTC::TC("qpdf", "qpdf-c called qpdf_set_content_normalization");
    qpdf->qpdf_writer->setContentNormalization(value);
}

void qpdf_set_qdf_mode(qpdf_data qpdf, QPDF_BOOL value)
{
    QTC::TC("qpdf", "qpdf-c called qpdf_set_qdf_mode");
    qpdf->qpdf_writer->setQDFMode(value);
}

void qpdf_set_static_ID(qpdf_data qpdf, QPDF_BOOL value)
{
    QTC::TC("qpdf", "qpdf-c called qpdf_set_static_ID");
    qpdf->qpdf_writer->setStaticID(value);
}

void qpdf_set_static_aes_IV(qpdf_data qpdf, QPDF_BOOL value)
{
    QTC::TC("qpdf", "qpdf-c called qpdf_set_static_aes_IV");
    qpdf->qpdf_writer->setStaticAesIV(value);
}

void qpdf_set_suppress_original_object_IDs(
    qpdf_data qpdf, QPDF_BOOL value)
{
    QTC::TC("qpdf", "qpdf-c called qpdf_set_suppress_original_object_IDs");
    qpdf->qpdf_writer->setSuppressOriginalObjectIDs(value);
}

void qpdf_set_preserve_encryption(qpdf_data qpdf, QPDF_BOOL value)
{
    QTC::TC("qpdf", "qpdf-c called qpdf_set_preserve_encryption");
    qpdf->qpdf_writer->setPreserveEncryption(value);
}

void qpdf_set_r2_encryption_parameters(
    qpdf_data qpdf, char const* user_password, char const* owner_password,
    QPDF_BOOL allow_print, QPDF_BOOL allow_modify,
    QPDF_BOOL allow_extract, QPDF_BOOL allow_annotate)
{
    QTC::TC("qpdf", "qpdf-c called qpdf_set_r2_encryption_parameters");
    qpdf->qpdf_writer->setR2EncryptionParameters(
	user_password, owner_password,
	allow_print, allow_modify, allow_extract, allow_annotate);
}

void qpdf_set_r3_encryption_parameters(
    qpdf_data qpdf, char const* user_password, char const* owner_password,
    QPDF_BOOL allow_accessibility, QPDF_BOOL allow_extract,
    qpdf_r3_print_e print, qpdf_r3_modify_e modify)
{
    QTC::TC("qpdf", "qpdf-c called qpdf_set_r3_encryption_parameters");
    qpdf->qpdf_writer->setR3EncryptionParameters(
	user_password, owner_password,
	allow_accessibility, allow_extract, print, modify);
}

void qpdf_set_r4_encryption_parameters(
    qpdf_data qpdf, char const* user_password, char const* owner_password,
    QPDF_BOOL allow_accessibility, QPDF_BOOL allow_extract,
    qpdf_r3_print_e print, qpdf_r3_modify_e modify,
    QPDF_BOOL encrypt_metadata, QPDF_BOOL use_aes)
{
    QTC::TC("qpdf", "qpdf-c called qpdf_set_r4_encryption_parameters");
    qpdf->qpdf_writer->setR4EncryptionParameters(
	user_password, owner_password,
	allow_accessibility, allow_extract, print, modify,
	encrypt_metadata, use_aes);
}

void qpdf_set_linearization(qpdf_data qpdf, QPDF_BOOL value)
{
    QTC::TC("qpdf", "qpdf-c called qpdf_set_linearization");
    qpdf->qpdf_writer->setLinearization(value);
}

void qpdf_set_minimum_pdf_version(qpdf_data qpdf, char const* version)
{
    QTC::TC("qpdf", "qpdf-c called qpdf_set_minimum_pdf_version");
    qpdf->qpdf_writer->setMinimumPDFVersion(version);
}

void qpdf_force_pdf_version(qpdf_data qpdf, char const* version)
{
    QTC::TC("qpdf", "qpdf-c called qpdf_force_pdf_version");
    qpdf->qpdf_writer->forcePDFVersion(version);
}

QPDF_ERROR_CODE qpdf_write(qpdf_data qpdf)
{
    QPDF_ERROR_CODE status = QPDF_SUCCESS;
    status = trap_errors(qpdf, &call_write);
    QTC::TC("qpdf", "qpdf-c called qpdf_write", status);
    return status;
}
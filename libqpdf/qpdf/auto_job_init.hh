//
// This file is automatically generated by generate_auto_job.
// Edits will be automatically overwritten if the build is
// run in maintainer mode.
//
// clang-format off
//
auto b = [this](void (ArgParser::*f)()) {
    return QPDFArgParser::bindBare(f, this);
};
auto p = [this](void (ArgParser::*f)(std::string const&)) {
    return QPDFArgParser::bindParam(f, this);
};

static char const* yn_choices[] = {"y", "n", 0};
static char const* password_mode_choices[] = {"bytes", "hex-bytes", "unicode", "auto", 0};
static char const* stream_data_choices[] = {"compress", "preserve", "uncompress", 0};
static char const* decode_level_choices[] = {"none", "generalized", "specialized", "all", 0};
static char const* object_streams_choices[] = {"disable", "preserve", "generate", 0};
static char const* remove_unref_choices[] = {"auto", "yes", "no", 0};
static char const* flatten_choices[] = {"all", "print", "screen", 0};
static char const* json_version_choices[] = {"1", "latest", 0};
static char const* json_key_choices[] = {"acroform", "attachments", "encrypt", "objectinfo", "objects", "outlines", "pagelabels", "pages", 0};
static char const* print128_choices[] = {"full", "low", "none", 0};
static char const* modify128_choices[] = {"all", "annotate", "form", "assembly", "none", 0};

this->ap.selectHelpOptionTable();
this->ap.addBare("version", b(&ArgParser::argVersion));
this->ap.addBare("copyright", b(&ArgParser::argCopyright));
this->ap.addBare("json-help", b(&ArgParser::argJsonHelp));
this->ap.addBare("show-crypto", b(&ArgParser::argShowCrypto));
this->ap.addBare("job-json-help", b(&ArgParser::argJobJsonHelp));
this->ap.selectMainOptionTable();
this->ap.addPositional(p(&ArgParser::argPositional));
this->ap.addBare("add-attachment", b(&ArgParser::argAddAttachment));
this->ap.addBare("allow-weak-crypto", [this](){c_main->allowWeakCrypto();});
this->ap.addBare("check", [this](){c_main->check();});
this->ap.addBare("check-linearization", [this](){c_main->checkLinearization();});
this->ap.addBare("coalesce-contents", [this](){c_main->coalesceContents();});
this->ap.addBare("copy-attachments-from", b(&ArgParser::argCopyAttachmentsFrom));
this->ap.addBare("decrypt", [this](){c_main->decrypt();});
this->ap.addBare("deterministic-id", [this](){c_main->deterministicId();});
this->ap.addBare("empty", b(&ArgParser::argEmpty));
this->ap.addBare("encrypt", b(&ArgParser::argEncrypt));
this->ap.addBare("externalize-inline-images", [this](){c_main->externalizeInlineImages();});
this->ap.addBare("filtered-stream-data", [this](){c_main->filteredStreamData();});
this->ap.addBare("flatten-rotation", [this](){c_main->flattenRotation();});
this->ap.addBare("generate-appearances", [this](){c_main->generateAppearances();});
this->ap.addBare("ignore-xref-streams", [this](){c_main->ignoreXrefStreams();});
this->ap.addBare("is-encrypted", [this](){c_main->isEncrypted();});
this->ap.addBare("keep-inline-images", [this](){c_main->keepInlineImages();});
this->ap.addBare("linearize", [this](){c_main->linearize();});
this->ap.addBare("list-attachments", [this](){c_main->listAttachments();});
this->ap.addBare("newline-before-endstream", [this](){c_main->newlineBeforeEndstream();});
this->ap.addBare("no-original-object-ids", [this](){c_main->noOriginalObjectIds();});
this->ap.addBare("no-warn", [this](){c_main->noWarn();});
this->ap.addBare("optimize-images", [this](){c_main->optimizeImages();});
this->ap.addBare("overlay", b(&ArgParser::argOverlay));
this->ap.addBare("pages", b(&ArgParser::argPages));
this->ap.addBare("password-is-hex-key", [this](){c_main->passwordIsHexKey();});
this->ap.addBare("preserve-unreferenced", [this](){c_main->preserveUnreferenced();});
this->ap.addBare("preserve-unreferenced-resources", [this](){c_main->preserveUnreferencedResources();});
this->ap.addBare("progress", [this](){c_main->progress();});
this->ap.addBare("qdf", [this](){c_main->qdf();});
this->ap.addBare("raw-stream-data", [this](){c_main->rawStreamData();});
this->ap.addBare("recompress-flate", [this](){c_main->recompressFlate();});
this->ap.addBare("remove-page-labels", [this](){c_main->removePageLabels();});
this->ap.addBare("replace-input", b(&ArgParser::argReplaceInput));
this->ap.addBare("requires-password", [this](){c_main->requiresPassword();});
this->ap.addBare("show-encryption", [this](){c_main->showEncryption();});
this->ap.addBare("show-encryption-key", [this](){c_main->showEncryptionKey();});
this->ap.addBare("show-linearization", [this](){c_main->showLinearization();});
this->ap.addBare("show-npages", [this](){c_main->showNpages();});
this->ap.addBare("show-pages", [this](){c_main->showPages();});
this->ap.addBare("show-xref", [this](){c_main->showXref();});
this->ap.addBare("static-aes-iv", [this](){c_main->staticAesIv();});
this->ap.addBare("static-id", [this](){c_main->staticId();});
this->ap.addBare("suppress-password-recovery", [this](){c_main->suppressPasswordRecovery();});
this->ap.addBare("suppress-recovery", [this](){c_main->suppressRecovery();});
this->ap.addBare("underlay", b(&ArgParser::argUnderlay));
this->ap.addBare("verbose", [this](){c_main->verbose();});
this->ap.addBare("warning-exit-0", [this](){c_main->warningExit0();});
this->ap.addBare("with-images", [this](){c_main->withImages();});
this->ap.addRequiredParameter("compression-level", [this](std::string const& x){c_main->compressionLevel(x);}, "level");
this->ap.addRequiredParameter("copy-encryption", [this](std::string const& x){c_main->copyEncryption(x);}, "file");
this->ap.addRequiredParameter("encryption-file-password", [this](std::string const& x){c_main->encryptionFilePassword(x);}, "password");
this->ap.addRequiredParameter("force-version", [this](std::string const& x){c_main->forceVersion(x);}, "version");
this->ap.addRequiredParameter("ii-min-bytes", [this](std::string const& x){c_main->iiMinBytes(x);}, "minimum");
this->ap.addRequiredParameter("job-json-file", [this](std::string const& x){c_main->jobJsonFile(x);}, "file");
this->ap.addRequiredParameter("json-object", [this](std::string const& x){c_main->jsonObject(x);}, "trailer");
this->ap.addRequiredParameter("keep-files-open-threshold", [this](std::string const& x){c_main->keepFilesOpenThreshold(x);}, "count");
this->ap.addRequiredParameter("linearize-pass1", [this](std::string const& x){c_main->linearizePass1(x);}, "filename");
this->ap.addRequiredParameter("min-version", [this](std::string const& x){c_main->minVersion(x);}, "version");
this->ap.addRequiredParameter("oi-min-area", [this](std::string const& x){c_main->oiMinArea(x);}, "minimum");
this->ap.addRequiredParameter("oi-min-height", [this](std::string const& x){c_main->oiMinHeight(x);}, "minimum");
this->ap.addRequiredParameter("oi-min-width", [this](std::string const& x){c_main->oiMinWidth(x);}, "minimum");
this->ap.addRequiredParameter("password", [this](std::string const& x){c_main->password(x);}, "password");
this->ap.addRequiredParameter("password-file", [this](std::string const& x){c_main->passwordFile(x);}, "password");
this->ap.addRequiredParameter("remove-attachment", [this](std::string const& x){c_main->removeAttachment(x);}, "attachment");
this->ap.addRequiredParameter("rotate", [this](std::string const& x){c_main->rotate(x);}, "[+|-]angle");
this->ap.addRequiredParameter("show-attachment", [this](std::string const& x){c_main->showAttachment(x);}, "attachment");
this->ap.addRequiredParameter("show-object", [this](std::string const& x){c_main->showObject(x);}, "trailer");
this->ap.addOptionalParameter("collate", [this](std::string const& x){c_main->collate(x);});
this->ap.addOptionalParameter("split-pages", [this](std::string const& x){c_main->splitPages(x);});
this->ap.addChoices("compress-streams", [this](std::string const& x){c_main->compressStreams(x);}, true, yn_choices);
this->ap.addChoices("decode-level", [this](std::string const& x){c_main->decodeLevel(x);}, true, decode_level_choices);
this->ap.addChoices("flatten-annotations", [this](std::string const& x){c_main->flattenAnnotations(x);}, true, flatten_choices);
this->ap.addChoices("json-key", [this](std::string const& x){c_main->jsonKey(x);}, true, json_key_choices);
this->ap.addChoices("keep-files-open", [this](std::string const& x){c_main->keepFilesOpen(x);}, true, yn_choices);
this->ap.addChoices("normalize-content", [this](std::string const& x){c_main->normalizeContent(x);}, true, yn_choices);
this->ap.addChoices("object-streams", [this](std::string const& x){c_main->objectStreams(x);}, true, object_streams_choices);
this->ap.addChoices("password-mode", [this](std::string const& x){c_main->passwordMode(x);}, true, password_mode_choices);
this->ap.addChoices("remove-unreferenced-resources", [this](std::string const& x){c_main->removeUnreferencedResources(x);}, true, remove_unref_choices);
this->ap.addChoices("stream-data", [this](std::string const& x){c_main->streamData(x);}, true, stream_data_choices);
this->ap.addChoices("json", [this](std::string const& x){c_main->json(x);}, false, json_version_choices);
this->ap.registerOptionTable("pages", b(&ArgParser::argEndPages));
this->ap.addPositional(p(&ArgParser::argPagesPositional));
this->ap.addRequiredParameter("password", p(&ArgParser::argPagesPassword), "password");
this->ap.registerOptionTable("encryption", b(&ArgParser::argEndEncryption));
this->ap.addPositional(p(&ArgParser::argEncPositional));
this->ap.registerOptionTable("40-bit encryption", b(&ArgParser::argEnd40BitEncryption));
this->ap.addChoices("extract", [this](std::string const& x){c_enc->extract(x);}, true, yn_choices);
this->ap.addChoices("annotate", [this](std::string const& x){c_enc->annotate(x);}, true, yn_choices);
this->ap.addChoices("print", [this](std::string const& x){c_enc->print(x);}, true, yn_choices);
this->ap.addChoices("modify", [this](std::string const& x){c_enc->modify(x);}, true, yn_choices);
this->ap.registerOptionTable("128-bit encryption", b(&ArgParser::argEnd128BitEncryption));
this->ap.addBare("cleartext-metadata", [this](){c_enc->cleartextMetadata();});
this->ap.addBare("force-V4", [this](){c_enc->forceV4();});
this->ap.addChoices("accessibility", [this](std::string const& x){c_enc->accessibility(x);}, true, yn_choices);
this->ap.addChoices("extract", [this](std::string const& x){c_enc->extract(x);}, true, yn_choices);
this->ap.addChoices("print", [this](std::string const& x){c_enc->print(x);}, true, print128_choices);
this->ap.addChoices("assemble", [this](std::string const& x){c_enc->assemble(x);}, true, yn_choices);
this->ap.addChoices("annotate", [this](std::string const& x){c_enc->annotate(x);}, true, yn_choices);
this->ap.addChoices("form", [this](std::string const& x){c_enc->form(x);}, true, yn_choices);
this->ap.addChoices("modify-other", [this](std::string const& x){c_enc->modifyOther(x);}, true, yn_choices);
this->ap.addChoices("modify", [this](std::string const& x){c_enc->modify(x);}, true, modify128_choices);
this->ap.addChoices("use-aes", [this](std::string const& x){c_enc->useAes(x);}, true, yn_choices);
this->ap.registerOptionTable("256-bit encryption", b(&ArgParser::argEnd256BitEncryption));
this->ap.addBare("cleartext-metadata", [this](){c_enc->cleartextMetadata();});
this->ap.addBare("force-R5", [this](){c_enc->forceR5();});
this->ap.addBare("allow-insecure", [this](){c_enc->allowInsecure();});
this->ap.addChoices("accessibility", [this](std::string const& x){c_enc->accessibility(x);}, true, yn_choices);
this->ap.addChoices("extract", [this](std::string const& x){c_enc->extract(x);}, true, yn_choices);
this->ap.addChoices("print", [this](std::string const& x){c_enc->print(x);}, true, print128_choices);
this->ap.addChoices("assemble", [this](std::string const& x){c_enc->assemble(x);}, true, yn_choices);
this->ap.addChoices("annotate", [this](std::string const& x){c_enc->annotate(x);}, true, yn_choices);
this->ap.addChoices("form", [this](std::string const& x){c_enc->form(x);}, true, yn_choices);
this->ap.addChoices("modify-other", [this](std::string const& x){c_enc->modifyOther(x);}, true, yn_choices);
this->ap.addChoices("modify", [this](std::string const& x){c_enc->modify(x);}, true, modify128_choices);
this->ap.registerOptionTable("underlay/overlay", b(&ArgParser::argEndUnderlayOverlay));
this->ap.addPositional(p(&ArgParser::argUOPositional));
this->ap.addRequiredParameter("to", [this](std::string const& x){c_uo->to(x);}, "page-range");
this->ap.addRequiredParameter("from", [this](std::string const& x){c_uo->from(x);}, "page-range");
this->ap.addRequiredParameter("repeat", [this](std::string const& x){c_uo->repeat(x);}, "page-range");
this->ap.addRequiredParameter("password", [this](std::string const& x){c_uo->password(x);}, "password");
this->ap.registerOptionTable("attachment", b(&ArgParser::argEndAttachment));
this->ap.addPositional(p(&ArgParser::argAttPositional));
this->ap.addBare("replace", [this](){c_att->replace();});
this->ap.addRequiredParameter("key", [this](std::string const& x){c_att->key(x);}, "attachment-key");
this->ap.addRequiredParameter("filename", [this](std::string const& x){c_att->filename(x);}, "filename");
this->ap.addRequiredParameter("creationdate", [this](std::string const& x){c_att->creationdate(x);}, "creation-date");
this->ap.addRequiredParameter("moddate", [this](std::string const& x){c_att->moddate(x);}, "modification-date");
this->ap.addRequiredParameter("mimetype", [this](std::string const& x){c_att->mimetype(x);}, "mime/type");
this->ap.addRequiredParameter("description", [this](std::string const& x){c_att->description(x);}, "description");
this->ap.registerOptionTable("copy attachment", b(&ArgParser::argEndCopyAttachment));
this->ap.addPositional(p(&ArgParser::argCopyAttPositional));
this->ap.addRequiredParameter("prefix", [this](std::string const& x){c_copy_att->prefix(x);}, "prefix");
this->ap.addRequiredParameter("password", [this](std::string const& x){c_copy_att->password(x);}, "password");
#include <qpdf/QPDFArgParser.hh>

#include <qpdf/QIntC.hh>
#include <qpdf/QPDFLogger.hh>
#include <qpdf/QPDFUsage.hh>
#include <qpdf/QTC.hh>
#include <qpdf/QUtil.hh>
#include <qpdf/Util.hh>
#include <qpdf/auto_job_completion_bash.hh>
#include <qpdf/auto_job_completion_zsh.hh>

#include <iostream>

using namespace qpdf;
using namespace std::literals;

QPDFArgParser::Members::Members(int argc, char const* const argv[]) :
    argc(argc),
    argv(argv)
{
    auto tmp = QUtil::make_unique_cstr(argv[0]);
    whoami = QUtil::getWhoami(tmp.get());
}

QPDFArgParser::QPDFArgParser(int argc, char const* const argv[], char const*) :
    QPDFArgParser(argc, argv)
{
}

QPDFArgParser::QPDFArgParser(int argc, char const* const argv[]) :
    m(new Members(argc, argv))
{
    selectHelpOptionTable();
    char const* help_choices[] = {"all", nullptr};
    // More help choices are added dynamically.
    addChoices("help", bindParam(&QPDFArgParser::argHelp, this), false, help_choices);
    addInvalidChoiceHandler("help", bindParam(&QPDFArgParser::invalidHelpArg, this));
    addBare("completion-bash", bindBare(&QPDFArgParser::argCompletionBash, this));
    addBare("completion-zsh", bindBare(&QPDFArgParser::argCompletionZsh, this));
    selectMainOptionTable();
}

void
QPDFArgParser::selectMainOptionTable()
{
    m->option_table = &m->main_option_table;
    m->option_table_name = "main";
}

void
QPDFArgParser::selectHelpOptionTable()
{
    m->option_table = &m->help_option_table;
    m->option_table_name = "help";
}

void
QPDFArgParser::selectOptionTable(std::string const& name)
{
    auto t = m->option_tables.find(name);
    if (t == m->option_tables.end()) {
        throw std::logic_error("QPDFArgParser: selecting unregistered option table " + name);
    }
    m->option_table = &(t->second);
    m->option_table_name = name;
}

void
QPDFArgParser::registerOptionTable(std::string const& name, bare_arg_handler_t end_handler)
{
    if (m->option_tables.contains(name)) {
        throw std::logic_error(
            "QPDFArgParser: registering already registered option table " + name);
    }
    m->option_tables[name];
    selectOptionTable(name);
    addBare("--", end_handler);
}

QPDFArgParser::OptionEntry&
QPDFArgParser::registerArg(std::string const& arg)
{
    if (m->option_table->contains(arg)) {
        throw std::logic_error(
            "QPDFArgParser: adding a duplicate handler for option " + arg + " in " +
            m->option_table_name + " option table");
    }
    return ((*m->option_table)[arg]);
}

void
QPDFArgParser::addPositional(param_arg_handler_t handler)
{
    OptionEntry& oe = registerArg("");
    oe.param_arg_handler = handler;
}

void
QPDFArgParser::addBare(std::string const& arg, bare_arg_handler_t handler)
{
    OptionEntry& oe = registerArg(arg);
    oe.parameter_needed = false;
    oe.bare_arg_handler = handler;
}

void
QPDFArgParser::addRequiredParameter(
    std::string const& arg, param_arg_handler_t handler, char const* parameter_name)
{
    OptionEntry& oe = registerArg(arg);
    oe.parameter_needed = true;
    oe.parameter_name = parameter_name;
    oe.param_arg_handler = handler;
}

void
QPDFArgParser::addOptionalParameter(std::string const& arg, param_arg_handler_t handler)
{
    OptionEntry& oe = registerArg(arg);
    oe.parameter_needed = false;
    oe.param_arg_handler = handler;
}

void
QPDFArgParser::addChoices(
    std::string const& arg, param_arg_handler_t handler, bool required, char const** choices)
{
    OptionEntry& oe = registerArg(arg);
    oe.parameter_needed = required;
    oe.param_arg_handler = handler;
    for (char const** i = choices; *i; ++i) {
        oe.choices.insert(*i);
    }
}

void
QPDFArgParser::addInvalidChoiceHandler(std::string const& arg, param_arg_handler_t handler)
{
    auto i = m->option_table->find(arg);
    if (i == m->option_table->end()) {
        throw std::logic_error(
            "QPDFArgParser: attempt to add invalid choice handler to unknown argument");
    }
    auto& oe = i->second;
    oe.invalid_choice_handler = handler;
}

void
QPDFArgParser::addFinalCheck(bare_arg_handler_t handler)
{
    m->final_check_handler = handler;
}

int
QPDFArgParser::argsLeft() const
{
    return m->argc - m->cur_arg - 1;
}

void
QPDFArgParser::argCompletionBash()
{
    for (auto const& line: AUTO_COMPLETION_BASH) {
        std::cout << line << "\n";
    }
}

void
QPDFArgParser::argCompletionZsh()
{
    for (auto const& line: AUTO_COMPLETION_ZSH) {
        std::cout << line << "\n";
    }
}

void
QPDFArgParser::argHelp(std::string const& p)
{
    QPDFLogger::defaultLogger()->info(getHelp(p));
    exit(0);
}

void
QPDFArgParser::invalidHelpArg(std::string const& p)
{
    usage(std::string("unknown help option") + (p.empty() ? "" : (" " + p)));
}

void
QPDFArgParser::handleArgFileArguments()
{
    // Support reading arguments from files. Create a new argv. Ensure that argv itself as well as
    // all its contents are automatically deleted by using shared pointers back to the pointers in
    // argv.
    m->new_argv.emplace_back(m->argv[0]);
    for (int i = 1; i < m->argc; ++i) {
        char const* argfile = nullptr;
        if ((strlen(m->argv[i]) > 1) && (m->argv[i][0] == '@')) {
            argfile = 1 + m->argv[i];
            if (strcmp(argfile, "-") != 0) {
                if (!QUtil::file_can_be_opened(argfile)) {
                    // The file's not there; treating as regular option
                    argfile = nullptr;
                }
            }
        }
        if (argfile) {
            readArgsFromFile(1 + m->argv[i]);
        } else {
            m->new_argv.emplace_back(m->argv[i]);
        }
    }
    m->argv_ph.reserve(1 + m->new_argv.size());
    for (auto const& a: m->new_argv) {
        m->argv_ph.push_back(a.data());
    }
    m->argc = QIntC::to_int(m->new_argv.size());
    m->argv_ph.push_back(nullptr);
    m->argv = m->argv_ph.data();
}

void
QPDFArgParser::usage(std::string const& message)
{
    throw QPDFUsage(message);
}

void
QPDFArgParser::readArgsFromFile(std::string const& filename)
{
    std::list<std::string> lines;
    if (filename == "-") {
        QTC::TC("libtests", "QPDFArgParser read args from stdin");
        lines = QUtil::read_lines_from_file(std::cin);
    } else {
        QTC::TC("libtests", "QPDFArgParser read args from file");
        lines = QUtil::read_lines_from_file(filename.c_str());
    }
    for (auto const& line: lines) {
        m->new_argv.emplace_back(line);
    }
}

void
QPDFArgParser::parseArgs()
{
    selectMainOptionTable();
    handleArgFileArguments();
    for (m->cur_arg = 1; m->cur_arg < m->argc; ++m->cur_arg) {
        bool help_option = false;
        bool end_option = false;
        auto oep = m->option_table->end();
        char const* arg = m->argv[m->cur_arg];
        std::string parameter;
        bool have_parameter = false;
        std::string o_arg(arg);
        std::string arg_s(arg);
        if (strcmp(arg, "--") == 0) {
            // Special case for -- option, which is used to break out of subparsers.
            oep = m->option_table->find("--");
            end_option = true;
            util::internal_error_if(
                oep == m->option_table->end(), "QPDFArgParser: -- handler not registered");
        } else if (arg[0] == '-' && strcmp(arg, "-") != 0) {
            ++arg;
            if (arg[0] == '-') {
                // Be lax about -arg vs --arg
                ++arg;
            } else {
                QTC::TC("libtests", "QPDFArgParser single dash");
            }

            // Prevent --=something from being treated as an empty arg by searching for = from after
            // the first character. We do this since the empty string in the option table is for
            // positional arguments. Besides, it doesn't make sense to have an empty option.
            arg_s = arg;
            size_t equal_pos = std::string::npos;
            if (!arg_s.empty()) {
                equal_pos = arg_s.find('=', 1);
            }
            if (equal_pos != std::string::npos) {
                have_parameter = true;
                parameter = arg_s.substr(equal_pos + 1);
                arg_s = arg_s.substr(0, equal_pos);
            }

            if ((m->argc == 2) && (m->cur_arg == 1) && m->help_option_table.contains(arg_s)) {
                // Handle help option, which is only valid as the sole option.
                QTC::TC("libtests", "QPDFArgParser help option");
                oep = m->help_option_table.find(arg_s);
                help_option = true;
            }

            if (!(help_option || arg_s.empty() || (arg_s.at(0) == '-'))) {
                oep = m->option_table->find(arg_s);
            }
        } else {
            // The empty string maps to the positional argument handler.
            QTC::TC("libtests", "QPDFArgParser positional");
            oep = m->option_table->find("");
            parameter = arg;
        }

        if (oep == m->option_table->end()) {
            QTC::TC("libtests", "QPDFArgParser unrecognized");
            std::string message = "unrecognized argument " + o_arg;
            if (m->option_table != &m->main_option_table) {
                message += " (" + m->option_table_name + " options must be terminated with --)";
            }
            usage(message);
        }

        OptionEntry& oe = oep->second;
        if ((oe.parameter_needed && !have_parameter) ||
            (!oe.choices.empty() && have_parameter && !oe.choices.contains(parameter))) {
            std::string message = "--" + arg_s + " must be given as --" + arg_s + "=";
            if (oe.invalid_choice_handler) {
                oe.invalid_choice_handler(parameter);
                // Method should call usage() or exit. Just in case it doesn't...
                message += "option";
            } else if (!oe.choices.empty()) {
                QTC::TC("libtests", "QPDFArgParser required choices");
                message += "{";
                bool first = true;
                for (auto const& choice: oe.choices) {
                    if (first) {
                        first = false;
                    } else {
                        message += ",";
                    }
                    message += choice;
                }
                message += "}";
            } else if (!oe.parameter_name.empty()) {
                QTC::TC("libtests", "QPDFArgParser required parameter");
                message += oe.parameter_name;
            } else {
                // should not be possible
                message += "option";
            }
            usage(message);
        }

        if (oe.bare_arg_handler) {
            if (have_parameter) {
                usage(
                    "--"s + arg_s + " does not take a parameter, but \"" + parameter +
                    "\" was given");
            }
            oe.bare_arg_handler();
        } else if (oe.param_arg_handler) {
            oe.param_arg_handler(parameter);
        }
        if (help_option) {
            exit(0);
        }
        if (end_option) {
            selectMainOptionTable();
        }
    }
    doFinalChecks();
}

std::string
QPDFArgParser::getProgname()
{
    return m->whoami;
}

void
QPDFArgParser::doFinalChecks()
{
    if (m->option_table != &(m->main_option_table)) {
        QTC::TC("libtests", "QPDFArgParser missing --");
        usage("missing -- at end of " + m->option_table_name + " options");
    }
    if (m->final_check_handler != nullptr) {
        m->final_check_handler();
    }
}

void
QPDFArgParser::addHelpFooter(std::string const& text)
{
    m->help_footer = "\n" + text;
}

void
QPDFArgParser::addHelpTopic(
    std::string const& topic, std::string const& short_text, std::string const& long_text)
{
    if (topic == "all") {
        throw std::logic_error("QPDFArgParser: can't register reserved help topic " + topic);
    }
    if (topic.empty() || topic.at(0) == '-') {
        throw std::logic_error("QPDFArgParser: help topics must not start with -");
    }
    if (m->help_topics.contains(topic)) {
        throw std::logic_error("QPDFArgParser: topic " + topic + " has already been added");
    }

    m->help_topics[topic] = HelpTopic(short_text, long_text);
    m->help_option_table["help"].choices.insert(topic);
}

void
QPDFArgParser::addOptionHelp(
    std::string const& option_name,
    std::string const& topic,
    std::string const& short_text,
    std::string const& long_text)
{
    if (!(option_name.length() > 2 && option_name.starts_with("--"))) {
        throw std::logic_error("QPDFArgParser: options for help must start with --");
    }
    if (m->option_help.contains(option_name)) {
        throw std::logic_error("QPDFArgParser: option " + option_name + " already has help");
    }
    auto ht = m->help_topics.find(topic);
    if (ht == m->help_topics.end()) {
        throw std::logic_error(
            "QPDFArgParser: unable to add option " + option_name + " to unknown help topic " +
            topic);
    }
    m->option_help[option_name] = HelpTopic(short_text, long_text);
    ht->second.options.insert(option_name);
    m->help_option_table["help"].choices.insert(option_name);
}

void
QPDFArgParser::getTopHelp(std::ostringstream& msg)
{
    msg << "Run \"" << m->whoami << " --help=topic\" for help on a topic." << '\n'
        << "Run \"" << m->whoami << " --help=--option\" for help on an option." << '\n'
        << "Run \"" << m->whoami << " --help=all\" to see all available help." << '\n'
        << '\n'
        << "Topics:" << '\n';
    for (auto const& i: m->help_topics) {
        msg << "  " << i.first << ": " << i.second.short_text << '\n';
    }
}

void
QPDFArgParser::getAllHelp(std::ostringstream& msg)
{
    getTopHelp(msg);
    auto show = [this, &msg](std::map<std::string, HelpTopic>& topics) {
        for (auto const& i: topics) {
            auto const& topic = i.first;
            msg << '\n' << "== " << topic << " (" << i.second.short_text << ") ==" << '\n' << '\n';
            getTopicHelp(topic, i.second, msg);
        }
    };
    show(m->help_topics);
    show(m->option_help);
    msg << '\n' << "====" << '\n';
}

void
QPDFArgParser::getTopicHelp(std::string const& name, HelpTopic const& ht, std::ostringstream& msg)
{
    if (ht.long_text.empty()) {
        msg << ht.short_text << '\n';
    } else {
        msg << ht.long_text;
    }
    if (!ht.options.empty()) {
        msg << '\n' << "Related options:" << '\n';
        for (auto const& i: ht.options) {
            msg << "  " << i << ": " << m->option_help[i].short_text << '\n';
        }
    }
}

std::string
QPDFArgParser::getHelp(std::string const& arg)
{
    std::ostringstream msg;
    if (arg.empty()) {
        getTopHelp(msg);
    } else {
        if (arg == "all") {
            getAllHelp(msg);
        } else if (m->option_help.contains(arg)) {
            getTopicHelp(arg, m->option_help[arg], msg);
        } else if (m->help_topics.contains(arg)) {
            getTopicHelp(arg, m->help_topics[arg], msg);
        } else {
            // should not be possible
            getTopHelp(msg);
        }
    }
    msg << m->help_footer;
    return msg.str();
}

#include "./cli.h"

#include <passwordfile/io/cryptoexception.h>
#include <passwordfile/io/entry.h>
#include <passwordfile/io/field.h>
#include <passwordfile/io/parsingexception.h>
#include <passwordfile/io/passwordfile.h>
#include <passwordfile/util/openssl.h>

#include <c++utilities/application/commandlineutils.h>
#include <c++utilities/conversion/stringconversion.h>
#include <c++utilities/io/ansiescapecodes.h>

#if defined(PLATFORM_UNIX)
#include <unistd.h>
#endif

#include <algorithm>
#include <cstdlib>
#include <functional>
#include <stdexcept>

using namespace std;
using namespace std::placeholders;
using namespace CppUtilities;
using namespace CppUtilities::EscapeCodes;
using namespace Io;

namespace Cli {

InputMuter::InputMuter()
{
#if defined(PLATFORM_UNIX)
    tcgetattr(STDIN_FILENO, &m_attr);
    termios newAttr = m_attr;
    newAttr.c_lflag &= ~static_cast<unsigned int>(ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newAttr);
#elif defined(PLATFORM_WINDOWS)
    m_cinHandle = GetStdHandle(STD_INPUT_HANDLE);
    m_mode = 0;
    GetConsoleMode(m_cinHandle, &m_mode);
    SetConsoleMode(m_cinHandle, m_mode & static_cast<DWORD>(~ENABLE_ECHO_INPUT));
#endif
}

InputMuter::~InputMuter()
{
#if defined(PLATFORM_UNIX)
    tcsetattr(STDIN_FILENO, TCSANOW, &m_attr);
#elif defined(PLATFORM_WINDOWS)
    SetConsoleMode(m_cinHandle, m_mode);
#endif
}

void clearConsole()
{
#if defined(PLATFORM_WINDOWS)
    auto hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
    auto csbi = CONSOLE_SCREEN_BUFFER_INFO();
    auto homeCoords = COORD{ 0, 0 };
    if (hStdOut == INVALID_HANDLE_VALUE) {
        return;
    }
    // get the number of cells in the current buffer
    if (!GetConsoleScreenBufferInfo(hStdOut, &csbi)) {
        return;
    }
    auto count = DWORD();
    auto cellCount = DWORD(csbi.dwSize.X * csbi.dwSize.Y);
    // fill the entire buffer with spaces
    if (!FillConsoleOutputCharacter(hStdOut, static_cast<TCHAR>(' '), cellCount, homeCoords, &count)) {
        return;
    }
    // fill the entire buffer with the current colors and attributes
    if (!FillConsoleOutputAttribute(hStdOut, csbi.wAttributes, cellCount, homeCoords, &count)) {
        return;
    }
    // move the cursor home
    SetConsoleCursorPosition(hStdOut, homeCoords);
#else
    EscapeCodes::setCursor(cout);
    EscapeCodes::eraseDisplay(cout);
#endif
}

InteractiveCli::InteractiveCli()
    : m_o(cout)
    , m_i(cin)
    , m_currentEntry(nullptr)
    , m_modified(false)
    , m_quit(false)
{
    Util::OpenSsl::init();
    CMD_UTILS_START_CONSOLE;
}

InteractiveCli::~InteractiveCli()
{
    Util::OpenSsl::clean();
}

int InteractiveCli::run(string_view file)
{
    if (!file.empty()) {
        openFile(file, PasswordFileOpenFlags::Default);
    }
    auto input = std::string();
    while (!m_quit) {
        std::getline(m_i, input);
        if (!input.empty()) {
            processCommand(input);
        }
    }
    return EXIT_SUCCESS;
}

void InteractiveCli::processCommand(const std::string &cmd)
{
#define CMD(value) !paramMissing &&cmd == value
#define CMD2(value1, value2) !paramMissing && (cmd == value1 || cmd == value2)
#define CMD_P(value) !paramMissing &&checkCommand(cmd, value, param, paramMissing)
#define CMD2_P(value1, value2) !paramMissing && (checkCommand(cmd, value1, param, paramMissing) || checkCommand(cmd, value2, param, paramMissing))

    auto param = std::string();
    auto paramMissing = false;
    if (CMD2("quit", "q")) {
        quit();
    } else if (CMD("q!")) {
        m_quit = true;
    } else if (CMD("wq")) {
        saveFile();
        m_quit = true;
    } else if (CMD2("clear", "c")) {
        clearConsole();
    } else if (CMD2_P("openreadonly", "or")) {
        openFile(param, PasswordFileOpenFlags::ReadOnly);
    } else if (CMD2_P("open", "o")) {
        openFile(param, PasswordFileOpenFlags::Default);
    } else if (CMD2("close", "c")) {
        closeFile();
    } else if (CMD2("save", "w")) {
        saveFile();
    } else if (CMD2_P("create", "cr")) {
        createFile(param);
    } else if (CMD("chpassphrase")) {
        changePassphrase();
    } else if (CMD("rmpassphrase")) {
        removePassphrase();
    } else if (CMD("pwd")) {
        pwd();
    } else if (CMD_P("cd")) {
        cd(param);
    } else if (CMD2("ls", "l")) {
        ls();
    } else if (CMD2("tree", "t")) {
        tree();
    } else if (CMD2_P("mknode", "mkn")) {
        makeEntry(EntryType::Node, param);
    } else if (CMD2_P("mkaccount", "mka")) {
        makeEntry(EntryType::Account, param);
    } else if (CMD2("rmentry", "rme")) {
        removeEntry(".");
    } else if (CMD2_P("rmentry", "rme")) {
        removeEntry(param);
    } else if (CMD2_P("rnentry", "rne")) {
        renameEntry(param);
    } else if (CMD2_P("mventry", "me")) {
        moveEntry(param);
    } else if (CMD2_P("readfield", "rf")) {
        readField(param);
    } else if (CMD2_P("setfield", "sf")) {
        setField(false, param);
    } else if (CMD2_P("setfieldpw", "sp")) {
        setField(true, param);
    } else if (CMD2_P("rmfield", "rf")) {
        removeField(param);
    } else if (CMD2("help", "?")) {
        printHelp();
    } else if (paramMissing) {
        m_o << "parameter is missing" << endl;
    } else {
        m_o << "command is unknown" << endl;
    }
}

Entry *InteractiveCli::resolvePath(const std::string &path)
{
    auto parts = splitString<std::vector<std::string>>(path, "/", EmptyPartsTreat::Merge);
    bool fromRoot = path.at(0) == '/';
    if (fromRoot && parts.empty()) {
        return m_file.rootEntry();
    } else {
        Entry *entry = fromRoot ? m_file.rootEntry() : m_currentEntry;
        for (const string &part : parts) {
            if (part == "..") {
                if (entry->parent()) {
                    entry = entry->parent();
                } else {
                    m_o << "can not resolve path; entry \"" << entry->label() << "\" is root" << endl;
                    return nullptr;
                }
            } else if (part != ".") {
                switch (entry->type()) {
                case EntryType::Account:
                    m_o << "can not resolve path; entry \"" << entry->label() << "\" is not a node entry" << endl;
                    return nullptr;
                case EntryType::Node:
                    for (Entry *child : (static_cast<NodeEntry *>(entry)->children())) {
                        if (child->label() == part) {
                            entry = child;
                            goto next;
                        }
                    }
                    m_o << "can not resolve path; entry \"" << entry->label() << "\" has no child \"" << part << "\"" << endl;
                    return nullptr;
                }
            }
        next:;
        }
        return entry;
    }
}

bool InteractiveCli::checkCommand(const std::string &str, const char *phrase, std::string &param, bool &paramMissing)
{
    for (auto i = str.cbegin(), end = str.cend(); i != end; ++i, ++phrase) {
        if (*phrase == 0) {
            if (*i == ' ') {
                if (++i != end) {
                    param.assign(i, end);
                    return true;
                } else {
                    paramMissing = true;
                }
            }
            return false;
        } else if (*i != *phrase) {
            return false;
        }
    }
    paramMissing = *phrase == 0;
    return false;
}

void InteractiveCli::openFile(std::string_view file, PasswordFileOpenFlags openFlags)
{
    if (m_file.isOpen()) {
        m_o << "file \"" << m_file.path() << "\" currently open; close first" << endl;
        return;
    }
    m_file.setPath(std::string(file));
    for (;;) {
        try {
            try {
                m_file.open(openFlags);
                if (m_file.isEncryptionUsed()) {
                    m_file.setPassword(askForPassphrase());
                }
                m_file.load();
                m_currentEntry = m_file.rootEntry();
                m_o << "file \"" << file << "\" opened" << endl;
            } catch (const ParsingException &) {
                m_o << "error occurred when parsing file \"" << file << "\"" << endl;
                throw;
            } catch (const CryptoException &) {
                m_o << "error occurred when decrypting file \"" << file << "\"" << endl;
                throw;
            } catch (const std::ios_base::failure &) {
                m_o << "IO error occurred when opening file \"" << file << "\"" << endl;
                throw;
            }
        } catch (const std::exception &e) {
            if (*e.what() != 0) {
                m_o << e.what() << endl;
            }
            if (confirmPrompt("Retry opening?", Response::Yes)) {
                m_file.close();
                continue;
            }
            m_file.clear();
            m_currentEntry = nullptr;
        }
        m_modified = false;
        return;
    }
}

void InteractiveCli::closeFile()
{
    if (!m_file.isOpen()) {
        m_o << "no file was opened" << endl;
        return;
    }
    m_file.clear();
    m_currentEntry = nullptr;
    m_o << "file closed" << endl;
}

void InteractiveCli::saveFile()
{
    if (!m_file.isOpen()) {
        m_o << "nothing to save; no file opened or created" << endl;
        return;
    }
    try {
        try {
            auto flags = PasswordFileSaveFlags::Compression | PasswordFileSaveFlags::PasswordHashing;
            if (!m_file.password().empty()) {
                flags |= PasswordFileSaveFlags::Encryption;
            }
            m_file.save(flags);
            m_o << "file \"" << m_file.path() << "\" saved" << endl;
        } catch (const ParsingException &) {
            m_o << "error occurred when parsing file \"" << m_file.path() << "\"" << endl;
            throw;
        } catch (const CryptoException &) {
            m_o << "error occurred when encrypting file \"" << m_file.path() << "\"" << endl;
            throw;
        } catch (const std::ios_base::failure &) {
            m_o << "IO error occurred when saving file \"" << m_file.path() << "\"" << endl;
            throw;
        }
    } catch (const exception &e) {
        if (*e.what() != 0) {
            m_o << e.what() << endl;
        }
        m_o << "file has been closed; try reopening the file" << endl;
        m_file.clear();
    }
    m_modified = false;
}

void InteractiveCli::createFile(const std::string &file)
{
    if (m_file.isOpen()) {
        m_o << "file \"" << m_file.path() << "\" currently open; close first" << endl;
        return;
    }

    m_file.setPath(file);
    try {
        try {
            m_file.create();
            m_file.generateRootEntry();
            m_currentEntry = m_file.rootEntry();
            m_o << "file \"" << file << "\" created and opened" << endl;
        } catch (const std::ios_base::failure &) {
            m_o << "IO error occurred when creating file \"" << file << "\"" << endl;
            throw;
        }
    } catch (const exception &e) {
        if (*e.what() != 0) {
            m_o << e.what() << endl;
        }
        m_file.clear();
        m_currentEntry = nullptr;
    }
    m_modified = false;
}

void InteractiveCli::changePassphrase()
{
    if (m_file.isOpen()) {
        m_o << "can not set passphrase; no file opened or created" << endl;
        return;
    }
    try {
        m_file.setPassword(askForPassphrase(true));
        m_modified = true;
        m_o << "passphrase changed; use save to apply" << endl;
    } catch (const runtime_error &) {
        m_o << "passphrase has not changed" << endl;
    }
}

void InteractiveCli::removePassphrase()
{
    if (!m_file.isOpen()) {
        m_o << "nothing to remove; no file opened or created" << endl;
        return;
    }
    if (!m_file.password().empty()) {
        m_file.clearPassword();
        m_o << "passphrase removed; use save to apply" << endl;
        m_modified = true;
    } else {
        m_o << "nothing to remove; no passphrase present on current file" << endl;
    }
}

void InteractiveCli::pwd()
{
    if (!m_file.isOpen()) {
        m_o << "no file open" << endl;
        return;
    }
    auto path = m_currentEntry->path();
    m_o << path.front() << ": /";
    path.pop_front();
    m_o << joinStrings(path, "/") << endl;
}

void InteractiveCli::cd(const std::string &path)
{
    if (!m_file.isOpen()) {
        m_o << "can not change directory; no file open" << endl;
        return;
    }
    if (Entry *entry = resolvePath(path)) {
        m_currentEntry = entry;
        m_o << "changed to \"" << entry->label() << "\"" << endl;
    }
}

void InteractiveCli::ls()
{
    if (!m_file.isOpen()) {
        m_o << "can not list any entries; no file open" << endl;
        return;
    }
    switch (m_currentEntry->type()) {
    case EntryType::Account: {
        m_o << "fields:";
        for (const Field &field : static_cast<AccountEntry *>(m_currentEntry)->fields()) {
            m_o << "\n" << field.name();
        }
        break;
    }
    case EntryType::Node: {
        m_o << "entries:";
        for (const Entry *entry : static_cast<NodeEntry *>(m_currentEntry)->children()) {
            m_o << "\n" << entry->label();
        }
        break;
    }
    }
    m_o << endl;
}

void InteractiveCli::tree()
{
    if (!m_file.isOpen()) {
        m_o << "can not print tree; no file open" << endl;
        return;
    }
    function<void(const Entry *entry, unsigned char level)> printEntries;
    printEntries = [&printEntries, this](const Entry *entry, unsigned char level) {
        for (unsigned char i = 0; i < level; ++i) {
            m_o << " ";
        }
        m_o << entry->label() << endl;
        if (entry->type() == EntryType::Node) {
            for (const Entry *child : (static_cast<const NodeEntry *>(entry)->children())) {
                printEntries(child, level + 2);
            }
        }
    };
    printEntries(m_currentEntry, 0);
}

void InteractiveCli::makeEntry(EntryType entryType, const std::string &label)
{
    if (!m_file.isOpen()) {
        m_o << "can not make entry; no file open" << endl;
        return;
    }
    switch (m_currentEntry->type()) {
    case EntryType::Node:
        switch (entryType) {
        case EntryType::Node:
            m_o << "node entry \"" << (new NodeEntry(label, static_cast<NodeEntry *>(m_currentEntry)))->label() << "\" created" << endl;
            break;
        case EntryType::Account:
            m_o << "account entry \"" << (new AccountEntry(label, static_cast<NodeEntry *>(m_currentEntry)))->label() << "\" created" << endl;
            break;
        }
        m_modified = true;
        break;
    case EntryType::Account:
        m_o << "can not make entry; current entry is no node entry" << endl;
    }
}

void InteractiveCli::removeEntry(const std::string &path)
{
    if (!m_file.isOpen()) {
        m_o << "can not remove entry; no file open" << endl;
        return;
    }
    if (Entry *entry = resolvePath(path)) {
        if (entry == m_file.rootEntry()) {
            m_o << "can not remove root entry" << endl;
        } else {
            if (entry == m_currentEntry) {
                m_currentEntry = entry->parent();
            }
            m_o << "removed entry \"" << entry->label() << "\"" << endl;
            delete entry;
            m_modified = true;
        }
    }
}

void InteractiveCli::renameEntry(const std::string &path)
{
    if (!m_file.isOpen()) {
        m_o << "can not rename entry; no file open" << endl;
        return;
    }
    if (Entry *entry = resolvePath(path)) {
        auto label = std::string();
        m_o << "enter new name: " << endl;
        std::getline(m_i, label);
        if (label.empty()) {
            m_o << "can not rename; new name is empty" << endl;
        } else {
            entry->setLabel(label);
            m_o << "entry renamed to \"" << entry->label() << "\"" << endl;
            m_modified = true;
        }
    }
}

void InteractiveCli::moveEntry(const std::string &path)
{
    if (!m_file.isOpen()) {
        m_o << "can not rename entry; no file open" << endl;
        return;
    }
    if (Entry *entry = resolvePath(path)) {
        auto newParentPath = std::string();
        m_o << "enter path of new parent: " << endl;
        std::getline(m_i, newParentPath);
        if (newParentPath.empty()) {
            m_o << "can not move; path of new parent is empty" << endl;
        } else {
            if (Entry *newParent = resolvePath(newParentPath)) {
                switch (newParent->type()) {
                case EntryType::Account:
                    m_o << "can not move; new parent must be a node entry" << endl;
                    break;
                case EntryType::Node:
                    if (entry->parent() == entry) {
                        m_o << "element not moved; parent doesn't change" << endl;
                    } else if (entry->type() == EntryType::Node && newParent->isIndirectChildOf(static_cast<NodeEntry *>(entry))) {
                        m_o << "can not move; new parent mustn't be child of the entry to move" << endl;
                    } else {
                        entry->setParent(static_cast<NodeEntry *>(newParent));
                        m_o << "entry moved to \"" << newParent->label() << "\"" << endl;
                        m_modified = true;
                    }
                }
            }
        }
    }
}

void InteractiveCli::readField(const std::string &fieldName)
{
    if (!m_file.isOpen()) {
        m_o << "can not read field; no file open" << endl;
        return;
    }
    if (m_currentEntry->type() != EntryType::Account) {
        m_o << "can not read field; current entry is no account entry" << endl;
        return;
    }

    const vector<Field> &fields = static_cast<AccountEntry *>(m_currentEntry)->fields();
    bool valuesFound = false;
    for (const Field &field : fields) {
        if (field.name() == fieldName) {
            m_o << field.value() << endl;
            valuesFound = true;
        }
    }
    if (!valuesFound) {
        m_o << "field \"" << fieldName << "\" does not exist" << endl;
    }
}

void InteractiveCli::setField(bool useMuter, const std::string &fieldName)
{
    if (!m_file.isOpen()) {
        m_o << "can not set field; no file open" << endl;
        return;
    }
    if (m_currentEntry->type() != EntryType::Account) {
        m_o << "can not set field; current entry is no account entry" << endl;
        return;
    }
    auto &fields = static_cast<AccountEntry *>(m_currentEntry)->fields();
    auto valuesFound = unsigned();
    auto value = std::string();
    m_o << "enter new value: ";
    if (useMuter) {
        InputMuter m;
        std::getline(m_i, value);
        m_o << endl << "repeat: ";
        auto repeat = std::string();
        std::getline(m_i, repeat);
        if (value != repeat) {
            m_o << "values do not match; field has not been altered" << endl;
            return;
        }
    } else {
        std::getline(m_i, value);
    }
    for (Field &field : fields) {
        if (field.name() == fieldName) {
            ++valuesFound;
            if (valuesFound == 1) {
                field.setValue(value);
            } else {
                m_o << "enter new value for " << valuesFound << ". field (with the specified field): ";
                value.clear();
                if (useMuter) {
                    InputMuter m;
                    getline(m_i, value);
                    m_o << endl << "repeat: ";
                    string repeat;
                    getline(m_i, repeat);
                    if (value == repeat) {
                        field.setValue(value);
                    } else {
                        m_o << "values do not match; field has not been altered" << endl;
                    }
                } else {
                    getline(m_i, value);
                    field.setValue(value);
                }
            }
            field.setType(useMuter ? FieldType::Password : FieldType::Normal);
        }
    }
    switch (valuesFound) {
    case 0:
        fields.emplace_back(static_cast<AccountEntry *>(m_currentEntry), fieldName, value);
        if (useMuter) {
            fields.back().setType(FieldType::Password);
        }
        m_o << "new field with value inserted" << endl;
        break;
    case 1:
        m_o << "value updated" << endl;
        m_modified = true;
        break;
    default:
        m_o << valuesFound << " values updated" << endl;
        m_modified = true;
    }
}

void InteractiveCli::removeField(const std::string &fieldName)
{
    if (!m_file.isOpen()) {
        m_o << "can not remove field; no file open" << endl;
        return;
    }
    if (m_currentEntry->type() != EntryType::Account) {
        m_o << "can not remove field; current entry is no account entry" << endl;
        return;
    }
    auto &fields = static_cast<AccountEntry *>(m_currentEntry)->fields();
    auto valuesFound = unsigned();
    for (const Field &field : fields) {
        if (field.name() == fieldName) {
            ++valuesFound;
        }
    }
    switch (valuesFound) {
    case 0:
        m_o << "can not remove field; specified field \"" << fieldName << "\" not found" << endl;
        break;
    case 1:
        fields.erase(remove_if(fields.begin(), fields.end(), [&fieldName](Field &field) { return field.name() == fieldName; }));
        m_o << "field removed" << endl;
        m_modified = true;
        break;
    default:
        valuesFound = 0;
        fields.erase(remove_if(fields.begin(), fields.end(), [this, &fieldName, &valuesFound](Field &field) {
            if (field.name() == fieldName) {
                m_o << "remove " << ++valuesFound << ". occurrence? [y]=yes, different key=no " << endl;
                string res;
                getline(m_i, res);
                return !(res == "y" || res == "yes");
            } else {
                return false;
            }
        }));
        m_o << valuesFound << " fields removed" << endl;
        m_modified = true;
    }
}

void InteractiveCli::printHelp()
{
    m_o << TextAttribute::Bold << "Command:       Description: \n"
        << TextAttribute::Reset
        << "quit,q         quits the application\n"
           "q!             forces the application to quit\n"
           "wq             saves the current file and quits the application\n"
           "clear,c        clears the console\n"
           "\n"
           "create,cr      creates a new file at the specified path\n"
           "openreadonly   opens the specified file (read-only)\n"
           "open,o         opens the specified file\n"
           "close,cl       closes the currently opened file\n"
           "\n"
           "save,w         saves the currently opened file\n"
           "chpassphrase   changes the passphrase\n"
           "rmpassphrase   removes the passphrase\n"
           "\n"
           "pwd            prints the path of the current entry\n"
           "cd             changes the current entry\n"
           "ls             lists the entries/fields of the current entry\n"
           "tree,t         shows all child entries of the current entry\n"
           "mknode,mkn     creates a node entry with the specified label in the current entry\n"
           "mkaccount,mka  creates an account entry with the specified label in the current entry\n"
           "rmentry,rme    removes the entry specified by its path\n"
           "rnentry,rne    renames the entry specified by its path\n"
           "mventry,me     moves the entry specified by its path\n"
           "\n"
           "readfield,rf   reads the specified field of the current account\n"
           "setfield,sf    sets the specified field of the current account\n"
           "setfieldpw,sp  sets the specified password field of the current account\n"
           "rmfield,rf     removes the specified field of the current account\n"
        << endl;
}

void InteractiveCli::quit()
{
    if (m_file.isOpen() && m_modified) {
        m_o << "file modified; use q! or wq" << endl;
    } else {
        m_quit = true;
    }
}

std::string InteractiveCli::askForPassphrase(bool confirm)
{
    if (confirm) {
        m_o << "enter new passphrase: ";
    } else {
        m_o << "enter passphrase: ";
    }
    m_o.flush();
    auto input1 = std::string();
    {
        auto m = InputMuter();
        std::getline(m_i, input1);
    }
    m_o << endl;
    if (input1.empty()) {
        m_o << "you did not enter a passphrase" << endl;
        return input1;
    }
    if (confirm) {
        m_o << "confirm new passphrase: ";
        m_o.flush();
        auto input2 = std::string();
        {
            auto m = InputMuter();
            std::getline(m_i, input2);
        }
        m_o << endl;
        if (input1 != input2) {
            m_o << "phrases do not match" << endl;
            throw std::runtime_error("confirmation failed");
        }
    }
    return input1;
}
} // namespace Cli

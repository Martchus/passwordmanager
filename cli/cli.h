#ifndef CLI_CLI_H
#define CLI_CLI_H

#include <passwordfile/io/passwordfile.h>

#if defined(PLATFORM_UNIX)
#include <termios.h>
#elif defined(PLATFORM_WINDOWS)
#include <windows.h>
#endif

#include <istream>
#include <ostream>
#include <string>
#include <vector>

namespace ApplicationUtilities {
typedef std::vector<std::string> StringVector;
}

namespace Io {
class Entry;
enum class EntryType : int;
}

namespace Cli {

class InputMuter {
public:
    InputMuter();
    ~InputMuter();

private:
#if defined(PLATFORM_UNIX)
    termios m_attr;
#elif defined(PLATFORM_WINDOWS)
    HANDLE m_cinHandle;
    DWORD m_mode;
#endif
};

void clearConsole();

class InteractiveCli {
public:
    InteractiveCli();
    void run(const std::string &file = std::string());
    void openFile(const std::string &file, bool readOnly);
    void closeFile();
    void saveFile();
    void createFile(const std::string &file);
    void changePassphrase();
    void removePassphrase();
    void pwd();
    void cd(const std::string &path);
    void ls();
    void tree();
    void makeEntry(Io::EntryType entryType, const std::string &label);
    void removeEntry(const std::string &path);
    void renameEntry(const std::string &path);
    void moveEntry(const std::string &path);
    void readField(const std::string &fieldName);
    void setField(bool useMuter, const std::string &fieldName);
    void removeField(const std::string &fieldName);
    void printHelp();
    void quit();

private:
    void processCommand(const std::string &cmd);
    Io::Entry *resolvePath(const std::string &path);
    static bool checkCommand(const std::string &str, const char *phrase, std::string &param, bool &paramMissing);
    std::string askForPassphrase(bool confirm = false);

    std::ostream &m_o;
    std::istream &m_i;
    Io::PasswordFile m_file;
    Io::Entry *m_currentEntry;
    bool m_modified;
    bool m_quit;
};
}

#endif // CLI_CLI_H

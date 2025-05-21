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
#include <string_view>

namespace Io {
class Entry;
enum class EntryType : int;
} // namespace Io

namespace Cli {

class InputMuter {
public:
    explicit InputMuter();
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
    explicit InteractiveCli();
    ~InteractiveCli();
    int run(std::string_view file);
    void openFile(std::string_view file, Io::PasswordFileOpenFlags openFlags);
    void closeFile();
    void saveFile();
    void createFile(const std::string &file);
    void changePassphrase();
    void removePassphrase();
    void pwd();
    void cd(const std::string &path);
    void ls();
    void tree();
    void search(const std::string &term);
    void duplicates();
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
} // namespace Cli

#endif // CLI_CLI_H

#include "./cli/cli.h"
#ifdef PASSWORD_MANAGER_GUI_QTWIDGETS
# include "./gui/initiategui.h"
#endif
#ifdef PASSWORD_MANAGER_GUI_QTQUICK
# include "./quickgui/initiatequick.h"
#endif

#include "resources/config.h"

#include <passwordfile/util/openssl.h>

#include <c++utilities/application/argumentparser.h>
#include <c++utilities/application/failure.h>
#include <c++utilities/application/commandlineutils.h>

#if defined(PASSWORD_MANAGER_GUI_QTWIDGETS) || defined(PASSWORD_MANAGER_GUI_QTQUICK)
# include <qtutilities/resources/qtconfigarguments.h>
# include <QString>
ENABLE_QT_RESOURCES_OF_STATIC_DEPENDENCIES
#else
# include <c++utilities/application/fakeqtconfigarguments.h>
#endif

#include <iostream>

using namespace std;
using namespace ApplicationUtilities;
using namespace Util;

int main(int argc, char *argv[])
{
    // init open ssl
    OpenSsl::init();
    // setup argument parser
    SET_APPLICATION_INFO;
    ArgumentParser parser;
    // file argument
    Argument fileArg("file", 'f', "specifies the file to be opened (or created when using --modify)");
    fileArg.setValueNames({"path"});
    fileArg.setRequiredValueCount(1);
    fileArg.setCombinable(true);
    fileArg.setRequired(false);
    fileArg.setImplicit(true);
    // Qt configuration arguments
    QT_CONFIG_ARGUMENTS qtConfigArgs;
    qtConfigArgs.qtWidgetsGuiArg().addSubArgument(&fileArg);
    // cli argument
    Argument cliArg("interactive-cli", 'i', "starts the interactive command line interface");
    cliArg.setSubArguments({&fileArg});
    // help argument
    HelpArgument helpArg(parser);
    parser.setMainArguments({&qtConfigArgs.qtWidgetsGuiArg(), &qtConfigArgs.qtQuickGuiArg(), &cliArg, &helpArg});
    // holds the application's return code
    int res = 0;
    // parse the specified arguments
    try {
        parser.parseArgs(argc, argv);
        if(cliArg.isPresent()) {
            Cli::InteractiveCli cli;
            if(fileArg.isPresent()) {
                cli.run(fileArg.values().front());
            } else {
                cli.run();
            }
        } else if(qtConfigArgs.areQtGuiArgsPresent()) {
            // run Qt gui if no arguments, --qt-gui or --qt-quick-gui specified, a file might be specified
#if defined(PASSWORD_MANAGER_GUI_QTWIDGETS) || defined(PASSWORD_MANAGER_GUI_QTQUICK)
            QString file;
            if(fileArg.isPresent()) {
                file = QString::fromLocal8Bit(fileArg.values().front());
            }
#endif
            if(qtConfigArgs.qtWidgetsGuiArg().isPresent()) {
#ifdef PASSWORD_MANAGER_GUI_QTWIDGETS
                res = QtGui::runWidgetsGui(argc, argv, qtConfigArgs, file);
#else
                CMD_UTILS_START_CONSOLE;
                cout << "The application has not been built with Qt widgets support." << endl;
#endif
            } else if(qtConfigArgs.qtQuickGuiArg().isPresent()) {
#ifdef PASSWORD_MANAGER_GUI_QTQUICK
                res = QtGui::runQuickGui(argc, argv, qtConfigArgs);
#else
                CMD_UTILS_START_CONSOLE;
                cout << "The application has not been built with Qt quick support." << endl;
#endif
            } else {
#if defined(PASSWORD_MANAGER_GUI_QTQUICK)
                res = QtGui::runQuickGui(argc, argv, qtConfigArgs);
#elif defined(PASSWORD_MANAGER_GUI_QTWIDGETS)
                res = QtGui::runWidgetsGui(argc, argv, qtConfigArgs, file);
#else
                CMD_UTILS_START_CONSOLE;
                cout << "See --help for usage." << endl;
#endif
            }
        }
    } catch(const Failure &ex) {
        CMD_UTILS_START_CONSOLE;
        cout << "Unable to parse arguments. " << ex.what() << "\nSee --help for available commands." << endl;
    }
    // clean open ssl
    OpenSsl::clean();
    return res;
}

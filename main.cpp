#include "./cli/cli.h"
#ifdef GUI_QTWIDGETS
# include "./gui/initiategui.h"
#endif
#ifdef GUI_QTQUICK
# include "./quickgui/initiatequick.h"
#endif

// include configuration from separate header file when building with CMake
#ifndef APP_METADATA_AVAIL
#include "resources/config.h"
#endif

#include <passwordfile/util/openssl.h>

#include <c++utilities/application/argumentparser.h>
#include <c++utilities/application/failure.h>
#include <c++utilities/application/commandlineutils.h>

#if defined(GUI_QTWIDGETS) || defined(GUI_QTQUICK)
# include <qtutilities/resources/qtconfigarguments.h>
#else
# include <c++utilities/application/fakeqtconfigarguments.h>
#endif

#include <QString>

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
    parser.setIgnoreUnknownArguments(true);
    // Qt configuration arguments
    QT_CONFIG_ARGUMENTS qtConfigArgs;
    // file argument
    Argument fileArg("file", "f", "specifies the file to be opened (or created when using --modify)");
    fileArg.setValueNames({"path"});
    fileArg.setRequiredValueCount(1);
    fileArg.setCombinable(true);
    fileArg.setRequired(false);
    fileArg.setImplicit(true);
    // cli argument
    Argument cliArg("interactive-cli", "i", "starts the interactive command line interface");
    // help argument
    HelpArgument helpArg(parser);
    parser.setMainArguments({&fileArg, &helpArg, &(qtConfigArgs.qtWidgetsGuiArg()), &(qtConfigArgs.qtQuickGuiArg()), &cliArg});
    // holds the application's return code
    int res = 0;
    // parse the specified arguments
    try {
        parser.parseArgs(argc, argv);
        if(cliArg.isPresent()) {
            Cli::InteractiveCli cli;
            if(fileArg.isPresent() && fileArg.valueCount() == 1) {
                cli.run(fileArg.value(0));
            } else {
                cli.run();
            }
        } else if(qtConfigArgs.areQtGuiArgsPresent()) {
            // run Qt gui if no arguments, --qt-gui or --qt-quick-gui specified, a file might be specified
            QString file;
            if(fileArg.valueCount() > 0) {
                file = QString::fromLocal8Bit(fileArg.value(0).c_str());
            }
            if(qtConfigArgs.qtWidgetsGuiArg().isPresent()) {
#ifdef GUI_QTWIDGETS
                res = QtGui::runWidgetsGui(argc, argv, qtConfigArgs, file);
#else
                CMD_UTILS_START_CONSOLE;
                cout << "The application has not been built with Qt widgets support." << endl;
#endif
            } else if(qtConfigArgs.qtQuickGuiArg().isPresent()) {
#ifdef GUI_QTQUICK
                res = QtGui::runQuickGui(argc, argv, qtConfigArgs);
#else
                CMD_UTILS_START_CONSOLE;
                cout << "The application has not been built with Qt quick support." << endl;
#endif
            } else {
#if defined(GUI_QTQUICK)
                res = QtGui::runQuickGui(argc, argv, qtConfigArgs);
#elif defined(GUI_QTWIDGETS)
                res = QtGui::runWidgetsGui(argc, argv, qtConfigArgs, file);
#else
                CMD_UTILS_START_CONSOLE;
                cout << "See --help for usage." << endl;
#endif
            }
        }
    } catch(Failure &ex) {
        CMD_UTILS_START_CONSOLE;
        cout << "Unable to parse arguments. " << ex.what() << "\nSee --help for available commands." << endl;
    }
    // clean open ssl
    OpenSsl::clean();
    return res;
}

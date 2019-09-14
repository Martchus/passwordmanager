#include "./cli/cli.h"
#ifdef PASSWORD_MANAGER_GUI_QTWIDGETS
#include "./gui/initiategui.h"
#endif
#ifdef PASSWORD_MANAGER_GUI_QTQUICK
#include "./quickgui/initiatequick.h"
#endif

#include "resources/config.h"
#include "resources/qtconfig.h"

#include <passwordfile/util/openssl.h>

#include <c++utilities/application/argumentparser.h>
#include <c++utilities/application/commandlineutils.h>
#include <c++utilities/misc/parseerror.h>

#if defined(PASSWORD_MANAGER_GUI_QTWIDGETS) || defined(PASSWORD_MANAGER_GUI_QTQUICK)
#include <QCoreApplication>
#include <QString>
#include <qtutilities/resources/qtconfigarguments.h>
ENABLE_QT_RESOURCES_OF_STATIC_DEPENDENCIES
#else
#include <c++utilities/application/fakeqtconfigarguments.h>
#endif

#include <iostream>

// force (preferably Qt Quick) GUI under Android
#ifdef Q_OS_ANDROID
#if defined(PASSWORD_MANAGER_GUI_QTWIDGETS) || defined(PASSWORD_MANAGER_GUI_QTQUICK)
#define PASSWORD_MANAGER_FORCE_GUI
#else
#error "Must build at least one kind of GUI under Android."
#endif
#endif

using namespace std;
using namespace CppUtilities;
using namespace Util;

int main(int argc, char *argv[])
{
    CMD_UTILS_CONVERT_ARGS_TO_UTF8;
    SET_APPLICATION_INFO;
    QT_CONFIG_ARGUMENTS qtConfigArgs;
    int returnCode = 0;

#ifndef PASSWORD_MANAGER_FORCE_GUI
    // setup argument parser
    ArgumentParser parser;
    // file argument
    Argument fileArg("file", 'f', "specifies the file to be opened (or created when using --modify)");
    fileArg.setValueNames({ "path" });
    fileArg.setRequiredValueCount(1);
    fileArg.setCombinable(true);
    fileArg.setRequired(false);
    fileArg.setImplicit(true);
    qtConfigArgs.qtWidgetsGuiArg().addSubArgument(&fileArg);
    qtConfigArgs.qtQuickGuiArg().addSubArgument(&fileArg);
    // cli argument
    Argument cliArg("interactive-cli", 'i', "starts the interactive command line interface");
    cliArg.setDenotesOperation(true);
    cliArg.setSubArguments({ &fileArg });
    // help argument
    HelpArgument helpArg(parser);
    parser.setMainArguments({ &qtConfigArgs.qtWidgetsGuiArg(), &qtConfigArgs.qtQuickGuiArg(), &cliArg, &helpArg });
    // parse the specified arguments
    parser.parseArgs(argc, argv);
#endif

#ifndef PASSWORD_MANAGER_FORCE_GUI
    // start either interactive CLI or GUI
    if (cliArg.isPresent()) {
        // init OpenSSL
        OpenSsl::init();

        Cli::InteractiveCli cli;
        if (fileArg.isPresent()) {
            cli.run(fileArg.firstValue());
        } else {
            cli.run();
        }

        // clean OpenSSL
        OpenSsl::clean();

    } else if (qtConfigArgs.areQtGuiArgsPresent()) {
#if defined(PASSWORD_MANAGER_GUI_QTWIDGETS) || defined(PASSWORD_MANAGER_GUI_QTQUICK)
        const auto file(fileArg.isPresent() ? QString::fromLocal8Bit(fileArg.firstValue()) : QString());
#endif
        if (qtConfigArgs.qtWidgetsGuiArg().isPresent()) {
#ifdef PASSWORD_MANAGER_GUI_QTWIDGETS
            returnCode = QtGui::runWidgetsGui(argc, argv, qtConfigArgs, file);
#else
            CMD_UTILS_START_CONSOLE;
            cerr << "The application has not been built with Qt widgets support." << endl;
#endif
        } else if (qtConfigArgs.qtQuickGuiArg().isPresent()) {
#ifdef PASSWORD_MANAGER_GUI_QTQUICK
            returnCode = QtGui::runQuickGui(argc, argv, qtConfigArgs, file);
#else
            CMD_UTILS_START_CONSOLE;
            cerr << "The application has not been built with Qt quick support." << endl;
#endif
        } else {
#if defined(PASSWORD_MANAGER_GUI_QTQUICK)
            returnCode = QtGui::runQuickGui(argc, argv, qtConfigArgs, file);
#elif defined(PASSWORD_MANAGER_GUI_QTWIDGETS)
            returnCode = QtGui::runWidgetsGui(argc, argv, qtConfigArgs, file);
#else
            CMD_UTILS_START_CONSOLE;
            cerr << "See --help for usage." << endl;
#endif
        }
    }
#else // PASSWORD_MANAGER_FORCE_GUI
#ifdef PASSWORD_MANAGER_GUI_QTQUICK
    returnCode = QtGui::runQuickGui(argc, argv, qtConfigArgs, QString());
#else
    returnCode = QtGui::runWidgetsGui(argc, argv, qtConfigArgs, QString());
#endif
#endif

    return returnCode;
}

#include "./cli/cli.h"

#ifdef PASSWORD_MANAGER_GUI_QTWIDGETS
#include "./gui/initiategui.h"
#endif
#ifdef PASSWORD_MANAGER_GUI_QTQUICK
#include "./quickgui/initiatequick.h"
#endif

#include "resources/config.h"
#include "resources/qtconfig.h"

#include <c++utilities/application/argumentparser.h>
#include <c++utilities/application/commandlineutils.h>
#include <c++utilities/misc/parseerror.h>

#if defined(PASSWORD_MANAGER_GUI_QTWIDGETS) || defined(PASSWORD_MANAGER_GUI_QTQUICK)
#define PASSWORD_MANAGER_GUI_QTWIDGETS_OR_QTQUICK
#include <QCoreApplication>
#include <QString>
#include <qtutilities/resources/qtconfigarguments.h>
ENABLE_QT_RESOURCES_OF_STATIC_DEPENDENCIES
#else
#include <c++utilities/application/fakeqtconfigarguments.h>
#endif

#include <cstdlib>
#include <iostream>

// force (preferably Qt Quick) GUI under Android
#if defined(Q_OS_ANDROID) || defined(Q_OS_WASM)
#ifdef PASSWORD_MANAGER_GUI_QTWIDGETS_OR_QTQUICK
#define PASSWORD_MANAGER_FORCE_GUI
#else
#error "Must configure building at least one kind of GUI under Android."
#endif
#endif

using namespace CppUtilities;

#ifndef PASSWORD_MANAGER_FORCE_GUI
static int fail(std::string_view error)
{
    CMD_UTILS_START_CONSOLE;
    std::cerr << error << std::endl;
    return EXIT_FAILURE;
}
#endif

// define macro to export main function if required
// note: This macro can be removed when depending on c++utilities 5.28.0.
#ifndef CPP_UTILITIES_MAIN_EXPORT
#ifdef PLATFORM_ANDROID
#define CPP_UTILITIES_MAIN_EXPORT CPP_UTILITIES_GENERIC_LIB_EXPORT
#else
#define CPP_UTILITIES_MAIN_EXPORT
#endif
#endif

CPP_UTILITIES_MAIN_EXPORT int main(int argc, char *argv[])
{
    CMD_UTILS_CONVERT_ARGS_TO_UTF8;
    SET_APPLICATION_INFO;

    // parse CLI arguments
    auto qtConfigArgs = QT_CONFIG_ARGUMENTS();
#ifndef PASSWORD_MANAGER_FORCE_GUI
    auto parser = ArgumentParser();
    auto fileArg = Argument("file", 'f', "specifies the file to be opened (or created when using --modify)");
    fileArg.setValueNames({ "path" });
    fileArg.setRequiredValueCount(1);
    fileArg.setCombinable(true);
    fileArg.setRequired(false);
    fileArg.setImplicit(true);
    qtConfigArgs.qtWidgetsGuiArg().addSubArgument(&fileArg);
    qtConfigArgs.qtQuickGuiArg().addSubArgument(&fileArg);
    auto cliArg = Argument("interactive-cli", 'i', "starts the interactive command line interface");
    cliArg.setDenotesOperation(true);
    cliArg.setSubArguments({ &fileArg });
    auto helpArg = HelpArgument(parser);
    parser.setMainArguments({ &qtConfigArgs.qtWidgetsGuiArg(), &qtConfigArgs.qtQuickGuiArg(), &cliArg, &helpArg });
    parser.parseArgs(argc, argv);

    // run CLI if CLI-argument is present
    if (cliArg.isPresent()) {
        return Cli::InteractiveCli().run(fileArg.isPresent() ? std::string(fileArg.firstValue()) : std::string());
    }

    // run GUI depending on which GUI-argument is present
    if (qtConfigArgs.areQtGuiArgsPresent()) {
#ifdef PASSWORD_MANAGER_GUI_QTWIDGETS_OR_QTQUICK
        const auto file = fileArg.isPresent() ? QString::fromLocal8Bit(fileArg.firstValue()) : QString();
#endif
        if (qtConfigArgs.qtWidgetsGuiArg().isPresent()) {
#ifdef PASSWORD_MANAGER_GUI_QTWIDGETS
            return QtGui::runWidgetsGui(argc, argv, qtConfigArgs, file);
#else
            return fail("The application has not been built with Qt Widgets GUI support.");
#endif
        } else if (qtConfigArgs.qtQuickGuiArg().isPresent()) {
#ifdef PASSWORD_MANAGER_GUI_QTQUICK
            return QtGui::runQuickGui(argc, argv, qtConfigArgs, file);
#else
            return fail("The application has not been built with Qt Quick GUI support.");
#endif
        }
    }
    return fail("See --help for usage.");

#else // PASSWORD_MANAGER_FORCE_GUI
#ifdef PASSWORD_MANAGER_GUI_QTQUICK
    return QtGui::runQuickGui(argc, argv, qtConfigArgs, QString());
#else
    return QtGui::runWidgetsGui(argc, argv, qtConfigArgs, QString());
#endif
#endif
}

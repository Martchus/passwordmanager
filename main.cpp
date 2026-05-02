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
#include <c++utilities/chrono/datetime.h>
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
#include <string>

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
    std::cerr << error << std::endl;
    return EXIT_FAILURE;
}
#endif

CPP_UTILITIES_MAIN_EXPORT int main(int argc, char *argv[])
{
    SET_APPLICATION_INFO;
    CMD_UTILS_CONVERT_ARGS_TO_UTF8;
    CMD_UTILS_START_CONSOLE;

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
    auto totpArg = OperationArgument("totp", '\0', "computes a time-based one-time password (TOTP)");
    auto urlArg = ConfigValueArgument("url", '\0', "URL, e.g. \"otpauth://…?secret=…&period=…&digits=…\"", {"url"});
    urlArg.setRequired(false);
    urlArg.setImplicit(true);
    auto timeArg = ConfigValueArgument("time", '\0', "time", {"ISO time stamp"});
    timeArg.setRequired(false);
    totpArg.setSubArguments({&urlArg, &timeArg});
    auto helpArg = HelpArgument(parser);
    parser.setMainArguments({ &qtConfigArgs.qtWidgetsGuiArg(), &qtConfigArgs.qtQuickGuiArg(), &cliArg, &totpArg, &helpArg });
    parser.parseArgs(argc, argv);

    // run CLI if CLI-argument is present
    if (cliArg.isPresent()) {
        return Cli::InteractiveCli().run(fileArg.isPresent() ? std::string(fileArg.firstValue()) : std::string());
    }

    // compute TOTP if TOTP-argument is present
    if (totpArg.isPresent()) {
        auto url = urlArg.isPresent() ? std::string(urlArg.firstValue()) : std::string();
        auto time = std::optional<DateTime>();
        try {
            if (timeArg.isPresent()) {
                time = DateTime::fromIsoStringGmt(timeArg.firstValue());
            }
        } catch (const std::runtime_error &e) {
            std::cerr << "Unable to parse specified time: " << e.what() << '\n';
            return EXIT_FAILURE;
        }
        try {
            if (url.empty() || url == "-") {
                std::getline(std::cin, url);
            }
            if (!time.has_value()) {
                time = DateTime::gmtNow();
            }
            std::cout << Util::OpenSsl::computeTOTP(url, time.value()) << std::endl;
            return EXIT_SUCCESS;
        } catch (const std::runtime_error &e) {
            std::cerr << "Unable to compute TOTP: " << e.what() << '\n';
            return EXIT_FAILURE;
        }
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

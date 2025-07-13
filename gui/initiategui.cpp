#include "./initiategui.h"

#include "./gui/mainwindow.h"

#include "resources/config.h"
#include "resources/qtconfig.h"

#include <QApplication> // ensure QGuiApplication is defined before resources.h for desktop file name

#include <qtutilities/resources/importplugin.h>
#include <qtutilities/resources/qtconfigarguments.h>
#include <qtutilities/resources/resources.h>
#include <qtutilities/settingsdialog/qtsettings.h>

#ifdef PASSWORD_MANAGER_SETUP_TOOLS_ENABLED
#include <qtutilities/setup/updater.h>

#include <c++utilities/misc/verification.h>
#endif

#include <passwordfile/util/openssl.h>

#include <QFile>
#include <QMessageBox>
#include <QSettings>

#ifdef PASSWORD_MANAGER_SETUP_TOOLS_ENABLED
#include <QNetworkAccessManager>
#endif

using namespace CppUtilities;
using namespace QtUtilities;
using namespace Util;

namespace QtGui {

// define public key and signature extension depending on verification backend
#ifdef PASSWORD_MANAGER_SETUP_TOOLS_ENABLED
// clang-format off
#define SYNCTHINGTRAY_SIGNATURE_EXTENSION ".openssl.sig"
constexpr auto signingKeyOpenSSL = std::string_view(
    R"(-----BEGIN EC PUBLIC KEY-----
MIGbMBAGByqGSM49AgEGBSuBBAAjA4GGAAQAWJAn1E7ZE5Q6H69oaV5sqCIppJdg
4bXDan9dJv6GOg70/t7q2CvwcwUXhV4FvCZxCHo25+rWYINfqKU2Utul8koAx8tK
59ohfOzI63I+CC76GfX41uRGU0P5i6hS7o/hgBLiVXqT0FgS2BMfmnLMUvUjqnI2
YQM7C55/5BM5Vrblkow=
-----END EC PUBLIC KEY-----
)");
// clang-format on
#endif

int runWidgetsGui(int argc, char *argv[], const QtConfigArguments &qtConfigArgs, const QString &file)
{
    SET_QT_APPLICATION_INFO;

    // init OpenSSL
    OpenSsl::init();

    // init application
    auto application = QApplication(argc, argv);
    QObject::connect(&application, &QCoreApplication::aboutToQuit, &OpenSsl::clean);

    // restore Qt settings
    auto qtSettings = QtSettings();
    auto settings = QtUtilities::getSettings(QStringLiteral(PROJECT_NAME));
    auto settingsError = QtUtilities::errorMessageForSettings(*settings);
    qtSettings.disableNotices();
    qtSettings.restore(*settings);
    qtSettings.apply();

    // initialize updater
#ifdef PASSWORD_MANAGER_SETUP_TOOLS_ENABLED
    auto *const nam = new QNetworkAccessManager;
    auto updateHandler = UpdateHandler(settings.get(), nam);
    nam->setParent(&updateHandler);
    auto verificationErrorMsgBox = QtUtilities::VerificationErrorMessageBox();
    updateHandler.updater()->setVerifier([&verificationErrorMsgBox](const QtUtilities::Updater::Update &update) {
        auto error = QString();
        if (update.signature.empty()) {
            error = QStringLiteral("empty/non-existent signature");
        } else {
            const auto res = CppUtilities::verifySignature(signingKeyOpenSSL, update.signature, update.data);
            error = QString::fromUtf8(res.data(), static_cast<QString::size_type>(res.size()));
        }
        if (!error.isEmpty()) {
            verificationErrorMsgBox.execForError(error);
        }
        return error;
    });
    updateHandler.applySettings();
    UpdateHandler::setMainInstance(&updateHandler);
#endif

    // apply settings specified via command line args
    qtConfigArgs.applySettings(qtSettings.hasCustomFont());
    LOAD_QT_TRANSLATIONS;

    // init widgets GUI
    if (!settingsError.isEmpty()) {
        QMessageBox::critical(nullptr, QCoreApplication::applicationName(), settingsError);
    }
    auto w = MainWindow(*settings, &qtSettings);
    w.show();
    if (!file.isEmpty()) {
        w.openFile(file);
    }

    // start event loop
    auto res = application.exec();

    // save settings to disk
    settings->sync();
    if (settingsError.isEmpty()) {
        settingsError = QtUtilities::errorMessageForSettings(*settings);
        if (!settingsError.isEmpty()) {
            QMessageBox::critical(nullptr, QCoreApplication::applicationName(), settingsError);
        }
    }

#ifdef PASSWORD_MANAGER_SETUP_TOOLS_ENABLED
    w.respawnIfRestartRequested();
#endif
    return res;
}
} // namespace QtGui


#include <QApplication>
#include <QCommandLineParser>
#include <QMenuBar>

#include <KAboutData>
#include <KActionCollection>
#include <KLocalizedString>
#include <KStandardAction>
#include <KXmlGuiWindow>


class MainWindow : public KXmlGuiWindow {
Q_OBJECT

public:
    MainWindow(QWidget *parent=0) : KXmlGuiWindow(parent){

      KStandardAction::quit(qApp, SLOT(quit()), actionCollection());

      setupGUI(Default);
    }

    //virtual ~MainWindow(){}
};

int main(int argc, char **argv) {
    QApplication app(argc, argv);

    KLocalizedString::setApplicationDomain("Kit");

    KAboutData aboutData(QStringLiteral("kit"), // The program name used internally. (componentName)
                         i18n("KIT"), // A displayable program name string. (displayName)
                         QStringLiteral("0.1"), // The program version string. (version)
                         i18n("Git Gui"), // Short description of what the app does. (shortDescription)
                         KAboutLicense::GPL, // The license this code is released under
                         i18n("(c) 2016"), // Copyright Statement
                         i18n(""), // Optional text shown in the About box.
                         QStringLiteral("..."), // The program homepage string
                         QStringLiteral("...")); // The bug report email address
    aboutData.addAuthor(i18n("Alexander Bikadorov"), i18n("Author"), QStringLiteral("abika@???"),
                         QStringLiteral("http://..."), QStringLiteral("ABika"));
    KAboutData::setApplicationData(aboutData);

    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addVersionOption();
    aboutData.setupCommandLine(&parser);
    parser.process(app);
    aboutData.processCommandLine(&parser);

    MainWindow* window = new MainWindow();
    window->show();

    return app.exec();
}

// needed for Q_OBJECT!
#include "main.moc"


#include "terminal.h"

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
    MainWindow(QWidget *parent = 0) : KXmlGuiWindow(parent) {

        KStandardAction::quit(qApp, SLOT(quit()), actionCollection());

        TerminalWidget *terminal = new TerminalWidget(this);

        setCentralWidget(terminal);

        setupGUI(Default);
    }

    // virtual ~MainWindow(){}
};

int main(int argc, char **argv) {
    QApplication app(argc, argv);

    KLocalizedString::setApplicationDomain("Kit");

    KAboutData aboutData(QStringLiteral("kit") /*componentName*/, i18n("Kit") /*displayName*/,
                         QStringLiteral("0.1"), /*version*/ i18n("Git Gui") /*shortDescription*/,
                         KAboutLicense::GPL /*license*/, i18n("(c) 2016") /* copyright */,
                         i18n("") /*about text*/, QStringLiteral("...") /* homepage*/,
                         QStringLiteral("...")) /*bug report email address*/;
    aboutData.addAuthor(i18n("Alexander Bikadorov"), i18n("Author"), QStringLiteral("abika@???"),
                        QStringLiteral("http://..."), QStringLiteral("ABika"));
    KAboutData::setApplicationData(aboutData);

    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addVersionOption();
    aboutData.setupCommandLine(&parser);
    parser.process(app);
    aboutData.processCommandLine(&parser);

    MainWindow *window = new MainWindow();
    window->show();

    return app.exec();
}

// needed for Q_OBJECT!
#include "main.moc"

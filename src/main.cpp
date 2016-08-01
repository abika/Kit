
#include "mainwindow.h"

#include <QApplication>
#include <QCommandLineParser>

#include <KAboutData>
#include <KLocalizedString>


int main(int argc, char **argv) {
    QApplication app(argc, argv);
    app.setWindowIcon(QIcon::fromTheme(QStringLiteral("svn-commit")));

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

/* This file is part of Kit.
   Copyright (C) 2017 Alexander Bikadorov

   This program  is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program  is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include "mainwindow.h"

#include <QApplication>
#include <QCommandLineParser>

#include <KAboutData>
#include <KLocalizedString>


int main(int argc, char **argv) {
    QApplication app(argc, argv);
    app.setWindowIcon(QIcon::fromTheme(QStringLiteral("svn-commit")));

    KLocalizedString::setApplicationDomain("Kit");

    KAboutData aboutData(QStringLiteral("kit"), i18n("Kit"), QStringLiteral("0.1"),
                         i18n("A terminal based Git GUI"), KAboutLicense::GPL,
                         i18n("Copyright (C) 2016 Alexander Bikadorov"),
                         i18n("") /*about text*/, QStringLiteral("https://github.com/abika/Kit"));
    aboutData.addAuthor(i18n("Alexander Bikadorov"), i18n("Author"),
                        QStringLiteral("alex.bikadorov@kdemail.net"), QStringLiteral(""),
                        QStringLiteral("abika"));
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

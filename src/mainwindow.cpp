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

#include "gitinterface.h"
#include "terminal.h"
#include "widgets/branchwidget.h"
#include "widgets/stashwidget.h"
#include "widgets/statuswidget.h"

#include <QMenuBar>
#include <QStatusBar>

#include <KActionCollection>
#include <KStandardAction>

MainWindow::MainWindow::MainWindow(QWidget *parent) : KXmlGuiWindow(parent) {

    KStandardAction::quit(qApp, SLOT(quit()), actionCollection());

    // git interface
    GitInterface *gitInterface = new GitInterface(this);
    connect(gitInterface, &GitInterface::updatedStatus,
            this, &MainWindow::updateStatusBar);

    // terminal
    TerminalWidget *terminal = new TerminalWidget(this);
    connect(terminal, &TerminalWidget::urlChanged,
            gitInterface, &GitInterface::startUpdate);
    connect(terminal, &TerminalWidget::urlChanged, this, &MainWindow::setTitle);
    connect(gitInterface, &GitInterface::repoChanged, terminal, &TerminalWidget::updateCommandLine);
    setCentralWidget(terminal);

    // dock widgets
    BranchWidget *branchWidget = new BranchWidget(this);
    branchWidget->setObjectName("branch_widget");
    connect(gitInterface, &GitInterface::updatedBranches,
            branchWidget, &BranchWidget::update);
    addDockWidget(Qt::LeftDockWidgetArea, branchWidget);

    StatusWidget *statusWidget = new StatusWidget(this);
    statusWidget->setObjectName("status_widget");
    connect(gitInterface, &GitInterface::updatedStatus,
            statusWidget, &StatusWidget::update);
    statusWidget->setFeatures(statusWidget->features() | QDockWidget::DockWidgetVerticalTitleBar);
    addDockWidget(Qt::BottomDockWidgetArea, statusWidget);

    StashWidget *stashWidget = new StashWidget(this);
    stashWidget->setObjectName("stash_widget");
    connect(gitInterface, &GitInterface::updatedStashes,
            stashWidget, &StashWidget::update);
    stashWidget->setFeatures(stashWidget->features() | QDockWidget::DockWidgetVerticalTitleBar);
    addDockWidget(Qt::BottomDockWidgetArea, stashWidget);

    // connect commands
    connect(branchWidget, &BranchWidget::branchChanged, gitInterface, &GitInterface::checkout);

    // systray icon
    m_systrayItem = new KStatusNotifierItem(this);
    m_systrayItem->setIconByName(QStringLiteral("svn-commit"));

    // init
    const QUrl currentDir = QUrl::fromLocalFile(QDir::currentPath());
    gitInterface->startUpdate(currentDir);
    setTitle(currentDir);

    setupGUI(Default);
}

void MainWindow::setTitle(const QUrl &url) {
    setWindowTitle(url.adjusted(QUrl::StripTrailingSlash).fileName());
}

void MainWindow::updateStatusBar(const QList<StatusEntry> &statusList) {
    int changed = 0, untracked = 0;
    for (const StatusEntry entry: statusList) {
        switch (entry.indexStatus) {
            case STATUS_MODIFIED:
            case STATUS_ADDED:
            case STATUS_DELETED:
            case STATUS_RENAMED:
            case STATUS_COPIED:
            case STATUS_UPDATED:
                changed++;
                break;
            case STATUS_UNTRACKED:
                untracked++;
            case STATUS_UNCHANGED:
            case STATUS_IGNORED:;
        }
    }

    QString text = changed ? i18n("%1 file changes staged for commit", changed) : i18n("Clean");
    if (untracked) {
        text += i18n(" | %1 untracked files", untracked);
    }

    this->statusBar()->showMessage(text);
}

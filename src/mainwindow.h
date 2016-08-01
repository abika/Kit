
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "branchwidget.h"
#include "gitinterface.h"
#include "terminal.h"

#include <QMenuBar>

#include <KActionCollection>
#include <KStandardAction>
#include <KXmlGuiWindow>

// disable the 'Floatable' feature for dock widgets
namespace {
    const QDockWidget::DockWidgetFeatures DefaultDockWidgetFeatures =
        QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetClosable;
}

class MainWindow : public KXmlGuiWindow {
    Q_OBJECT

  public:
    MainWindow(QWidget *parent = 0) : KXmlGuiWindow(parent) {

        KStandardAction::quit(qApp, SLOT(quit()), actionCollection());

        // dock widgets
        BranchWidget *branchWidget = new BranchWidget(this);
        // branchList->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
        addDockWidget(Qt::LeftDockWidgetArea, branchWidget);
        // viewMenu->addAction(branchList->toggleViewAction());

        // terminal
        TerminalWidget *terminal = new TerminalWidget(this);
        setCentralWidget(terminal);

        // git interface
        GitInterface *gitInterface = new GitInterface(this);
        connect(terminal, SIGNAL(urlChanged(const QUrl &)),
                gitInterface, SLOT(startUpdate(const QUrl &)));

        connect(gitInterface, SIGNAL(updatedBranches(const QStringList &)),
                branchWidget, SLOT(update(const QStringList &)));

        // init
        gitInterface->startUpdate(terminal->currentDirectory());

        setupGUI(Default);
    }

    // virtual ~MainWindow(){}
};

#endif // MAINWINDOW_H

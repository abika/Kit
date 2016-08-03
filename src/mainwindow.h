
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

        // git interface
        GitInterface *gitInterface = new GitInterface(this);

        // terminal
        TerminalWidget *terminal = new TerminalWidget(this);
        connect(terminal, SIGNAL(urlChanged(const QUrl &)),
                gitInterface, SLOT(startUpdate(const QUrl &)));
        setCentralWidget(terminal);

        // dock widgets
        BranchWidget *branchWidget = new BranchWidget(this);
        connect(gitInterface, SIGNAL(updatedBranches(const QList<BranchEntry> &)),
                branchWidget, SLOT(update(const QList<BranchEntry> &)));
        addDockWidget(Qt::LeftDockWidgetArea, branchWidget);

        // connect commands
        connect(branchWidget, SIGNAL(branchChanged(QString)), gitInterface, SLOT(checkout(QString)));

        // init
        gitInterface->startUpdate(terminal->currentDirectory());

        setupGUI(Default);
    }

    // virtual ~MainWindow(){}
};

#endif // MAINWINDOW_H

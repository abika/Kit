
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "branchwidget.h"
#include "gitinterface.h"
#include "statuswidget.h"
#include "terminal.h"

#include <QMenuBar>

#include <KActionCollection>
#include <KStandardAction>
#include <KXmlGuiWindow>

// disable the 'Floatable' feature for dock widgets
// TODO does not work?
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
        connect(terminal, SIGNAL(urlChanged(const QUrl &)), this, SLOT(setTitle(const QUrl &)));
        connect(gitInterface, SIGNAL(repoChanged()), terminal, SLOT(updateCommandLine()));
        setCentralWidget(terminal);

        // dock widgets
        BranchWidget *branchWidget = new BranchWidget(this);
        branchWidget->setObjectName("branch_widget");
        connect(gitInterface, SIGNAL(updatedBranches(QList<BranchEntry>)),
                branchWidget, SLOT(update(QList<BranchEntry>)));
        addDockWidget(Qt::LeftDockWidgetArea, branchWidget);
        StatusWidget *statusWidget = new StatusWidget(this);
        statusWidget->setObjectName("status_widget");
        connect(gitInterface, SIGNAL(updatedStatus(QList<StatusEntry>)),
                statusWidget, SLOT(update(QList<StatusEntry>)));
        statusWidget->setFeatures(statusWidget->features() |
                                  QDockWidget::DockWidgetVerticalTitleBar);
        addDockWidget(Qt::BottomDockWidgetArea, statusWidget);

        // connect commands
        connect(branchWidget, SIGNAL(branchChanged(QString)), gitInterface, SLOT(checkout(QString)));

        // init
        const QUrl currentDir = QUrl::fromLocalFile(QDir::currentPath());
        gitInterface->startUpdate(currentDir);
        setTitle(currentDir);

        setupGUI(Default);
    }

private slots:
    void setTitle(const QUrl &url) {
        setWindowTitle(url.adjusted(QUrl::StripTrailingSlash).fileName());
    }

    // virtual ~MainWindow(){}
};

#endif // MAINWINDOW_H

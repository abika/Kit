
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "gitinterface.h"
#include "terminal.h"
#include "widgets/branchwidget.h"
#include "widgets/statuswidget.h"

#include <QMenuBar>
#include <QStatusBar>

#include <KActionCollection>
#include <KStandardAction>
#include <KStatusNotifierItem>
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
    explicit MainWindow(QWidget *parent = 0) : KXmlGuiWindow(parent) {

        KStandardAction::quit(qApp, SLOT(quit()), actionCollection());

        // git interface
        GitInterface *gitInterface = new GitInterface(this);
        connect(gitInterface, SIGNAL(updatedStatus(QList<StatusEntry>)),
                this, SLOT(updateStatusBar(QList<StatusEntry>)));

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

        // systray icon
        m_systrayItem = new KStatusNotifierItem(this);
        m_systrayItem->setIconByName(QStringLiteral("svn-commit"));

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

    void updateStatusBar(const QList<StatusEntry> &statusList) {
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

private:
  KStatusNotifierItem *m_systrayItem;

    // virtual ~MainWindow(){}
};

#endif // MAINWINDOW_H

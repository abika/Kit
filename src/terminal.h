
#ifndef TERMINALWIDGET_H
#define TERMINALWIDGET_H

#include "widget.h"

#include <signal.h>

#include <QDir>
#include <QQueue>
#include <QShowEvent>
#include <QVBoxLayout>

#include <KIO/Job>
#include <KIO/JobUiDelegate>
#include <KJobWidgets>
#include <KParts/ReadOnlyPart>
#include <KPluginFactory>
#include <KPluginLoader>
#include <KService>
#include <KShell>
#include <kde_terminal_interface.h>

class TerminalInterface;
class QVBoxLayout;
class QWidget;

namespace KIO {
    class StatJob;
}

namespace KParts {
    class ReadOnlyPart;
}
class KJob;

/**
 * @brief Shows the terminal which is synchronized with the URL of the active view.
 *
 * Code borrowed from Dolphin. Copyright (C) 2007-2010 by Peter Penz <peter.penz19@gmail.com>
 */
class TerminalWidget : public Widget {
    Q_OBJECT

  public:
    TerminalWidget(QWidget *parent = 0)
        : Widget(parent), m_clearTerminal(true), m_mostLocalUrlJob(0), m_layout(0), m_terminal(0),
          m_terminalWidget(0), m_konsolePart(0), m_konsolePartCurrentDirectory(),
          m_sendCdToTerminalHistory() {
        m_layout = new QVBoxLayout(this);
        m_layout->setMargin(0);
    }

    virtual ~TerminalWidget() {}

  public slots:
    void terminalExited() {
        m_terminal = 0;
        emit hideTerminalPanel();
    }
    void dockVisibilityChanged() {
        // Only react when the DockWidget itself (not some parent) is hidden.
        // This way we don't respond when e.g. Dolphin is minimized.
        if (parentWidget() && parentWidget()->isHidden() && m_terminal &&
            (m_terminal->foregroundProcessId() == -1)) {
            // Make sure that the following "cd /" command will not affect the view.
            disconnect(m_konsolePart, SIGNAL(currentDirectoryChanged(QString)), this,
                       SLOT(slotKonsolePartCurrentDirectoryChanged(QString)));

            // Make sure this terminal does not prevent unmounting any removable drives
            changeDir(QUrl::fromLocalFile(QStringLiteral("/")));

            // Because we have disconnected from the part's currentDirectoryChanged()
            // signal, we have to update m_konsolePartCurrentDirectory manually.
            // If this was not done, showing the panel again might not set the part's working
            // directory correctly.
            m_konsolePartCurrentDirectory = '/';
        }
    }

  signals:
    void hideTerminalPanel();

    /**
     * Is emitted if the an URL change is requested.
     */
    void changeUrl(const QUrl &url);

  protected:
    virtual bool urlChanged() {
        if (!url().isValid()) {
            return false;
        }

        const bool sendInput =
            m_terminal && (m_terminal->foregroundProcessId() == -1) && isVisible();
        if (sendInput) {
            changeDir(url());
        }

        return true;
    }

    virtual void showEvent(QShowEvent *event) Q_DECL_OVERRIDE {
        if (event->spontaneous()) {
            Widget::showEvent(event);
            return;
        }

        if (!m_terminal) {
            m_clearTerminal = true;
            KPluginFactory *factory = 0;
            KService::Ptr service = KService::serviceByDesktopName(QStringLiteral("konsolepart"));
            if (service) {
                factory = KPluginLoader(service->library()).factory();
            }
            m_konsolePart = factory ? (factory->create<KParts::ReadOnlyPart>(this)) : 0;
            if (m_konsolePart) {
                connect(m_konsolePart, &KParts::ReadOnlyPart::destroyed, this,
                        &TerminalWidget::terminalExited);
                m_terminalWidget = m_konsolePart->widget();
                m_layout->addWidget(m_terminalWidget);
                m_terminal = qobject_cast<TerminalInterface *>(m_konsolePart);
            }
        }
        if (m_terminal) {
            m_terminal->showShellInDir(url().toLocalFile());
            changeDir(url());
            m_terminalWidget->setFocus();
            connect(m_konsolePart, SIGNAL(currentDirectoryChanged(QString)), this,
                    SLOT(slotKonsolePartCurrentDirectoryChanged(QString)));
        }

        Widget::showEvent(event);
    }

  private slots:
    void slotMostLocalUrlResult(KJob *job) {
        KIO::StatJob *statJob = static_cast<KIO::StatJob *>(job);
        const QUrl url = statJob->mostLocalUrl();
        if (url.isLocalFile()) {
            sendCdToTerminal(url.toLocalFile());
        }

        m_mostLocalUrlJob = 0;
    }
    void slotKonsolePartCurrentDirectoryChanged(const QString &dir) {
        m_konsolePartCurrentDirectory = QDir(dir).canonicalPath();

        // Only emit a changeUrl signal if the directory change was caused by the user inside the
        // terminal, and not by sendCdToTerminal(QString).
        while (!m_sendCdToTerminalHistory.empty()) {
            if (m_konsolePartCurrentDirectory == m_sendCdToTerminalHistory.dequeue()) {
                return;
            }
        }

        const QUrl url(QUrl::fromLocalFile(dir));
        emit changeUrl(url);
    }

  private:
    void changeDir(const QUrl &url) {
        delete m_mostLocalUrlJob;
        m_mostLocalUrlJob = 0;

        if (url.isLocalFile()) {
            sendCdToTerminal(url.toLocalFile());
        } else {
            m_mostLocalUrlJob = KIO::mostLocalUrl(url, KIO::HideProgressInfo);
            if (m_mostLocalUrlJob->ui()) {
                KJobWidgets::setWindow(m_mostLocalUrlJob, this);
            }
            connect(m_mostLocalUrlJob, &KIO::StatJob::result, this,
                    &TerminalWidget::slotMostLocalUrlResult);
        }
    }
    void sendCdToTerminal(const QString &dir) {
        if (dir == m_konsolePartCurrentDirectory) {
            m_clearTerminal = false;
            return;
        }

        if (!m_clearTerminal) {
            // The TerminalV2 interface does not provide a way to delete the
            // current line before sending a new input. This is mandatory,
            // otherwise sending a 'cd x' to a existing 'rm -rf *' might
            // result in data loss. As workaround SIGINT is send.
            const int processId = m_terminal->terminalProcessId();
            if (processId > 0) {
                kill(processId, SIGINT);
            }
        }

        m_terminal->sendInput(" cd " + KShell::quoteArg(dir) + '\n');

        // We want to ignore the currentDirectoryChanged(QString) signal, which we
        // will receive after the directory change, because this directory change is not caused by
        // a "cd" command that the user entered in the panel. Therefore, we have to remember 'dir'.
        // Note that it could also be a symbolic link -> remember the 'canonical' path.
        m_sendCdToTerminalHistory.enqueue(QDir(dir).canonicalPath());

        if (m_clearTerminal) {
            m_terminal->sendInput(QStringLiteral(" clear\n"));
            m_clearTerminal = false;
        }
    }

  private:
    bool m_clearTerminal;
    KIO::StatJob *m_mostLocalUrlJob;

    QVBoxLayout *m_layout;
    TerminalInterface *m_terminal;
    QWidget *m_terminalWidget;
    KParts::ReadOnlyPart *m_konsolePart;
    QString m_konsolePartCurrentDirectory;
    QQueue<QString> m_sendCdToTerminalHistory;
};

#endif // TERMINALWIDGET_H

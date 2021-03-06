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

#ifndef TERMINALWIDGET_H
#define TERMINALWIDGET_H

#include <signal.h>

#include <QDebug>
#include <QDir>
#include <QQueue>
#include <QVBoxLayout>
#include <QWidget>

#include <KParts/ReadOnlyPart>
#include <KPluginFactory>
#include <KPluginLoader>
#include <KService>
#include <KShell>
#include <kde_terminal_interface.h>

namespace KParts {
    class ReadOnlyPart;
}

/**
 * @brief Shows the terminal.
 *
 * Code modified from Dolphin. Copyright (C) 2007-2010 by Peter Penz <peter.penz19@gmail.com>
 */
class TerminalWidget : public QWidget {
    Q_OBJECT

  public:
    explicit TerminalWidget(QWidget *parent = 0)
        : QWidget(parent), m_clearTerminal(true), m_layout(0), m_terminal(0),
          m_terminalWidget(0), m_konsolePart(0), m_konsolePartCurrentDirectory(),
          m_sendCdToTerminalHistory() {
        m_layout = new QVBoxLayout(this);
    }

    QUrl currentDirectory() const { return m_konsolePartCurrentDirectory; }

    virtual ~TerminalWidget() {}

    void initialize(const QString &path) {
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
        if (m_terminal) {
            m_terminal->showShellInDir(path);
            m_terminalWidget->setFocus();
            connect(m_konsolePart, SIGNAL(currentDirectoryChanged(QString)), this,
                    SLOT(slotKonsolePartCurrentDirectoryChanged(QString)));
            slotKonsolePartCurrentDirectoryChanged(path);
        }
    }

  public slots:
    void terminalExited() {
        m_terminal = 0;
        emit hideTerminalPanel();
    }

    void updateCommandLine() {
        deleteLineInput();
        m_terminal->sendInput("\n");
    }

    void changeUrl(const QUrl &url) {
        if (!m_terminal) {
            qDebug() << "not initialized";
            return;
        }

        if (!url.isValid() || !url.isLocalFile()) {
            qDebug() << "not valid local URL: "<<url;
            return;
        }

        sendCdToTerminal(url.toLocalFile());
    }

  signals:
    void hideTerminalPanel();

    /**
     * Is emitted if the current terminal URL changed
     */
    void urlChanged(const QUrl &url);

  protected:

  private slots:

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
        qDebug() << "URL changed: " << url;
        emit urlChanged(url);
    }

  private:
    void sendCdToTerminal(const QString &dir) {
        if (dir == m_konsolePartCurrentDirectory) {
            m_clearTerminal = false;
            return;
        }

        if (!m_clearTerminal) {
            deleteLineInput();
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

    void deleteLineInput() {
        // The TerminalV2 interface does not provide a way to delete the
        // current line before sending a new input. This is mandatory,
        // otherwise sending a 'cd x' to a existing 'rm -rf *' might
        // result in data loss. As workaround SIGINT is send.
        const int processId = m_terminal->terminalProcessId();
        if (processId > 0) {
            kill(processId, SIGINT);
        }
    }

  private:
    bool m_clearTerminal;

    QVBoxLayout *m_layout;
    TerminalInterface *m_terminal;
    QWidget *m_terminalWidget;
    KParts::ReadOnlyPart *m_konsolePart;
    QString m_konsolePartCurrentDirectory;
    QQueue<QString> m_sendCdToTerminalHistory;
};

#endif // TERMINALWIDGET_H

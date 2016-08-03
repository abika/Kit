#ifndef GITINTERFACE_H
#define GITINTERFACE_H

#include <QDateTime>
#include <QDebug>
#include <QObject>
#include <QProcess>
#include <QUrl>

struct BranchEntry {
    QString name;
    QDateTime date;
    bool isHead;

    BranchEntry() {} // needed by Q_DECLARE_METATYPE
    BranchEntry(QString n, QDateTime d, bool h) : name(n), date(d), isHead(h) {}
};

/**
 * @brief Communication interface between Git repo and application.
 */
class GitInterface : public QObject {
    Q_OBJECT
  public:
    explicit GitInterface(QObject *parent = 0) : QObject(parent), m_lastUrl() {}

  signals:
    void updatedBranches(const QList<BranchEntry> &branches);

  public slots:
    void startUpdate(const QUrl &url) {
        m_lastUrl = url;
        QString output =
            run(argStart(url) << "for-each-ref"
                              << "--shell"
                              << "--format=%(HEAD) %(refname:short) %(authordate:iso8601)"
                              << "refs/heads/");
        if (output.isEmpty())
            return;

        const QStringList branches = output.split(QRegExp("[\r\n]"), QString::SkipEmptyParts);
        // parse output; example line: "'*' 'master' '2016-02-11 16:09:38 +0100'"
        QList<BranchEntry> branchList;
        for (QString line : branches) {
            QStringList values = line.split("'", QString::SkipEmptyParts);
            bool isHead = values[0] == "*";
            QString name = values[2];
            QDateTime date = QDateTime::fromString(values[4], Qt::ISODate);
            branchList.append(BranchEntry(name, date, isHead));
        }

        emit updatedBranches(branchList);
    }

    void checkout(const QString &branchName) {
        run(argStart(m_lastUrl)
            // TODO does not work<
            //<< "read-tree" << "-um" << "HEAD" << branchname
            << "checkout" << branchName);

        // TODO show error output

        startUpdate(m_lastUrl);
    }

  private:
    static QStringList argStart(const QUrl &url) {
        QStringList arguments;
        arguments << "-C" << url.toLocalFile();
        return arguments;
    }

    QString run(const QStringList &arguments) {
        QProcess *gitProcess = new QProcess(this);
        gitProcess->start("/usr/bin/git", arguments, QIODevice::ReadOnly);
        // TODO blocking
        bool success = gitProcess->waitForFinished(3000);
        if (!success) {
            qDebug() << "process error; prog=" << gitProcess->program()
                     << " args=" << gitProcess->arguments() << " "
                     << " error=" << gitProcess->error();
            return "";
        }
        if (gitProcess->exitCode()) {
            qDebug() << "process failed; prog=" << gitProcess->program()
                     << " args=" << gitProcess->arguments() << " "
                     << " exitCode=" << gitProcess->exitCode()
                     << " errOutput: " << gitProcess->readAllStandardError();
        }
        return QString::fromUtf8(gitProcess->readAllStandardOutput().data());
    }

    QUrl m_lastUrl;
};

Q_DECLARE_METATYPE(BranchEntry)

#endif // GITINTERFACE_H

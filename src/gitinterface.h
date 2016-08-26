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

enum FileStatus {
    STATUS_UNCHANGED = ' ',
    STATUS_MODIFIED = 'M',
    STATUS_ADDED = 'A',
    STATUS_DELETED = 'D',
    STATUS_RENAMED = 'R',
    STATUS_COPIED = 'C',
    STATUS_UPDATED = 'U',
    STATUS_UNTRACKED = '?',
    STATUS_IGNORED = '!'
};

struct StatusEntry {
    QString name;
    FileStatus statusIndex;
    FileStatus statusTree;
    QString renamedFrom;

    StatusEntry() {} // needed by Q_DECLARE_METATYPE
    StatusEntry(QString n, FileStatus x, FileStatus y, QString r = QString())
        : name(n), statusIndex(x), statusTree(y), renamedFrom(r) {}
};

/**
 * @brief Communication interface between Git repo and application.
 */
class GitInterface : public QObject {
    Q_OBJECT
  public:
    explicit GitInterface(QObject *parent = 0) : QObject(parent), m_lastRoot() {}

  public slots:
    void startUpdate(const QUrl &url) {
        const QUrl root = gitRoot(url);
        if (root == m_lastRoot) {
            // nothing to do
            return;
        }
        m_lastRoot = root;

        updateBranches(url);
        updateStatus(url);
    }

    void checkout(const QString &branchName) {
        run(argStart(m_lastRoot)
            // TODO does not work<
            //<< "read-tree" << "-um" << "HEAD" << branchname
            << "checkout" << branchName);

        // TODO show error output

        emit repoChanged();

        startUpdate(m_lastRoot);
    }

signals:
  void updatedBranches(const QList<BranchEntry> &branches);
  void updatedStatus(const QList<StatusEntry> &statuses);
  void repoChanged();

  private:
    void updateBranches(const QUrl &url) {
        QList<BranchEntry> branchList;

        if (url.isEmpty()) {
            emit updatedBranches(branchList);
            return;
        }

        const QString output =
            run(argStart(url) << "for-each-ref"
                              << "--shell"
                              << "--format=%(HEAD) %(refname:short) %(authordate:iso8601)"
                              << "refs/heads/");

        const QStringList branches = output.split(QRegExp("[\r\n]"), QString::SkipEmptyParts);
        // parse output; example line: "'*' 'master' '2016-02-11 16:09:38 +0100'"
        for (const QString line : branches) {
            const QStringList values = line.split("'", QString::SkipEmptyParts);
            const bool isHead = values[0] == "*";
            const QString name = values[2];
            const QDateTime date = QDateTime::fromString(values[4], Qt::ISODate);
            branchList.append(BranchEntry(name, date, isHead));
        }

        emit updatedBranches(branchList);
    }

    void updateStatus(const QUrl &url) {
        QList<StatusEntry> statusList;

        if (url.isEmpty()) {
            emit updatedStatus(statusList);
            return;
        }

        const QString output = run(argStart(url) << "status"
                                   << "-z");

        const QStringList lines = output.split(QChar::Tabulation, QString::SkipEmptyParts);
        for (int i=0; i < lines.length(); i++) {
            const QString line = lines[i];
            const FileStatus statusIndex = static_cast<FileStatus>(line[0].toLatin1());
            const FileStatus statusTree = static_cast<FileStatus>(line[1].toLatin1());
            const QString file = line.mid(3);
            statusList.append(StatusEntry(file, statusIndex, statusTree,
                                          // if renamed next line is the file renamed file
                                          statusIndex == STATUS_RENAMED ? lines[++i] : QString()));
        }
        emit updatedStatus(statusList);
    }

    QUrl gitRoot(const QUrl &url) {
        const QString output = run(argStart(url) << "rev-parse"
                                   << "--show-toplevel");
        return QUrl::fromLocalFile(output);
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
        QByteArray rawOutput = gitProcess->readAllStandardOutput();
        // 'git status -z' divides by \0 character
        return QString::fromUtf8(rawOutput.replace(QChar::Null, QChar::Tabulation));
    }

    static QStringList argStart(const QUrl &url) {
        QStringList arguments;
        arguments << "-C" << url.toLocalFile();
        return arguments;
    }

    static FileStatus letterToStatus(const QChar &c) {
        switch (c.toLatin1()) {
            case ' ': return STATUS_UNCHANGED;
            case 'A': return STATUS_ADDED;
        }

        return STATUS_IGNORED;
        if (c == 'A') return STATUS_ADDED;
        if (c == 'M') return STATUS_MODIFIED;
    }

    QUrl m_lastRoot;
};

Q_DECLARE_METATYPE(BranchEntry)
Q_DECLARE_METATYPE(StatusEntry)

#endif // GITINTERFACE_H

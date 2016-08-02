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
    explicit GitInterface(QObject *parent = 0) : QObject(parent) {}

  signals:
    void updatedBranches(const QList<BranchEntry> &branches);

  public slots:
    void startUpdate(const QUrl &url) {
        QString program = "/usr/bin/git";
        QStringList arguments;
        arguments << "-C" << url.toLocalFile() << "for-each-ref"
                  << "--shell"
                  << "--format=%(HEAD) %(refname:short) %(authordate)"
                  << "refs/heads/";

        QProcess *gitProcess = new QProcess(this);
        gitProcess->start(program, arguments, QIODevice::ReadOnly);
        // TODO blocking
        bool success = gitProcess->waitForFinished(3000);
        if (!success) {
            qDebug() << "process error; prog=" << gitProcess->program()
                     << " args=" << gitProcess->arguments() << " "
                     << " error=" << gitProcess->error();
            return;
        }

        QString output = QString::fromUtf8(gitProcess->readAllStandardOutput().data());
        const QStringList branches = output.split(QRegExp("[\r\n]"), QString::SkipEmptyParts);

        // parse output; example line: "* master Wed Jul 9 07:11:08 2016 +0200"
        QList<BranchEntry> branchList;
        for (QString line : branches) {
            const QStringList values = line.split("'", QString::SkipEmptyParts);
            const bool isHead = values[0] == "*";
            const QString name = values[2];
            const QDateTime date = QDateTime::fromString(values[2], "ddd MMM d hh:mm:ss YYYY ");
            branchList.append(BranchEntry(name, date, isHead));
        }

        emit updatedBranches(branchList);
    }
};

Q_DECLARE_METATYPE(BranchEntry)

#endif // GITINTERFACE_H

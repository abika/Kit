#ifndef GITINTERFACE_H
#define GITINTERFACE_H

#include <QDebug>
#include <QObject>
#include <QProcess>
#include <QUrl>

/**
 * @brief Communication interface between Git repo and application.
 */
class GitInterface : public QObject {
    Q_OBJECT
  public:
    explicit GitInterface(QObject *parent = 0) : QObject(parent) {}

  signals:
    void updatedBranches(const QStringList &branches);

  public slots:
    void startUpdate(const QUrl &url) {
        QString program = "/usr/bin/git";
        QStringList arguments;
        arguments << "-C" << url.toLocalFile();
        arguments << "branch"
                  << "--all";

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
        const QStringList branches = output.split(QRegExp("[\r\n]"));
        emit updatedBranches(branches);
    }
};

#endif // GITINTERFACE_H

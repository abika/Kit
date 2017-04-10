
#include "gitinterface.h"

#include <QDebug>
#include <QDir>

GitInterface::GitInterface(QObject *parent) : QObject(parent), m_root(), m_watcher(this) {

    connect(&m_watcher, &QFileSystemWatcher::fileChanged, this, &GitInterface::slotFileChanged);
}

void GitInterface::startUpdate(const QUrl &url) {
    const QUrl root = gitRoot(url);
    if (root == m_root) {
        // nothing to do
        return;
    }
    m_root = root;

    updateBranches(url);
    updateStatus(url);
    updateStashes(url);

    const QStringList watchingFiles = m_watcher.files();
    if (!watchingFiles.isEmpty()) {
        m_watcher.removePaths(watchingFiles);
    }
    const QDir gitDir = QDir(QDir(m_root.path()).filePath(".git"));
    m_watcher.addPath(gitDir.filePath("index")); // changes: status
    m_watcher.addPath(gitDir.filePath("HEAD"));  // changes: branches
}

void GitInterface::checkout(const QString &branchName) {
    gitOutput(argStart(m_root)
        // TODO does not work
        //<< "read-tree" << "-um" << "HEAD" << branchname
        << "checkout" << branchName);

    // TODO show error output

    emit repoChanged();

    startUpdate(m_root);
}

void GitInterface::updateBranches(const QUrl &url) {
    QList<BranchEntry> branchList;

    if (url.isEmpty()) {
        emit updatedBranches(branchList);
        return;
    }

    const QString output =
        gitOutput(argStart(url) << "for-each-ref"
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

void GitInterface::updateStatus(const QUrl &url) {
    QList<StatusEntry> statusList;

    if (url.isEmpty()) {
        emit updatedStatus(statusList);
        return;
    }

    // NOTE: "porcelain" but stable output
    const QString output = gitOutput(argStart(url) << "status" << "-z");

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

void GitInterface::updateStashes(const QUrl &url)
{
    // NOTE: 'git-stash' is only a shell script.
    // See https://github.com/git/git/blob/master/git-stash.sh

    QList<StashEntry> stashList;

    if (url.isEmpty()) {
        emit updatedStashes(stashList);
        return;
    }

    // do we have at least one stash?
    bool noStash = gitExitCode(argStart(url) << "rev-parse" << "--verify" << "refs/stash");
    if (noStash) {
        emit updatedStashes(stashList);
        return;
    }

    // TODO porcelain
    const QString output = gitOutput(argStart(url) << "log"
                               << "--format='%gd' '%gs'"
                               << "-g" << "--first-parent" <<"-m" << "refs/stash" << "--");

    const QStringList branches = output.split(QRegExp("[\r\n]"), QString::SkipEmptyParts);
    // parse output; example line: 'stash@{0}' 'WIP on master: 99db58d Separated header'"
    for (const QString line : branches) {
        const QStringList values = line.split("'", QString::SkipEmptyParts);
        stashList.append(StashEntry(values[0], values[2]));
    }

    emit updatedStashes(stashList);
}

QUrl GitInterface::gitRoot(const QUrl &url) {
    const QString output = gitOutput(argStart(url) << "rev-parse"
                               << "--show-toplevel");
    return QUrl::fromLocalFile(output.trimmed());
}

QString GitInterface::gitOutput(const QStringList &arguments) {
    QProcess *gitProcess = gitRun(arguments);
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

int GitInterface::gitExitCode(const QStringList &arguments) {
    QProcess *gitProcess = gitRun(arguments);
    return gitProcess->exitCode();
}

QProcess *GitInterface::gitRun(const QStringList &arguments) {
    QProcess *gitProcess = new QProcess(this);
    gitProcess->deleteLater();
    // TODO absolute path?
    gitProcess->start("/usr/bin/git", arguments, QIODevice::ReadOnly);
    // TODO blocking
    bool success = gitProcess->waitForFinished(3000);
    if (!success) {
        qDebug() << "process error; prog=" << gitProcess->program()
                 << " args=" << gitProcess->arguments() << " "
                 << " error=" << gitProcess->error();
    }
    return gitProcess;
}

QStringList GitInterface::argStart(const QUrl &url) {
    QStringList arguments;
    arguments << "-C" << url.toLocalFile();
    return arguments;
}

FileStatus GitInterface::letterToStatus(const QChar &c) {
    switch (c.toLatin1()) {
        case ' ': return STATUS_UNCHANGED;
        case 'A': return STATUS_ADDED;
    }

    return STATUS_IGNORED;
    if (c == 'A') return STATUS_ADDED;
    if (c == 'M') return STATUS_MODIFIED;
}

void GitInterface::slotFileChanged(const QString &path) {
    qDebug() << "file changed: " << path;
    const QString name = QFileInfo(path).fileName();

    if (name == "index") {
        updateStatus(m_root);
    } else if (name == "HEAD") {
        updateBranches(m_root);
    }

    // bug in QFileSystemWatcher, file not watched anymore
    m_watcher.addPath(path);
}

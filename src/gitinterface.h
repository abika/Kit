#ifndef GITINTERFACE_H
#define GITINTERFACE_H

#include <QDateTime>
#include <QFileSystemWatcher>
#include <QObject>
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
    FileStatus indexStatus;
    FileStatus treeStatus;
    QString renamedFrom;

    StatusEntry() {} // needed by Q_DECLARE_METATYPE
    StatusEntry(QString n, FileStatus x, FileStatus y, QString r = QString())
        : name(n), indexStatus(x), treeStatus(y), renamedFrom(r) {}
};

/**
 * @brief Communication interface between Git repo and application.
 */
class GitInterface : public QObject {
    Q_OBJECT
  public:
    explicit GitInterface(QObject *parent = 0);

  public slots:
    void startUpdate(const QUrl &url);
    void checkout(const QString &branchName);

  signals:
    void updatedBranches(const QList<BranchEntry> &branches);
    void updatedStatus(const QList<StatusEntry> &statuses);
    void repoChanged();

  private:
    void updateBranches(const QUrl &url);
    void updateStatus(const QUrl &url);

    QUrl gitRoot(const QUrl &url);

    QString run(const QStringList &arguments);

    static QStringList argStart(const QUrl &url);
    static FileStatus letterToStatus(const QChar &c);

    QUrl m_root; // current git root directory
    QFileSystemWatcher m_watcher; // watch for changes in .git directory

  private slots:
    void slotFileChanged(const QString &path);
};

Q_DECLARE_METATYPE(BranchEntry)
Q_DECLARE_METATYPE(StatusEntry)

#endif // GITINTERFACE_H

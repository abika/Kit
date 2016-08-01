#ifndef BRANCHWIDGET_H
#define BRANCHWIDGET_H

#include <QDockWidget>
#include <QListWidget>

#include <KLocalizedString>


class BranchWidget : public QDockWidget {
    Q_OBJECT

  public:
    BranchWidget(QWidget *parent = 0) : QDockWidget(parent) {
        setWindowTitle(i18n("Branches"));

        branchList = new QListWidget(this);
        setWidget(branchList);
    }

  public slots:
    void update(const QStringList &branches) {
        branchList->clear();
        branchList->addItems(branches);
    }

  private:
    QListWidget *branchList;
};

#endif // BRANCHWIDGET_H

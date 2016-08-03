#ifndef BRANCHWIDGET_H
#define BRANCHWIDGET_H

#include "gitinterface.h"

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
    void update(const QList<BranchEntry> &branches) {
        branchList->clear();
        for (BranchEntry entry : branches) {
            QListWidgetItem *item = new QListWidgetItem(entry.name);
            item->setToolTip(i18n("Last commit:") + " " +
                             entry.date.toString("ddd MMM d yyyy - hh:mm:ss"));
            branchList->addItem(item);
            if (entry.isHead) {
                item->setSelected(true);
            }
        }
        QList<QListWidgetItem *> selectedItems = branchList->selectedItems();
        if (!selectedItems.isEmpty()) {
            branchList->scrollToItem(selectedItems.first());
        }
    }

  private:
    QListWidget *branchList;
};

#endif // BRANCHWIDGET_H

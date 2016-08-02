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
        // list->setSelectionMode(QAbstractItemView::SingleSelection);
        setWidget(branchList);
    }

  public slots:
    void update(const QList<BranchEntry> &branches) {
        branchList->clear();
        for (BranchEntry entry : branches) {
            QListWidgetItem *item = new QListWidgetItem(entry.name);
            branchList->addItem(item);
            if (entry.isHead) {
                item->setSelected(true);
            }
            // TODO add dates
        }
        QList<QListWidgetItem*> selectedItems = branchList->selectedItems();
        if (!selectedItems.isEmpty()) {
            branchList->scrollToItem(selectedItems.first());
        }
    }

  private:
    QListWidget *branchList;
};

#endif // BRANCHWIDGET_H

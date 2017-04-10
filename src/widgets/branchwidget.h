#ifndef BRANCHWIDGET_H
#define BRANCHWIDGET_H

#include "../gitinterface.h"

#include <QDockWidget>
#include <QListWidget>

#include <KLocalizedString>


/**
 * @brief GUI widget showing a list of branches
 */
class BranchWidget : public QDockWidget {
    Q_OBJECT

  public:
    explicit BranchWidget(QWidget *parent = 0) : QDockWidget(parent) {
        setWindowTitle(i18n("Branches"));

        m_branchList = new QListWidget(this);
        connect(m_branchList, &QListWidget::itemActivated,
                [=](QListWidgetItem *item) { emit branchChanged(item->text()); });

        setWidget(m_branchList);
    }

  public slots:
    void update(const QList<BranchEntry> &branches) {
        m_branchList->clear();
        for (BranchEntry entry : branches) {
            QListWidgetItem *item = new QListWidgetItem(entry.name);
            item->setToolTip(i18n("Last commit:") + " " +
                             entry.date.toString("ddd MMM d yyyy - hh:mm:ss"));
            m_branchList->addItem(item);
            if (entry.isHead) {
                // TODO disable user selection
                item->setSelected(true);
            }
        }
        QList<QListWidgetItem *> selectedItems = m_branchList->selectedItems();
        if (!selectedItems.isEmpty()) {
            m_branchList->scrollToItem(selectedItems.first());
        }
    }

  signals:
    void branchChanged(const QString &);

  private:
    QListWidget *m_branchList;
};

#endif // BRANCHWIDGET_H

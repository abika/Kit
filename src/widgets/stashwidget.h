#ifndef STASHWIDGET_H
#define STASHWIDGET_H

#include <QDockWidget>
#include <QHeaderView>
#include <QTreeWidget>

#include <KLocalizedString>

#include "../gitinterface.h"

class StashWidget : public QDockWidget {
    Q_OBJECT
  public:
    explicit StashWidget(QWidget *parent = 0) : QDockWidget(parent) {
        setWindowTitle(i18n("Stash"));

        m_treeWidget = new QTreeWidget();
        m_treeWidget->setSelectionMode(QAbstractItemView::NoSelection);
        m_treeWidget->setRootIsDecorated(false);
        m_treeWidget->setHeaderLabels(QStringList() << i18n("Reference") << i18n("Subject"));
        m_treeWidget->header()->setStretchLastSection(true);
        setWidget(m_treeWidget);
    }

  public slots:
    void update(const QList<StashEntry> &stashList) {
        m_treeWidget->clear();
        QList<QTreeWidgetItem *> items;
        for (const StashEntry entry : stashList) {
            QStringList ss;
            ss << entry.selector << entry.subject;
            items.append(new QTreeWidgetItem(ss));
        }
        m_treeWidget->insertTopLevelItems(0, items);
    }

  private:
    QTreeWidget *m_treeWidget;
};

#endif // STASHWIDGET_H

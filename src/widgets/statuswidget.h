/* This file is part of Kit.
   Copyright (C) 2017 Alexander Bikadorov

   This program  is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program  is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef STATUSWIDGET_H
#define STATUSWIDGET_H

#include "../gitinterface.h"

#include <QDockWidget>
#include <QHeaderView>
#include <QTreeWidget>

#include <KLocalizedString>

class StatusWidget : public QDockWidget {
    Q_OBJECT
  public:
    explicit StatusWidget(QWidget *parent = 0) : QDockWidget(parent) {
        setWindowTitle(i18n("Status"));

        m_treeWidget = new QTreeWidget();
        m_treeWidget->setSelectionMode(QAbstractItemView::NoSelection);
        m_treeWidget->setRootIsDecorated(false);
        m_treeWidget->setHeaderLabels(QStringList() << i18n("Status") << i18n("Staged")
                                      << i18n("File") << i18n("Old Name"));
        m_treeWidget->header()->setStretchLastSection(false);
        m_treeWidget->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
        m_treeWidget->header()->setSectionResizeMode(2, QHeaderView::Stretch);
        setWidget(m_treeWidget);
    }

  public slots:
    void update(const QList<StatusEntry> &statusList) {
        m_treeWidget->clear();
        QList<QTreeWidgetItem *> items;
        for (const StatusEntry entry: statusList) {
            QStringList ss;
            ss << statusText(entry.treeStatus) << statusText(entry.indexStatus, true)
               << entry.name << entry.renamedFrom;
            items.append(new QTreeWidgetItem(ss));
        }
        m_treeWidget->insertTopLevelItems(0, items);
    }

  private:
    static QString statusText(FileStatus status, bool index = false) {
        switch(status) {
            case STATUS_UNCHANGED: return "";
            case STATUS_MODIFIED: return i18n("Modified");
            case STATUS_ADDED: return i18n("Added");
            case STATUS_DELETED: return i18n("Deleted");
            case STATUS_RENAMED: return i18n("Renamed");
            case STATUS_COPIED: return i18n("Copied");
            case STATUS_UPDATED: return i18n("Updated");
            case STATUS_UNTRACKED: return index ? "" : i18n("Untracked");
            case STATUS_IGNORED: return index ? "" : i18n("(Ignored)");
            default: return "UNKNOWN STATUS";
        }
    }

    QTreeWidget *m_treeWidget;
};

#endif // STATUSWIDGET_H

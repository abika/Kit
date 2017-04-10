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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QDockWidget>

#include <KStatusNotifierItem>
#include <KXmlGuiWindow>

// disable the 'Floatable' feature for dock widgets
// TODO does not work?
namespace {
    const QDockWidget::DockWidgetFeatures DefaultDockWidgetFeatures =
        QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetClosable;
}

struct StatusEntry;

class MainWindow : public KXmlGuiWindow {
    Q_OBJECT

  public:
    explicit MainWindow(QWidget *parent = 0);
    // virtual ~MainWindow(){}

  private slots:
    void setTitle(const QUrl &url);

    void updateStatusBar(const QList<StatusEntry> &statusList);

  private:
    KStatusNotifierItem *m_systrayItem;
};

#endif // MAINWINDOW_H

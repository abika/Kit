
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

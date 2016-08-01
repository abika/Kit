
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "terminal.h"

#include <QMenuBar>

#include <KActionCollection>
#include <KStandardAction>
#include <KXmlGuiWindow>

class MainWindow : public KXmlGuiWindow {
    Q_OBJECT

  public:
    MainWindow(QWidget *parent = 0) : KXmlGuiWindow(parent) {

        KStandardAction::quit(qApp, SLOT(quit()), actionCollection());

        TerminalWidget *terminal = new TerminalWidget(this);

        setCentralWidget(terminal);

        setupGUI(Default);
    }

    // virtual ~MainWindow(){}
};

#endif // MAINWINDOW_H

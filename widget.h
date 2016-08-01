
#ifndef WIDGET_H
#define WIDGET_H

#include <QUrl>
#include <QWidget>

class Widget : public QWidget {
    Q_OBJECT

  public:
    explicit Widget(QWidget *parent = 0) : QWidget(parent), m_url() {}

    /** Returns the current set URL of the active view. */
    QUrl url() const { return m_url; }

  signals:

  public slots:

  private:
    QUrl m_url;
};

#endif // WIDGET_H

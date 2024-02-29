#ifndef APPRESTORATION_H
#define APPRESTORATION_H

#include <QWidget>
#include <QTimer>

namespace Ui {
class appRestoration;
}

class appRestoration : public QWidget
{
    Q_OBJECT

public:
    explicit appRestoration(QWidget *parent = nullptr);
    ~appRestoration();

    void updateHostHardwareDetails( QString _macAddress, QString _srNumber );

private:
    Ui::appRestoration *ui;
};

#endif // APPRESTORATION_H

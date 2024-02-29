#ifndef NODEINFOWIDGET_H
#define NODEINFOWIDGET_H

#include <QWidget>

namespace Ui {
class NodeInfoWidget;
}

class NodeInfoWidget : public QWidget
{
    Q_OBJECT

public:
    explicit NodeInfoWidget(QWidget *parent = nullptr);
    ~NodeInfoWidget();

private:
    Ui::NodeInfoWidget *ui;
};

#endif // NODEINFOWIDGET_H

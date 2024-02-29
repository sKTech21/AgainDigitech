#include "nodeinfowidget.h"
#include "ui_nodeinfowidget.h"

NodeInfoWidget::NodeInfoWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::NodeInfoWidget)
{
    ui->setupUi(this);
}

NodeInfoWidget::~NodeInfoWidget()
{
    delete ui;
}

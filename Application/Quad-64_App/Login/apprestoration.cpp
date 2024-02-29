#include "apprestoration.h"
#include "ui_apprestoration.h"

appRestoration::appRestoration(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::appRestoration)
{
    ui->setupUi(this);
    ui->mesgLbl->clear();
    ui->otpLEdit->setEnabled( false );
}

appRestoration::~appRestoration()
{
    delete ui;
}

void appRestoration::updateHostHardwareDetails( QString _macAddress, QString _srNumber )
{
    ui->macAddressLbl->setText( _macAddress );
    _srNumber.remove( 0, 25 );
    ui->srNumberLbl->setText( QString( "*-%1" ).arg( _srNumber ) );
}

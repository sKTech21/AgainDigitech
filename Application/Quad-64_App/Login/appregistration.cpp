#include "appregistration.h"
#include "ui_appregistration.h"
#include "../connectivity.h"
#include "../commonutility.h"

appRegistration::appRegistration(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::appRegistration)
{
    ui->setupUi(this);
    ui->messageLbl->clear();

    registrationReqSent = false;

    timerObj = new QTimer(this);
    connect( timerObj, &QTimer::timeout, this, &appRegistration::timeElapsedSlot );
    timerObj->setSingleShot( true );
}

appRegistration::~appRegistration()
{
    delete ui;
}

void appRegistration::updateHostHardwareDetails( QString _macAddress, QString _srNumber )
{
    ui->macAddressLbl->setText( _macAddress );
    _srNumber.remove( 0, 25 );
    ui->srNumberLbl->setText( QString( "*-%1" ).arg( _srNumber ) );
}

bool appRegistration::validateUserInputFields( )
{
    if( ui->setUpByUserIDLEdit->text().isEmpty() ||
        ui->setUpByPassLEdit->text().isEmpty() ||
        ui->clientIDLEdit->text().isEmpty() ||
        ui->clientNameLEdit->text().isEmpty() ||
        ui->clientAddressLEdit->text().isEmpty() )
    {
        ui->messageLbl->setText( "Please ensure that all required details are entered" );
        timerObj->start( MESSAGE_ON_SCREEN_TIMEOUT );
        return false;
    }
    return true;
}

void appRegistration::on_registerTBtn_clicked( )
{
    if( checkIfInternetIsAvailable( ) != true )
    {
        ui->messageLbl->setText( "Please verify the internet connection status" );
        timerObj->start( MESSAGE_ON_SCREEN_TIMEOUT );
        return;
    }

    if( validateUserInputFields( ) == true )
    {
        ui->registerTBtn->setEnabled( false );
        ui->messageLbl->setText( "Request has been sent. Please await a response." );
        registrationReqSent = true;
        emit sendRegistrationRequest( );
        timerObj->start( 10000 );
    }
}

void appRegistration::timeElapsedSlot( )
{
    ui->messageLbl->clear();

    if( !ui->registerTBtn->isEnabled() )
    {
        registrationReqSent = false;
        ui->registerTBtn->setEnabled( true );
        ui->messageLbl->setText( "The request has timed out. Please try again." );
        timerObj->start( MESSAGE_ON_SCREEN_TIMEOUT );
    }
}

void appRegistration::updateRegistrationResponseOnUI( QString _message )
{
    ui->messageLbl->clear();
    timerObj->stop();
    ui->messageLbl->setText( _message );
    timerObj->start( MESSAGE_ON_SCREEN_TIMEOUT );
}

QString appRegistration::getManUserID( )
{
    return ui->setUpByUserIDLEdit->text();
}

QString appRegistration::getManPassword()
{
    return ui->setUpByPassLEdit->text();
}

QString appRegistration::getClientID()
{
    return ui->clientIDLEdit->text();
}

QString appRegistration::getClientName()
{
    return ui->clientNameLEdit->text();
}

QString appRegistration::getClientLocation()
{
    return ui->clientAddressLEdit->text();
}

bool appRegistration::isRegistrationRequestSentToCloud( )
{
    return registrationReqSent;
}

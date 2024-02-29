#ifndef APPREGISTRATION_H
#define APPREGISTRATION_H

#include <QWidget>
#include <QTimer>

namespace Ui {
class appRegistration;
}

class appRegistration : public QWidget
{
    Q_OBJECT

public:
    explicit appRegistration(QWidget *parent = nullptr);
    ~appRegistration();

    void updateHostHardwareDetails( QString _macAddress, QString _srNumber );
    void updateRegistrationResponseOnUI( QString _message );
    QString getManUserID( );
    QString getManPassword();
    QString getClientID();
    QString getClientName();
    QString getClientLocation();
    bool isRegistrationRequestSentToCloud( );

private slots:
    void on_registerTBtn_clicked( );
    void timeElapsedSlot( );

private:
    Ui::appRegistration *ui;
    QTimer      *timerObj;
    bool        registrationReqSent;

    bool validateUserInputFields( );

signals:
    void sendRegistrationRequest( );
};

#endif // APPREGISTRATION_H

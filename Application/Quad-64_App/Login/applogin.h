#ifndef APPLOGIN_H
#define APPLOGIN_H

#include <QWidget>
#include <QMouseEvent>
#include <QLabel>

#include "appregistration.h"
#include "apprestoration.h"
#include "login.h"

typedef enum
{
    APP_REGISTRATION_SCREEN,
    APP_RESTORATION_SCREEN,
    APP_LOGIN_SCREEN
}en_loginScreen;

namespace Ui {
class appLogin;
}

class appLogin : public QWidget
{
    Q_OBJECT

public:
    explicit appLogin(QWidget *parent = nullptr);
    ~appLogin();

    void setLoginScreen( en_loginScreen _screen );
    en_loginScreen getLoginScreen( );
    void updateUIForSelectedScreen( );

public slots:
    void sendApplicationRegistartionRequestToCloud( );
    void httpReplyHandlerInAppLogin( QString _mesgFromCloud, bool _status );

private slots:
    void on_registrationTBtn_clicked();
    void on_restorationTBtn_clicked();

private:
    Ui::appLogin *ui;

    en_loginScreen  selectedScreen;
    QWidget         *CurrentWidget;
    appRegistration *appRegistrationObj;
    appRestoration  *appRestorationObj;
    login           *loginObj;
    QLabel          *hoverMessageLabel;

    void updateUIParametersOnInit( );
    void getHostHardwareDetails( );
    QString getMacAddress( );
    void beforeOptionClose( );
    void updateWidgetToSelectedScreen( QWidget *_widget );
    void on_loginScreen_selected( );

    /* Private function declaration */
    bool eventFilter(QObject *obj, QEvent *evnt);
    void setBorderColor(QLabel *_obj , bool _setOrReset, bool isLeftBorder);

    bool checkIfAppRegistrationResponseValid( QString _mesgFromCloud );

signals:
    void httpPOSTRequest( QString _URL, QString _message );
    void registrationCompletedSuccessfully( );
};

#endif // APPLOGIN_H

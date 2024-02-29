#ifndef HOMEPAGE_H
#define HOMEPAGE_H

#include <QMainWindow>

#include "Login/applogin.h"
#include "cloudhandeling.h"

QT_BEGIN_NAMESPACE
namespace Ui { class HomePage; }
QT_END_NAMESPACE

class HomePage : public QMainWindow
{
    Q_OBJECT

public:
    HomePage(QWidget *parent = nullptr);
    ~HomePage();

public slots:
    void httpReplyHandlerInHomePage( QString _mesgFromCloud, bool _status );
    void registrationStatusHandler( );

private:
    Ui::HomePage *ui;

    appLogin *objAppLogin;
    bool appInitDone;

    // Class Objects
    cloudHandeling *cloudHandelingObj;

    void initObjectsOnPowerON( );
    void createNewDatabaseFile( );
    bool isApplicationRegistrationRequired( );
};
#endif // HOMEPAGE_H

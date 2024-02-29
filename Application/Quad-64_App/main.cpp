#include "homepage.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    HomePage w;
    // w.show();

    /* //Enable this code to show image as application icon
    QIcon *ico_1 = new QIcon();
    ico_1->addPixmap( QPixmap( "NodeConnected.png" ) );
    objHomePage.setWindowIcon( *ico_1 );
    a.setWindowIcon( *ico_1 );
    */

    return a.exec();
}

//https://github.com/shenghe/FreeSQLiteEncryption

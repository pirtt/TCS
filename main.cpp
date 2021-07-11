#include "TCSWin.h"
#include <QApplication>
#include <QFile>
int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    TCSWin &win = TCSWin::GetInstance();
    QFile skin(":/tcs.css");
    if (skin.open(QFile::ReadOnly))
    {
        app.setStyleSheet(skin.readAll());
        skin.close();
    }
    win.show();
    return app.exec();
}

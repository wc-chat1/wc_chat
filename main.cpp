#include "widget.h"
#include "tcpclient.h"
#include <QApplication>
#include <QPropertyAnimation>
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    //慢慢出现的效果
    Widget w;
    QPropertyAnimation *animation = new QPropertyAnimation(&w,"windowOpacity");
    animation->setDuration(1000);
    animation->setStartValue(0);
    animation->setEndValue(1);
    animation->start();
    w.show();
    return a.exec();
}

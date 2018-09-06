#include "chat.h"
#include "ui_chat.h"
#include "tcpserver.h"
#include "main_game_window.h"
#include <QEvent>
#include <QKeyEvent>
#include <QDateTime>
#include <QHostInfo>
#include <QScrollBar>
#include <QNetworkInterface>
#include <QProcess>
#include <QMessageBox>
#include <QFileDialog>
#include <QHostAddress>
#include <QColorDialog>
#include <QDebug>

chat::chat(QString pasvusername, QString pasvuserip)
    :ui(new Ui::chat)
{
    ui->setupUi(this);
    //隐藏标题栏，实现点击任务栏图标可以实现最小化
    this->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowSystemMenuHint | Qt::WindowMinimizeButtonHint);

    //设置光标在输入框
    ui->textEditWrite->setFocusPolicy(Qt::StrongFocus);
    ui->textEditRead->setFocusPolicy(Qt::NoFocus);
    ui->textEditWrite->setFocus();

    //自动调用eventFliter函数,来实现按下回车键发送信息
    ui->textEditWrite->installEventFilter(this);

    a = 0;
    is_opened = false;
    xpasvusername = pasvusername;
    xpasvuserip = pasvuserip;

    ui->label->setText(tr("  与 %1 聊天中   IP:%2").arg(xpasvusername).arg(xpasvuserip));

    xchat = new QUdpSocket(this);
    xport = 45456;
    xchat->bind(QHostAddress(getIP()), xport );
    connect(xchat, &QUdpSocket::readyRead, this, &chat::processPendingDatagrams);

//    server = new TcpServer(this);
//    connect(server,SIGNAL(sendFileName(QString)),this,SLOT(sentFileName(QString)));
    connect(ui->textEditWrite,&QTextEdit::currentCharFormatChanged,this,&chat::currentFormatChanged);

}

chat::~chat()
{
    is_opened = false;
    delete ui;
}


//通过私聊套接字发送到对方的私聊专用端口上
void chat::sendMessage(MessageType type, QString serverAddress)  //发送信息
{
    QByteArray data;
    QDataStream out(&data,QIODevice::WriteOnly);
    QString localHostName = QHostInfo::localHostName();
    QString address = getIP();
    out << type << getUserName() << localHostName;


    switch(type)
    {
    //    case ParticipantLeft:
    //        {
    //            break;
    //        }
    case Message :
    {
        if(ui->textEditWrite->toPlainText() == "")
        {
            return;
        }
        else
        {
            //将ip地址和得到的消息内容输入out数据流
            out <<address <<getMessage();
            ui->textEditRead->verticalScrollBar()->setValue(ui->textEditRead->verticalScrollBar()->maximum());
            break;
        }
    }
        //    case FileName:
        //            {
        //                QString clientAddress = xpasvuserip;
        //                out << address << clientAddress << fileName;
        //                break;
        //            }
    case Refuse:
    {
        out << serverAddress;
        break;
    }
    }
    xchat->writeDatagram(data,data.length(),QHostAddress(xpasvuserip), xport);
    xchat->writeDatagram(data,data.length(),QHostAddress(address), xport);

}

void chat::processPendingDatagrams()
{
    while(xchat->hasPendingDatagrams())
    {
        QByteArray datagram;
        datagram.resize(xchat->pendingDatagramSize());
        xchat->readDatagram(datagram.data(),datagram.size());
        QDataStream in(&datagram,QIODevice::ReadOnly);
        int messageType;
        in >> messageType;
        QString userName,localHostName,ipAddress,messagestr;
        QString time = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
        switch(messageType)
        {
        case Xchat:
        {
            break;
        }
        case Message:
        {
            in >>userName >>localHostName >>ipAddress >>messagestr;
            ui->textEditRead->setTextColor(QColor(23, 142, 150, 255));
            ui->textEditRead->setCurrentFont(QFont("微软雅黑",10));
            ui->textEditRead->append(localHostName+"    "+ time );//与主机名聊天中
            ui->textEditRead->append(messagestr);

            this->show();//解决bug1.收到私聊消息后才显示
            is_opened = true;
            break;
        }
            //        case FileName:
            //        {
            //            in >>userName >>localHostName >> ipAddress;
            //            QString clientAddress,fileName;
            //            in >> clientAddress >> fileName;
            //            hasPendingFile(userName,ipAddress,clientAddress,fileName);
            //            break;
            //        }
            //        case Refuse:
            //        {
            //            in >> userName >> localHostName;
            //            QString serverAddress;
            //            in >> serverAddress;
            //            QString ipAddress = getIP();

            //            if(ipAddress == serverAddress)
            //            {
            //                server->refused();
            //            }
            //            break;
            //        }
            //        case ParticipantLeft:
            //        {
            //            in >>userName >>localHostName;
            //            participantLeft(userName,localHostName,time);
            //            QMessageBox::information(0,tr("本次对话关闭"),tr("对方结束了对话"),QMessageBox::Ok);
            //            a = 1;
            //            ui->textEditRead->clear();
            //            //is_opened = true;
            //            //    this->is_opened = false;
            //            ui->~chat();
            //            close();
            //            //    delete ui;
            //            //    ui = 0;
            //            break;
            //        }
        }
    }

}

void chat::currentFormatChanged(const QTextCharFormat &format)
{
    //当编辑器的字体格式改变时，我们让部件状态也随之改变
    ui->fontComboBox->setCurrentFont(format.font());

    if(format.fontPointSize()<9)//如果字体大小出错，因为我们最小的字体为9
    {
        ui->fontSizeComboBox->setCurrentIndex(3);
    }
    else
    {
        ui->fontSizeComboBox->setCurrentIndex(ui->fontSizeComboBox->findText(QString::number(format.fontPointSize())));

    }

    ui->textbold->setChecked(format.font().bold());
    ui->textitalic->setChecked(format.font().italic());
    ui->textUnderline->setChecked(format.font().underline());
    color = format.foreground().color();
}

QString chat::getIP()  //获取ip地址
{
    QString localName = QHostInfo::localHostName();
    QList<QHostAddress> list = QHostInfo::fromName(localName).addresses();
    foreach (QHostAddress address, list)
    {
        if(address.protocol() == QAbstractSocket::IPv4Protocol) //我们使用IPv4地址
            return address.toString();
    }
    return 0;
}

QString chat::getUserName()  //获取用户名
{
    QStringList envVariables;
    envVariables << "USERNAME.*" << "USER.*" << "USERDOMAIN.*"
                 << "HOSTNAME.*" << "DOMAINNAME.*";
    QStringList environment = QProcess::systemEnvironment();
    foreach (QString string, envVariables)
    {
        int index = environment.indexOf(QRegExp(string));
        if (index != -1)
        {

            QStringList stringList = environment.at(index).split('=');
            if (stringList.size() == 2)
            {
                return stringList.at(1);
                break;
            }
        }
    }
    return false;
}

QString chat::getMessage()  //获得要发送的信息
{
    QString msg = ui->textEditWrite->toHtml();
    ui->textEditWrite->clear();
    ui->textEditWrite->setFocus();
    return msg;
}

//实现按回车键发送信息
bool chat::eventFilter(QObject *target, QEvent *event)
{
    //判断目标是否是输入框
    if(target == ui->textEditWrite)
    {
        //判断事件是否是按下键盘
        if(event->type() == QEvent::KeyPress)
        {
            QKeyEvent *e = static_cast<QKeyEvent *>(event);
            //判断是否按下回车键
            if(e->key() == Qt::Key_Return)
            {
                //调用发送按钮的槽函数
                on_SendButton_clicked();
                return true;
            }
        }
    }
}



void chat::on_emojiButton_clicked()
{

}

void chat::on_textButton_clicked(bool checked)
{

}

void chat::on_voiceButton_clicked()
{

}

void chat::on_sendfile_clicked()
{

}

void chat::on_imageButton_clicked()
{

}

void chat::on_save_clicked()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save File"),"C:",  tr("文件(*.txt)"));
    if (fileName.isEmpty())
    {
        return;

    }
    QFile *newFile = new QFile;
    newFile ->setFileName(fileName);
    bool ok = newFile->open(QIODevice::WriteOnly);
    if(ok)
    {
        QTextStream out(newFile);
        out<<ui->textEditRead->toPlainText();
        newFile->close();
        delete newFile;
    }
    else
    {
        QMessageBox::information(this, "error", "fail to save the file");

    }

}

void chat::on_SendButton_clicked()
{
    sendMessage(Message);
}

void chat::on_fontComboBox_currentFontChanged(const QFont &f)
{
    ui->textEditWrite->setCurrentFont(f);
    ui->textEditWrite->setFocus();
}

void chat::on_fontSizeComboBox_currentIndexChanged(const QString &size)
{
    ui->textEditWrite->setFontPointSize(size.toDouble());
    ui->textEditWrite->setFocus();
}

void chat::on_textbold_clicked(bool checked)
{
    checked = ((ui->textEditWrite->fontWeight())!= QFont::Bold);
    if(checked)
        ui->textEditWrite->setFontWeight(QFont::Bold);
    else
        ui->textEditWrite->setFontWeight(QFont::Normal);
    ui->textEditWrite->setFocus();
}

void chat::on_textitalic_clicked(bool checked)
{
    checked =!(ui->textEditWrite->fontItalic());
    ui->textEditWrite->setFontItalic(checked);
    ui->textEditWrite->setFocus();
}

void chat::on_textUnderline_clicked(bool checked)
{
    checked =!(ui->textEditWrite->fontUnderline());
    ui->textEditWrite->setFontUnderline(checked);
    ui->textEditWrite->setFocus();
}

void chat::on_clear_clicked()
{
    ui->textEditRead->clear();
}

void chat::on_textcolor_clicked()
{
    color = QColorDialog::getColor(color,this);
    if(color.isValid())
    {
        ui->textEditWrite->setTextColor(color);
        ui->textEditWrite->setFocus();
    }
}

void chat::on_hideButton_clicked()
{
    if( windowState() != Qt::WindowMinimized )
    {
        setWindowState( Qt::WindowMinimized );
    }
}

void chat::on_closeButton_clicked()
{
    this->close();
    //sendMessage(ParticipantLeft);
}

void chat::mousePressEvent(QMouseEvent *e)
{
    last = e->globalPos();
}
void chat::mouseMoveEvent(QMouseEvent *e)
{
    int dx = e->globalX() - last.x();
    int dy = e->globalY() - last.y();
    last = e->globalPos();
    move(x()+dx, y()+dy);
}
void chat::mouseReleaseEvent(QMouseEvent *e)
{
    int dx = e->globalX() - last.x();
    int dy = e->globalY() - last.y();
    move(x()+dx, y()+dy);
}

void chat::on_gameButton_clicked()
{
    MainGameWindow *w = new MainGameWindow(this);
    w->show();
}


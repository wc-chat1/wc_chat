#ifndef CHAT_H
#define CHAT_H

#include "tcpserver.h"
#include <QDialog>
#include <QUdpSocket>
#include <QTextCharFormat>
namespace Ui {
class chat;
}

enum MessageType
{
    Message,
    NewParticipant,
    ParticipantLeft,
    FileName,
    Refuse,
    Xchat
};

class chat : public QDialog
{
    Q_OBJECT

public:
    chat(QString pasvusername, QString pasvuserip);
    QString xpasvuserip;
    QString xpasvusername;
    ~chat();
    QUdpSocket *xchat;
    qint32 xport;

    quint16 a;
    bool is_opened;
    void sendMessage(MessageType type,QString serverAddress="");

protected:
    void mousePressEvent(QMouseEvent *e);
    void mouseMoveEvent(QMouseEvent *e);
    void mouseReleaseEvent(QMouseEvent *e);
    bool eventFilter(QObject *target, QEvent *event);

private:
    Ui::chat *ui;
    QPoint last;
    QColor color;
    TcpServer *server;
    QString fileName;
    QString getIP();
    QString getUserName();
    QString getMessage();


private slots:
    void processPendingDatagrams();
    void currentFormatChanged(const QTextCharFormat &format);

    void on_emojiButton_clicked();
    void on_textButton_clicked(bool checked);
    void on_voiceButton_clicked();
    void on_sendfile_clicked();
    void on_imageButton_clicked();
    void on_save_clicked();
    void on_SendButton_clicked();
    void on_fontComboBox_currentFontChanged(const QFont &f);
    void on_fontSizeComboBox_currentIndexChanged(const QString &size);
    void on_textbold_clicked(bool checked);
    void on_textitalic_clicked(bool checked);
    void on_textUnderline_clicked(bool checked);
    void on_clear_clicked();
    void on_textcolor_clicked();
    void on_gameButton_clicked();
    void on_hideButton_clicked();
    void on_closeButton_clicked();

};

#endif // CHAT_H

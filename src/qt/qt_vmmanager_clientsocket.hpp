/*
* 86Box	A hypervisor and IBM PC system emulator that specializes in
*		running old operating systems and software designed for IBM
*		PC systems and compatibles from 1981 through fairly recent
*		system designs based on the PCI bus.
*
*		This file is part of the 86Box distribution.
*
*		Header file for 86Box VM manager client socket module
*
*
*
* Authors:	cold-brewed
*
*		Copyright 2022 cold-brewed and the 86Box development team
*/

#ifndef QT_VMMANAGER_CLIENTSOCKET_HPP
#define QT_VMMANAGER_CLIENTSOCKET_HPP

#include "qt_vmmanager_protocol.hpp"
#include <QEvent>
#include <QLocalServer>
#include <QLocalSocket>
#include <QObject>
#include <QWidget>

class VMManagerClientSocket : public QWidget {
    Q_OBJECT

public:
    explicit VMManagerClientSocket(QObject* object = nullptr);
    bool IPCConnect(QString server);

signals:
    void pause();
    void ctrlaltdel();
    void showsettings();
    void resetVM();
    void request_shutdown();
    void force_shutdown();
    void dialogstatus(bool open);

public slots:
    void clientRunningStateChanged(VMManagerProtocol::RunningState state);

private:
//    QLocalServer *server;
    QString server_name;
    QLocalSocket *socket;
    bool server_connected;
    void connected();
    void disconnected();
    void connectionError(QLocalSocket::LocalSocketError socketError);

    // Main convenience send function
    void sendMessage(VMManagerProtocol::ClientMessage protocol_message);
    // Send message with optional params array convenience function
    void sendMessageWithList(VMManagerProtocol::ClientMessage protocol_message, QStringList list);
    // Send message with optional json object convenience function
    void sendMessageWithObject(VMManagerProtocol::ClientMessage protocol_message, QJsonObject json);
    // Full send message function called by all convenience functions
    void sendMessageFull(VMManagerProtocol::ClientMessage protocol_message, QStringList list, QJsonObject json);
    void jsonReceived(const QJsonObject &json);

    void dataReady();

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

};

#endif // QT_VMMANAGER_CLIENTSOCKET_HPP

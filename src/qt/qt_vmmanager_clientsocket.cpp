/*
* 86Box	A hypervisor and IBM PC system emulator that specializes in
*		running old operating systems and software designed for IBM
*		PC systems and compatibles from 1981 through fairly recent
*		system designs based on the PCI bus.
*
*		This file is part of the 86Box distribution.
*
*		86Box VM manager client socket module
*
*
*
* Authors:	cold-brewed
*
*		Copyright 2022 cold-brewed and the 86Box development team
*/

#include "qt_vmmanager_clientsocket.hpp"
#include "qt_vmmanager_protocol.hpp"
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

extern "C" {
#include "86box/plat.h"
}

VMManagerClientSocket::VMManagerClientSocket(QObject* obj)
{
    socket = new QLocalSocket;

}

void
VMManagerClientSocket::dataReady()
{
    // emit signal?
    QDataStream stream(socket);
    stream.setVersion(QDataStream::Qt_5_7);
    QByteArray jsonData;
    for (;;) {
        // start a transaction
        stream.startTransaction();
        // try to read the data
        stream >> jsonData;
        if (stream.commitTransaction()) {
            // first try to successfully read some data
            // need to also make sure it's valid json
            QJsonParseError parse_error{};
            // try to create a document with the data received
            const QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonData, &parse_error);
            if (parse_error.error == QJsonParseError::NoError) {
                // the data received was valid json
                if (jsonDoc.isObject()) {
                    // and is a valid json object
                    // parse the json
                    jsonReceived(jsonDoc.object());
                }
            }
            // If the received data isn't valid json,
            // loop and try to read more if available
        } else {
            // read failed, socket is reverted to its previous state (before the transaction)
            // Exit the loop and wait for more data to become available
            break;
        }
    }

}
bool
VMManagerClientSocket::IPCConnect(QString server)
{
    server_name = server;
    connect(socket, &QLocalSocket::connected, this, &VMManagerClientSocket::connected);
    connect(socket, &QLocalSocket::disconnected, this, &VMManagerClientSocket::disconnected);
    connect(socket, &QLocalSocket::errorOccurred, this, &VMManagerClientSocket::connectionError);
//    connect(socket, &QLocalSocket::error, this, &VMManagerClientSocket::connectionError);
    connect(socket, &QLocalSocket::readyRead, this, &VMManagerClientSocket::dataReady);

    socket->connectToServer(server_name);

    if(!socket->isValid()) {
        qInfo("Could not connect to server: %s", qPrintable(socket->errorString()));
        return false;
    }

    qInfo("Connection Successful");
    return true;
}
void
VMManagerClientSocket::connected()
{
    // TODO: signal
    qDebug("Connected to %s", qPrintable(server_name));
}
void
VMManagerClientSocket::disconnected()
{
    // TODO: signal
    qDebug("Disconnected from %s", qPrintable(server_name));
}
void
VMManagerClientSocket::sendMessage(VMManagerProtocol::ClientMessage protocol_message)
{
    sendMessageFull(protocol_message, QStringList(), QJsonObject());
}
void
VMManagerClientSocket::sendMessageWithList(VMManagerProtocol::ClientMessage protocol_message, QStringList list)
{
    sendMessageFull(protocol_message, list, QJsonObject());
}
void
VMManagerClientSocket::sendMessageWithObject(VMManagerProtocol::ClientMessage protocol_message, QJsonObject json)
{
    sendMessageFull(protocol_message, QStringList(), json);
}
void
VMManagerClientSocket::sendMessageFull(VMManagerProtocol::ClientMessage protocol_message, QStringList list, QJsonObject json)
{
    QDataStream clientStream(socket);
    clientStream.setVersion(QDataStream::Qt_5_7);
    auto packet = new VMManagerProtocol(VMManagerProtocol::Sender::Client);
    auto jsonMessage = packet->protocolClientMessage(protocol_message);
    if (!list.isEmpty()) {
        jsonMessage["list"] = QJsonArray::fromStringList(list);
    }
    // TODO: Add the logic for including objects
    if(!json.isEmpty()) {
        jsonMessage["params"] = json;
    }
    clientStream << QJsonDocument(jsonMessage).toJson(QJsonDocument::Compact);
}
void
VMManagerClientSocket::jsonReceived(const QJsonObject &json)
{
    // The serialization portion has already validated the message as json,
    // ensure it has the required fields
    if (!VMManagerProtocol::hasRequiredFields(json)) {
        // TODO: Error handling of some sort, emit signals
        qDebug("Invalid message received from client: required fields missing. Object:");
        qDebug() << json;
        return;
    }
    //    qDebug() << Q_FUNC_INFO << json;

    // Parsing happens here. When adding new types, make sure to first add them
    // to VMManagerProtocol::ManagerMessage and then add it to the list here.
    // If a signal needs to be emitted, add that as well and connect to slots
    // as appropriate.

    auto message_type = VMManagerProtocol::getManagerMessageType(json);
    switch (message_type) {
        case VMManagerProtocol::ManagerMessage::Pause:
            qDebug("Pause command received from manager");
            emit pause();
            break;
        case VMManagerProtocol::ManagerMessage::ResetVM:
            qDebug("Reset VM command received from manager");
            emit resetVM();
            break;
        case VMManagerProtocol::ManagerMessage::ShowSettings:
            qDebug("Show settings command received from manager");
            emit showsettings();
            break;
        case VMManagerProtocol::ManagerMessage::CtrlAltDel:
            qDebug("CtrlAltDel command received from manager");
            emit ctrlaltdel();
            break;
        case VMManagerProtocol::ManagerMessage::RequestShutdown:
            qDebug("RequestShutdown command received from manager");
            emit request_shutdown();
            break;
        case VMManagerProtocol::ManagerMessage::ForceShutdown:
            qDebug("ForceShutdown command received from manager");
            emit force_shutdown();
            break;
        case VMManagerProtocol::ManagerMessage::VMMScreenshot:
            qDebug("VM manager screenshot command received from manager");
            emit requestVMMScreenshot();
            break;
        case VMManagerProtocol::ManagerMessage::RequestStatus:
            qDebug("Status request command received from manager");
            break;
        default:
            qDebug("Unknown client message type received:");
            qDebug() << json;
            return;
    }
}
void
VMManagerClientSocket::connectionError(QLocalSocket::LocalSocketError socketError)
{
    qInfo("An connection error has occurred: ");
    switch (socketError) {
        case QLocalSocket::ServerNotFoundError:
            qInfo("Server not found");
            break;
        case QLocalSocket::ConnectionRefusedError:
            qInfo("Connection refused");
            break;
        case QLocalSocket::PeerClosedError:
            qInfo("Peer closed");
            break;
        default:
            qInfo() << "QLocalSocket::LocalSocketError " << socketError;
            break;
    }
}

bool
VMManagerClientSocket::eventFilter(QObject *obj, QEvent *event)
{
    auto running_state = VMManagerProtocol::RunningState::Unknown;

    if (socket->state() == QLocalSocket::ConnectedState) {
        if (event->type() == QEvent::WindowBlocked) {
            running_state = dopause ? VMManagerProtocol::RunningState::PausedWaiting : VMManagerProtocol::RunningState::RunningWaiting;
            clientRunningStateChanged(running_state);
        } else if (event->type() == QEvent::WindowUnblocked) {
            running_state = dopause ? VMManagerProtocol::RunningState::Paused : VMManagerProtocol::RunningState::Running;
            clientRunningStateChanged(running_state);
        }
    }
    return QObject::eventFilter(obj, event);
}
void
VMManagerClientSocket::clientRunningStateChanged(VMManagerProtocol::RunningState state)
{
    QJsonObject extra_object;
    extra_object["status"] = static_cast<int>(state);
    sendMessageWithObject(VMManagerProtocol::ClientMessage::RunningStateChanged, extra_object);

}
void
VMManagerClientSocket::vmmScreenshotAck()
{
    sendMessage(VMManagerProtocol::ClientMessage::VMMScreenshotAck);
}

#ifndef TCP_CLIENT_H
#define TCP_CLIENT_H

#include <QObject>
#include <QTcpSocket>
#include "model/atsc_config.h"

class TcpClient : public QObject {
    Q_OBJECT
public:
    explicit TcpClient(QObject* parent = nullptr);
    bool connectToServer(const QString& host = "127.0.0.1", int port = 6000);
    void disconnectFromServer();
    bool isConnected() const;
    bool sendConfig(const AtscConfig& config);

signals:
    void connected();
    void disconnected();
    void errorOccurred(const QString& message);

private:
    QTcpSocket* m_socket;
    bool sendCommand(const QString& cmd);
};

#endif

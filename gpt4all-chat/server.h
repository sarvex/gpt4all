#ifndef SERVER_H
#define SERVER_H

#include <QObject>
#include <QtHttpServer/QHttpServer>

class Server : public QObject
{
    Q_OBJECT

public:
    static Server *globalInstance();

private Q_SLOTS:

private:
    QHttpServer *m_server;

private:
    explicit Server();
    ~Server() {}
    friend class MyServer;
};

#endif // SERVER_H

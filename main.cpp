#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QTcpServer>
#include <QTcpSocket>
#include <QTimer>
#include <QTextStream>

class MyServer : public QTcpServer {
    Q_OBJECT

public:
    MyServer(QObject *parent = nullptr) : QTcpServer(parent) {
        connect(this, &QTcpServer::newConnection, this, &MyServer::onNewConnection);
    }

private slots:
    void onNewConnection() {
        QTcpSocket *clientSocket = nextPendingConnection();
        connect(clientSocket, &QTcpSocket::disconnected, clientSocket, &QTcpSocket::deleteLater);

        // Start sending "hello" every second
        QTimer *timer = new QTimer(this);
        connect(timer, &QTimer::timeout, [clientSocket]() {
            if (clientSocket->state() == QAbstractSocket::ConnectedState) {
                clientSocket->write("hello\n");
                clientSocket->flush();
            }

            qDebug() << "Sent hello!";
        });
        timer->start(1000); // 1000 milliseconds = 1 second
    }
};

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;

    MyServer server;
    if (!server.listen(QHostAddress::LocalHost, 1235)) {
        qCritical() << "Unable to start the server:" << server.errorString();
        return 1;
    }

    qDebug() << "Server started on port 1235";

    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);
    engine.loadFromModule("MohsenSimpleTcpServer", "Main");

    return app.exec();
}

#include "main.moc"

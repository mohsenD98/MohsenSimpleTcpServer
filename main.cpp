#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QTcpServer>
#include <QTcpSocket>
#include <QTimer>
#include <QTextStream>
#include <QRandomGenerator>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>

static int counter = 0;

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

                // Define the start byte
                uint8_t startByte = 0xFE;

                // Define three uint8 values
                uint8_t num1 = 1;
                uint8_t num2 = 2;
                uint8_t num3 = 3;

                // Create a JSON object
                QJsonObject jsonObject;
                jsonObject["name"] = "test";
                jsonObject["value"] = 42;
                jsonObject["flag"] = true;

                // Convert JSON object to QByteArray
                QJsonDocument jsonDoc(jsonObject);
                QByteArray jsonPayload = jsonDoc.toJson();

                // Determine length of the payload
                uint32_t payloadLength = jsonPayload.size();

                // Create the message QByteArray using a QDataStream
                QByteArray message;
                QDataStream stream(&message, QIODevice::WriteOnly);
                stream.setByteOrder(QDataStream::LittleEndian); // Assuming Big Endian as before

                stream << static_cast<uint8_t>(startByte);
                stream << static_cast<uint8_t>(num1);
                stream << static_cast<uint8_t>(num2);
                stream << static_cast<uint8_t>(num3);
                stream << static_cast<uint32_t>(payloadLength);  // This will write the payload length as a uint32

                // Append the JSON payload
                message.append(jsonPayload);

                // Send the message to the client
                clientSocket->write(message);
                clientSocket->flush();

                qDebug() << "Sent message:" << message;
            }
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

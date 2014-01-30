#include "dialog.h"
#include "ui_dialog.h"
#include <QTime>

Dialog::Dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog),
    m_nNextBlockSize(0)
{
    ui->setupUi(this);
    ui->lePort->setText("1234");
    ui->teOutput->setReadOnly(true);
}

Dialog::~Dialog()
{
    delete ui;
}

void Dialog::slotReadyRead()
{
    QDataStream in(m_pTcpSocket);
    in.setVersion(QDataStream::Qt_4_7);
    for (;;) {
        if (!m_nNextBlockSize) {
            if (m_pTcpSocket->bytesAvailable() < (int)sizeof(quint16)) {
                break;
            }
            in >> m_nNextBlockSize;
        }

        if (m_pTcpSocket->bytesAvailable() < m_nNextBlockSize) {
            break;
        }
        QTime   time;
        QString str;
        in >> time >> str;

        ui->teOutput->append(time.toString() + " Друг: " + str);
        m_nNextBlockSize = 0;
    }
}

void Dialog::slotError(QAbstractSocket::SocketError err)
{
    QString strError =
            "Error: " + (err == QAbstractSocket::HostNotFoundError ?
                             "The host was not found." :
                             err == QAbstractSocket::RemoteHostClosedError ?
                                 "The remote host is closed." :
                                 err == QAbstractSocket::ConnectionRefusedError ?
                                     "The connection was refused." :
                                     QString(m_pTcpSocket->errorString())
                                     );
    ui->teOutput->append(strError);
}

void Dialog::slotConnected()
{
    ui->btnSend->setEnabled(true);
    ui->btnConnect->setEnabled(false);
    ui->teOutput->append("Соединение прошло успешно! :D");
}

void Dialog::on_btnSend_clicked()
{
    QByteArray  arrBlock;
    QDataStream out(&arrBlock, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_7);
    out << quint16(0) << QTime::currentTime() << ui->leInput->text();

    out.device()->seek(0);
    out << quint16(arrBlock.size() - sizeof(quint16));

    m_pTcpSocket->write(arrBlock);
    ui->teOutput->append(QTime::currentTime().toString()+" "+QString("Я: %1").arg(ui->leInput->text()));
    ui->leInput->setText("");
}

void Dialog::on_btnConnect_clicked()
{
    m_pTcpSocket = new QTcpSocket(this);

    // Соединяемся с удалённым сервером по указанному адресу и номеру порта
    QString strHost = ui->leHost->text();
    int nPort = ui->lePort->text().toInt();
    m_pTcpSocket->connectToHost(strHost, nPort);

    // Настраиваем сигнал-слотовое взаимодействие
    connect(m_pTcpSocket, SIGNAL(connected()), SLOT(slotConnected()));
    connect(m_pTcpSocket, SIGNAL(readyRead()), SLOT(slotReadyRead()));
    connect(m_pTcpSocket, SIGNAL(error(QAbstractSocket::SocketError)),
            this,         SLOT(slotError(QAbstractSocket::SocketError))
            );
//    connect(ui->leInput, SIGNAL(returnPressed()),
//            this,        SLOT(on_btnSend_clicked())
//            );
}

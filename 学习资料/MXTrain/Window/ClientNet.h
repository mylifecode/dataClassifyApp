#include <QtCore>
#include <QtNetwork>
#include <QDataStream>
#include <QIODevice>
#include <QNetworkSession>

namespace MX
{

	class ClientNet : public QObject
	{
		Q_OBJECT
	public:
		ClientNet(QObject* parent) :QObject(parent)
			, mNetworkSession(0)
			, mHostName("127.0.0.1")
			, mPort(3271)
			, mTcpSocket(nullptr)
			, mBlockSize(0)
		{
			mTcpSocket = new QTcpSocket(this);
			mTcpSocket->connectToHost(mHostName, mPort);
			connect(mTcpSocket, SIGNAL(readyRead()), SLOT(receive()));

			if (checkSessionRequired())
			{
				saveSessionConfig();
			}
			startTimer(800);
		}

	Q_SIGNALS:
		void received(const QVariantMap &data);

	private:
		QByteArray  intToByte(int i)
		{
			QByteArray abyte0;
			abyte0.resize(4);
			abyte0[0] = (uchar)(0x000000ff & i);
			abyte0[1] = (uchar)((0x0000ff00 & i) >> 8);
			abyte0[2] = (uchar)((0x00ff0000 & i) >> 16);
			abyte0[3] = (uchar)((0xff000000 & i) >> 24);
			return abyte0;
		}

		int bytesToInt(QByteArray bytes) {
			int addr = bytes[0] & 0x000000FF;
			addr |= ((bytes[1] << 8) & 0x0000FF00);
			addr |= ((bytes[2] << 16) & 0x00FF0000);
			addr |= ((bytes[3] << 24) & 0xFF000000);
			return addr;
		}

		bool checkSessionRequired()
		{
			QNetworkConfigurationManager manager;
			if (manager.capabilities() & QNetworkConfigurationManager::NetworkSessionRequired)
			{
				// Get saved network configuration
				QSettings settings(QSettings::UserScope, QLatin1String("QtProject"));
				settings.beginGroup(QLatin1String("QtNetwork"));
				const QString id = settings.value(QLatin1String("DefaultNetworkConfiguration")).toString();
				settings.endGroup();

				// If the saved network configuration is not currently discovered use the system default
				QNetworkConfiguration config = manager.configurationFromIdentifier(id);
				if ((config.state() & QNetworkConfiguration::Discovered) !=
					QNetworkConfiguration::Discovered)
				{
					config = manager.defaultConfiguration();
				}

				mNetworkSession = new QNetworkSession(config, this);
				connect(mNetworkSession, SIGNAL(opened()), this, SLOT(sessionOpened()));
				mNetworkSession->open();
				return true;
			}
			return false;
		}

		void saveSessionConfig()
		{
			QNetworkConfiguration config = mNetworkSession->configuration();
			QString id;
			if (config.type() == QNetworkConfiguration::UserChoice)
				id = mNetworkSession->sessionProperty(QLatin1String("UserChoiceConfiguration")).toString();
			else
				id = config.identifier();

			QSettings settings(QSettings::UserScope, QLatin1String("QtProject"));
			settings.beginGroup(QLatin1String("QtNetwork"));
			settings.setValue(QLatin1String("DefaultNetworkConfiguration"), id);
			settings.endGroup();
		}

		void timerEvent(QTimerEvent *)
		{
			connectToHost();
		}

		void connectToHost()
		{
			if (mTcpSocket->state() == QAbstractSocket::UnconnectedState ||
				mTcpSocket->state() == QAbstractSocket::ClosingState)
			{
				mTcpSocket->connectToHost(mHostName, mPort);
			}
		}

	public Q_SLOTS:
		void send(const QVariantMap &data)
		{
			QJsonObject jsonObj = QJsonObject::fromVariantMap(data);
			QJsonDocument jsonDoc(jsonObj);
			send(jsonDoc.toJson(QJsonDocument::Compact));
		}

		void send(const QByteArray &data)
		{
			if (mTcpSocket != nullptr)
			{
				QDataStream in(mTcpSocket);
				QByteArray head = intToByte(data.length());//占4字节
				QByteArray msg;
				msg.append(head);
				msg.append(data);
				in.writeRawData(msg.data(), msg.length());
#if 0
				QFile f("./client-send.txt");
				if (f.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append))
				{
					QTextStream in(&f);
					in << QString(data) << "\n";
				}
				f.close();
#endif
			}
		}

		void receive()
		{
			QTcpSocket* tcpClient = qobject_cast<QTcpSocket*>(sender());
			QDataStream in(tcpClient);
			qint64 bytesAvailable = tcpClient->bytesAvailable();
			while (tcpClient->bytesAvailable() > 0)
			{
				if (mBlockSize == 0)
				{
					if (bytesAvailable < sizeof(qint32))
						return;
					QByteArray lenBytes = intToByte(0);//占4字节
					in.readRawData(lenBytes.data(), lenBytes.length());
					mBlockSize = bytesToInt(lenBytes);
				}

				if (tcpClient->bytesAvailable() < mBlockSize)
				{
					break;
				}

				QByteArray packetBytes;
				packetBytes.resize(mBlockSize);
				in.readRawData(packetBytes.data(), packetBytes.length());
#if 0
				QFile f("./client-received.txt");
				if (f.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append))
				{
					QTextStream in(&f);
					in << packetBytes << "\n";
				}
				f.close();
#endif 
				QJsonDocument jsonDoc = QJsonDocument::fromJson(packetBytes.data());
				Q_EMIT received(jsonDoc.object().toVariantMap());
				mBlockSize = 0;
			}
		}

	private:
		qint32 mPort;
		qint32 mBlockSize;
		QTcpSocket *mTcpSocket;
		QString mHostName;
		QNetworkSession *mNetworkSession;
	};
}

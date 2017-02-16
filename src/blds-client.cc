/*! \file blds-client.cc
 * 
 * Implementation of BldsClient class.
 *
 * (C) 2017 Benjamin Naecker bnaecker@stanford.edu
 */

#include "blds-client.h"

#include "libdata-source/include/base-source.h" // for (de)serialization methods

BldsClient::BldsClient(const QString& hostname, quint16 port, QObject *parent) :
	QObject(parent),
	m_hostname(hostname),
	m_port(port)
{
	m_socket = new QTcpSocket(this);
	m_stream.setDevice(m_socket);
	m_stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
	m_stream.setByteOrder(QDataStream::LittleEndian);

	QObject::connect(m_socket, &QAbstractSocket::connected,
			this, &BldsClient::connected);
	QObject::connect(m_socket, &QAbstractSocket::disconnected,
			this, &BldsClient::disconnected);
	QObject::connect(m_socket, 
			static_cast<void(QAbstractSocket::*)(QAbstractSocket::SocketError)>(
				&QAbstractSocket::error),
			this, [&,this](QAbstractSocket::SocketError /*err*/) -> void {
				emit error(m_socket->errorString());
			});
	QObject::connect(m_socket, &QAbstractSocket::readyRead,
			this, &BldsClient::handleReadyRead);
}

BldsClient::~BldsClient()
{
	if (isConnected())
		m_socket->disconnectFromHost();
}

bool BldsClient::isConnected() const
{
	return m_socket->state() == QAbstractSocket::ConnectedState;
}

void BldsClient::connect()
{
	if (isConnected()) {
		emit error("Already connected to BLDS");
		return;
	}
	m_socket->connectToHost(m_hostname, m_port);
}

void BldsClient::disconnect()
{
	if (!isConnected()) {
		emit error("Not connected to BLDS");
	}
	m_socket->disconnectFromHost();
}

void BldsClient::createSource(const QString& type, const QString& location)
{
	QByteArray buffer { "create-source\n" };
	buffer.append(type);
	buffer.append("\n");
	buffer.append(location);
	m_stream << buffer;
}

void BldsClient::deleteSource()
{
	QByteArray buffer { "delete-source\n" };
	m_stream << buffer;
}

void BldsClient::startRecording()
{
	QByteArray buffer { "start-recording\n" };
	m_stream << buffer;
}

void BldsClient::stopRecording()
{
	QByteArray buffer { "stop-recording\n" };
	m_stream << buffer;
}

void BldsClient::requestAllData(bool request)
{
	m_requestAllData = request;
	QByteArray buffer { "get-all-data\n" };
	quint32 size = buffer.size() + 1;
	m_stream << size;
	m_socket->write(buffer);
	m_stream << request;
}

void BldsClient::getData(float start, float stop)
{
	QByteArray buffer { "get-data\n" };
	quint32 msgSize = buffer.size() + 2 * sizeof(float);
	m_stream << msgSize;
	m_socket->write(buffer);
	m_stream << start << stop;
}

void BldsClient::get(const QString& param)
{
	QByteArray buffer { "get\n" };
	buffer.append(param.toUtf8());
	buffer.append("\n");
	m_stream << buffer;
}

void BldsClient::getSource(const QString& param)
{
	QByteArray buffer { "get-source\n" };
	buffer.append(param.toUtf8());
	buffer.append("\n");
	m_stream << buffer;
}

void BldsClient::set(const QString& param, const QVariant& data)
{
	QByteArray buffer { "set\n" };
	buffer.append(param);
	buffer.append("\n");
	if ( (param == "save-file") || (param == "save-directory") ) {
		buffer.append(data.toByteArray());
	} else if ( (param == "recording-length") || (param == "read-interval") ) {
		quint32 val = data.toUInt();
		auto oldSize = buffer.size();
		buffer.resize(oldSize + sizeof(val));
		std::memcpy(buffer.data() + oldSize, &val, sizeof(val));
	}
	m_stream << buffer;
}

void BldsClient::setSource(const QString& param, const QVariant& data)
{
	QByteArray buffer = { "set-source\n" };
	buffer.append(BaseSource::serialize(param.toUtf8() + "\n", data));
	m_stream << buffer;
}

void BldsClient::handleReadyRead()
{
	do {
		quint32 size = 0;
		if (m_socket->peek(reinterpret_cast<char*>(&size), sizeof(size)) <
				static_cast<qint64>(sizeof(size))) {
			return; // size not yet available
		}

		/* Check if full message is available */
		if (m_socket->bytesAvailable() < static_cast<qint64>(size - sizeof(size))) {
			return;
		}

		m_socket->read(sizeof(size)); // ignore size, already read
		handleMessage(size);
	} while (true);
}

void BldsClient::handleMessage(quint32 size)
{
	/* Read message type */
	auto type = m_socket->readLine();
	if (type.size() == 0) {
		emit error("Received malformed message from BLDS");
		return;
	}
	size -= type.size();
	type.chop(1); // remove newline

	if (type == "source-created") {
		handleCreateSourceResponse(size);
	} else if (type == "source-deleted") {
		handleDeleteSourceResponse(size);
	} else if (type == "set") {
		handleSetResponse(size);
	} else if (type == "get") {
		handleGetResponse(size);
	} else if (type == "set-source") {
		handleSetSourceResponse(size);
	} else if (type == "get-source") {
		handleGetSourceResponse(size);
	} else if (type == "recording-started") {
		handleStartRecordingResponse(size);
	} else if (type == "recording-stopped") {
		handleStopRecordingResponse(size);
	} else if (type == "data") {
		handleDataMessage(size);
	} else if (type == "get-all-data") {
		handleRequestAllDataResponse(size);
	} else if (type == "error") {
		emit error(QString::fromUtf8(m_socket->read(size)));
	} else {
		emit error("Unknown message type received from BLDS: " + 
				m_socket->read(size));
	}
}

bool BldsClient::parseSuccessAndStringMessage(quint32 size, QString& msg)
{
	bool success;
	m_stream >> success;
	size -= 1;
	if (!success) {
		msg = QString::fromUtf8(m_socket->read(size));
	}
	return success;
}

void BldsClient::handleCreateSourceResponse(quint32 size)
{
	QString msg;
	bool success = parseSuccessAndStringMessage(size, msg);
	emit sourceCreated(success, msg);
}

void BldsClient::handleDeleteSourceResponse(quint32 size)
{
	QString msg;
	bool success = parseSuccessAndStringMessage(size, msg);
	emit sourceDeleted(success, msg);
}

void BldsClient::handleSetResponse(quint32 size)
{
	bool success;
	m_stream >> success;
	size -= 1;
	QString param = QString::fromUtf8(m_socket->readLine());
	size -= param.size();
	param.chop(1);
	QString msg = QString::fromUtf8(m_socket->read(size));
	emit setResponse(param, success, msg);
}

void BldsClient::handleGetResponse(quint32 size)
{
	bool success;
	m_stream >> success;
	size -= 1;
	QString param = QString::fromUtf8(m_socket->readLine());
	size -= param.size();
	param.chop(1);
	QVariant data;

	if ( (param == "save-file") || (param == "save-directory") ) {
		data = QString::fromUtf8(m_socket->read(size));
	} else if ( (param == "recording-length") || (param == "read-interval") ) {
		quint32 val = 0;
		m_socket->read(reinterpret_cast<char*>(&val), sizeof(val));
		data = val;
	} else {
		data = QString::fromUtf8(m_socket->read(size)); // error message
	}
	emit getResponse(param, success, data);
}

void BldsClient::handleSetSourceResponse(quint32 size)
{
	bool success;
	m_stream >> success;
	size -= 1;
	QString param = QString::fromUtf8(m_socket->readLine());
	size -= param.size();
	param.chop(1);
	QString msg = QString::fromUtf8(m_socket->read(size));
	emit setSourceResponse(param, success, msg);
}

void BldsClient::handleGetSourceResponse(quint32 size)
{
	bool success;
	m_stream >> success;
	size -= 1;
	QString param = QString::fromUtf8(m_socket->readLine());
	size -= param.size();
	param.chop(1);
	auto buffer = m_socket->read(size);
	QVariant data = BaseSource::deserialize(param.toUtf8(), buffer);
	emit getSourceResponse(param, success, data);
}

void BldsClient::handleStartRecordingResponse(quint32 size)
{
	QString msg;
	bool success = parseSuccessAndStringMessage(size, msg);
	emit recordingStarted(success, msg);
}

void BldsClient::handleStopRecordingResponse(quint32 size)
{
	QString msg;
	bool success = parseSuccessAndStringMessage(size, msg);
	emit recordingStopped(success, msg);
}

void BldsClient::handleRequestAllDataResponse(quint32 size)
{
	QString msg;
	bool success = parseSuccessAndStringMessage(size, msg);
	emit requestAllDataResponse(success, msg);
}

void BldsClient::handleDataMessage(quint32 size)
{
	emit data(DataFrame::deserialize(m_socket->read(size)));
}

void BldsClient::handleError(quint32 size)
{
	QString msg = QString::fromUtf8(m_socket->read(size));
	emit error(msg);
}


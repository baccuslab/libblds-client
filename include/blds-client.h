/*! \file blds-client.h
 *
 * Header file declaring the BldsClient class, used to interact
 * with the BLDS application.
 *
 * (C) 2017 Benjamin Naecker bnaecker@stanford.edu
 */

#ifndef BLDS_CLIENT_H
#define BLDS_CLIENT_H

#ifdef COMPILE_LIBBLDS_CLIENT
# define VISIBILITY Q_DECL_EXPORT
#else
# define VISIBILITY Q_DECL_IMPORT
#endif

#include "blds/include/data-frame.h"

#include <armadillo>

#include <QtCore>
#include <QtNetwork>

/*! \class BldsClient
 * The main class of `libblds-client`.
 */
class BldsClient : public QObject {
	Q_OBJECT

	public:
		BldsClient(const QString& hostname = "localhost", 
				quint16 port = 12345, QObject *parent = nullptr);

		~BldsClient();

		BldsClient(const BldsClient&) = delete;
		BldsClient& operator=(const BldsClient&) = delete;
		BldsClient(BldsClient&&) = delete;

	public:
		bool isConnected() const;

	public slots:

		void connect();
		void disconnect();
		void createSource(const QString& type = "mcs", const QString& location = "");
		void deleteSource();
		void startRecording();
		void stopRecording();
		void setSource(const QString& param, const QVariant& data);
		void getSource(const QString& param);
		void set(const QString& param, const QVariant& data);
		void get(const QString& param);
		void requestAllData(bool request = true);
		void getData(float start, float stop);

	signals:
		void connected();
		void disconnected();
		void sourceCreated(bool success, const QString& msg);
		void sourceDeleted(bool success, const QString& msg);
		void recordingStarted(bool success, const QString& msg);
		void recordingStopped(bool success, const QString& msg);
		void setSourceResponse(const QString& param, bool success, const QString& msg);
		void getSourceResponse(const QString& param, bool success, const QVariant& data);
		void setResponse(const QString& param, bool success, const QString& msg);
		void getResponse(const QString& param, bool success, const QVariant& data);
		void requestAllDataResponse(bool success, const QString& msg);
		void data(const DataFrame& frame);
		void error(const QString& msg);

	private slots:
		void handleReadyRead();

	private:
		void handleMessage(quint32 size);
		bool parseSuccessAndStringMessage(quint32 size, QString& msg);
		void handleCreateSourceResponse(quint32 size);
		void handleDeleteSourceResponse(quint32 size);
		void handleSetResponse(quint32 size);
		void handleGetResponse(quint32 size);
		void handleSetSourceResponse(quint32 size);
		void handleGetSourceResponse(quint32 size);
		void handleStartRecordingResponse(quint32 size);
		void handleStopRecordingResponse(quint32 size);
		void handleRequestAllDataResponse(quint32 size);
		void handleDataMessage(quint32 size);
		void handleError(quint32 size);

		QTcpSocket* m_socket;
		QDataStream m_stream;
		bool m_requestAllData;
		QString m_hostname;
		quint16 m_port;
};

#endif


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
 *
 * The BldsClient class supports remote communication with the Baccus Lab
 * Data Server application (BLDS). Clients connect to the server, and can
 *  	- query or manipulate the server's parameters
 *  	- create, delete, or modify a data source
 *  	- create, delete, start, or stop a recording
 *  	- retrieve chunks of data from the server
 *
 * The class provides mechanisms for communicating with the BLDS using the
 * lower-level line-based protocol over TCP, and for making the supported
 * status requests via HTTP.
 */
class BldsClient : public QObject {
	Q_OBJECT

		/*! The port on which the BLDS listens for HTTP requests. */
		const quint16 BldsHttpPort = 8000;

		/*! Path for making HTTP requests about the server's status. */
		const QString BldsServerStatusPath = "/status";

		/*! Path for making HTTP requests about the source's status. */
		const QString BldsSourceStatusPath = "/source";

	public:
		
		/*! Construct a BldsClient.
		 *
		 * \param hostname The hostname or IP address at which to connect.
		 * \param port The port at which to connect to the BLDS.
		 * \param parent The parent QObject.
		 */
		BldsClient(const QString& hostname = "localhost", 
				quint16 port = 12345, QObject *parent = nullptr);

		/*! Delete a client object. */
		~BldsClient();

		/* Copying is not supported */
		BldsClient(const BldsClient&) = delete;
		BldsClient& operator=(const BldsClient&) = delete;
		BldsClient(BldsClient&&) = delete;

	public:

		/*! Return true if the client is connected, false otherwise. */
		bool isConnected() const;

		/*! Return the hostname of the BLDS. */
		QString hostname() const;

		/*! Return the port number of the BLDS. */
		quint16 port() const;

		/*! Return a formatted version of the address.
		 *
		 * This is formatted as `<hostname>:<port>`.
		 */
		QString address() const;

	public slots:

		/*! Connect to the BLDS. */
		void connect();

		/*! Disconnect from the BLDS. */
		void disconnect();

		/*! Request that the BLDS create a data source.
		 *
		 * \param type The type of source to create.
		 * \parma location A location idenitifer for the type of source
		 * 	to create. See the documentation of `libdata-source` for more
		 * 	details.
		 */
		void createSource(const QString& type = "mcs", const QString& location = "");

		/*! Request that the BLDS deleted the current data source. */
		void deleteSource();

		/*! Request that the BLDS start recording data. */
		void startRecording();

		/*! Request that the BLDS stop an active recording. */
		void stopRecording();

		/*! Send a request to set a named parameter of the source.
		 *
		 * \param param The name of the parameter to set.
		 * \param data Variant containing the encoded value to set the parameter to.
		 */
		void setSource(const QString& param, const QVariant& data);

		/*! Get the value of a named parameter of the source.
		 *
		 * \param param The name of the parameter to retrieve the value of.
		 */
		void getSource(const QString& param);

		/*! Send a request to set a named parameter of the server.
		 *
		 * \param param The name of the parameter to set.
		 * \param data Variant containing the encoded value to set the parameter to.
		 */
		void set(const QString& param, const QVariant& data);

		/*! Get the value of a named parameter of the source.
		 *
		 * \param param The name of the parameter to retrieve the value of.
		 */
		void get(const QString& param);

		/*! Send a request for the BLDS to send all data as it is available.
		 *
		 * \param request True to request all data, false to cancel a previous request.
		 */
		void requestAllData(bool request = true);

		/*! Get a delimited chunk of data.
		 *
		 * \param start The start time of the data chunk to retrieve.
		 * \param stop The stop time of the data chunk to retrieve.
		 */
		void getData(float start, float stop);

		/*! Send an HTTP request for the server's overall status. */
		void requestServerStatus();

		/*! Send an HTTP request for the source's overall status. */
		void requestSourceStatus();

	signals:

		/*! Emitted when the client connects to the BLDS.
		 *
		 * \param made True if the connection was made successfully, else false.
		 */
		void connected(bool made);

		/*! Emitted when the client disconnects from the BLDS. */
		void disconnected();

		/*! Emitted upon receipt of a response to a request to create a
		 * data source.
		 *
		 * \param success True if the request succeeded, false otherwise.
		 * \param msg If the request failed, this contains an error message.
		 */
		void sourceCreated(bool success, const QString& msg);

		/*! Emitted upon receipt of a response to a request to delete a
		 * data source.
		 *
		 * \param success True if the request succeeded, false otherwise.
		 * \param msg If the request failed, this contains an error message.
		 */
		void sourceDeleted(bool success, const QString& msg);

		/*! Emitted upon receipt of a response to a request to start 
		 * a recording.
		 *
		 * \param success True if the request succeeded, false otherwise.
		 * \param msg If the request failed, this contains an error message.
		 */
		void recordingStarted(bool success, const QString& msg);

		/*! Emitted upon receipt of a response to a request to stop 
		 * a recording.
		 *
		 * \param success True if the request succeeded, false otherwise.
		 * \param msg If the request failed, this contains an error message.
		 */
		void recordingStopped(bool success, const QString& msg);

		/*! Emitted upon receipt of a response to a request to set a
		 * named parameter of the data source.
		 *
		 * \param param The name of the parameter.
		 * \param success True if the request succeeded, false otherwise.
		 * \param msg If the request failed, this contains an error message.
		 */
		void setSourceResponse(const QString& param, bool success, const QString& msg);

		/*! Emitted upon receipt of a response to a request to get a
		 * named parameter of the data source.
		 *
		 * \param param The name of the parameter.
		 * \param success True if the parameter is valid, false otherwise.
		 * \param data If the request succeeded, this contains the data 
		 * 	for the parameter encoded in a variant. If the request failed,
		 * 	this contains an error message, encoded as a QString.
		 */
		void getSourceResponse(const QString& param, bool success, const QVariant& data);

		/*! Emitted upon receipt of a response to a request to set a
		 * named parameter of the server.
		 *
		 * \param param The name of the parameter.
		 * \param success True if the request succeeded, false otherwise.
		 * \param msg If the request failed, this contains an error message.
		 */
		void setResponse(const QString& param, bool success, const QString& msg);

		/*! Emitted upon receipt of a response to a request to get a
		 * named parameter of the server.
		 *
		 * \param param The name of the parameter.
		 * \param success True if the request succeeded, false otherwise.
		 * \param data If the request succeeded, this contains the data 
		 * 	for the parameter encoded in a variant. If the request failed,
		 * 	this contains an error message, encoded as a QString.
		 */
		void getResponse(const QString& param, bool success, const QVariant& data);

		/*! Emitted upon receipt of a response to a request for all 
		 * future data from the server.
		 *
		 * \param success True if the request succeeded, false otherwise.
		 * \param data If the request failed, this contains an error message.
		 */
		void requestAllDataResponse(bool success, const QString& msg);

		/*! Emitted when a new frame of data is received.
		 *
		 * \param frame The DataFrame containing the data. This class contains
		 * 	the start time, stop time, and raw data itself for a single chunk
		 * 	of data.
		 */
		void data(const DataFrame& frame);

		/*! Emitted when the client receives an error message from the server,
		 * or when an internal error occurs.
		 *
		 * \param msg The error message.
		 */
		void error(const QString& msg);

		/*! Emitted when the server's status is received.
		 *
		 * \param json A JSON dictionary containing all parameters and values.
		 */
		void serverStatus(QJsonObject json);

		/*! Emitted when the source's status is received.
		 *
		 * \param json A JSON dictionary containing all parameters and values.
		 */
		void sourceStatus(bool exists, QJsonObject json);

	private slots:

		/* Handle new data available on the client's socket. */
		void handleReadyRead();

		/* Handle a finished HTTP reply for the server's status. */
		void handleServerStatusReply(QNetworkReply* reply);

		/* Handle a finished HTTP reply for the source's status. */
		void handleSourceStatusReply(QNetworkReply* reply);

	private:

		/* Handle new messages on the socket of the given size. */
		void handleMessage(quint32 size);

		/* Parse messsages which contain only a success boolean and 
		 * a string in the case of failure.
		 */
		bool parseSuccessAndStringMessage(quint32 size, QString& msg);

		/* Handle a response to create a data source. */
		void handleCreateSourceResponse(quint32 size);

		/* Handle a response to delete a data source. */
		void handleDeleteSourceResponse(quint32 size);

		/* Handle a response to set a server parameter. */
		void handleSetResponse(quint32 size);

		/* Handle a response to get a server parameter. */
		void handleGetResponse(quint32 size);

		/* Handle a response to set a source parameter. */
		void handleSetSourceResponse(quint32 size);

		/* Handle a response to get a source parameter. */
		void handleGetSourceResponse(quint32 size);

		/* Handle a response to start a recording. */
		void handleStartRecordingResponse(quint32 size);

		/* Handle a response to stop a recording. */
		void handleStopRecordingResponse(quint32 size);

		/* Handle a response to a request for all future data. */
		void handleRequestAllDataResponse(quint32 size);

		/* Handle a response to a request for data. */
		void handleDataMessage(quint32 size);

		/* Handle an error message. */
		void handleError(quint32 size);

		/* The socket with which the client communicates to the BLDS. */
		QTcpSocket* m_socket;

		/* Data stream object for simplifying serialization of data. */
		QDataStream m_stream;

		/* True if the client requests all data. */
		bool m_requestAllData;

		/* Hostname of the BLDS. */
		QString m_hostname;

		/* Port of the BLDS. */
		quint16 m_port;

		/* Manager for sending/receiving HTTP request. */
		QNetworkAccessManager* m_manager;

		/* Request object used to make status HTTP requests. */
		QNetworkRequest m_serverRequest;
		QNetworkRequest m_sourceRequest;

		/* The URLs for the above requests. Changes when the 
		 * request changes.
		 */
		QUrl m_serverUrl;
		QUrl m_sourceUrl;
};

#endif


#include "blds-client.h"

#include <QtTest/QtTest>

class TestLibBldsClient : public QObject {
	Q_OBJECT

	private slots:
		void testConnectDisconnect();
		void testCreateDelete();
		void testServerGetSet();
		void testStartStop();
};

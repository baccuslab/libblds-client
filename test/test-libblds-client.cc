#include "test-libblds-client.h"

void TestLibBldsClient::testConnectDisconnect()
{
	BldsClient client;
	QSignalSpy connectSpy(&client, &BldsClient::connected);
	QSignalSpy disconnectSpy(&client, &BldsClient::disconnected);

	client.connect();
	QVERIFY(connectSpy.wait(1000));
	client.disconnect();
	QVERIFY(disconnectSpy.wait(1000));
}

void TestLibBldsClient::testCreateDelete()
{
	BldsClient client;
	client.connect();

	QSignalSpy createSpy(&client, &BldsClient::sourceCreated);
	QSignalSpy deleteSpy(&client, &BldsClient::sourceDeleted);

	client.createSource("file", "/Users/bnaecker/file-cabinet/stanford/baccuslab/spike-sorting/extract/2015-01-27a.h5");
	QVERIFY(createSpy.wait(1000));

	client.deleteSource();
	QVERIFY(deleteSpy.wait(1000));
}

void TestLibBldsClient::testServerGetSet()
{
	BldsClient client;
	client.connect();

	QSignalSpy getSpy(&client, &BldsClient::getResponse);
	QSignalSpy setSpy(&client, &BldsClient::setResponse);

	client.get("read-interval");
	QVERIFY(getSpy.wait(1000));
	auto args = getSpy.takeFirst();
	QVERIFY(args.size() == 3);
	QVERIFY(args.at(0).toString() == "read-interval");
	QVERIFY(args.at(1).toBool() == true);
	QVERIFY(args.at(2).toUInt() == 10);

	client.set("read-interval", 100);
	QVERIFY(setSpy.wait(1000));
	args = setSpy.takeFirst();
	QVERIFY(args.size() == 3);
	QVERIFY(args.at(0).toString() == "read-interval");
	QVERIFY(args.at(1).toBool() == true);

	client.get("read-interval");
	QVERIFY(getSpy.wait(1000));
	args = getSpy.takeFirst();
	QVERIFY(args.size() == 3);
	QVERIFY(args.at(2).toUInt() == 100);

	client.set("read-interval", 10);
	QVERIFY(setSpy.wait(1000));
	args = setSpy.takeFirst();
	QVERIFY(args.size() == 3);
	QVERIFY(args.at(0).toString() == "read-interval");
	QVERIFY(args.at(1).toBool() == true);

	client.get("read-interval");
	QVERIFY(getSpy.wait(1000));
	args = getSpy.takeFirst();
	QVERIFY(args.size() == 3);
	QVERIFY(args.at(2).toUInt() == 10);

	client.get("invalid-parameter");
	QVERIFY(getSpy.wait(1000));
	args = getSpy.takeFirst();
	QVERIFY(args.size() == 3);
	QVERIFY(args.at(0).toString() == "invalid-parameter");
	QVERIFY(args.at(1).toBool() == false);
	QVERIFY(args.at(2).canConvert(QVariant::String));
}

void TestLibBldsClient::testStartStop()
{
	BldsClient client;
	client.connect();
	
	QSignalSpy startSpy(&client, &BldsClient::recordingStarted);
	QSignalSpy stopSpy(&client, &BldsClient::recordingStopped);

	client.startRecording();
	QVERIFY(startSpy.wait(1000));
	auto args = startSpy.takeFirst();
	QVERIFY(args.size() == 2);
	QVERIFY(args.at(0).toBool() == false);

	client.stopRecording();
	QVERIFY(stopSpy.wait(1000));
	args = stopSpy.takeFirst();
	QVERIFY(args.size() == 2);
	QVERIFY(args.at(0).toBool() == false);

	client.createSource("file", "/Users/bnaecker/file-cabinet/stanford/baccuslab/spike-sorting/extract/2015-01-27a.h5");

	client.stopRecording();
	QVERIFY(stopSpy.wait(1000));
	args = stopSpy.takeFirst();
	QVERIFY(args.size() == 2);
	QVERIFY(args.at(0).toBool() == false);

	client.startRecording();
	QVERIFY(startSpy.wait(1000));
	args = startSpy.takeFirst();
	QVERIFY(args.size() == 2);
	QVERIFY(args.at(0).toBool() == true);

	client.startRecording();
	QVERIFY(startSpy.wait(1000));
	args = startSpy.takeFirst();
	QVERIFY(args.size() == 2);
	QVERIFY(args.at(0).toBool() == false);

	client.stopRecording();
	QVERIFY(stopSpy.wait(1000));
	args = stopSpy.takeFirst();
	QVERIFY(args.size() == 2);
	QVERIFY(args.at(0).toBool() == true);

	QSignalSpy deleteSpy(&client, &BldsClient::sourceDeleted);
	client.deleteSource();
	QVERIFY(deleteSpy.wait(1000));
}

QTEST_MAIN(TestLibBldsClient);


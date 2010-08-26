#ifndef INDIGOCONFIGURATION_H
#define INDIGOCONFIGURATION_H

#include <string>
#include <vector>
#include <set>
#include <map>

#include <Poco/SharedPtr.h>
#include <Poco/Mutex.h>

using namespace std;

using namespace Poco;

class IndigoConfiguration
{
public:
	static const IndigoConfiguration &init(
		const string &serverName,
		const string &address,
		int port,
		int backlog,
		int minThreads,
		int maxThreads,
		int maxQueued,
		const map<string, string> &shares,
		const map<string, string> &mimeTypes
		);
	static const IndigoConfiguration &get();

	void validate() const;

	const string &getServerName() const;
	const string &getAddress() const;
	int getPort() const;
	int getBacklog() const;
	int getMinThreads() const;
	int getMaxThreads() const;
	int getMaxQueued() const;

	const set<string> &getShares() const;
	const string &getSharePath(const string &share) const;

	const string &getMimeType(const string &extension) const;

private:
	IndigoConfiguration(
		const string &serverName,
		const string &address,
		int port,
		int backlog,
		int minThreads,
		int maxThreads,
		int maxQueued,
		const map<string, string> &shares,
		const map<string, string> &mimeTypes
		);

	static IndigoConfiguration *singleton;

	const string serverName;
	const string address;
	const int port;
	const int backlog;
	const int minThreads;
	const int maxThreads;
	const int maxQueued;
	const map<string, string> shares;
	const map<string, string> mimeTypes;

	set<string> sharesSet;

	static const string defaultPath;
	static const string defaultMimeType;
};

#endif //INDIGOCONFIGURATION_H

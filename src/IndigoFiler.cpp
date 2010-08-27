
#include <string>
#include <vector>
#include <map>
#include <iostream>
#include <sstream>
#include <memory>

#include "Poco/Util/ServerApplication.h"
#include "Poco/Net/HTTPServer.h"
#include "Poco/Util/HelpFormatter.h"

#include "IndigoFiler.h"
#include "IndigoConfiguration.h"
#include "IndigoRequestHandler.h"

using namespace std;

using namespace Poco;
using namespace Poco::Util;
using namespace Poco::Net;

class IndigoRequestHandlerFactory: public HTTPRequestHandlerFactory
{
public:
	HTTPRequestHandler *createRequestHandler(const HTTPServerRequest &request)
	{
		return new IndigoRequestHandler();
	}
};

class ThreadPoolCollector: public Runnable
{
public:
	ThreadPoolCollector(ThreadPool &pool):
		pool(pool),
		stopCollection()
	{
	}

	void run()
	{
		while (!stopCollection.tryWait(1000))
		{
			pool.collect();
		}
	}

	void stopCollecting()
	{
		stopCollection.set();
	}

private:
	ThreadPool &pool;
	Event stopCollection;
};

class IndigoFiler: public ServerApplication
{
public:
	IndigoFiler(): helpRequested(false)
	{
	}

protected:
	void initialize(Application &self)
	{
		ServerApplication::initialize(self);
		loadConfiguration();
	}

	void uninitialize()
	{
		ServerApplication::uninitialize();
	}

	void defineOptions(OptionSet &options)
	{
		ServerApplication::defineOptions(options);
		options.addOption(
			Option("help", "h", "display help information on command line arguments")
				.required(false)
				.repeatable(false));
	}

	void handleOption(const string &name, const string &value)
	{
		ServerApplication::handleOption(name, value);

		if (name == "help")
			helpRequested = true;
	}

	void displayHelp()
	{
		HelpFormatter helpFormatter(options());
		helpFormatter.setCommand(commandName());
		helpFormatter.setUsage("OPTIONS");
		helpFormatter.setHeader("A simple web server for static content.");
		helpFormatter.format(cerr);
	}

	map<string, string> readShares()
	{
		const string sharesSection = "VirtualRoot";

		map<string, string> shares;

		AbstractConfiguration::Keys keys;
		config().keys(sharesSection, keys);

		for (size_t i = 0; i < keys.size(); i++)
		{
			const string &shareName = keys[i];
			if (shareName.empty())
				continue;

			const string &sharePath = config().getString(sharesSection + "." + shareName, "");
			if (sharePath.empty())
				continue;

			shares[shareName] = sharePath;
		}

		return shares;
	}

	map<string, string> readMimeTypes()
	{
		map<string, string> mimeTypes;
		// left blank for now
		return mimeTypes;
	}

	int main(const vector<string> &args)
	{
		if (helpRequested)
		{
			displayHelp();
		}
		else
		{
			const string serverSection = "Server";

			bool collectIdleThreads = (config().getString(serverSection + "." + "collectIdleThreads", "no") == "yes");

			string root = config().getString(serverSection + "." + "root", "virtual");
			if (root == "virtual")
				root = "";

			const IndigoConfiguration &configuration = IndigoConfiguration::init(
				config().getString(serverSection + "." + "name", ""),
				config().getString(serverSection + "." + "address", "0.0.0.0"),
				config().getInt(serverSection + "." + "port", 80),
				config().getInt(serverSection + "." + "backlog", 64),
				config().getInt(serverSection + "." + "minThreads", 2),
				config().getInt(serverSection + "." + "maxThreads", 16),
				config().getInt(serverSection + "." + "maxQueued", 64),
				collectIdleThreads,
				root,
				readShares(),
				readMimeTypes()
				);
			configuration.validate();

			HTTPServerParams::Ptr params = new HTTPServerParams;
			params->setMaxThreads(configuration.getMaxThreads());
			params->setMaxQueued(configuration.getMaxQueued());
			params->setServerName(configuration.getServerName());
			params->setSoftwareVersion(APP_NAME_WORD "/" APP_VERSION);

			HTTPRequestHandlerFactory::Ptr factory = new IndigoRequestHandlerFactory();

			ThreadPool pool(configuration.getMinThreads(), configuration.getMaxThreads());

			SocketAddress saddr(configuration.getAddress(), configuration.getPort());
			ServerSocket sock(saddr, configuration.getBacklog());

			auto_ptr<ThreadPoolCollector> collector(NULL);
			auto_ptr<Thread> collectorThread(NULL);
			if (configuration.getCollectIdleThreads())
			{
				collector.reset(new ThreadPoolCollector(pool));
				collectorThread.reset(new Thread());
				collectorThread->start(*collector);
			}

			HTTPServer srv(factory, pool, sock, params);

			srv.start();
			waitForTerminationRequest();
			srv.stop();

			if (collectorThread.get())
			{
				collector->stopCollecting();
				collectorThread->join();
			}
		}

		return EXIT_OK;
	}

private:
	bool helpRequested;
};

int main(int argc, char **argv)
{
	IndigoFiler indigo;
	return indigo.run(argc, argv);
}

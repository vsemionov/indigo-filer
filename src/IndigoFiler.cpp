
#include <string>
#include <vector>
#include <map>
#include <iostream>
#include <sstream>

#include "Poco/Util/ServerApplication.h"
#include "Poco/Net/HTTPServer.h"
#include "Poco/Util/HelpFormatter.h"

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
		// TODO: init mime types
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
			const IndigoConfiguration &configuration = IndigoConfiguration::init(
				config().getString("Server.name", ""),
				config().getString("Server.address", "0.0.0.0"),
				config().getInt("Server.port", 80),
				config().getInt("Server.backlog", 64),
				config().getInt("Server.minThreads", 2),
				config().getInt("Server.maxThreads", 16),
				config().getInt("Server.maxQueued", 64),
				readShares(),
				readMimeTypes()
				);
			configuration.validate();

			HTTPServerParams::Ptr params = new HTTPServerParams;
			params->setMaxThreads(configuration.getMaxThreads());
			params->setMaxQueued(configuration.getMaxQueued());
			params->setServerName(configuration.getServerName());
			params->setSoftwareVersion("IndigoFiler/0.1");

			HTTPRequestHandlerFactory::Ptr factory = new IndigoRequestHandlerFactory();

			ThreadPool pool(configuration.getMinThreads(), configuration.getMaxThreads());

			SocketAddress saddr(configuration.getAddress(), configuration.getPort());
			ServerSocket sock(saddr, configuration.getBacklog());

			HTTPServer srv(factory, pool, sock, params);

			srv.start();
			waitForTerminationRequest();
			srv.stop();
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

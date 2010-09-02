
/*
 * Copyright (C) 2010, Victor Semionov
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright notice,
 *       this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright notice,
 *       this list of conditions and the following disclaimer in the documentation
 *       and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
 * CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
 * OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 * USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
 * DAMAGE.
 */

#include <string>
#include <vector>
#include <map>
#include <iostream>

#include "Poco/Util/ServerApplication.h"
#include "Poco/Net/HTTPServer.h"
#include "Poco/String.h"
#include "Poco/FileStream.h"
#include "Poco/LineEndingConverter.h"
#include "Poco/StringTokenizer.h"
#include "Poco/Util/HelpFormatter.h"

#include "IndigoFiler.h"
#include "IndigoConfiguration.h"
#include "IndigoRequestHandler.h"
#include "ThreadPoolCollector.h"

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
	IndigoFiler(): helpRequested(false), versionRequested(false)
	{
	}

protected:
	void initialize(Application &self)
	{
		ServerApplication::initialize(self);

		string configPath = locateConfiguration(APP_NAME_UNIX "." "ini");
		loadConfiguration(configPath);
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

		options.addOption(
			Option("version", "v", "display version information")
				.required(false)
				.repeatable(false));
	}

	void handleOption(const string &name, const string &value)
	{
		ServerApplication::handleOption(name, value);

		if (name == "help")
			helpRequested = true;
		else if (name == "version")
			versionRequested = true;
	}

	int main(const vector<string> &args)
	{
		if (helpRequested)
		{
			displayHelp();
		}
		else if (versionRequested)
		{
			displayVersion();
		}
		else
		{
			const string serverSection = "Server";

			string collect = config().getString(serverSection + "." + "collectIdleThreads", "no");
			bool collectIdleThreads = (toLower(collect) == "yes");

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
			params->setSoftwareVersion(APP_NAME_SHORT "/" APP_VERSION);

			HTTPRequestHandlerFactory::Ptr factory = new IndigoRequestHandlerFactory();

			ThreadPool pool(configuration.getMinThreads(), configuration.getMaxThreads());

			SocketAddress saddr(configuration.getAddress(), configuration.getPort());
			ServerSocket sock(saddr, configuration.getBacklog());

			ThreadPoolCollector collector(pool);

			HTTPServer srv(factory, pool, sock, params);

			if (configuration.getCollectIdleThreads())
				collector.startCollecting();

			srv.start();

			waitForTerminationRequest();

			srv.stop();

			collector.stopCollecting();
		}

		return EXIT_OK;
	}

private:
	void displayHelp()
	{
		HelpFormatter helpFormatter(options());
		helpFormatter.setCommand(commandName());
		helpFormatter.setUsage("[options]");
		helpFormatter.setHeader("A simple web server for static content.");
		helpFormatter.format(cout);
	}

	void displayVersion()
	{
		cout << APP_NAME " " APP_VERSION << endl;
		cout << APP_COPYRIGHT_NOTICE;
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

	void readMimeTypes(string filename, map<string, string> &mimeTypes)
	{
		string filepath = locateConfiguration(filename);

		FileInputStream fin(filepath);
		InputLineEndingConverter in(fin, LineEnding::NEWLINE_LF);

		string line;
		while (getline(in, line))
		{
			line = line.substr(0, line.find('#'));

			StringTokenizer tok(line, " \t", StringTokenizer::TOK_IGNORE_EMPTY);
			const int cnt = tok.count();
			if (cnt >= 2)
			{
				const string &type = tok[0];

				for (int i = 1; i < cnt; i++)
				{
					const string &ext = tok[i];
					mimeTypes[ext] = type;
				}
			}
		}
	}

	map<string, string> readMimeTypes()
	{
		map<string, string> mimeTypes;

		readMimeTypes("mime.types", mimeTypes);
		readMimeTypes("mime.types.extra", mimeTypes);
		readMimeTypes("mime.types.user", mimeTypes);

		return mimeTypes;
	}

	string locateConfiguration(const string &filename)
	{
		string dir = config().getString("application.dir");
		Path path(dir);
		path.makeDirectory().setFileName(filename);
		return path.toString();
	}

	bool helpRequested;
	bool versionRequested;
};

int main(int argc, char **argv)
{
	IndigoFiler indigo;
	return indigo.run(argc, argv);
}

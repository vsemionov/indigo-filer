
#include <climits>

#include <string>
#include <vector>
#include <set>
#include <ostream>
#include <sstream>

#include "Poco/Util/ServerApplication.h"
#include "Poco/File.h"
#include "Poco/DirectoryIterator.h"

#include "IndigoRequestHandler.h"
#include "IndigoConfiguration.h"

using namespace std;

using namespace Poco;
using namespace Poco::Util;
using namespace Poco::Net;

IndigoRequestHandler::IndigoRequestHandler()
{
}

void IndigoRequestHandler::handleRequest(HTTPServerRequest &request, HTTPServerResponse &response)
{
	Path uriPath;
	const IndigoConfiguration &configuration = IndigoConfiguration::get();

	bool loggable;
	bool goodRequest = isGoodRequest(request, &loggable, &uriPath);

	logRequest(request, loggable);

	if (!goodRequest)
	{
		sendBadRequest(response);
		return;
	}

	if (uriPath.isDirectory() && uriPath.depth() == 0)
	{
		sendRootDirectory(response);
		return;
	}

	const string &shareName = uriPath[0];
	const string &sharePath = configuration.getSharePath(shareName);
	if (sharePath.empty())
	{
		sendNotFound(response);
		return;
	}

	Path fsPath = resolveFsPath(uriPath, sharePath);
	string target = fsPath.toString();

	try
	{
		File f(target);
		if (f.isDirectory())
		{
			if (uriPath.isDirectory())
			{
				sendDirectory(response, target, uriPath.toString(Path::PATH_UNIX));
			}
			else
			{
				Path uriDirPath = uriPath;
				directorize(uriDirPath);
				redirectToDirectory(response, uriDirPath.toString(Path::PATH_UNIX));
			}
		}
		else
		{
			string ext = fsPath.getExtension();
			string mediaType = configuration.getMimeType(ext);
			response.sendFile(target, mediaType);
		}
	}
	catch (FileNotFoundException &fnfe)
	{
		sendNotFound(response);
	}
	catch (FileAccessDeniedException &fade)
	{
		sendForbidden(response);
	}
}

void IndigoRequestHandler::directorize(Path &path)
{
	if (path.isFile())
	{
		string fileName = path.getFileName();
		path = path.parent();
		path.pushDirectory(fileName);
	}
}

void IndigoRequestHandler::filize(Path &path)
{
	directorize(path);

	if (path.depth() > 0)
	{
		string fileName = path[path.depth() - 1];
		path.popDirectory();
		path.setFileName(fileName);
	}
}

Path IndigoRequestHandler::resolveFsPath(const Path &uriPath, const string &sharePath)
{
	Path fsPath(sharePath);

	directorize(fsPath);

	for (int d = 1; d <= uriPath.depth(); d++)
	{
		fsPath.pushDirectory(uriPath[d]);
	}

	filize(fsPath);

	return fsPath;
}

void IndigoRequestHandler::sendDirectoryListing(HTTPServerResponse &response, const string &dirURI, const vector<string> &entries, bool root)
{
	ostream &out = response.send();

	out << "<html>" << endl;
	out << "<head>" << endl;
	out << "<title>";
	out << "Index of " << dirURI;
	out << "</title>" << endl;
	out << "</head>" << endl;
	out << "<body>" << endl;
	out << "<h1>";
	out << "Index of " << dirURI;
	out << "</h1>" << endl;

	if (!root)
	{
		out << "<a href=\"../\">..</a><br>" << endl;
	}

	int l = entries.size();
	for (int i = 0; i < l; i++)
	{
		out << "<a href=\"" << entries[i] << "\">" << entries[i] << "</a>" << "<br>" << endl;
	}

	out << "</body>" << endl;
	out << "</html>" << endl;
}

void IndigoRequestHandler::sendRootDirectory(HTTPServerResponse &response)
{
	const IndigoConfiguration &configuration = IndigoConfiguration::get();
	const set<string> &shares = configuration.getShares();
	vector<string> entries;
	set<string>::const_iterator it;
	set<string>::const_iterator end = shares.end();
	for (it = shares.begin(); it != end; ++it)
	{
		const string &shareName = *it;
		const string &sharePath = configuration.getSharePath(shareName);
		if (!sharePath.empty())
		{
			try
			{
				Path fsPath = resolveFsPath(Path("/", Path::PATH_UNIX), sharePath);
				File f(fsPath);
				string entry = shareName;
				if (f.isDirectory())
					entry += '/';
				entries.push_back(entry);
			}
			catch (FileException &fe)
			{
			}
		}
	}

	sendDirectoryListing(response, "/", entries, true);
}

void IndigoRequestHandler::sendDirectory(HTTPServerResponse &response, const string &path, const string &dirURI)
{
	vector<string> entries;
	DirectoryIterator it(path);
	DirectoryIterator end;
	while (it != end)
	{
		string entry = it.name();
		if (it->isDirectory())
			entry += '/';
		entries.push_back(entry);
		++it;
	}

	sendDirectoryListing(response, dirURI, entries, false);
}

void IndigoRequestHandler::redirectToDirectory(HTTPServerResponse &response, const string &dirURI)
{
	// Not done with response.redirect(dirURI)
	// because that would send a 302 found, instead of 301 moved permanently

	response.setStatusAndReason(HTTPResponse::HTTP_MOVED_PERMANENTLY);
	response.setContentLength(0);
	response.setChunkedTransferEncoding(false);
	response.set("Location", dirURI);
	response.send();
}

bool IndigoRequestHandler::isGoodRequest(const HTTPServerRequest &request, bool *loggable, Path *uriPath)
{
	const string &method = request.getMethod();
	const string &uri = request.getURI();

	bool good = true;
	*loggable = true;

	if (method != "GET")
	{
		good = false;
		if (method.length() > 32)
			*loggable = false;
	}

	if (uri.length() > 1024)
	{
		good =  false;
		*loggable = false;
	}
	else
	{
		uriPath->assign(uri, Path::PATH_UNIX);
		if (!uriPath->isAbsolute())
			good = false;
	}

	return good;
}

void IndigoRequestHandler::logRequest(const HTTPServerRequest &request, bool loggable)
{
	ServerApplication &app = dynamic_cast<ServerApplication &>(Application::instance());
	if (app.isInteractive())
	{
		const string &method = request.getMethod();
		const string &uri = request.getURI();
		string host = request.clientAddress().host().toString();

		string logString;
		if (loggable)
			logString = host + " - " + method + " " + uri;
		else
			logString = "Bad request from " + host;

		app.logger().information(logString);
	}
}

void IndigoRequestHandler::sendError(HTTPServerResponse &response, int code, const string &msg)
{
	response.setStatusAndReason(HTTPResponse::HTTPStatus(code));
	response.setChunkedTransferEncoding(true);
	response.setContentType("text/html");

	ostringstream ostr;
	ostr << code;

	ostream &out = response.send();
	out << "<html>";
	out << "<head><title>" + ostr.str() + " " + msg + "</title></head>";
	out << "<body><h1>" + msg + "</h1></body>";
	out << "</html>";
}

void IndigoRequestHandler::sendBadRequest(HTTPServerResponse &response)
{
	sendError(response, HTTPResponse::HTTP_BAD_REQUEST, "Bad Request");
}

void IndigoRequestHandler::sendNotImplemented(HTTPServerResponse &response)
{
	sendError(response, HTTPResponse::HTTP_NOT_IMPLEMENTED, "Not Implemented");
}

void IndigoRequestHandler::sendNotFound(HTTPServerResponse &response)
{
	sendError(response, HTTPResponse::HTTP_NOT_FOUND, "Not Found");
}

void IndigoRequestHandler::sendForbidden(HTTPServerResponse &response)
{
	sendError(response, HTTPResponse::HTTP_FORBIDDEN, "Forbidden");
}

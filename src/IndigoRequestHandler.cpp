
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

POCO_DECLARE_EXCEPTION(, ShareNotFoundException, ApplicationException)
POCO_IMPLEMENT_EXCEPTION(ShareNotFoundException, ApplicationException, "ShareNotFoundException")

IndigoRequestHandler::IndigoRequestHandler()
{
}

void IndigoRequestHandler::handleRequest(HTTPServerRequest &request, HTTPServerResponse &response)
{
	const IndigoConfiguration &configuration = IndigoConfiguration::get();

	const string &method = request.getMethod();
	const string &uri = request.getURI();

	Path uriPath;

	bool process = true;
	bool loggable = true;

	if (method != "GET")
	{
		process = false;
		if (method.length() > 32)
			loggable = false;
		sendMethodNotAllowed(response);
	}

	if (uri.length() > 256)
	{
		loggable = false;
		if (uri.length() > 4096)
		{
			process = false;
			sendRequestURITooLong(response);
		}
	}

	if (process)
	{
		uriPath.assign(uri, Path::PATH_UNIX);
		if (!uriPath.isAbsolute())
		{
			process = false;
			sendBadRequest(response);
		}
	}

	logRequest(request, loggable);

	if (!process)
		return;

	if (uriPath.isDirectory() && uriPath.depth() == 0)
	{
		if (configuration.getRoot().empty())
		{
			sendVirtualRootDirectory(response);
			return;
		}
	}

	try
	{
		const Path &fsPath = resolveFSPath(uriPath);
		const string &target = fsPath.toString();

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
				uriDirPath.makeDirectory();
				redirectToDirectory(response, uriDirPath.toString(Path::PATH_UNIX), false);
			}
		}
		else
		{
			const string &ext = fsPath.getExtension();
			const string &mediaType = configuration.getMimeType(ext);
			response.sendFile(target, mediaType);
		}
	}
	catch (ShareNotFoundException &snfe)
	{
		sendNotFound(response);
	}
	catch (FileNotFoundException &fnfe)
	{
		sendNotFound(response);
	}
	catch (FileAccessDeniedException &fade)
	{
		sendForbidden(response);
	}
	catch (FileException &fe)
	{
		sendInternalServerError(response);
	}
	catch (PathSyntaxException &pse)
	{
		sendNotImplemented(response);
	}
	catch (...)
	{
		sendInternalServerError(response);
	}
}

Path IndigoRequestHandler::resolveFSPath(const Path &uriPath)
{
	const IndigoConfiguration &configuration = IndigoConfiguration::get();

	const string &shareName = uriPath[0];
	string base = configuration.getSharePath(shareName);
	bool share = true;

	if (base.empty())
	{
		base = configuration.getRoot();
		if (base.empty())
			throw ShareNotFoundException();

		share = false;
	}

	Path fsPath(base);

	if (share)
	{
		fsPath.makeDirectory();

		const int d = uriPath.depth();
		for (int i = 1; i <= d; i++)
			fsPath.pushDirectory(uriPath[i]);
	}
	else
	{
		fsPath.append(uriPath);
	}

	fsPath.makeFile();
	return fsPath;
}

void IndigoRequestHandler::sendDirectoryListing(HTTPServerResponse &response, const string &dirURI, const vector<string> &entries)
{
	bool root = (dirURI == "/");

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
		out << "<a href=\"../\">&lt;Parent Directory&gt;</a><br>" << endl;
	}

	const int l = entries.size();
	for (int i = 0; i < l; i++)
	{
		out << "<a href=\"" << entries[i] << "\">" << entries[i] << "</a>" << "<br>" << endl;
	}

	out << "</body>" << endl;
	out << "</html>" << endl;
}

void IndigoRequestHandler::sendVirtualRootDirectory(HTTPServerResponse &response)
{
	const IndigoConfiguration &configuration = IndigoConfiguration::get();
	const set<string> &shares = configuration.getShares();

	vector<string> entries;

	set<string>::const_iterator it;
	const set<string>::const_iterator &end = shares.end();
	for (it = shares.begin(); it != end; ++it)
	{
		const string &shareName = *it;
		try
		{
			const Path &fsPath = resolveFSPath(Path("/" + shareName, Path::PATH_UNIX));
			File f(fsPath);

			if (!f.isHidden())
			{
				string entry = shareName;
				if (f.isDirectory())
					entry += '/';

				entries.push_back(entry);
			}
		}
		catch (ShareNotFoundException &snfe)
		{
		}
		catch (FileException &fe)
		{
		}
		catch (PathSyntaxException &pse)
		{
		}
	}

	sendDirectoryListing(response, "/", entries);
}

void IndigoRequestHandler::sendDirectory(HTTPServerResponse &response, const string &path, const string &dirURI)
{
	vector<string> entries;

	DirectoryIterator it(path);
	DirectoryIterator end;
	while (it != end)
	{
		try
		{
			if (!it->isHidden())
			{
				string entry = it.name();
				if (it->isDirectory())
					entry += '/';

				entries.push_back(entry);
			}
		}
		catch (FileException &fe)
		{
		}
		catch (PathSyntaxException &pse)
		{
		}

		++it;
	}

	sendDirectoryListing(response, dirURI, entries);
}

void IndigoRequestHandler::redirectToDirectory(HTTPServerResponse &response, const string &dirURI, bool permanent)
{
	if (!permanent)
	{
		response.redirect(dirURI);
	}
	else
	{
		response.setStatusAndReason(HTTPResponse::HTTP_MOVED_PERMANENTLY);
		response.setContentLength(0);
		response.setChunkedTransferEncoding(false);
		response.set("Location", dirURI);
		response.send();
	}
}

void IndigoRequestHandler::logRequest(const HTTPServerRequest &request, bool loggable)
{
	const ServerApplication &app = dynamic_cast<ServerApplication &>(Application::instance());
	if (app.isInteractive())
	{
		const string &method = request.getMethod();
		const string &uri = request.getURI();
		const string &host = request.clientAddress().host().toString();

		string logString;
		if (loggable)
			logString = host + " - " + method + " " + uri;
		else
			logString = "Request from " + host;

		app.logger().information(logString);
	}
}

void IndigoRequestHandler::sendError(HTTPServerResponse &response, int code)
{
	if (response.sent())
		return;

	response.setStatusAndReason(HTTPResponse::HTTPStatus(code));
	response.setChunkedTransferEncoding(true);
	response.setContentType("text/html");

	const string &reason = response.getReason();

	ostringstream ostr;
	ostr << code;

	ostream &out = response.send();
	out << "<html>";
	out << "<head><title>" + ostr.str() + " " + reason + "</title></head>";
	out << "<body><h1>" + reason + "</h1></body>";
	out << "</html>";
}

void IndigoRequestHandler::sendMethodNotAllowed(HTTPServerResponse &response)
{
	sendError(response, HTTPResponse::HTTP_METHOD_NOT_ALLOWED);
}

void IndigoRequestHandler::sendRequestURITooLong(HTTPServerResponse &response)
{
	sendError(response, HTTPResponse::HTTP_REQUESTURITOOLONG);
}

void IndigoRequestHandler::sendBadRequest(HTTPServerResponse &response)
{
	sendError(response, HTTPResponse::HTTP_BAD_REQUEST);
}

void IndigoRequestHandler::sendNotImplemented(HTTPServerResponse &response)
{
	sendError(response, HTTPResponse::HTTP_NOT_IMPLEMENTED);
}

void IndigoRequestHandler::sendNotFound(HTTPServerResponse &response)
{
	sendError(response, HTTPResponse::HTTP_NOT_FOUND);
}

void IndigoRequestHandler::sendForbidden(HTTPServerResponse &response)
{
	sendError(response, HTTPResponse::HTTP_FORBIDDEN);
}

void IndigoRequestHandler::sendInternalServerError(HTTPServerResponse &response)
{
	sendError(response, HTTPResponse::HTTP_INTERNAL_SERVER_ERROR);
}


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

#include <climits>

#include <string>
#include <vector>
#include <set>
#include <ostream>
#include <iostream>

#include "Poco/Util/ServerApplication.h"
#include "Poco/URI.h"
#include "Poco/File.h"
#include "Poco/DirectoryIterator.h"
#include "Poco/NumberFormatter.h"

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
	logRequest(request);

	const string &method = request.getMethod();

	if (method != HTTPRequest::HTTP_GET)
	{
		sendMethodNotAllowed(response);
		return;
	}

	URI uri;
	try
	{
		uri = request.getURI();
	}
	catch (SyntaxException &se)
	{
		sendBadRequest(response);
		return;
	}

	const string processedURI = uri.getPath();
	const Path uriPath(processedURI, Path::PATH_UNIX);

	if (!uriPath.isAbsolute())
	{
		sendBadRequest(response);
		return;
	}

	const IndigoConfiguration &configuration = IndigoConfiguration::get();

	try
	{
		if (uriPath.isDirectory() && uriPath.depth() == 0)
		{
			if (configuration.virtualRoot())
			{
				sendVirtualIndex(response);
				return;
			}
		}

		const Path fsPath = resolveFSPath(uriPath);
		const string target = fsPath.toString();

		File f(target);

		if (uriPath.isDirectory())
		{
			if (f.isDirectory())
			{
				sendDirectoryIndex(response, target, processedURI);
			}
			else
			{
				sendNotFound(response);
			}
		}
		else
		{
			if (f.isDirectory())
			{
				Path uriDirPath = uriPath;
				uriDirPath.makeDirectory();
				redirectToDirectory(response, uriDirPath.toString(Path::PATH_UNIX), false);
			}
			else
			{
				sendFile(response, fsPath);
			}
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

		int d = uriPath.depth();
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

void IndigoRequestHandler::sendFile(HTTPServerResponse &response, const Path &path)
{
	string ext = path.getExtension();
	const string &mediaType = IndigoConfiguration::get().getMimeType(ext);
	response.sendFile(path.toString(), mediaType);
}

void IndigoRequestHandler::sendFile(HTTPServerResponse &response, const string &path)
{
	string ext = Path(path).getExtension();
	const string &mediaType = IndigoConfiguration::get().getMimeType(ext);
	response.sendFile(path, mediaType);
}

void IndigoRequestHandler::sendDirectoryListing(HTTPServerResponse &response, const string &uri, const vector<string> &entries)
{
	bool root = (uri == "/");

	response.setContentType("text/html");
	response.setContentLength(HTTPResponse::UNKNOWN_CONTENT_LENGTH);
	response.setChunkedTransferEncoding(true);

	ostream &out = response.send();

	out << "<html>" << endl;
	out << "<head>" << endl;
	out << "<title>";
	out << "Index of " << uri;
	out << "</title>" << endl;
	out << "</head>" << endl;
	out << "<body>" << endl;
	out << "<h1>";
	out << "Index of " << uri;
	out << "</h1>" << endl;

	if (!root)
	{
		out << "<a href=\"../\">&lt;Parent Directory&gt;</a><br>" << endl;
	}

	int l = entries.size();
	for (int i = 0; i < l; i++)
	{
		out << "<a href=\"" << entries[i] << "\">" << entries[i] << "</a>" << "<br>" << endl;
	}

	out << "</body>" << endl;
	out << "</html>" << endl;
}

Path IndigoRequestHandler::findVirtualIndex()
{
	const IndigoConfiguration &configuration = IndigoConfiguration::get();

	const set<string> &indexes = configuration.getIndexes();

	set<string>::const_iterator it;
	set<string>::const_iterator end = indexes.end();
	for (it = indexes.begin(); it != end; ++it)
	{
		try
		{
			Path indexURI = Path('/' + *it, Path::PATH_UNIX);
			Path index = resolveFSPath(indexURI);
			File f(index);
			if (f.isFile())
			{
				return index;
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

	return Path(false);
}

void IndigoRequestHandler::sendVirtualIndex(HTTPServerResponse &response)
{
	const IndigoConfiguration &configuration = IndigoConfiguration::get();

	Path index = findVirtualIndex();
	if (index.isAbsolute())
	{
		sendFile(response, index);
		return;
	}

	if (!configuration.getAutoIndex())
		throw ShareNotFoundException();

	const set<string> &shares = configuration.getShares();
	vector<string> entries;

	set<string>::const_iterator it;
	set<string>::const_iterator end = shares.end();
	for (it = shares.begin(); it != end; ++it)
	{
		const string &shareName = *it;
		try
		{
			Path shareURI = Path('/' + shareName, Path::PATH_UNIX);
			Path fsPath = resolveFSPath(shareURI);
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

string IndigoRequestHandler::findDirectoryIndex(const string &base)
{
	const IndigoConfiguration &configuration = IndigoConfiguration::get();

	const set<string> &indexes = configuration.getIndexes(true);

	set<string>::const_iterator it;
	set<string>::const_iterator end = indexes.end();
	for (it = indexes.begin(); it != end; ++it)
	{
		try
		{
			string index = base;
			if (index[index.length() - 1] != Path::separator())
				index += Path::separator();
			index += *it;

			File f(index);
			if (f.isFile())
			{
				return index;
			}
		}
		catch (FileException &fe)
		{
		}
		catch (PathSyntaxException &pse)
		{
		}
	}

	return "";
}

void IndigoRequestHandler::sendDirectoryIndex(HTTPServerResponse &response, const string &path, const string &uri)
{
	const IndigoConfiguration &configuration = IndigoConfiguration::get();

	string index = findDirectoryIndex(path);
	if (!index.empty())
	{
		sendFile(response, index);
		return;
	}

	if (!configuration.getAutoIndex())
		throw FileNotFoundException();

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

	sendDirectoryListing(response, uri, entries);
}

void IndigoRequestHandler::redirectToDirectory(HTTPServerResponse &response, const string &uri, bool permanent)
{
	if (!permanent)
	{
		response.redirect(uri);
	}
	else
	{
		response.setStatusAndReason(HTTPResponse::HTTP_MOVED_PERMANENTLY);
		response.setContentLength(0);
		response.setChunkedTransferEncoding(false);
		response.set("Location", uri);
		response.send();
	}
}

void IndigoRequestHandler::logRequest(const HTTPServerRequest &request)
{
	const ServerApplication &app = dynamic_cast<ServerApplication &>(Application::instance());
	if (app.isInteractive())
	{
		const string &method = request.getMethod();
		const string &uri = request.getURI();
		string host = request.clientAddress().host().toString();

		string logString = host + " - " + method + " " + uri;

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

	ostream &out = response.send();
	out << "<html>";
	out << "<head><title>" + NumberFormatter::format(code) + " " + reason + "</title></head>";
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

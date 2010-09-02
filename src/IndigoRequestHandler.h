
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

#ifndef INDIGOREQUESTHANDLER_H
#define INDIGOREQUESTHANDLER_H

#include "Poco/Net/HTTPRequestHandler.h"
#include "Poco/Net/HTTPServerRequest.h"
#include "Poco/Net/HTTPServerResponse.h"
#include "Poco/Path.h"

using namespace std;

using namespace Poco;
using namespace Poco::Net;

class IndigoRequestHandler: public HTTPRequestHandler
{
public:
	IndigoRequestHandler();

	void handleRequest(HTTPServerRequest &request, HTTPServerResponse &response);

private:
	Path resolveFSPath(const Path &uriPath);
	void sendDirectoryListing(HTTPServerResponse &response, const string &dirURI, const vector<string> &entries);
	void sendVirtualRootDirectoryIndex(HTTPServerResponse &response);
	void sendDirectoryIndex(HTTPServerResponse &response, const string &path, const string &dirURI);
	void redirectToDirectory(HTTPServerResponse &response, const string &dirURI, bool permanent);
	void logRequest(const HTTPServerRequest &request);
	void sendError(HTTPServerResponse &response, int code);
	void sendMethodNotAllowed(HTTPServerResponse &response);
	void sendRequestURITooLong(HTTPServerResponse &response);
	void sendBadRequest(HTTPServerResponse &response);
	void sendNotImplemented(HTTPServerResponse &response);
	void sendNotFound(HTTPServerResponse &response);
	void sendForbidden(HTTPServerResponse &response);
	void sendInternalServerError(HTTPServerResponse &response);
};

#endif //INDIGOREQUESTHANDLER_H

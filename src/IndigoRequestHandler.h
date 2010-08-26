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
	void directorize(Path &path);
	void filize(Path &path);
	Path resolveFSPath(const Path &uriPath);
	void sendDirectoryListing(HTTPServerResponse &response, const string &dirURI, const vector<string> &entries, bool root);
	void sendRootDirectory(HTTPServerResponse &response);
	void sendDirectory(HTTPServerResponse &response, const string &path, const string &dirURI);
	void redirectToDirectory(HTTPServerResponse &response, const string &dirURI, bool permanent);
	bool isGoodRequest(const HTTPServerRequest &request, bool *loggable, Path *uriPath);
	void logRequest(const HTTPServerRequest &request, bool loggable);
	void sendError(HTTPServerResponse &response, int code, const string &msg);
	void sendBadRequest(HTTPServerResponse &response);
	void sendNotImplemented(HTTPServerResponse &response);
	void sendNotFound(HTTPServerResponse &response);
	void sendForbidden(HTTPServerResponse &response);
};

#endif //INDIGOREQUESTHANDLER_H

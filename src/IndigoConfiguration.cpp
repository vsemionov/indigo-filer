
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
#include <algorithm>

#include <Poco/Path.h>
#include <Poco/Exception.h>
#include <Poco/String.h>

#include "IndigoConfiguration.h"

using namespace std;

using namespace Poco;

IndigoConfiguration *IndigoConfiguration::singleton = NULL;

const string IndigoConfiguration::defaultPath = "";
const string IndigoConfiguration::defaultMimeType = "application/octet-stream";

const IndigoConfiguration &IndigoConfiguration::init(
		const string &serverName,
		const string &address,
		int port,
		int backlog,
		int minThreads,
		int maxThreads,
		int maxQueued,
		int timeout,
		bool keepalive,
		int keepaliveTimeout,
		int maxKeepaliveRequests,
		int idleTime,
		int threadIdleTime,
		bool collectIdleThreads,
		const string &root,
		const vector<string> &indexes,
		bool autoIndex,
		const unordered_map<string, string> &shares,
		const unordered_map<string, string> &mimeTypes
		)
{
	poco_assert(singleton == NULL);

	singleton = new IndigoConfiguration(
		serverName,
		address,
		port,
		backlog,
		minThreads,
		maxThreads,
		maxQueued,
		timeout,
		keepalive,
		keepaliveTimeout,
		maxKeepaliveRequests,
		idleTime,
		threadIdleTime,
		collectIdleThreads,
		root,
		indexes,
		autoIndex,
		shares,
		mimeTypes
		);

	return get();
}

const IndigoConfiguration &IndigoConfiguration::get()
{
	poco_assert(singleton != NULL);

	return *singleton;
}

IndigoConfiguration::IndigoConfiguration(
	const string &serverName,
	const string &address,
	int port,
	int backlog,
	int minThreads,
	int maxThreads,
	int maxQueued,
	int timeout,
	bool keepalive,
	int keepaliveTimeout,
	int maxKeepaliveRequests,
	int idleTime,
	int threadIdleTime,
	bool collectIdleThreads,
	const string &root,
	const vector<string> &indexes,
	bool autoIndex,
	const unordered_map<string, string> &shares,
	const unordered_map<string, string> &mimeTypes
	):
		serverName(serverName),
		address(address),
		port(port),
		backlog(backlog),
		minThreads(minThreads),
		maxThreads(maxThreads),
		maxQueued(maxQueued),
		timeout(timeout),
		keepalive(keepalive),
		keepaliveTimeout(keepaliveTimeout),
		maxKeepaliveRequests(maxKeepaliveRequests),
		idleTime(idleTime),
		threadIdleTime(threadIdleTime),
		collectIdleThreads(collectIdleThreads),
		root(root),
		indexes(indexes),
		indexesNative(),
		autoIndex(autoIndex),
		shares(shares),
		mimeTypes(mimeTypes),
		shareVec()
{
	for (unordered_map<string, string>::const_iterator it = shares.begin(); it != shares.end(); ++it)
	{
		shareVec.push_back(it->first);
	}
	sort(shareVec.begin(), shareVec.end());

	for (vector<string>::const_iterator it = indexes.begin(); it != indexes.end(); ++it)
	{
		string index = *it;
		replaceInPlace(index, string("/"), string(1, Path::separator()));
		indexesNative.push_back(index);
	}
}

void IndigoConfiguration::validate() const
{
	if (!root.empty())
	{
		Path p(root);
		if (!p.isAbsolute())
			throw ApplicationException("\"" + root + "\" is not an absolute path");
	}

	for (unordered_map<string, string>::const_iterator it = shares.begin(); it != shares.end(); ++it)
	{
		const string &shareName = it->first;
		const string &sharePath = it->second;
		Path p;

		string uri = '/' + shareName + '/';
		p.assign(uri, Path::PATH_UNIX);
		string resolved = p.toString(Path::PATH_UNIX);
		if (resolved != uri || p.depth() != 1)
			throw ApplicationException("\"" + shareName + "\" is not a valid share name");

		p.assign(sharePath);
		if (!p.isAbsolute())
			throw ApplicationException("\"" + sharePath + "\" is not an absolute path");
	}
}

const string &IndigoConfiguration::getServerName() const
{
	return serverName;
}

const string &IndigoConfiguration::getAddress() const
{
	return address;
}

int IndigoConfiguration::getPort() const
{
	return port;
}

int IndigoConfiguration::getBacklog() const
{
	return backlog;
}

int IndigoConfiguration::getMinThreads() const
{
	return minThreads;
}

int IndigoConfiguration::getMaxThreads() const
{
	return maxThreads;
}

int IndigoConfiguration::getMaxQueued() const
{
	return maxQueued;
}

int IndigoConfiguration::getTimeout() const
{
	return timeout;
}

bool IndigoConfiguration::getKeepalive() const
{
	return keepalive;
}

int IndigoConfiguration::getKeepaliveTimeout() const
{
	return keepaliveTimeout;
}

int IndigoConfiguration::getMaxKeepaliveRequests() const
{
	return maxKeepaliveRequests;
}

int IndigoConfiguration::getIdleTime() const
{
	return idleTime;
}

int IndigoConfiguration::getThreadIdleTime() const
{
	return threadIdleTime;
}

bool IndigoConfiguration::getCollectIdleThreads() const
{
	return collectIdleThreads;
}

const string &IndigoConfiguration::getRoot() const
{
	return root;
}

const vector<string> &IndigoConfiguration::getIndexes(bool native) const
{
	if (!native)
		return indexes;
	else
		return indexesNative;
}

bool IndigoConfiguration::getAutoIndex() const
{
	return autoIndex;
}

const vector<string> &IndigoConfiguration::getShares() const
{
	return shareVec;
}

const string &IndigoConfiguration::getSharePath(const string &share) const
{
	unordered_map<string, string>::const_iterator it = shares.find(share);
	if (it != shares.end())
		return it->second;
	else
		return defaultPath;
}

const string &IndigoConfiguration::getMimeType(const string &extension) const
{
	unordered_map<string, string>::const_iterator it = mimeTypes.find(extension);
	if (it != mimeTypes.end())
		return it->second;
	else
		return defaultMimeType;
}

bool IndigoConfiguration::virtualRoot() const
{
	return getRoot().empty();
}


/*
 * Copyright (c) 2010, Victor Semionov
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
#include <set>
#include <map>

#include <Poco/Path.h>
#include <Poco/Exception.h>

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
		bool collectIdleThreads,
		const string &root,
		const map<string, string> &shares,
		const map<string, string> &mimeTypes
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
		collectIdleThreads,
		root,
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
	bool collectIdleThreads,
	const string &root,
	const map<string, string> &shares,
	const map<string, string> &mimeTypes
	):
		serverName(serverName),
		address(address),
		port(port),
		backlog(backlog),
		minThreads(minThreads),
		maxThreads(maxThreads),
		maxQueued(maxQueued),
		collectIdleThreads(collectIdleThreads),
		root(root),
		shares(shares),
		mimeTypes(mimeTypes),
		sharesSet()
{
	map<string, string>::const_iterator it;
	for (it = shares.begin(); it != shares.end(); ++it)
	{
		sharesSet.insert(it->first);
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

	map<string, string>::const_iterator it;
	for (it = shares.begin(); it != shares.end(); ++it)
	{
		Path p(it->second);
		if (!p.isAbsolute())
			throw ApplicationException("\"" + it->second + "\" is not an absolute path");
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

bool IndigoConfiguration::getCollectIdleThreads() const
{
	return collectIdleThreads;
}

const string &IndigoConfiguration::getRoot() const
{
	return root;
}

const set<string> &IndigoConfiguration::getShares() const
{
	return sharesSet;
}

const string &IndigoConfiguration::getSharePath(const string &share) const
{
	const map<string, string>::const_iterator &it = shares.find(share);
	if (it != shares.end())
		return it->second;
	else
		return defaultPath;
}

const string &IndigoConfiguration::getMimeType(const string &extension) const
{
	const map<string, string>::const_iterator &it = mimeTypes.find(extension);
	if (it != mimeTypes.end())
		return it->second;
	else
		return defaultMimeType;
}

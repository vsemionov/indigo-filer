
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

#ifndef INDIGOCONFIGURATION_H
#define INDIGOCONFIGURATION_H

#include <string>
#include <vector>
#include <set>
#include <map>

using namespace std;

class IndigoConfiguration
{
public:
	static const IndigoConfiguration &init(
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
		const set<string> &indexes,
		bool autoIndex,
		const map<string, string> &shares,
		const map<string, string> &mimeTypes
		);
	static const IndigoConfiguration &get();

	void validate() const;

	const string &getServerName() const;
	const string &getAddress() const;
	int getPort() const;
	int getBacklog() const;
	int getMinThreads() const;
	int getMaxThreads() const;
	int getMaxQueued() const;
	int getTimeout() const;
	bool getKeepalive() const;
	int getKeepaliveTimeout() const;
	int getMaxKeepaliveRequests() const;
	int getIdleTime() const;
	int getThreadIdleTime() const;
	bool getCollectIdleThreads() const;
	const string &getRoot() const;
	const set<string> &getIndexes(bool native = false) const;
	bool getAutoIndex() const;
	const set<string> &getShares() const;
	const string &getSharePath(const string &share) const;
	const string &getMimeType(const string &extension) const;
	bool virtualRoot() const;

private:
	IndigoConfiguration(
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
		const set<string> &indexes,
		bool autoIndex,
		const map<string, string> &shares,
		const map<string, string> &mimeTypes
		);

	static IndigoConfiguration *singleton;

	const string serverName;
	const string address;
	const int port;
	const int backlog;
	const int minThreads;
	const int maxThreads;
	const int maxQueued;
	const int timeout;
	const bool keepalive;
	const int keepaliveTimeout;
	const int maxKeepaliveRequests;
	const int idleTime;
	const int threadIdleTime;
	const bool collectIdleThreads;
	const string root;
	const set<string> indexes;
	set<string> indexesNative;
	const bool autoIndex;
	const map<string, string> shares;
	const map<string, string> mimeTypes;

	set<string> sharesSet;

	static const string defaultPath;
	static const string defaultMimeType;
};

#endif //INDIGOCONFIGURATION_H

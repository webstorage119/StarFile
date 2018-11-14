/**************************************************************************
  Some wrapper functions for sftp operations:readdir,read,write,delete,etc.
  It is based on libssh 2.1.8.

  All sftp operations are non-block.


    Created : baixiangcpp@gmail.com
    Date    : 2018/10/28
    Website : http://ilovecpp.com

**************************************************************************/

#pragma once

#include "libssh2_config.h"

#ifdef HAVE_WINSOCK2_H
#include <winsock2.h>
#include <WS2tcpip.h>
#endif
#ifdef HAVE_SYS_SOCKET_H
# include <sys/socket.h>
#endif
#ifdef HAVE_NETINET_IN_H
# include <netinet/in.h>
#endif
#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif
#ifdef HAVE_ARPA_INET_H
# include <arpa/inet.h>
#endif
#ifdef HAVE_INTTYPES_H
# include <inttypes.h>
#endif

#include <cstdio>
#include <vector>
#include <memory>
#include <string>
#include <libssh2.h>
#include <libssh2_sftp.h>
#include "Utils.h"

namespace LIBSSH
{

	class Session
	{
		//void init_libssh2()
		//{
		//	std::lock_guard<std::mutex> guard(mutex_);
		//	if (libssh2_init(0) != 0)
		//		on_error("libssh2_init");
		//}

	public:

		Session();

		~Session();

		//only passwd authentication method is available
		void Login(const char* name, const char* passwd);

	protected:
    
        SOCKET           sock_;

		LIBSSH2_SESSION* session_;

		void Open(const char* host, int port);

		void on_error(const char* str)
		{
            //TODO: error function,not exit
			fprintf(stderr, "%s failed with error\n", str);
			exit(-1);
		}
	};

	class SSHItem
	{
	public:
		SSHItem(CONST CHAR* perm,const char* name, bool folder, unsigned long mtime, libssh2_uint64_t size)
			:isfolder(folder), mtime(mtime), filesize(size)
		{
			LPWSTR tmpwstr = UTF8ToUnicode(name);
			this->name = tmpwstr;
			CoTaskMemFree(tmpwstr);

            tmpwstr = UTF8ToUnicode(perm);
            this->perm = tmpwstr;
            this->perm = this->perm.substr(1,10);
            CoTaskMemFree(tmpwstr);

		}

		SSHItem(const SSHItem& rdir)
		{
			this->name = rdir.name;
			this->isfolder = rdir.isfolder;
			this->mtime = rdir.mtime;
			this->filesize = rdir.filesize;
		}

		bool             isfolder;
		unsigned long    mtime;
		libssh2_uint64_t filesize;
        std::wstring     perm;
		std::wstring     name;
	};

	class SFTPSession : public Session
	{
	public:
		SFTPSession(const char* host, int port, const char* name, const char* passwd);

		~SFTPSession();

		void SetBlock(int block)
		{
			libssh2_session_set_blocking(session_, block);
		}

		std::vector<std::shared_ptr<SSHItem>>  ListDirectory(const char* path);

		std::string Read(CONST CHAR* file);

	private:
		LIBSSH2_SFTP* sftp_session_;
	};

}
#include "SSHOperation.h"

namespace LIBSSH
{

    Session::Session()
    {
#ifdef WIN32
        WSADATA wsadata;
        int err = WSAStartup(MAKEWORD(2, 0), &wsadata);
        if (err != 0)
            on_error("WSAStartup");
#endif

        sock_ = socket(AF_INET, SOCK_STREAM, 0);
        if (sock_ < 0)
            on_error("socket");


        session_ = libssh2_session_init();
        if (!session_)
            on_error("libssh2_session_init");

    }

    Session::~Session()
    {

#ifdef WIN32
        closesocket(sock_);
#else
        close(_sock);
#endif

        libssh2_session_disconnect(session_, "Normal Shutdown, Thank you for playing");
        libssh2_session_free(session_);

    }

    void Session::Open(const char* host, int port)
    {
        struct sockaddr_in sin;

        sin.sin_family = AF_INET;
        sin.sin_port = htons(port);
        inet_pton(AF_INET, host, &sin.sin_addr);

        if (connect(sock_, (struct sockaddr*)(&sin), sizeof(struct sockaddr_in)) != 0)
            on_error("connect");

        if (libssh2_session_handshake(session_, sock_))
            on_error("libssh2_session_handshake");

    }


    void Session::Login(const char* username, const char* passwd)
    {
        if (libssh2_userauth_password(session_, username, passwd))
            on_error("libssh2_userauth_password");
    }

    //sftp
    SFTPSession::SFTPSession(const char* host, int port, const char* name, const char* passwd)
    {
        Open(host, port);
        Login(name, passwd);
        sftp_session_ = libssh2_sftp_init(session_);
        if (!sftp_session_)
            on_error("libssh2_sftp_init");

        SetBlock(FALSE);
    }

    SFTPSession::~SFTPSession()
    {
        libssh2_sftp_shutdown(sftp_session_);
    }

    //File type
    //#define LIBSSH2_SFTP_S_IFMT         0170000     /* type of file mask */
    //#define LIBSSH2_SFTP_S_IFIFO        0010000     /* named pipe (fifo) */
    //#define LIBSSH2_SFTP_S_IFCHR        0020000     /* character special */
    //#define LIBSSH2_SFTP_S_IFDIR        0040000     /* directory */
    //#define LIBSSH2_SFTP_S_IFBLK        0060000     /* block special */
    //#define LIBSSH2_SFTP_S_IFREG        0100000     /* regular */
    //#define LIBSSH2_SFTP_S_IFLNK        0120000     /* symbolic link */
    //#define LIBSSH2_SFTP_S_IFSOCK       0140000     /* socket */




    std::vector<std::shared_ptr<SSHItem>>  SFTPSession::ListDirectory(const char* path)
    {
        std::vector<std::shared_ptr<SSHItem>> dirs;
        LIBSSH2_SFTP_HANDLE* sftp_handle = NULL;

        do
        {
            sftp_handle = libssh2_sftp_opendir(sftp_session_, path);

            if ((!sftp_handle) &&
                (libssh2_session_last_errno(session_) != LIBSSH2_ERROR_EAGAIN))
            {
                //TODO:
                //error

            }
        } while (!sftp_handle);

        do
        {
            char mem[512];
            char longentry[512];
            LIBSSH2_SFTP_ATTRIBUTES attrs;
            int rc = 0;
            while ((rc = libssh2_sftp_readdir_ex(sftp_handle, mem, sizeof(mem),
                longentry, sizeof(longentry), &attrs))
                == LIBSSH2_ERROR_EAGAIN)
            {
                ;
            }

            if (rc > 0)
            {
                if (!strcmp(mem, ".") || !strcmp(mem, ".."))
                    continue;

                std::shared_ptr<SSHItem> item(new SSHItem(longentry, mem,
                    (attrs.permissions&LIBSSH2_SFTP_S_IFDIR),
                    attrs.mtime,
                    attrs.filesize));
                dirs.push_back(item);
            }
            else
            {
                break;
            }


        } while (true);

        libssh2_sftp_closedir(sftp_handle);

        return dirs;
    }

    std::string SFTPSession::Read(CONST CHAR* file)
    {
        int ret = 0;
        std::string content;
        LIBSSH2_SFTP_HANDLE* sftp_handle = NULL;
        libssh2_uint64_t len = 0;

        do {

            sftp_handle = libssh2_sftp_open(sftp_session_, file,
                LIBSSH2_FXF_READ,
                LIBSSH2_SFTP_S_IRUSR | LIBSSH2_SFTP_S_IWUSR |
                LIBSSH2_SFTP_S_IRGRP | LIBSSH2_SFTP_S_IROTH);

            if (!sftp_handle)
            {
                ret = libssh2_session_last_errno(session_);
                if (ret != LIBSSH2_ERROR_EAGAIN)
                {
                    //TODO:
                    //error
                }
            }

        } while (!sftp_handle);



        do
        {
            int rc = 0;
            char buf[512] = { 0 };
            while ((rc = libssh2_sftp_read(sftp_handle, buf, sizeof(buf) - 1)) ==
                LIBSSH2_ERROR_EAGAIN) {

            }
            if (rc <= 0)
                break;

            buf[sizeof(buf) - 1] = 0;
            content += buf;
            memset(buf, 0, sizeof(buf));

        } while ( TRUE );




        libssh2_sftp_close(sftp_handle);

        return content;
    }

}
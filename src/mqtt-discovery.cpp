#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <ifaddrs.h>
#include <syslog.h>

#define MAX_MSG_LEN 4096

const unsigned char magic[] = {0xde, 0xad, 0xfa, 0xce,
                               0xb0, 0x0b, 0x1e, 0xdd};
const unsigned short int listen_port = 2112;
const unsigned short int mqtt_port = 1883;

int main(int argc, char **argv)
{
    int sockfd;
    unsigned char buf[MAX_MSG_LEN];
    char tmpBuf[30];
    struct sockaddr_in servaddr, cliaddr;
    socklen_t len = sizeof(cliaddr);
    int numBytes;

    openlog(0, LOG_PID, LOG_USER);

    memset(&servaddr, 0, sizeof(servaddr));
    memset(&cliaddr, 0, sizeof(cliaddr));

    // Find the eth0 interface
    struct in_addr eth0_addr;
    eth0_addr.s_addr = 0;
    struct ifaddrs *addrs;
    if (getifaddrs(&addrs) != 0)
    {
        syslog(LOG_ERR, "getifaddrs() failed: [%d] %s", errno, strerror(errno));
        exit(EXIT_FAILURE);
    }

    struct ifaddrs *tmp = addrs;
    while (1)
    {
        // Only care about INET addrs
        if (tmp->ifa_addr->sa_family == AF_INET)
        {
            if (strcmp(tmp->ifa_name, "eth0") == 0)
            {
                struct sockaddr_in *addr = (struct sockaddr_in *)tmp->ifa_addr;
                eth0_addr = addr->sin_addr;
                break;
            }
        }

        if ((tmp = tmp->ifa_next) == 0)
        {
            break;
        }
    }
    freeifaddrs(addrs);

    if (eth0_addr.s_addr == 0)
    {
        syslog(LOG_ERR, "eth0 interface not found");
        exit(EXIT_FAILURE);
    }

    // Create a socket
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        syslog(LOG_ERR, "socket create failed: [%d] %s", errno, strerror(errno));
        exit(EXIT_FAILURE);
    }

    int value = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &value, sizeof(value)) != 0)
    {
        syslog(LOG_ERR, "set broadcast socket option failed: [%d] %s", errno, strerror(errno));
        exit(EXIT_FAILURE);
    }

    // Bind to PORT on eth0
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(listen_port);
    if (bind(sockfd, (const struct sockaddr *)&servaddr,
             sizeof(servaddr)) < 0)
    {
        syslog(LOG_ERR, "bind failed: [%d] %s", errno, strerror(errno));
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in sockname;
    socklen_t sockname_len = sizeof(sockname);
    if (getsockname(sockfd, (struct sockaddr *)&sockname, &sockname_len) != 0)
    {
        syslog(LOG_ERR, "getsockname failed: [%d] %s", errno, strerror(errno));
        exit(EXIT_FAILURE);
    }

    syslog(LOG_NOTICE, "listening at: %s:%d", inet_ntoa(eth0_addr), ntohs(sockname.sin_port));

    // Receive messages forever
    while (1)
    {
        numBytes = recvfrom(sockfd, (char *)buf, MAX_MSG_LEN,
                            MSG_WAITALL, (struct sockaddr *)&cliaddr,
                            &len);

        bool match = true;
        for (int i = 0; i < 8; i++)
        {
            if (buf[i] != magic[i])
            {
                match = false;
                break;
            }
        }

        if (match)
        {
            sprintf(tmpBuf, "%d.%d.%d.%d:%d",
                    buf[8], buf[9], buf[10], buf[11],
                    ((buf[12] & 0x00ff) << 8) + (buf[13] & 0x00ff));
            syslog(LOG_INFO, "client query received from %s", tmpBuf);

            unsigned long tmp = ntohl(eth0_addr.s_addr);
            buf[8] = (tmp >> 24) & 0x000000ff;
            buf[9] = (tmp >> 16) & 0x000000ff;
            buf[10] = (tmp >> 8) & 0x000000ff;
            buf[11] = tmp & 0x000000ff;
            // buf[8] = 52;
            // buf[9] = 40;
            // buf[10] = 171;
            // buf[11] = 126;
            buf[12] = (mqtt_port >> 8) & 0x000000ff;
            buf[13] = mqtt_port & 0x000000ff;

            if (sendto(sockfd, buf, numBytes,
                       MSG_CONFIRM, (const struct sockaddr *)&cliaddr,
                       len) == -1)
            {
                syslog(LOG_ERR, "failed to send response to client at %s: [%d] %s",
                       tmpBuf, errno, strerror(errno));
            }
            else
            {
                syslog(LOG_INFO, "response sent to client at %s", tmpBuf);
            }
        }
    }

    return 0;
}
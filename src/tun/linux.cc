// SPDX-License-Identifier: MIT
#if defined(__linux__) || defined(__linux)

#include "tun/tun.h"
#include "utility/address.h"
#include <arpa/inet.h>
#include <fcntl.h>
#include <linux/if_tun.h>
#include <memory>
#include <net/if.h>
#include <net/route.h>
#include <queue>
#include <spdlog/spdlog.h>
#include <string>
#include <sys/ioctl.h>
#include <unistd.h>

namespace {
class LinuxTun {
public:
    int setName(const std::string &name) {
        this->name = name;
        return 0;
    }

    int setIP(uint32_t ip) {
        this->ip = ip;
        return 0;
    }

    int getIP() {
        return this->ip;
    }

    int setMask(uint32_t mask) {
        this->mask = mask;
        return 0;
    }

    int setMTU(int mtu) {
        this->mtu = mtu;
        return 0;
    }

    int setTimeout(int timeout) {
        this->timeout = timeout;
        return 0;
    }

    // 上面的所有操作都只是保存变量,在这里真正开始执行操作
    int up() {
        // 从 /dev/net/tun 里取一个 tun 设备的文件描述符,只有 Linux 可以这样,其他系统需要其他手段
        this->tunFd = open("/dev/net/tun", O_RDWR);
        if (this->tunFd < 0) {
            spdlog::critical("open /dev/net/tun failed");
            return -1;
        }

        // 设置设备名
        struct ifreq ifr;
        bzero(&ifr, sizeof(ifr));
        strncpy(ifr.ifr_name, this->name.c_str(), IFNAMSIZ);
        ifr.ifr_flags = IFF_TUN | IFF_NO_PI;
        if (ioctl(this->tunFd, TUNSETIFF, &ifr) == -1) {
            spdlog::critical("set tun interface failed: fd {} name {}", this->tunFd, this->name);
            return -1;
        }

        // 创建 socket, 并通过这个 socket 更新网卡的其他配置
        struct sockaddr_in *addr;
        addr = (struct sockaddr_in *)&ifr.ifr_addr;
        addr->sin_family = AF_INET;
        int sockfd = socket(addr->sin_family, SOCK_DGRAM, 0);
        if (sockfd == -1) {
            spdlog::critical("create socket failed");
            return -1;
        }

        // 设置地址
        addr->sin_addr.s_addr = Candy::Address::hostToNet(this->ip);
        if (ioctl(sockfd, SIOCSIFADDR, (caddr_t)&ifr) == -1) {
            spdlog::critical("set ip address failed: ip {:08x}", this->ip);
            close(sockfd);
            exit(1);
        }

        // 设置掩码
        addr->sin_addr.s_addr = Candy::Address::hostToNet(this->mask);
        if (ioctl(sockfd, SIOCSIFNETMASK, (caddr_t)&ifr) == -1) {
            spdlog::critical("set mask failed: mask {:08x}", this->mask);
            close(sockfd);
            return -1;
        }

        // 设置 MTU
        ifr.ifr_mtu = this->mtu;
        if (ioctl(sockfd, SIOCSIFMTU, (caddr_t)&ifr) == -1) {
            spdlog::critical("set mtu failed: mtu {}", this->mtu);
            close(sockfd);
            exit(1);
        }

        // up 网卡
        ifr.ifr_ifru.ifru_flags |= IFF_UP;
        if (ioctl(sockfd, SIOCSIFFLAGS, &ifr) == -1) {
            spdlog::critical("interface up failed");
            close(sockfd);
            return -1;
        }

        // 设置路由
        struct rtentry route;
        bzero(&route, sizeof(route));

        addr = (struct sockaddr_in *)&route.rt_dst;
        addr->sin_family = AF_INET;
        addr->sin_addr.s_addr = Candy::Address::hostToNet(this->ip);

        addr = (struct sockaddr_in *)&route.rt_genmask;
        addr->sin_family = AF_INET;
        addr->sin_addr.s_addr = Candy::Address::hostToNet(this->mask);

        route.rt_dev = (char *)this->name.c_str();
        route.rt_flags = RTF_UP | RTF_HOST;
        if (ioctl(sockfd, SIOCADDRT, &route) == -1) {
            spdlog::critical("set route failed");
            close(sockfd);
            return -1;
        }

        close(sockfd);

        return 0;
    }

    int down() {
        close(this->tunFd);
        return 0;
    }

    int read(std::string &buffer) {
        struct timeval timeout = {.tv_sec = this->timeout};
        fd_set set;

        FD_ZERO(&set);
        FD_SET(this->tunFd, &set);

        int ret = select(this->tunFd + 1, &set, NULL, NULL, &timeout);
        if (ret < 0) {
            spdlog::error("select failed: error {}", ret);
            return -1;
        }
        if (ret == 0) {
            return 0;
        }

        buffer.resize(this->mtu);
        int n = ::read(this->tunFd, buffer.data(), buffer.size());
        if (n <= 0) {
            spdlog::warn("tun read failed: error {}", n);
            return -1;
        }
        buffer.resize(n);
        return n;
    }

    int write(const std::string &buffer) {
        return ::write(this->tunFd, buffer.c_str(), buffer.size());
    }

private:
    std::string name;
    uint32_t ip;
    uint32_t mask;
    int mtu;
    int timeout;
    int tunFd;
};
}; // namespace

namespace Candy {

Tun::Tun() {
    this->impl = std::make_shared<LinuxTun>();
}

Tun::~Tun() {
    this->impl.reset();
}

int Tun::setName(const std::string &name) {
    std::shared_ptr<LinuxTun> tun;

    tun = std::any_cast<std::shared_ptr<LinuxTun>>(this->impl);
    tun->setName(name);
    return 0;
}

int Tun::setAddress(const std::string &cidr) {
    std::shared_ptr<LinuxTun> tun;
    Address address;

    if (address.cidrUpdate(cidr)) {
        return -1;
    }
    spdlog::info("client tun address: {}", address.getCidr());
    tun = std::any_cast<std::shared_ptr<LinuxTun>>(this->impl);
    if (tun->setIP(address.getIp())) {
        return -1;
    }
    if (tun->setMask(address.getMask())) {
        return -1;
    }
    return 0;
}

uint32_t Tun::getIP() {
    std::shared_ptr<LinuxTun> tun;
    tun = std::any_cast<std::shared_ptr<LinuxTun>>(this->impl);
    return tun->getIP();
}

int Tun::setMTU(int mtu) {
    std::shared_ptr<LinuxTun> tun;
    tun = std::any_cast<std::shared_ptr<LinuxTun>>(this->impl);
    if (tun->setMTU(mtu)) {
        return -1;
    }
    return 0;
}

int Tun::setTimeout(int timeout) {
    std::shared_ptr<LinuxTun> tun;
    tun = std::any_cast<std::shared_ptr<LinuxTun>>(this->impl);
    if (tun->setTimeout(timeout)) {
        return -1;
    }
    return 0;
}

int Tun::up() {
    std::shared_ptr<LinuxTun> tun;
    tun = std::any_cast<std::shared_ptr<LinuxTun>>(this->impl);
    return tun->up();
}

int Tun::down() {
    std::shared_ptr<LinuxTun> tun;
    tun = std::any_cast<std::shared_ptr<LinuxTun>>(this->impl);
    return tun->down();
}

int Tun::read(std::string &buffer) {
    std::shared_ptr<LinuxTun> tun;
    tun = std::any_cast<std::shared_ptr<LinuxTun>>(this->impl);
    return tun->read(buffer);
}

int Tun::write(const std::string &buffer) {
    std::shared_ptr<LinuxTun> tun;
    tun = std::any_cast<std::shared_ptr<LinuxTun>>(this->impl);
    return tun->write(buffer);
}

}; // namespace Candy

#endif

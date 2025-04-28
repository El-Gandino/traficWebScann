#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include <thread>
#include <map>
#include <cstdlib>
#include <sstream>
#include <unistd.h>      // close
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>  // sockaddr_in
#include <arpa/inet.h>   // inet_ntoa
#include <net/if.h>      // struct ifreq
#include <sys/ioctl.h>   // ioctl
#include <cstring>       // Add this for strncpy
#include <net/ethernet.h> // For Ethernet header
#include <netinet/if_ether.h> // For ARP header
#include <linux/if_packet.h> // For sockaddr_ll
#include <mutex>

#include "../include/logs.h" // Include the header file for Logs class
#include "../include/stream.h" 
class LanScanner {
public:
    struct DeviceInfo {
        std::string macAddress;
        std::string ipv4;
        std::string ipv6;
        std::string hostName;
        std::string dhcpStartDate;
        std::string dhcpEndDate;
    };
    std::string getPersonnalIPAddress(const std::string& interface) {
        int fd;
        struct ifreq ifr;
        fd = socket(AF_INET, SOCK_DGRAM, 0);
        if (fd == -1) {
            Logs::getInstance( Stream::getInstance()->logFilePath)->writeError("Failed to create socket for IP address retrieval.");
            perror("socket");
            return "";
        }
        ifr.ifr_addr.sa_family = AF_INET;
        strncpy(ifr.ifr_name, interface.c_str(), IFNAMSIZ-1);    
        if (ioctl(fd, SIOCGIFADDR, &ifr) == -1) {
            perror("ioctl");
            close(fd);
            return "";
        }
        close(fd);
        struct sockaddr_in* ipaddr = reinterpret_cast<struct sockaddr_in*>(&ifr.ifr_addr);
        Logs::getInstance( Stream::getInstance()->logFilePath)->writeInfo("Server IP Address: " + std::string(inet_ntoa(ipaddr->sin_addr)));
        return inet_ntoa(ipaddr->sin_addr);
    }
    std::string getBroadcastAddress(const std::string& interface) {
        int fd = socket(AF_INET, SOCK_DGRAM, 0);
        if (fd == -1) {
            Logs::getInstance( Stream::getInstance()->logFilePath)->writeError("Failed to create socket for broadcast address retrieval.");
            perror("socket");
            return "";
        }
    
        struct ifreq ifr;
        strncpy(ifr.ifr_name, interface.c_str(), IFNAMSIZ - 1);
    
        // Get the IP address
        if (ioctl(fd, SIOCGIFADDR, &ifr) == -1) {
            Logs::getInstance( Stream::getInstance()->logFilePath)->writeError("Failed to get IP address.");
            perror("ioctl SIOCGIFADDR");
            close(fd);
            return "";
        }
        struct sockaddr_in* ipaddr = reinterpret_cast<struct sockaddr_in*>(&ifr.ifr_addr);
        uint32_t ip = ipaddr->sin_addr.s_addr;
    
        // Get the subnet mask
        if (ioctl(fd, SIOCGIFNETMASK, &ifr) == -1) {
            Logs::getInstance( Stream::getInstance()->logFilePath)->writeError("Failed to get subnet mask.");
            perror("ioctl SIOCGIFNETMASK");
            close(fd);
            return "";
        }
        struct sockaddr_in* netmask = reinterpret_cast<struct sockaddr_in*>(&ifr.ifr_netmask);
        uint32_t mask = netmask->sin_addr.s_addr;
    
        close(fd);
    
        // Calculate the broadcast address
        uint32_t broadcast = (ip & mask) | ~mask;
        struct in_addr broadcastAddr;
        broadcastAddr.s_addr = broadcast;
    
        return inet_ntoa(broadcastAddr);
    }     
    /*Envoyer ARP Broadcast*/
    void sendArpBroadcast(const std::string& interface) {
        int sock = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ARP));
        if (sock < 0) {
            Logs::getInstance( Stream::getInstance()->logFilePath)->writeError("Failed to create socket for ARP broadcast.");
            perror("Socket creation failed");
            return;
        }
        struct sockaddr_ll broadcastAddr = {0};
        broadcastAddr.sll_family = AF_PACKET;
        broadcastAddr.sll_protocol = htons(ETH_P_ARP);
        broadcastAddr.sll_ifindex = if_nametoindex(interface.c_str());
        broadcastAddr.sll_halen = ETH_ALEN;
        memset(broadcastAddr.sll_addr, 0xFF, ETH_ALEN); // Broadcast MAC address

        // Construct ARP packet
        unsigned char buffer[42]; // Ethernet header (14 bytes) + ARP header (28 bytes)
        memset(buffer, 0, sizeof(buffer));

        struct ether_header* ethHeader = (struct ether_header*)buffer;
        memset(ethHeader->ether_dhost, 0xFF, ETH_ALEN); // Destination MAC: Broadcast
        memset(ethHeader->ether_shost, 0x00, ETH_ALEN); // Source MAC: Fill with your MAC
        ethHeader->ether_type = htons(ETH_P_ARP);
        struct ether_arp* arpHeader = (struct ether_arp*)(buffer + sizeof(struct ether_header));
        arpHeader->ea_hdr.ar_hrd = htons(ARPHRD_ETHER);
        arpHeader->ea_hdr.ar_pro = htons(ETH_P_IP);
        arpHeader->ea_hdr.ar_hln = ETH_ALEN;
        arpHeader->ea_hdr.ar_pln = 4;
        arpHeader->ea_hdr.ar_op = htons(ARPOP_REQUEST);
        memset(arpHeader->arp_tha, 0x00, ETH_ALEN); // Target MAC: Unknown
        inet_pton(AF_INET, "192.168.1.255", arpHeader->arp_tpa); // Target IP: Broadcast
        memset(arpHeader->arp_sha, 0x00, ETH_ALEN); // Source MAC: Fill with your MAC
        inet_pton(AF_INET, "192.168.1.13", arpHeader->arp_spa); // Source IP: Your IP
        // Send ARP packet
        if (sendto(sock, buffer, sizeof(buffer), 0, (struct sockaddr*)&broadcastAddr, sizeof(broadcastAddr)) < 0) {
            Logs::getInstance( Stream::getInstance()->logFilePath)->writeError("Failed to send ARP broadcast.");
            perror("Failed to send ARP broadcast");
        } else {
            Logs::getInstance( Stream::getInstance()->logFilePath)->writeInfo("ARP broadcast sent successfully.");
            std::cout << "ARP broadcast sent successfully." << std::endl;// A comm en production
        }
        close(sock);
    }
    /**[scanner IP en ping] -- controle les IP silencieux et complette la table. */
    void scanSilentIPs(const std::string& interface) {
        std::string broadcastIP = getBroadcastAddress(interface);
        if (broadcastIP.empty()) {
            Logs::getInstance( Stream::getInstance()->logFilePath)->writeError("Failed to get broadcast address.");
            std::cerr << "Failed to get broadcast address." << std::endl;
            return;
        }
        // Extract the subnet from the broadcast address
        size_t lastDot = broadcastIP.find_last_of('.');
        if (lastDot == std::string::npos) {
            Logs::getInstance( Stream::getInstance()->logFilePath)->writeError("Invalid broadcast address format.");
            std::cerr << "Invalid broadcast address format." << std::endl;
            return;
        }
        std::string subnet = broadcastIP.substr(0, lastDot);
        // Ping all IPs in the subnet
        std::vector<std::thread> threads;
        std::vector<std::string> activeIPs;
        int silentCount = 0;
        std::mutex mutex;

        for (int i = 1; i < 255; ++i) {
            threads.emplace_back([subnet, i, &activeIPs, &silentCount, &mutex]() {
                std::string ipToPing = subnet + "." + std::to_string(i);
                std::string command = "ping -c 1 -W 1 " + ipToPing + " > /dev/null 2>&1";
                if (system(command.c_str()) == 0) {
                    std::lock_guard<std::mutex> lock(mutex);
                    activeIPs.push_back(ipToPing);
                } else {
                    std::lock_guard<std::mutex> lock(mutex);
                    ++silentCount;
                }
            });
        }
        for (auto& thread : threads) {
            thread.join();
        }
        
        Logs::getInstance( Stream::getInstance()->logFilePath)->writeInfo("Scan complete. Active IPs found: " + std::to_string(activeIPs.size()) + ", Silent IPs: " + std::to_string(silentCount));
       
    }
    /*[faire tableaa MAC IPv4 IPv6 STARTTIME ENDTIME]*/
    std::vector<DeviceInfo> getDeviceInfoFromArpTable() {
        std::vector<DeviceInfo> devices;
        FILE* arpFile = fopen("/proc/net/arp", "r");
        if (!arpFile) {
            perror("Failed to open ARP table");
            return devices;
        }

        char line[256];
        fgets(line, sizeof(line), arpFile); // Skip the header line

        while (fgets(line, sizeof(line), arpFile)) {
            std::istringstream iss(line);
            std::string ip, hwType, flags, mac, mask, device;
            if (!(iss >> ip >> hwType >> flags >> mac >> mask >> device)) {
                continue;
            }

            if (mac == "00:00:00:00:00:00") {
                continue; // Skip incomplete entries
            }

            DeviceInfo deviceInfo;
            deviceInfo.ipv4 = ip;
            deviceInfo.macAddress = mac;
            deviceInfo.hostName = ""; // Hostname resolution can be added if needed
            deviceInfo.ipv6 = ""; // IPv6 not available in ARP table
            deviceInfo.dhcpStartDate = ""; // DHCP info not available in ARP table
            deviceInfo.dhcpEndDate = ""; // DHCP info not available in ARP table

            devices.push_back(deviceInfo);
        }

        fclose(arpFile);
        return devices;
    }

};
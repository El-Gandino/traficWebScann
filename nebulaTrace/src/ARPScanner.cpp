#include <iostream>
#include <vector>
#include <string>
#include <netdb.h>
#include <arpa/inet.h>
#include <cstring>
#include <stdexcept>
#include <fstream>
#include <sstream>
#include <iostream>

class ARPScanner {
public:
    struct ARPEntry {
        std::string macAddress;
        std::string ipv4Address;
        std::string ipv6Address;
        std::string dhcpEndTime;
        std::string hostname; // Added hostname field
    };

    ARPScanner() = default;

    std::vector<ARPEntry> scanARPTable() {
        std::vector<ARPEntry> arpTable;

        // Simulate reading ARP table (replace this with actual system-specific code)
        std::ifstream arpFile("/proc/net/arp");
        if (!arpFile.is_open()) {
            throw std::runtime_error("Failed to open ARP table file.");
        }

        std::string line;
        std::getline(arpFile, line); // Skip the header line
        while (std::getline(arpFile, line)) {
            std::istringstream iss(line);
            std::string ip, hwType, flags, mac, mask, device;
            if (!(iss >> ip >> hwType >> flags >> mac >> mask >> device)) {
                continue;
            }
            ARPEntry entry;

            entry.macAddress = mac;
            entry.ipv4Address = ip;
            entry.ipv6Address = ""; // Placeholder, as ARP is for IPv4
            entry.dhcpEndTime = getDHCPLeaseEndTime(ip); // Simulate DHCP lease end time
            entry.hostname = getHostnameFromIP(ip); // Get hostname from IP address
            
            arpTable.push_back(entry);
        }

        return arpTable;
    }

    std::string getHostnameFromIP(const std::string& ip) {
        struct sockaddr_in sa;
        char host[NI_MAXHOST];
        sa.sin_family = AF_INET;
        inet_pton(AF_INET, ip.c_str(), &sa.sin_addr);

        int res = getnameinfo((struct sockaddr*)&sa, sizeof(sa), host, sizeof(host), NULL, 0, NI_NAMEREQD);
        if (res == 0) {
            return std::string(host);
        } else {
            return "Unknown";
        }
    }

    void printARPTable(const std::vector<ARPEntry>& arpTable) {
        std::cout << "Dumping ARP Table:" << std::endl;
        for (const auto& entry : arpTable) {
            std::cout << "IPv4 Address: " << entry.ipv4Address
                      << ", MAC Address: " << entry.macAddress
                      << ", DHCP Lease End Time: " << entry.dhcpEndTime
                      << ", Hostname: " << entry.hostname << std::endl;
        }
        std::cout << "End of ARP Table." << std::endl;
    }

private:
    std::string getDHCPLeaseEndTime(const std::string& ip) {
        // Simulate fetching DHCP lease end time (replace with actual implementation)
        return "2023-12-31 23:59:59"; // Placeholder
    }
};


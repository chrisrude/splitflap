#ifndef NETWORK_H
#define NETWORK_H

#pragma once

#include "../core/logger.h"

#include <esp_sntp.h>
#include <esp_wifi.h>
#include <sys/time.h>
#include <WiFi.h>
#include <WiFiMulti.h>

#include <ArduinoLog.h>

#define ASSERT(args...) assert(args)
#define DEBUG(args...) m_logger.log(args)
#define ERROR(args...) m_logger.log(args)
#define IF_DEBUG(args...) args


class Network
{
public:
    // Number of seconds to wait for WiFi connection
    const static uint16_t WIFI_CONNECT_TIMEOUT_MS = 10000;

    // we will append this string to the end of the
    // user-supplied hostname, so that multiple devices
    // with the same firmware can be distinguished
    const char *MAC_ADDRESS_FORMAT = "%02X:%02X:%02X:%02X:%02X:%02X";

    // length of the MAC address string we'll get from the above
    // format, not including the null
    const static size_t MAC_ADDRESS_FORMAT_LENGTH = 17;

    // length of the shortened mac address
    const static size_t MAC_ADDRESS_STUB_FORMAT_LENGTH = 5;

    // max length of the hostname we'll use, not including the null
    // terminator.
    //
    // limit taken from set_esp_netif_hostname in
    // framework-arduinoespressif32/libraries/WiFi/src/WiFiGeneric.h
    const static size_t MAX_HOSTNAME_LENGTH = 32 - 1;

    // Account for the MAC address, and the hyphen we'll put before it.
    // The remaining characters are for the user-supplied hostname base.
    // This does NOT account for the null terminator.
    const static size_t MAX_BASE_HOSTNAME_LENGTH = MAX_HOSTNAME_LENGTH - MAC_ADDRESS_STUB_FORMAT_LENGTH - 1;

    const static uint16_t DEFAULT_NTP_SYNC_TIMEOUT_MS = 20000;
    const static uint16_t NTP_POLLING_INTERVAL_MS = 10;

#define NTP_POLLING_INTERVAL_MS 10

    Network(Logger& logger)
        : m_wifiMulti(), m_logger(logger)
    {
        m_fWifiConnected = false;
        m_strMacAddress[0] = '\0';
        m_strHostname[0] = '\0';
    }

    bool begin(const char *hostname_base, const char *ssid, const char *password)
    {
        assert(MAX_BASE_HOSTNAME_LENGTH >= strlen(hostname_base));

        // format the mac address into a string
        {
            // first we need to get the address in byte form.
            // we only use 6 bytes from the mac address, but
            // technically the WiFi.macAddress() function
            // can potentially return 8.  So allocate 8 to be safe.
            uint8_t mac[8];
            WiFi.macAddress(mac);
            snprintf(m_strMacAddress, MAC_ADDRESS_FORMAT_LENGTH + 1, MAC_ADDRESS_FORMAT,
                     mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

            // Generate a shortened "unique" string from the MAC address.
            // Unique is in quotes because in order to make it short,
            // we'll assume that the first 3 bytes, which represent
            // the manufacturer, are always the same.  So we'll only
            // encode the final 3 bits.
            // Then we'll use a base32 encoding, which will give us
            // 5 bits per character.  We'll use 5 characters, which
            // will give us 25 bits, which is more than enough to
            // represent the 24 bits of the second half of the mac
            // address.
            //                            aaaaabbb bbcccccd ddddeeee
            // xxxxxxxx-xxxxxxxx-xxxxxxxx-xxxxxxxx-xxxxxxxx-xxxxxxxx
            // Since we're just after the last 3, we can put everything
            // into a single 32-bit integer, and then shift and mask.
            char strMacStub[MAC_ADDRESS_STUB_FORMAT_LENGTH + 1];
            static const char alphanum[] = // 32 chars total
                "0123456789"
                "abcdefghjkmnprstuvwxyz";
            uint32_t mac32 = (mac[3] << 16) | (mac[4] << 8) | mac[5];

            for (int i = 0; i < MAC_ADDRESS_STUB_FORMAT_LENGTH; i++)
            {
                strMacStub[MAC_ADDRESS_STUB_FORMAT_LENGTH - 1 - i] = alphanum[mac32 & 0x1f];
                mac32 >>= 5;
            }
            strMacStub[MAC_ADDRESS_STUB_FORMAT_LENGTH] = '\0';

            // format the full hostname, which is:
            //    hostname_base + "-" + mac_address_stub
            snprintf(m_strHostname, MAX_HOSTNAME_LENGTH + 1, "%s-%s",
                     hostname_base, strMacStub);
        }

        // we apparently need to set the hostname BEFORE we set the
        // wifi mode, or the hostname will not be set on the wifi
        // interface (though it is still set for mdns)
        // see https://github.com/espressif/arduino-esp32/issues/6700
        bool result = WiFi.hostname(m_strHostname);
        if (!result)
        {
            ERROR("Network: failed to set wifi hostname!");
            return false;
        }

        result = m_wifiMulti.addAP(ssid, password);
        if (!result)
        {
            ERROR("Network: failed to add wifi AP!");
            return false;
        }

        DEBUG("Network: SSID:");
        DEBUG(ssid);
        DEBUG("Network: MAC address:");
        DEBUG(m_strMacAddress);
        DEBUG("Network: hostname:");
        DEBUG(m_strHostname);

        return reconnect_wifi();
    }

    /// @brief  Reconnect to wifi if we're not already connected.
    /// @return true if wifi is connected after the call.
    bool reconnect_wifi()
    {
        if (m_fWifiConnected)
        {
            return true;
        }
        DEBUG("Network: connecting to wifi...");

        // Setup wifi.  It's safe to do this multiple times
        // (it will just return true if it's already in the
        // requested mode)
        bool result = WiFi.mode(WIFI_STA);
        if (!result)
        {
            ERROR("Network: failed to set wifi mode!");
            return false;
        }

        wl_status_t status = (wl_status_t)m_wifiMulti.run(WIFI_CONNECT_TIMEOUT_MS);
        switch (status)
        {
        case WL_CONNECTED:
            // fall through
            break;

        case WL_NO_SSID_AVAIL:
            ERROR("Network: no SSID available");
            return false;

        case WL_CONNECT_FAILED:
            ERROR("Network: failed to connect to wifi");
            return false;

        default:
            ERROR("Network: unknown status when connecting to wifi");
            return false;
        }

        m_fWifiConnected = true;
        DEBUG("Network: connected to wifi AP");
        DEBUG(get_ssid());

        IF_DEBUG(WiFi.printDiag(Serial));
        DEBUG("Network: IP address: ");
        DEBUG(get_ip_address());

        return true;
    }

    bool is_wifi_connected() const
    {
        return m_fWifiConnected;
    }

    const char *get_ssid() const
    {
        if (!m_fWifiConnected)
        {
            return "";
        }
        wifi_config_t conf;
        esp_wifi_get_config((wifi_interface_t)WIFI_IF_STA, &conf);

        const char *ssid = reinterpret_cast<const char *>(conf.sta.ssid);
        return ssid;
    }

    const char *get_mac_address() const
    {
        return m_strMacAddress;
    }

    const char *get_hostname() const
    {
        return m_strHostname;
    }

    const char *get_ip_address() const
    {
        return WiFi.localIP().toString().c_str();
    }

protected:
    WiFiMulti m_wifiMulti;

    Logger& m_logger;

    char m_strMacAddress[MAC_ADDRESS_FORMAT_LENGTH + 1];
    char m_strMacStub[MAC_ADDRESS_STUB_FORMAT_LENGTH + 1];
    char m_strHostname[MAX_HOSTNAME_LENGTH + 1];

    bool m_fWifiConnected;
};

#endif // NETWORK_H

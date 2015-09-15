// Copyright (c) 2015-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <SoapySDR/Device.hpp>
#include <sstream>

template <typename Type>
std::string toString(const std::vector<Type> &options)
{
    std::stringstream ss;
    if (options.empty()) return "";
    for (size_t i = 0; i < options.size(); i++)
    {
        if (not ss.str().empty()) ss << ", ";
        ss << options[i];
    }
    return ss.str();
}

std::string toString(const SoapySDR::Range &range)
{
    std::stringstream ss;
    ss << "[" << range.minimum() << ", " << range.maximum() << "]";
    return ss.str();
}

std::string toString(const SoapySDR::RangeList &range, const double scale)
{
    std::stringstream ss;
    for (size_t i = 0; i < range.size(); i++)
    {
        if (not ss.str().empty()) ss << ", ";
        ss << "[" << (range[i].minimum()/scale) << ", " << (range[i].maximum()/scale) << "]";
    }
    return ss.str();
}

std::string toString(const std::vector<double> &nums, const double scale)
{
    std::stringstream ss;

    if (nums.size() > 3)
    {
        ss << "[" << (nums.front()/scale) << ", " << (nums.back()/scale) << "]";
        return ss.str();
    }

    for (size_t i = 0; i < nums.size(); i++)
    {
        if (not ss.str().empty()) ss << ", ";
        ss << (nums[i]/scale);
    }
    return "[" + ss.str() + "]";
}

static std::string probeChannel(SoapySDR::Device *device, const int dir, const size_t chan)
{
    std::stringstream ss;

    std::string dirName = (dir==SOAPY_SDR_TX)?"TX":"RX";
    ss << std::endl;
    ss << "----------------------------------------------------" << std::endl;
    ss << "-- " << dirName << " Channel " << chan << std::endl;
    ss << "----------------------------------------------------" << std::endl;

    ss << "  Full-duplex: " << (device->getFullDuplex(dir, chan)?"YES":"NO") << std::endl;

    //antennas
    std::string antennas = toString(device->listAntennas(dir, chan));
    if (not antennas.empty()) ss << "  Antennas: " << antennas << std::endl;

    //corrections
    std::vector<std::string> correctionsList;
    if (device->hasDCOffsetMode(dir, chan)) correctionsList.push_back("DC removal");
    if (device->hasDCOffset(dir, chan)) correctionsList.push_back("DC offset");
    if (device->hasIQBalance(dir, chan)) correctionsList.push_back("IQ balance");
    std::string corrections = toString(correctionsList);
    if (not corrections.empty()) ss << "  Corrections: " << corrections << std::endl;

    //gains
    ss << "  Full gain range: " << toString(device->getGainRange(dir, chan)) << " dB" << std::endl;
    std::vector<std::string> gainsList = device->listGains(dir, chan);
    for (size_t i = 0; i < gainsList.size(); i++)
    {
        const std::string name = gainsList[i];
        ss << "    " << name << " gain range: " << toString(device->getGainRange(dir, chan, name)) << " dB" << std::endl;
    }

    //frequencies
    ss << "  Full freq range: " << toString(device->getFrequencyRange(dir, chan), 1e6) << " MHz" << std::endl;
    std::vector<std::string> freqsList = device->listFrequencies(dir, chan);
    for (size_t i = 0; i < freqsList.size(); i++)
    {
        const std::string name = freqsList[i];
        ss << "    " << name << " freq range: " << toString(device->getFrequencyRange(dir, chan, name), 1e6) << " MHz" << std::endl;
    }

    //rates
    ss << "  Sample rates: " << toString(device->listSampleRates(dir, chan), 1e6) << " MHz" << std::endl;

    //bandwidths
    const std::vector<double> bws = device->listBandwidths(dir, chan);
    if (not bws.empty()) ss << "  Filter bandwidths: " << toString(bws, 1e6) << " MHz" << std::endl;

    //sensors
    std::string sensors = toString(device->listSensors(dir, chan));
    if (not sensors.empty()) ss << "  Sensors: " << sensors << std::endl;

    return ss.str();
}

std::string SoapySDRDeviceProbe(SoapySDR::Device *device)
{
    std::stringstream ss;

    /*******************************************************************
     * Identification info
     ******************************************************************/
    ss << std::endl;
    ss << "----------------------------------------------------" << std::endl;
    ss << "-- Device identification" << std::endl;
    ss << "----------------------------------------------------" << std::endl;

    ss << "  driver=" << device->getDriverKey() << std::endl;
    ss << "  hardware=" << device->getHardwareKey() << std::endl;
    SoapySDR::Kwargs info = device->getHardwareInfo();
    for (SoapySDR::Kwargs::const_iterator it = info.begin(); it != info.end(); ++it)
    {
        ss << "  " << it->first << "=" << it->second << std::endl;
    }

    /*******************************************************************
     * Available peripherals
     ******************************************************************/
    ss << std::endl;
    ss << "----------------------------------------------------" << std::endl;
    ss << "-- Peripheral summary" << std::endl;
    ss << "----------------------------------------------------" << std::endl;

    size_t numRxChans = device->getNumChannels(SOAPY_SDR_RX);
    size_t numTxChans = device->getNumChannels(SOAPY_SDR_TX);
    ss << "  Channels: " << numRxChans << " Rx, " << numTxChans << " Tx" << std::endl;

    ss << "  Timestamps: " << (device->hasHardwareTime()?"YES":"NO") << std::endl;

    std::string clockSources = toString(device->listClockSources());
    if (not clockSources.empty()) ss << "  Clock sources: " << clockSources << std::endl;

    std::string timeSources = toString(device->listTimeSources());
    if (not timeSources.empty()) ss << "  Time sources: " << timeSources << std::endl;

    std::string sensors = toString(device->listSensors());
    if (not sensors.empty()) ss << "  Sensors: " << sensors << std::endl;

    std::string gpios = toString(device->listGPIOBanks());
    if (not gpios.empty()) ss << "  GPIOs: " << gpios << std::endl;

    std::string uarts = toString(device->listUARTs());
    if (not uarts.empty()) ss << "  UARTs: " << uarts << std::endl;

    /*******************************************************************
     * Per-channel info
     ******************************************************************/
    for (size_t chan = 0; chan < numRxChans; chan++)
    {
        ss << probeChannel(device, SOAPY_SDR_RX, chan);
    }
    for (size_t chan = 0; chan < numTxChans; chan++)
    {
        ss << probeChannel(device, SOAPY_SDR_TX, chan);
    }

    return ss.str();
}
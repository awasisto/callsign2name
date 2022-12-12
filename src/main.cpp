/*
 * Copyright (C) 2022 Andika Wasisto
 *
 * callsign2name is free software: you can redistribute it and/or modify it under
 * the terms of the GNU Affero General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * callsign2name is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Affero General Public License for
 * more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with callsign2name.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "pugixml.hpp"
#include <algorithm>
#include <boost/algorithm/string.hpp>
#include <boost/program_options.hpp>
#include <cpr/cpr.h>
#include <iostream>
#include <nlohmann/json.hpp>

namespace po = boost::program_options;

#ifdef _WIN32

#include <windows.h>

std::string maskedPasswordInput()
{
    const char BACKSPACE = 8;
    const char RETURN = 13;
    std::string password;
    char ch;
    DWORD conMode;
    DWORD dwRead;
    HANDLE hIn = GetStdHandle(STD_INPUT_HANDLE);
    GetConsoleMode(hIn, &conMode);
    SetConsoleMode(hIn, conMode & ~(ENABLE_ECHO_INPUT | ENABLE_LINE_INPUT));
    while (ReadConsoleA(hIn, &ch, 1, &dwRead, NULL) && ch != RETURN) {
        if (ch == BACKSPACE) {
            if (password.length() != 0) {
                std::cout << "\b \b";
                password.resize(password.length() - 1);
            }
        } else {
            password += ch;
            std::cout << '*';
        }
    }
    std::cout << std::endl;
    return password;
}

#else

#include <termios.h>

int getch()
{
    int ch;
    struct termios tOld, tNew;
    tcgetattr(STDIN_FILENO, &tOld);
    tNew = tOld;
    tNew.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &tNew);
    ch = getchar();
    tcsetattr(STDIN_FILENO, TCSANOW, &tOld);
    return ch;
}

std::string maskedPasswordInput()
{
    const char BACKSPACE = 127;
    const char RETURN = 10;
    std::string password;
    char ch;
    while ((ch = getch()) != RETURN) {
        if (ch == BACKSPACE) {
            if (password.length() != 0) {
                std::cout << "\b \b";
                password.resize(password.length() - 1);
            }
        } else {
            password += ch;
            std::cout << '*';
        }
    }
    std::cout << std::endl;
    return password;
}

#endif

std::string getOwnerName(std::string callsign, std::string qrzUsername, std::string qrzPassword)
{
    boost::replace_all(callsign, "\xD8", "0");
    boost::replace_all(callsign, "\xF8", "0");

    std::string sdppiErrorDescription;

    cpr::Response response = cpr::Get(
        cpr::Url { "https://iar-ikrap.postel.go.id/registrant/searchDataIar" },
        cpr::Parameters {
            { "callsign", callsign }
        },
        cpr::Timeout{3000}
    );
    if (response.status_code == 200) {
        pugi::xml_document xmlDocument;
        pugi::xml_parse_result xmlParseResult = xmlDocument.load_string(nlohmann::json::parse(response.text).get<std::string>().c_str());
        if (xmlParseResult) {
            std::string ownerName = xmlDocument.first_child().first_child().last_child().child_value();
            boost::trim(ownerName);
            boost::to_upper(ownerName);
            return ownerName;
        } else {
            sdppiErrorDescription = xmlParseResult.description();
        }
    } else {
        sdppiErrorDescription = response.error.message;
    }

    if (qrzUsername.empty()) {
        throw std::runtime_error("Failed to get the owner name of callsign " + callsign + ". Error: " + sdppiErrorDescription);
    }

    response = cpr::Get(
        cpr::Url { "https://xmldata.qrz.com/xml/" },
        cpr::Parameters {
            { "callsign", callsign },
            { "username", qrzUsername },
            { "password", qrzPassword }
        },
        cpr::Timeout{3000}
    );
    if (response.status_code == 200) {
        pugi::xml_document xmlDocument;
        pugi::xml_parse_result xmlParseResult = xmlDocument.load_string(response.text.c_str());
        if (xmlParseResult) {
            std::string ownerName = xmlDocument.child("QRZDatabase").child("Callsign").child("fname").child_value()
                + std::string(" ")
                + xmlDocument.child("QRZDatabase").child("Callsign").child("ownerName").child_value();
            std::transform(ownerName.begin(), ownerName.end(), ownerName.begin(), ::toupper);
            boost::trim(ownerName);
            boost::to_upper(ownerName);
            return ownerName;
        } else {
            throw std::runtime_error("Failed to get the owner name of callsign " + callsign + ". Error: " + xmlParseResult.description());
        }
    } else {
        throw std::runtime_error("Failed to get the owner name of callsign " + callsign + ". Error: " + response.error.message);
    }
}

int main(int argc, char** argv)
{
    po::options_description optionsDescription("Options", 120);
    optionsDescription.add_options()
        ("help,h", "Show help")
        ("callsign,c", po::value<std::string>()->value_name("CALLSIGN"), "Callsign")
        ("callsign-list,l", po::value<std::string>()->value_name("FILE"), "Callsign list file. One callsign per line. Output CSV file must be specified")
        ("output-csv,o", po::value<std::string>()->value_name("FILE"), "Output CSV file")
        ("qrz-username,q", po::value<std::string>()->value_name("USERNAME"), "Your QRZ username (if you also want to use QRZ in addition to SDPPI)");
    po::variables_map variablesMap;
    po::store(po::parse_command_line(argc, argv, optionsDescription), variablesMap);
    po::notify(variablesMap);

    if (variablesMap.count("help") > 0
        || (variablesMap.count("callsign") == 0 && variablesMap.count("callsign-list") == 0)
        || (variablesMap.count("callsign-list") > 0 && variablesMap.count("output-csv") == 0)) {

        std::cout << "Usage: " << argv[0] << " [options]\n\n";
        std::cout << optionsDescription << std::endl;
        std::cout << "Example:\n";
        std::cout << "  " << argv[0] << " -c k7mds\n";
        std::cout << "  " << argv[0] << " -l callsigns.txt -o callsigns.csv -q yd0bjp\n";
        return 0;
    }

    std::string qrzUsername;
    std::string qrzPassword;

    if (variablesMap.count("qrz-username") > 0) {
        qrzUsername = variablesMap["qrz-username"].as<std::string>();
        std::cout << "Enter your QRZ password: ";
        qrzPassword = maskedPasswordInput();
    }

    if (variablesMap.count("callsign-list") == 0) {
        std::string callsign = variablesMap["callsign"].as<std::string>();
        if (variablesMap.count("output-csv") == 0) {
            try {
                std::cout << getOwnerName(callsign, qrzUsername, qrzPassword) << std::endl;
            } catch (std::exception& e) {
                std::cout << e.what() << std::endl;
                return 1;
            }
        } else {
            std::ofstream outputCsv(variablesMap["output-csv"].as<std::string>(), std::ios::out | std::ios::app);
            std::string ownerName;
            try {
                ownerName = getOwnerName(callsign, qrzUsername, qrzPassword);
            } catch (std::exception& e) {
                std::cout << e.what() << std::endl;
            }
            outputCsv << callsign << "," << ownerName << std::endl;
            std::cout << "Processed 1 callsign(s)" << std::endl;
        }
    } else {
        std::ifstream callsignListFile(variablesMap["callsign-list"].as<std::string>());
        std::ofstream outputCsv(variablesMap["output-csv"].as<std::string>(), std::ios::out | std::ios::app);
        std::string callsign;
        int callsignsProcessed = 0;
        while (std::getline(callsignListFile, callsign)) {
            boost::trim_right(callsign);
            std::string ownerName;
            try {
                ownerName = getOwnerName(callsign, qrzUsername, qrzPassword);
            } catch (std::exception& e) {
                std::cout << e.what() << std::endl;
            }
            outputCsv << callsign << "," << ownerName << std::endl;
            std::cout << "Processed " << ++callsignsProcessed << " callsign(s)" << std::endl;
        }
        if (variablesMap.count("callsign") > 0) {
            callsign = variablesMap["callsign"].as<std::string>();
            std::string ownerName;
            try {
                ownerName = getOwnerName(callsign, qrzUsername, qrzPassword);
            } catch (std::exception& e) {
                std::cout << e.what() << std::endl;
            }
            outputCsv << callsign << "," << ownerName << std::endl;
        }
    }

    return 0;
}
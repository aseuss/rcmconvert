/* src/command_parser.cpp
 *
 * Copyright 2014,2015 Andreas Seuss
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 * */

#include "command_parser.h"
#include <algorithm>
#include <iostream>
#include <iterator>
#include <iomanip>

static const int kHelpLeftIndent = 4;
static const int kHelpBoolMiddleIndent = 9;
static const int kHelpValueMiddleIndent = 8;

CommandParser::CommandParser(int argc, char **argv) {
    mProgramName = argv[0];
    for (int i = 1; i < argc; ++i) {
        mArgList.push_back(argv[i]);
    }
    mHelpDialog << "Usage: " << mProgramName << " ";
    mUsageString = "[options] file";
}

void CommandParser::addHelpOption(const std::string &option, const std::string &helpText) {
    mOptions.insert(std::pair<std::string, bool>(option, false));

    mOptionDesc << std::setw(kHelpLeftIndent) << option << " " << std::setw(kHelpBoolMiddleIndent);
    mOptionDesc << " " << helpText << std::endl;

    mHelpOption = option;
}

void CommandParser::addBoolOption(const std::string &option, const std::string &helpText) {
    mOptions.insert(std::pair<std::string, bool>(option, false));

    mOptionDesc << std::setw(kHelpLeftIndent) << option << " " << std::setw(kHelpBoolMiddleIndent);
    mOptionDesc << " " << helpText << std::endl;
}

bool CommandParser::boolOption(const std::string &option, bool defaultVal) const {
    if (mBoolResults.find(option) != mBoolResults.end()) {
        return mBoolResults.at(option);
    } else {
        return defaultVal;
    }

}

void CommandParser::addValueOption(const std::string &option, const std::string &paramText,
        const std::string &helpText) {
    mOptions.insert(std::pair<std::string, bool>(option, true));

    mOptionDesc << std::setw(kHelpLeftIndent) << option << " " << std::left;
    mOptionDesc << std::setw(kHelpValueMiddleIndent);
    mOptionDesc << paramText << " " << helpText << std::endl << std::right;
}

std::string CommandParser::valueOption(const std::string &option,
        const std::string &defaultVal) const {
    if (mValueResults.find(option) != mValueResults.end()) {
        return mValueResults.at(option);
    } else {
        return defaultVal;
    }
}
    
std::list<std::string> CommandParser::trailingArgs() const {
    return mTrailingArgs;
}

void CommandParser::setUsageString(const std::string &usage) {
    mUsageString = usage;
}

void CommandParser::appendToPreDescText(const std::string &text) {
    mPreDescriptionText << text << std::endl;
}

void CommandParser::appendToPostDescText(const std::string &text) {
    mPostDescriptionText << text << std::endl;
}

int CommandParser::parse() {
    std::list<std::string>::iterator listIt = mArgList.begin();
    std::map<std::string, bool>::iterator mapIt;
    int status = 0;

    while (listIt != mArgList.end()) {
        mapIt = mOptions.find(*listIt);
        if (mapIt == mOptions.end()) {
            // the parser does not know the option. Check whether it might be a trailing
            // argument which could be added to the list of trailing arguments
 
            if ((*listIt).compare(0, 1, "-")) {
                // only add strings that don't seem to be option strings
                mTrailingArgs.push_back(*listIt);
            } else {
                std::stringstream error;
                error << "invalid option -- '" << *listIt << "'";
                showError(error);

                status = -1;
                break;
            }
            listIt = mArgList.erase(listIt);
            continue;
        }

        // check whether we have an option with assigned value
        if (mapIt->second) {
            // remove everything in the trailing list. If we hit this code
            // the elements contained in the list were no trailing elements obviously
            mTrailingArgs.clear();

            // workaround for missing std::next in c++ < c++11
            std::list<std::string>::iterator prevIt = listIt;
            std::list<std::string>::iterator nextIt = ++listIt;
            listIt = prevIt;
            if (listIt != mArgList.end() && nextIt != mArgList.end() && (*nextIt).compare(0,1,"-")) {
                std::string value = *nextIt;
                mValueResults[*listIt] = value;
                listIt = mArgList.erase(listIt, ++nextIt);
            } else {
                std::stringstream error;
                error << "expected value after option: '" << *listIt << " VALUE'";
                showError(error);

                listIt = mArgList.erase(listIt);

                status = -1;
                break;
            }
        } else {
            // remove everything in the trailing list. If we hit this code
            // the elements contained in the list were no trailing elements obviously
            mTrailingArgs.clear();

            mBoolResults[*listIt] = true;
            listIt = mArgList.erase(listIt);
        }
        listIt = mArgList.begin();
    }

    if (status < 0) {
        clearParsingData();
    }
    return status;
}

void CommandParser::showHelpDialog() {
    mHelpDialog << mUsageString << std::endl << std::endl;
    if (mPreDescriptionText.tellp() > 0) {
        mHelpDialog << mPreDescriptionText.rdbuf() << std::endl;
    }
    mHelpDialog << mOptionDesc.rdbuf();
    if (mPostDescriptionText.tellp() > 0) {
        mHelpDialog << std::endl << mPostDescriptionText.rdbuf();
    }
    std::cout << std::endl << mHelpDialog.str() << std::endl;
}

void CommandParser::showError(const std::stringstream &error) const {
    std::cerr << std::endl << mProgramName << ": " << error.rdbuf() << std::endl;
    if (!mHelpOption.empty()) {
        std::cerr << "Try '" << mProgramName << " " << mHelpOption << "' ";
        std::cerr << "for more information." << std::endl;
    }
    std::cerr << std::endl;
}

void CommandParser::clearParsingData() {
    mBoolResults.clear();
    mValueResults.clear();
    mTrailingArgs.clear();
}


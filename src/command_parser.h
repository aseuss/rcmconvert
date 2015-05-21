/* src/command_parser.h
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

#ifndef COMMAND_PARSER_H
#define COMMAND_PARSER_H

#include <string>
#include <sstream>
#include <list>
#include <map>

/**
 * This class provides the functionality of a simple command line
 * parser. There is two kinds of options: boolean options and options
 * that take a parameter. The boolean options will be set to true if
 * they are found on the command line. The value options require a
 * parameter and will store the value if found on the command line.
 * All trailing strings that do not start with a dash '-' will be
 * inserted into a list of trailing arguments that can be queried
 * after parsing. Basic error handling is also included. It will be
 * checked that a value option gets a value assigned. Unknown options
 * are also recognized.
 *
 * Example:
 *
 *  CommandParser parser(argc, argv);
 *
 *  parser.addHelpOption("-h", "display this help screen");
 *  parser.addBoolOption("-l", "show a list");
 *  parser.addValueOption("-o", "FILE", "write output to FILE");
 *
 *  if (!parser.parse()) {
 *      return 0;
 *  }
 *
 *  std::cout << parser.boolOption("-f") << std::endl;
 *  std::cout << parser.valueOption("-o") << std::endl;
 */
class CommandParser {
public:
    /**
     * C'tor for command parser.
     *
     * @param argc number of arguments to the program
     * @param argv the list of arguments to the program
     */
    CommandParser(int argc, char **argv);

    /**
     * Add a boolean option to the list of parseable options. Typically this would
     * be a flag like -n or something similar.
     *
     * @param option the option string (e.g. "-n")
     * @param helpText the text that should explain the option in the help dialog
     */
    void addBoolOption(const std::string &option, const std::string &helpText);

    /**
     * Add a help option to the list of parseable options. Typically this is "-h"
     * for "help". If this option is encountered while parsing the command line
     * the help dialog is displayed and the method parse() returns false. This can
     * be used for example to interrupt the program flow and exit while displaying
     * the help dialog.
     *
     * @param option the option string (recommended to use "-h" here)
     * @param helpText the text that should explain the option in the help dialog
     */
    void addHelpOption(const std::string &option, const std::string &helpText);

    /**
     * Add an option to the list of parseable options that takes an argument. This
     * could for example be used for taking in file names and the like.
     *
     * @param option the option string
     * @param paramText parameter name that will be displayed in the help dialog
     *                  after the option
     * @param helpText the text that should explain the option and its parameter
     *                 in the help dialog
     */
    void addValueOption(const std::string &option, const std::string &paramText,
            const std::string &helpText);

    /**
     * Query whether an boolean option has been set on the command line. The method
     * takes an optional parameter that determines the default value if the option
     * was not set on the command line. This method should not be called if parse()
     * returns a value < 0. If it is called anyway the default value will be returned.
     *
     * @param option the boolean option for which the value should be queried
     * @param defaultValue the optional default value. Defaults to false
     * @return returns the value of the option if it was set, the default value otherwise
     */
    bool boolOption(const std::string &option, bool defaultValue = false) const;

    /**
     * Query the value of an option with argument. The method takes an optional parameter
     * that is used as default value. This method should not be called if parse() returns
     * a value < 0. If it is called anyway the default value will be returned.
     *
     * @param option the option name
     * @param defaultValue an optional default value which defaults to the empty string ""
     * @return returns the value of the option or the default, if the option was not set
     */
    std::string valueOption(const std::string &option, const std::string &defaultValue = "") const;

    /**
     * Query all trailing command line arguments. These are the arguments that are part
     * of the command line after the last option (like -n or any other option). The list
     * may be empty. Usually this would be used to take in the name of a file which is
     * the input to a program. Empty if parse() returns a value < 0.
     *
     * @return a list of trailing command line arguments
     */
    std::list<std::string> trailingArgs() const;

    /**
     * Method that will initiate parsing of the command line arguments that are
     * passed in to the c'tor of this class. Returns false if an error occured or
     * if the option that enables the help dialog is encountered. It is recommended
     * to interrupt the program flow in that case (end the program). The methods that
     * allow to query the parsed values will return the default values since the
     * internal state gets reset.
     *
     * @return returns 0 if everything is ok, -1 on error
     */
    int parse();

    /**
     * Display the help dialog. This can be used to trigger the display of the
     * help dialog from application code to support error handling.
     */
    void showHelpDialog();

    /**
     * Method to customize the usage line. The usage line will always begin with the
     * word usage and the command name. The rest of the string is customizeable and
     * can be adapted. A generic default string is set so it's optional to call this
     * method.
     *
     * @param usage the usage string
     */
    void setUsageString(const std::string &usage);

    /**
     * This method allows to add a short descriptive text before the description
     * of the command line options. It can be called several times.
     *
     * @param text the text to be added
     */
    void appendToPreDescText(const std::string &text);

    /**
     * This method allows to add a short descriptive text after the description
     * of the command line options. It can be called several times.
     *
     * @param text the text to be added
     */
    void appendToPostDescText(const std::string &text);

    /**
     * This method is internally used to format error messages in a default way.
     * The method can be used by an application to display error messages related
     * to the command line options.
     *
     * @param error a stringstream that describes the error
     */
    void showError(const std::stringstream &error) const;

private:

    /**
     * Simple helper method to clear internal data structures.
     */
    void clearParsingData();

    // argument list from the command line
    std::list<std::string> mArgList;

    // all valid command line options
    std::map<std::string, bool> mOptions;

    // the results of the boolean options
    std::map<std::string, bool> mBoolResults;

    // the results of the value options
    std::map<std::string, std::string> mValueResults;

    // the trailing arguments
    std::list<std::string> mTrailingArgs;

    // the string that gets displayed when the help dialog is shown
    std::stringstream mHelpDialog;

    // the usage string for the help dialog
    std::string mUsageString;

    // the stream to append the option descriptions
    std::stringstream mOptionDesc;

    // the stream for pre- and post-description help text
    std::stringstream mPreDescriptionText;
    std::stringstream mPostDescriptionText;

    // the option that is displayed if an error occurs
    std::string mHelpOption;

    // the name by which the program was called
    std::string mProgramName;
};

#endif // COMMAND_PARSER_H


cmake_minimum_required(VERSION 3.0)

project(libtextgen)

add_library(libtextgen generator io program)

add_executable(textgen main)
target_link_libraries(textgen libtextgen)


enable_testing()

set(HELP_STR "\t-h\tprint help")

add_test(NAME PrintHelp COMMAND textgen -h)
set_tests_properties(PrintHelp PROPERTIES PASS_REGULAR_EXPRESSION ${HELP_STR})

add_test(NAME WithoutArguments COMMAND textgen)
set_tests_properties(WithoutArguments PROPERTIES PASS_REGULAR_EXPRESSION ${HELP_STR})

add_test(NAME InvalidArgument COMMAND textgen -u something)
set_tests_properties(InvalidArgument PROPERTIES WILL_FAIL 1)

add_test(NAME MissingArgument COMMAND textgen -w)
set_tests_properties(MissingArgument PROPERTIES WILL_FAIL 1)

set(TEN_STR "one two three four five six seven eight nine ten ")
set(ELEVEN_TWENTY_STR "eleven twelve thirteen fourteen fifteen sixteen seventeen eighteen nineteen twenty ")
set(TWENTY_STR "${TEN_STR}${ELEVEN_TWENTY_STR}")
file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/twenty.txt ${TWENTY_STR})

add_test(NAME TrainModel COMMAND textgen -t -o twenty.model file:///${CMAKE_CURRENT_BINARY_DIR}/twenty.txt)

add_test(NAME GenerateText COMMAND textgen -g -i twenty.model)
set_tests_properties(GenerateText PROPERTIES PASS_REGULAR_EXPRESSION "^${TWENTY_STR}\n$")

add_test(NAME TrainModelFail COMMAND textgen -t -o twenty.model ${CMAKE_CURRENT_BINARY_DIR}/twenty.txt)
set_tests_properties(TrainModelFail PROPERTIES WILL_FAIL 1)

add_test(NAME GenerateTextFail COMMAND textgen -g -i twenty.model)
set_tests_properties(GenerateTextFail PROPERTIES WILL_FAIL 1)

add_test(NAME Default COMMAND textgen -t -g file:///${CMAKE_CURRENT_BINARY_DIR}/twenty.txt)
set_tests_properties(Default PROPERTIES PASS_REGULAR_EXPRESSION "^${TWENTY_STR}\n$")

add_test(NAME DefaultFail COMMAND textgen -t -g ${CMAKE_CURRENT_BINARY_DIR}/twenty.txt)
set_tests_properties(DefaultFail PROPERTIES WILL_FAIL 1)

add_test(NAME Limit COMMAND textgen -t -g -w 10 file:///${CMAKE_CURRENT_BINARY_DIR}/twenty.txt)
set_tests_properties(Limit PROPERTIES PASS_REGULAR_EXPRESSION "^${TEN_STR}\n$")

add_test(NAME Nothing COMMAND textgen -t -g -w 0 file:///${CMAKE_CURRENT_BINARY_DIR}/twenty.txt)
set_tests_properties(Nothing PROPERTIES PASS_REGULAR_EXPRESSION "^$")

add_test(NAME Prefix COMMAND textgen -t -g -p ten file:///${CMAKE_CURRENT_BINARY_DIR}/twenty.txt)
set_tests_properties(Prefix PROPERTIES PASS_REGULAR_EXPRESSION "^${ELEVEN_TWENTY_STR}\n$")

add_test(NAME InvalidPrefix COMMAND textgen -t -g -p hundred file:///${CMAKE_CURRENT_BINARY_DIR}/twenty.txt)
set_tests_properties(InvalidPrefix PROPERTIES PASS_REGULAR_EXPRESSION "^$")

add_test(NAME LongPrefix COMMAND textgen -t -g -n 10 -p ${TEN_STR} file:///${CMAKE_CURRENT_BINARY_DIR}/twenty.txt)
set_tests_properties(LongPrefix PROPERTIES PASS_REGULAR_EXPRESSION "^${ELEVEN_TWENTY_STR}\n$")

add_test(NAME InvalidLongPrefix COMMAND textgen -t -g -n 11 -p ${TEN_STR} file:///${CMAKE_CURRENT_BINARY_DIR}/twenty.txt)
set_tests_properties(InvalidLongPrefix PROPERTIES PASS_REGULAR_EXPRESSION "^$")

add_test(NAME Regex COMMAND textgen -t -g -r \\b\\w{3}\\b file:///${CMAKE_CURRENT_BINARY_DIR}/twenty.txt)
set_tests_properties(Regex PROPERTIES PASS_REGULAR_EXPRESSION "^one two six ten \n$")

add_test(NAME MissingRegex COMMAND textgen -t -g -r \\d+ file:///${CMAKE_CURRENT_BINARY_DIR}/twenty.txt)
set_tests_properties(MissingRegex PROPERTIES PASS_REGULAR_EXPRESSION "^$")

add_test(NAME InvalidRegex COMMAND textgen -t -g -r \\x+ file:///${CMAKE_CURRENT_BINARY_DIR}/twenty.txt)
set_tests_properties(InvalidRegex PROPERTIES WILL_FAIL 1)

add_test(NAME Locale COMMAND textgen -t -g -l C file:///${CMAKE_CURRENT_BINARY_DIR}/twenty.txt)
set_tests_properties(Locale PROPERTIES PASS_REGULAR_EXPRESSION "^${TWENTY_STR}\n$")

add_test(NAME InvalidLocale COMMAND textgen -t -g -l invalid file:///${CMAKE_CURRENT_BINARY_DIR}/twenty.txt)
set_tests_properties(InvalidLocale PROPERTIES WILL_FAIL 1)

set(BLAKE_STR "Think in the morning. Act in the noon. Eat in the evening. Sleep in the night.")
file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/blake.txt ${BLAKE_STR})

add_test(NAME Blake1 COMMAND textgen -t -g -r \\S+ -l C file:///${CMAKE_CURRENT_BINARY_DIR}/blake.txt)
set_tests_properties(Blake1 PROPERTIES PASS_REGULAR_EXPRESSION "^think in the.+night. \n$")

add_test(NAME Blake2 COMMAND textgen -t -g -n 4 -w 8 -r \\S+ -l C file:///${CMAKE_CURRENT_BINARY_DIR}/blake.txt)
set_tests_properties(Blake2 PROPERTIES PASS_REGULAR_EXPRESSION "^think in the morning. act in the noon. \n$")

add_test(NAME Blake3 COMMAND textgen -t -g -n 4 -p "Act in the noon." -r \\S+ -l C file:///${CMAKE_CURRENT_BINARY_DIR}/blake.txt)
set_tests_properties(Blake3 PROPERTIES PASS_REGULAR_EXPRESSION "^eat in the evening. sleep in the night. \n$")

add_test(NAME TwoUrls COMMAND textgen -t -g file:///${CMAKE_CURRENT_BINARY_DIR}/twenty.txt file:///${CMAKE_CURRENT_BINARY_DIR}/blake.txt)
set_tests_properties(TwoUrls PROPERTIES PASS_REGULAR_EXPRESSION "^one|think .+ twenty|night \n$")

add_test(NAME TwoUrlsConcurrency COMMAND textgen -t -g -c 1 file:///${CMAKE_CURRENT_BINARY_DIR}/twenty.txt file:///${CMAKE_CURRENT_BINARY_DIR}/blake.txt)
set_tests_properties(TwoUrlsConcurrency PROPERTIES PASS_REGULAR_EXPRESSION "^one|think .+ twenty|night \n$")

add_test(NAME TwoUrlsPrefix1 COMMAND textgen -t -g -p ten file:///${CMAKE_CURRENT_BINARY_DIR}/twenty.txt file:///${CMAKE_CURRENT_BINARY_DIR}/blake.txt)
set_tests_properties(TwoUrlsPrefix1 PROPERTIES PASS_REGULAR_EXPRESSION "^${ELEVEN_TWENTY_STR}\n$")

add_test(NAME TwoUrlsPrefix2 COMMAND textgen -t -g -p noon file:///${CMAKE_CURRENT_BINARY_DIR}/twenty.txt file:///${CMAKE_CURRENT_BINARY_DIR}/blake.txt)
set_tests_properties(TwoUrlsPrefix2 PROPERTIES PASS_REGULAR_EXPRESSION "^eat in the.+night \n$")

add_test(NAME HttpsEnglishUtf8 COMMAND textgen -t -g -l en_US.UTF-8 https://www.gutenberg.org/files/2600/2600-0.txt)

add_test(NAME HttpsRussianUtf8 COMMAND textgen -t -g -l en_US.UTF-8 https://www.gutenberg.org/files/14741/14741-0.txt)

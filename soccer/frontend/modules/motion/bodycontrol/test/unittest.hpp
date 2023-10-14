#pragma once
#include <sstream>
#include <deque>
#include <regex>

using namespace std;

#define TEST TestStream(the_one_global_unittest, true)
#define UNIT TestStream(the_one_global_unittest, false)
#define UNIT_CHECKPOINT {if (the_one_global_unittest) the_one_global_unittest->isAll();};

#define UNIT_FAIL std::cout << "[\033[1;31mFAIL\033[0m] (" << name << "): \033[0;31m"
#define UNIT_GOOD std::cout << "[\033[1;32mGOOD\033[0m] (" << name << "): \033[1;32m"
#define UNIT_DUMP std::cout << "[\033[1;33mDUMP\033[0m] (" << name << "): \033[1;33m"
#define UNIT_PIPE std::cout << "[    ] (" << name << "): "
#define UNIT_TEST std::cout << "[\033[1;34mTEST\033[0m] (" << name << "): \033[1;34m"
#define UNIT_EL "\033[0m" << std::endl;
#define UNIT_FLIP std::cout << "\033[38;5;196m" << "(ノಠಗಠ)ノ彡┻━┻" << UNIT_EL;

#define UNITTESTING

class UnitTest;
UnitTest * the_one_global_unittest;

class UnitTest {
    public:
        UnitTest(const char * unitTestName, bool dump_i = false, bool dump_t = false):
            testInputs(),
            misstep(),
            failCounter(0),
            collectionCounter(0),
            testCounter(0),
            name(unitTestName),
            has_reported(false),
            dump_inputs(dump_i),
            dump_tests(dump_t)
        {
            if (the_one_global_unittest) the_one_global_unittest->report();
            the_one_global_unittest = this;
            reset();
        }
        ~UnitTest(){
            the_one_global_unittest = NULL;
            report();
        }
        void report(){
            if (has_reported) return;
            has_reported = true;
            if (0 == failCounter and collectionCounter == testCounter){
                UNIT_GOOD << testCounter << "/" << testCounter << " successful tests" << UNIT_EL;
                return;
            }
            if (collectionCounter > testCounter)  {
                UNIT_FAIL "Something wasn't tested." << UNIT_EL;
                for (auto s : testInputs) UNIT_DUMP << s << UNIT_EL;
            }
            if (collectionCounter < testCounter)
                UNIT_FAIL << testCounter-collectionCounter << " more inputs expected" << UNIT_EL;
            if (failCounter)
                UNIT_FAIL << failCounter << "/" << testCounter << " failed tests" << UNIT_EL;
            UNIT_FLIP;
        }
        void reset(){
            testInputs.clear();
            failCounter = 0; collectionCounter = 0; testCounter = 0;
            has_reported = false;

        }
        void collectInput(string input){
            if (dump_inputs)
                UNIT_PIPE << input << UNIT_EL;
            testInputs.push_back(input);
            collectionCounter += 1;
        }
        void testCollection(string input){
            if (dump_tests)
                UNIT_TEST << input << UNIT_EL;
            testCounter += 1;
            if (testInputs.empty()){
                failCounter += 1;
                UNIT_FAIL << "expected: " << input << UNIT_EL;
                UNIT_FAIL << "got:      No input left" << UNIT_EL;
                misstep = input;
                return;
            }
            bool retry = false;
            bool matches = regex_match(testInputs[0],
                    regex(input.c_str(), regex_constants::extended));
            if (not matches){
                if (not misstep.empty() ) {
                    matches = regex_match(testInputs[0],
                        regex(misstep.c_str(), regex_constants::extended));
                    if (matches) {
                        if (dump_tests){
                            UNIT_TEST << "match from backlog: " << misstep << UNIT_EL;
                            UNIT_GOOD << testInputs[0] << UNIT_EL;
                        }
                        misstep = string();
                        retry = true;
                    }
                } else {
                    failCounter += 1;
                    if (testInputs.size() > 1){
                        matches = regex_match(testInputs[1],
                            regex(input.c_str(), regex_constants::extended));
                    }
                    if (not matches) {
                        UNIT_FAIL << "expected: " << input << UNIT_EL;
                        UNIT_FAIL << "got:      " << testInputs[0] << UNIT_EL;
                    } else {
                        UNIT_FAIL << "couldn't match: " << testInputs[0] << UNIT_EL;
                        retry = true;
                    }
                }
            } else if (dump_tests){
                UNIT_GOOD << testInputs[0] << UNIT_EL;
            }
            testInputs.pop_front();
            if (retry){
                testCounter -= 1;
                testCollection(input);
            }
        }
        void isAll(){
            testCounter += 1; collectionCounter += 1;
            if (dump_tests) UNIT_TEST << "UNIT_CHECKPOINT" << UNIT_EL;
            if (testInputs.empty() and misstep.empty()){
                if (dump_tests)
                    UNIT_GOOD << "Checkpoint OK" << UNIT_EL;
            } else {
                failCounter += 1;
                UNIT_FAIL << "There are still unmatched inputs:" << UNIT_EL;
                if (not testInputs.empty()) {
                    for (auto s : testInputs)
                        UNIT_DUMP << s << UNIT_EL;
                }
                if (not misstep.empty()){
                    UNIT_FAIL << "No match for: " << misstep << UNIT_EL;
                }
            }
        }

    private:
        deque<string> testInputs;
        string misstep;
        int failCounter;
        int collectionCounter;
        int testCounter;
        const char * name;
        bool has_reported;
        bool dump_inputs;
        bool dump_tests;
};

class TestStream: public stringstream {
public:
    TestStream(UnitTest * globalTest, bool collecting):
        unitTest(globalTest),
        collecting(collecting)
        {}
    ~TestStream(){
        if (unitTest){
            if (collecting){
                unitTest->collectInput(stringstream::str());
            } else {
                unitTest->testCollection(stringstream::str());
            }
        }
    }
private:
    UnitTest * unitTest;
    bool collecting;
};

template <typename T>
TestStream& operator <<(TestStream& testStream, T const& value) {
        testStream << value;
            return testStream;
}




/*
 * common_compare_test.cpp
 *
 *  Created on: Oct 25, 2016
 *      Author: Joost Huizinga
 */
#ifndef STATIC_BOOST
#define BOOST_TEST_DYN_LINK
#endif
#define BOOST_TEST_MODULE misc_compare
#include <boost/test/unit_test.hpp>
#include <boost/test/test_tools.hpp>

#include <modules/continue/global_options.hpp>
#include "common_compare.hpp"
using namespace sferes::compare;

/**
* Make available program's arguments to all tests, receiving
* this fixture.
*/
struct ArgsFixture {
   ArgsFixture(): argc(boost::unit_test::framework::master_test_suite().argc),
           argv(boost::unit_test::framework::master_test_suite().argv){}
   int argc;
   char **argv;
};


BOOST_FIXTURE_TEST_CASE(misc_compare_test, ArgsFixture)
{
    sferes::options::parse_and_init(argc, argv, false);
    std::vector<float> vector1;
    vector1.push_back(0.1f);
    vector1.push_back(0.2f);
    vector1.push_back(0.3f);
    vector1.push_back(0.4f);
    vector1.push_back(0.5f);
    vector1.push_back(0.6f);
    vector1.push_back(0.7f);
    vector1.push_back(0.8f);
    vector1.push_back(0.9f);

    std::vector<float> vector2;
    vector2.push_back(0.9f);
    vector2.push_back(0.2f);
    vector2.push_back(0.1f);
    vector2.push_back(0.3f);
    vector2.push_back(0.7f);
    vector2.push_back(0.4f);
    vector2.push_back(0.6f);
    vector2.push_back(0.8f);
    vector2.push_back(0.5f);

    std::vector<float> vector3;
    vector3 = vector2;
    sort(vector3);
    BOOST_CHECK_EQUAL_COLLECTIONS(vector3.begin(), vector3.end(), vector1.begin(), vector1.end());
    BOOST_CHECK_EQUAL(min(vector2), 0.1f);
    BOOST_CHECK_EQUAL(max(vector2), 0.9f);
    BOOST_CHECK_CLOSE(average(vector2), 0.5f, 0.001);
    BOOST_CHECK_CLOSE(l1diff(vector1, vector2), 0.2222222222222222f, 0.001);
    BOOST_CHECK_CLOSE(multiply(3.0f, 4.0f), 12.0f, 0.001);
    BOOST_CHECK_CLOSE(mult(vector1), 0.0003628800000000001f, 0.001);
    BOOST_CHECK_CLOSE(getPercentile(vector1, 0.5f), 0.5f, 0.001);
    BOOST_CHECK_CLOSE(getPercentile(vector1, 0.75f), 0.69999999999999996f, 0.001);
    BOOST_CHECK_CLOSE(getPercentile(vector1, 0.25f), 0.29999999999999999f, 0.001);

    stats result = sortAndCalcStats(vector2);
    BOOST_CHECK_EQUAL(result.min, 0.1f);
    BOOST_CHECK_EQUAL(result.max, 0.9f);
    BOOST_CHECK_EQUAL(result.avg, 0.5f);
    BOOST_CHECK_EQUAL(result.median, 0.5f);
    BOOST_CHECK_EQUAL(result.first_q, 0.29999999999999999f);
    BOOST_CHECK_EQUAL(result.third_q, 0.69999999999999996f);

}





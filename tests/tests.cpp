#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>
#include <producer_consumer.h>

// TEST_CASE("single_customer_test") {
//     vector<int> data{1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
//     CHECK(run_threads(1, 100, data, false) == 55);
//     }

TEST_CASE("single_customer_no_sleep_test") {
    vector<int> data{1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    CHECK(run_threads(1, 0, data, false) == 55);
    }

TEST_CASE("single_customer_many_data_test") {
    vector<int> data{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30};
    CHECK(run_threads(1, 100, data, false) == 465);
    }

TEST_CASE("many_customers_test") {
    vector<int> data{1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    CHECK(run_threads(5, 100, data, false) == 55);
    }

TEST_CASE("many_customers_no_sleep_test") {
    vector<int> data{1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    CHECK(run_threads(5, 0, data, false) == 55);
    }

TEST_CASE("many_customer_many_data_test") {
    vector<int> data{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30};
    CHECK(run_threads(5, 100, data, false) == 465);
    }
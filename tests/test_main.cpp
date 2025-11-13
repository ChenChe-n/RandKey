#include <iostream>

int run_charset_tests();
int run_generator_tests();
int run_cli_tests();

int main()
{
    int failures = 0;
    failures += run_charset_tests();
    failures += run_generator_tests();
    failures += run_cli_tests();

    if (failures > 0)
    {
        std::cerr << failures << " test case(s) failed" << std::endl;
        return 1;
    }

    std::cout << "All tests passed" << std::endl;
    return 0;
}

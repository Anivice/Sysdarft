#include <config.h>
#include <iostream>

int main()
{
    {
        const auto config = load_config("good_example.conf");
        for (const auto&[fst, snd] : config) {
            for (const auto&[key, val] : snd) {
                std::cout << fst << ": " << key << " = " << val << std::endl;
            }
        }
    }

    try {
        const auto config = load_config("bad_example.conf");
        for (const auto&[fst, snd] : config) {
            for (const auto&[key, val] : snd) {
                std::cout << fst << ": " << key << " = " << val << std::endl;
            }
        }

        return 1;
    } catch (ConfigError & err) {
        std::cout << err.what() << std::endl;
        return 0;
    }
}

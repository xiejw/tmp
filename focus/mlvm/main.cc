#include <format>
#include <iostream>
#include <string_view>

int
main()
{
        std::string_view a{R"(
c = @a + @b;
d = c + c;
d
)"};
        for (auto i = 0; i < a.size(); i++) {
                char c = a[i];
                std::cout << std::format("{:2} {}\n", i, c);
        }
}

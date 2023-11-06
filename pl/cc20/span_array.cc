#include <iostream>
#include <span>

int
main()
{
        int  arr[]    = {1, 2, 3};
        auto span_arr = std::span(arr);

        std::cout << "total size " << span_arr.size() << "\n";
        std::cout << "total size bytes " << span_arr.size_bytes() << "\n";

        for (int v : span_arr) {
                std::cout << v << ", ";
        }
        std::cout << "\n";

        return 0;
}

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;

int add(const std::vector<int> &v) {
        int sum = 0;
        for (auto item: v) sum += item;
        return sum;
}

PYBIND11_MODULE(fn, m) {
        m.doc() = "pybind11 example plugin"; // optional module docstring

        m.def("add", &add, "A function that adds two numbers");
}

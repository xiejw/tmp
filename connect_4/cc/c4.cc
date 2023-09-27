#include <iostream>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;

int select_next_move(const std::vector<int> &v) {
        int sum = 0;
        for (auto item: v) std::cout << item << "," ;
        std::cout << "\n";
        return 0;
}

PYBIND11_MODULE(xai_c4, m) {
        m.doc() = "parallel mcts algorithm"; // optional module docstring

        m.def("select_next_move", &select_next_move, "A function to select next pos to play");
}

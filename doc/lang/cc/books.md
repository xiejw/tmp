Book: The C++ Programing Language
==================================

```
P159/S6.3.5 Initialization
    X x {v} can work in all context
    auto a {1} has different semantics after c++17

P161/S6.3.5.1 Auto init
   If no init, global/namespace local static/static member is init to {}.

P161 No auto init
    Local var and obj create in the heap are not init'ed unless they are class
    with a default constructor.

   Try new int{4}

S7.3.2.1 Raw string
    R"()"
    R"***()***"

P211/ POD
    is_pod<>::value
    trivial default constructor is OK.

P221/S8.4.1 Enum class cast to int
    enum class try static_cast<int>
```



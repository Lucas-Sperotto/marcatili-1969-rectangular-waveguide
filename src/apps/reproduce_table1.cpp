#include "marcatili/placeholder_app.hpp"

int main(int argc, char** argv) {
    return marcatili::RunPlaceholderApp(
        {"reproduce_table1",
         "Reproduce Table I for monomode rectangular dielectric guides."},
        argc,
        argv
    );
}


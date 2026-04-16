#include "marcatili/placeholder_app.hpp"

int main(int argc, char** argv) {
    return marcatili::RunPlaceholderApp(
        {"solve_single_guide",
         "Solve the single rectangular dielectric guide from Marcatili (1969)."},
        argc,
        argv
    );
}


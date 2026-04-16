#include "marcatili/placeholder_app.hpp"

int main(int argc, char** argv) {
    return marcatili::RunPlaceholderApp(
        {"solve_coupler",
         "Solve the directional coupler model from Marcatili (1969)."},
        argc,
        argv
    );
}


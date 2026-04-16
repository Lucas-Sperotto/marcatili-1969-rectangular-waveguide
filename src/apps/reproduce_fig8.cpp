#include "marcatili/placeholder_app.hpp"

int main(int argc, char** argv) {
    return marcatili::RunPlaceholderApp(
        {"reproduce_fig8",
         "Reproduce Figure 8 for the guide with a low-impedance boundary."},
        argc,
        argv
    );
}


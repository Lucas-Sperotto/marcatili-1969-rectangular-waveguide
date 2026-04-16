#include "marcatili/placeholder_app.hpp"

int main(int argc, char** argv) {
    return marcatili::RunPlaceholderApp(
        {"reproduce_fig10",
         "Reproduce Figure 10 coupling curves for the E^x_11 mode."},
        argc,
        argv
    );
}


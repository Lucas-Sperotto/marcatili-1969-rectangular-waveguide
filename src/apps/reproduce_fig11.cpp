#include "marcatili/placeholder_app.hpp"

int main(int argc, char** argv) {
    return marcatili::RunPlaceholderApp(
        {"reproduce_fig11",
         "Reproduce Figure 11 coupling curves for the E^y_11 mode."},
        argc,
        argv
    );
}


#include "marcatili/placeholder_app.hpp"

int main(int argc, char** argv) {
    return marcatili::RunPlaceholderApp(
        {"reproduce_fig6",
         "Reproduce Figure 6 dispersion curves for the single-guide problem."},
        argc,
        argv
    );
}


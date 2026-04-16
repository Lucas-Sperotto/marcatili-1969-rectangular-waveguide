#include "marcatili/placeholder_app.hpp"

int main(int argc, char** argv) {
    return marcatili::RunPlaceholderApp(
        {"reproduce_fig7",
         "Reproduce Figure 7 design nomogram for the rectangular guide."},
        argc,
        argv
    );
}


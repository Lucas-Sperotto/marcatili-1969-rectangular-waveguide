# Ideia inicial para estrutura do repositporio

é necessário revisar

marcatili-1969-rectangular-waveguide/
├── README.md
├── LICENSE
├── .gitignore
├── AGENTS.md
├── CMakeLists.txt
├── docs/
│   ├── 00_overview.md
│   ├── 01_article_scope.md
│   ├── 02_symbol_dictionary.md
│   ├── 03_translation_intro_and_problem.md
│   ├── 04_translation_single_guide.md
│   ├── 05_translation_directional_coupler.md
│   ├── 06_translation_appendix.md
│   ├── 07_derivation_notes.md
│   ├── 08_validation_plan.md
│   ├── 09_reproduction_checklist.md
│   └── figures/
├── refs/
│   ├── bib/
│   └── notes/
├── data/
│   ├── input/
│   │   ├── cases_single_guide/
│   │   └── cases_coupler/
│   ├── reference/
│   │   ├── digitized_figures/
│   │   └── expected_tables/
│   └── output/
│       ├── csv/
│       └── logs/
├── include/
│   ├── core/
│   ├── math/
│   ├── physics/
│   └── io/
├── src/
│   ├── core/
│   ├── math/
│   ├── physics/
│   └── io/
├── apps/
│   ├── solve_single_guide.cpp
│   ├── solve_coupler.cpp
│   ├── reproduce_fig6.cpp
│   ├── reproduce_fig7.cpp
│   ├── reproduce_fig8.cpp
│   ├── reproduce_fig10.cpp
│   ├── reproduce_fig11.cpp
│   └── reproduce_table1.cpp
├── scripts/
│   ├── plot_fig6.py
│   ├── plot_fig7.py
│   ├── plot_fig8.py
│   ├── plot_fig10.py
│   ├── plot_fig11.py
│   ├── compare_curves.py
│   └── digitize_reference.py
├── tests/
│   ├── test_transcendental_roots.cpp
│   ├── test_closed_form_limits.cpp
│   ├── test_single_guide_cases.cpp
│   └── test_coupler_cases.cpp
└── run/
    ├── build.sh
    ├── run_single_guide.sh
    ├── run_coupler.sh
    ├── reproduce_all.sh
    └── clean.sh

## Prerequisites
[Doxygen](https://doxygen.nl/) and [Graphviz](https://graphviz.org/) must first be installed.

To build the documentation, invoke:
````
doxygen
````

To build the documentation through sphinx (experimental!), invoke:
````
doxygen
sphinx-build -b html -Dbreathe_projects.harp_core_rp2040=doc_out/xml . doc_out/sphinx/
````

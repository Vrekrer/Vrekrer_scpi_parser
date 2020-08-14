Vrekrer SCPI Parser
====================

A lightweight class-based parser for SCPI commands on the Arduino platform.

Building Documentation Locally
--------------------------------

** 1. Install the toolchain **
Install [Anaconda (miniconda)](https://docs.conda.io/en/latest/miniconda.html). This fantastic little package manager was initially written for python, but has expanded to a huge conglomeration of open-source software, and is by far the easiest way to install it.

Install [Sphinx](https://www.sphinx-doc.org/en/master/). This is the core tool used to generate all the documentation for this project, it's job is to take the source code and a few other files and compile them into HTML. You must first have Sphinx installed on your local system, The install instructions can be found [here](https://www.sphinx-doc.org/en/master/usage/installation.html) if you do not already have Sphinx installed. As previously mentioned, I recommend installing Sphinx with [Anaconda](https://www.anaconda.com/), as it makes the whole process pretty trivial. In this case, you only need to run:
```
conda install sphinx
```

Install [Sphinx_rtd_theme](https://pypi.org/project/sphinx-rtd-theme/) is the theme used for this documentation (contains all the styling and everything). I prefer
```
conda install -c anaconda sphinx_rtd_theme
```

Install [Doxygen](https://www.doxygen.nl/index.html). Old as time itself, this is the intermediary used to generate the documentation from the source code. Unfortanutely, while Sphinx is truly wonderful, it doesn't (as of this writing) directly support C++. Doxygen is a 
```
conda install -c conda-forge doxygen
```

Install [Breathe](https://pypi.org/project/breathe/). This is the link between doxygen and Sphinx. It takes the XML output from Doxygen and pipes it into Sphinx to actually generate the documentation. Strictly, it is an 'extension' for Sphinx.
```
conda install -c conda-forge breathe
```

Install [Exhale](https://pypi.org/project/exhale/). A neat little package that makes Breathe much more usable. In the future I hope this is included in the breathe package itself. This is the only package unavailable via anaconda (currently). Fortunately, this only means one extra command is needed

```
conda install pip
pip install exhale

```

** 2. Navigate to the 'docs' directory and run make command **
Make sure you are currently in the 'docs' subdirectory of the project (where this README.md file is located) and run:
```
make html
```

** 3. Open the generated index.html file **
The HTML files will be located in a new directory in the 'docs' directory called '\_build'. Navigate to this directory, open the index.html file, and there is the documentation!

Contribute
-------------
To contribute to this documentation or this project, fork [this project](https://github.com/Vrekrer/Vrekrer_scpi_parser) on github, make your changes, and initiate a pull request. There's a great tutorial on how to actually do that [here](https://blog.scottlowe.org/2015/01/27/using-fork-branch-git-workflow/).

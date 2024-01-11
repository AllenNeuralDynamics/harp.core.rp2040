# Configuration file for the Sphinx documentation builder.
#
# For the full list of built-in configuration values, see the documentation:
# https://www.sphinx-doc.org/en/master/usage/configuration.html


from cgitb import html


# Breathe configuration
breathe_default_project = "harp_core_rp2040"

# -- Project information -----------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#project-information

project = 'Harp Core RP2040'
copyright = "2023, Allen Institute of Neural Dynamics"
author = "Allen Institute of Neural Dynamics"
release = "0.0.1" #tigerasi.__version__

# -- General configuration ---------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#general-configuration

extensions = [
    "breathe",              # C++ documentation tool
    "sphinx.ext.napoleon",  # enable numpy and google style docstring parsing.
    "sphinx.ext.autodoc",  # enable doc generation from code docstrings.
    "enum_tools.autoenum",
]

templates_path = ['_templates']
exclude_patterns = []

# -- Options for HTML output -------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#options-for-html-output

html_theme = 'furo'
#html_theme = 'classic'
#html_theme = 'alabaster'
#html_theme_options = {
#    #'page_width': 'auto',  # alabaseter theme spec
#    #'fixed_sidebar': True,  # alabaster theme spec
#    #'sidebar_width': '320px',  # alabaster theme spec
##    'body_min_width': '70%', # classic theme spec
##    'sidebarwidth': '320px', # classic theme spec
#}
# 'classic' theme sidebar settings.
#html_sidebars = {'**': ['localtoc.html', 'sourcelink.html',
#                        'searchbox.html', 'globaltoc.html']}
html_static_path = ['_static']
html_favicon = "_static/favicon.ico"
html_theme_options = {
    "light_logo": "light-logo.svg",
    "dark_logo": "dark-logo.svg",
}


# Sphinx Options.
autoclass_content = 'both'
toc_object_entries_show_parents = 'hide' # hide the full method path.

# If true, "Created using Sphinx" is shown in the HTML footer. Default is True.
html_show_sphinx = False

# If true, "(C) Copyright ..." is shown in the HTML footer. Default is True.
html_show_copyright = False

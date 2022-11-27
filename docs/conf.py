import expelliarmus

project = "Expelliarmus"
copyright = "2022 - present"
author = "Fabrizio Ottati, Gregor Lenz"

master_doc = "index"

extensions = [
    "myst_nb",
    "sphinx.ext.autodoc",
    "sphinx.ext.autosummary",
    "sphinx.ext.napoleon",
    "sphinx.ext.viewcode",
    "autoapi.extension",
]

autodoc_typehints = "both"
autoapi_type = "python"
autoapi_dirs = ["../expelliarmus"]
autoapi_options = [
    "members",
    "undoc-members",
    "show-inheritance",
    "show-module-summary",
    "special-members",
    "imported-members",
]

# Napoleon settings
napoleon_google_docstring = True
napoleon_numpy_docstring = True

# MyST settings
nb_execution_mode = "off"
nb_execution_timeout = 300
nb_execution_show_tb = True
# nb_execution_excludepatterns = ["large_datasets.ipynb"]
suppress_warnings = ["myst.header"]

# Add any paths that contain templates here, relative to this directory.
templates_path = ["_templates"]

# List of patterns, relative to source directory, that match files and
# directories to ignore when looking for source files.
# This pattern also affects html_static_path and html_extra_path.
exclude_patterns = ["_build", "Thumbs.db", ".DS_Store"]

# -- Options for HTML output -------------------------------------------------
html_theme = "sphinx_book_theme"
# html_title = expelliarmus.__version__
html_logo = "_static/Logo.png"
html_favicon = "_static/favicon.png"
html_show_sourcelink = True
html_sourcelink_suffix = ""

html_theme_options = {
    "repository_url": "https://github.com/fabhertz95/expelliarmus",
    "use_repository_button": True,
    "use_issues_button": True,
    "use_edit_page_button": True,
    "repository_branch": "develop",
    "path_to_docs": "docs",
    "use_fullscreen_button": True,
}

# Add any paths that contain custom static files (such as style sheets) here,
# relative to this directory. They are copied after the builtin static files,
# so a file named "default.css" will overwrite the builtin "default.css".
html_static_path = ["_static"]

# Deciding which members to add to the documentation.
def skip_internal_modules(app, what, name, obj, skip, options):
    if what == "module" or (what == "package"):
        skip = True
    return skip


def setup(sphinx):
    sphinx.connect("autoapi-skip-member", skip_internal_modules)

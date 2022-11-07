from setuptools import Extension, setup
import os

setup(
        name="expelliarmus",
        install_requires=["numpy"],
        author="Fabrizio Ottati", 
        author_email="fabriziottati@gmail.com", 
        maintainer="Fabrizio Ottati",
        maintainer_email="fabriziottati@gmail.com", 
        py_modules=[os.path.join("expelliarmus", "expelliarmus")],
        license="GPL V2",
        include_dirs=[os.path.join("expelliarmus", "src")],
        ext_modules=[
            Extension(
                name="expelliarmus", 
                sources=[os.path.join("expelliarmus", "src", "expelliarmus.c")],
                language="c",
                ), 
            ],
        )


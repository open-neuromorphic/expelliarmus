# Inspired by https://github.com/himbeles/ctypes-example.
import pathlib
from distutils.command.build_ext import build_ext as build_ext_orig

from setuptools import Extension, setup


class CTypesExtension(Extension):
    pass


class build_ext(build_ext_orig):
    def build_extension(self, ext):
        self._ctypes = isinstance(ext, CTypesExtension)
        return super().build_extension(ext)

    def get_export_symbols(self, ext):
        if self._ctypes:
            return ext.export_symbols
        return super().get_export_symbols(ext)

    def get_ext_filename(self, ext_name):
        return super().get_ext_filename(ext_name)


with open("README.md", "r") as file:
    long_description = file.read()

setup(
    name="expelliarmus",
    install_requires=["numpy"],
    author="Fabrizio Ottati, Gregor Lenz",
    author_email="fabriziottati@gmail.com, mail@lenzgregor.com",
    maintainer="Fabrizio Ottati, Gregor Lenz",
    maintainer_email="fabriziottati@gmail.com, mail@lenzgregor.com",
    url="https://github.com/fabhertz95/expelliarmus",
    version="1.1.8",
    long_description=long_description,
    long_description_content_type="text/markdown",
    packages=[
        "expelliarmus",
        "expelliarmus.wizard",
    ],
    ext_modules=[
        CTypesExtension(
            "expelliarmus",
            [
                str(pathlib.Path("expelliarmus", "src", "wizard.c")),
                str(pathlib.Path("expelliarmus", "src", "dat.c")),
                str(pathlib.Path("expelliarmus", "src", "evt2.c")),
                str(pathlib.Path("expelliarmus", "src", "evt3.c")),
            ],
        ),
    ],
    cmdclass={"build_ext": build_ext},
)

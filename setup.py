# Inspired by https://github.com/himbeles/ctypes-example.
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


setup(
    name="expelliarmus",
    install_requires=["numpy"],
    author="Fabrizio Ottati, Gregor Lenz",
    author_email="fabriziottati@gmail.com, mail@lenzgregor.com",
    maintainer="Fabrizio Ottati, Gregor Lenz",
    maintainer_email="fabriziottati@gmail.com, mail@lenzgregor.com",
    version="0.1.0",
    py_modules=["expelliarmus.expelliarmus_wrapper"],
    ext_modules=[
        CTypesExtension(
            "expelliarmus.expelliarmus",
            [os.path.join("expelliarmus", "src", "expelliarmus.c")],
        ),
    ],
    cmdclass={"build_ext": build_ext},
)

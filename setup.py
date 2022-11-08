from setuptools import Extension, setup
import os

# setup(
#         name="expelliarmus",
#         install_requires=["numpy"],
#         author="Fabrizio Ottati", 
#         author_email="fabriziottati@gmail.com", 
#         maintainer="Fabrizio Ottati",
#         maintainer_email="fabriziottati@gmail.com", 
#         py_modules=[os.path.join("expelliarmus", "expelliarmus")],
#         license="GPL V2",
#         include_dirs=[os.path.join("expelliarmus", "src")],
#         ext_modules=[
#             Extension(
#                 name="expelliarmus", 
#                 sources=[os.path.join("expelliarmus", "src", "expelliarmus.c"), os.path.join("expelliarmus", "src", "python_wrapper.c")],
#                 language="c",
#                 ), 
#             ],
#         )

from distutils.command.build_ext import build_ext as build_ext_orig
class CTypesExtension(Extension): pass
class build_ext(build_ext_orig):

    def build_extension(self, ext): 
        self._ctypes = isinstance(ext, CTypesExtension)
        return super().build_extension(ext)

    def get_export_symbols(self, ext):
        if self._ctypes:
            return ext.export_symbols
        return super().get_export_symbols(ext)

    def get_ext_filename(self, ext_name):
        if self._ctypes:
            return ext_name + ".so"
        return super().get_ext_filename(ext_name)

setup(
        name="expelliarmus", 
        install_requires=["numpy"], 
        author="Fabrizio Ottati", 
        author_email="fabriziottati@gmail.com", 
        version="0.0.0",
        py_modules=["expelliarmus.expelliarmus_mod"],
        ext_modules=[
            CTypesExtension(
                "expelliarmus.expelliarmus", 
                [os.path.join("expelliarmus", "src", "expelliarmus.c")],
                ), 
            ], 
        cmdclass = {'build_ext': build_ext}, 
        )

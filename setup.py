from setuptools import setup, Extension

module = Extension('parseutils',
                  sources=['pu/main.c'])

setup(name='parseutils',
      version='0.1',
      description='Utility for parse strings.',
      ext_modules=[module])

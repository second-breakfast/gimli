from setuptools import setup
from os import path

this_directory = path.abspath(path.dirname(__file__))
with open(path.join(this_directory, 'README.md'), encoding='utf-8') as f:
    long_description = f.read()

setup(
    name='gimli',
    version='0.2.4',
    description='Mines for system information. "AND MY AXE!" -Gimli',
    long_description=long_description,
    long_description_content_type='text/markdown',
    url='https://github.com/second-breakfast/gimli',
    author='Peregrin Took',
    author_email='peregrin.took@example.com',
    license='MIT',
    packages=['gimli'],
    scripts=['bin/gimli'],
    zip_safe=False
)

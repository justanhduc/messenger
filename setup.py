from setuptools import setup, find_packages
import messenger_client

setup(
    name='messenger',
    version=str(messenger_client.__VERSION__),
    packages=find_packages(),
    description='a cross-server plugin for Task Spooler',
    long_description=str('Messenger enables using communication '
                         'from host to Task Spoolers in other servers'),
    license="MIT",
    entry_points={
        'console_scripts': ['ms=messenger_client.main:main'],
    }
)

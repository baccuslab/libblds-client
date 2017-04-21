from setuptools import setup

setup(name='bldsclient',
        version='0.0.1',
        description='Python library for acting as a client of the Baccus Lab Data Server',
        author='Benjamin Naecker',
        author_email='bnaecker@fastmail.com',
        url='https://github.com/baccuslab/libblds-client',
        long_description='''
            The bldsclient module contains code for interacting with the Baccus Lab
            Data Server application. It allows users to connect to the server and 
            manipulate the managed data source. This includes creating and deleting
            sources, setting server or recording parameters, setting data source
            parameters, starting/stopping recordings, and collecting data from
            the server.
            ''',
        classifiers=[
            'Intended Audience :: Science/Research',
            'Topic :: Scientific/Engineering',
            'License :: OSI Approved :: GPL',
            'Operating System :: OS Independent',
            'Programming Language :: Python :: 3'
        ],
        install_requires=['numpy>=1.11']
    )


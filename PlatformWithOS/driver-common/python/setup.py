#!/usr/bin/env python

'''This package contains direct wrappers for the Pervasive Displays EPDs
'''

from distutils.core import setup
from distutils.extension import Extension
from sys import version

epdirect_version = '0.8'

# patch distutils if it's too old to cope with the "classifiers" or
# "download_url" keywords
from sys import version
if version < '2.2.3':
    from distutils.dist import DistributionMetadata
    DistributionMetadata.classifiers = None
    DistributionMetadata.download_url = None

if __name__ == '__main__':
    setup(
        name = 'epdirect',
        version = epdirect_version_,
        description = 'EPD direct API for repaper.org',
        long_description = __doc__,
        author = 'Lars Immsich',
        author_email='lars@ibp.de',
        license='PSF',
        platforms=['posix'],
        url='http://github.com/larsimmisch/gratis/',
        classifiers = [
            'Development Status :: 4 - Beta',
            'Intended Audience :: Developers',
            'License :: OSI Approved :: Python Software Foundation License',
            'Operating System :: POSIX :: Linux',
            'Programming Language :: Python :: 2',
            'Programming Language :: Python :: 3',
            'Topic :: System :: Hardware',
            ],
        ext_modules=[Extension('epdirect',['epdirect.c'])]
    )

python-xxhash
=============

.. image:: https://travis-ci.org/ifduyue/python-xxhash.svg?branch=master
    :target: https://travis-ci.org/ifduyue/python-xxhash
    :alt: Build Status

.. image:: https://pypip.in/version/xxhash/badge.svg
    :target: https://warehouse.python.org/project/xxhash/
    :alt: Latest Version

.. image:: https://pypip.in/download/xxhash/badge.svg
    :target: https://warehouse.python.org/project/xxhash/
    :alt: Downloads

.. image:: https://pypip.in/py_versions/xxhash/badge.svg
    :target: https://warehouse.python.org/project/xxhash/
    :alt: Supported Python versions

.. image:: https://pypip.in/license/xxhash/badge.svg
    :target: https://warehouse.python.org/project/xxhash/
    :alt: License

xxhash is a Python binding for the `xxHash library <http://code.google.com/p/xxhash/>`_ by Yann Collet.

Installation
------------
::

    $ pip install xxhash

Usage
-----

Two functions (xxh32 and xxh64) return 32bit or 64bit hash
integer of an input string. For example:

::

    >>> import xxhash
    >>> xxhash.xxh64("Nobody inspects the spammish repetition")
    18144624926692707313L

An optional start value (or seed) can be used to alter the result predictably.

::

    >>> xxhash.xxh32('a') == xxhash.xxh32('a', 0) == xxhash.xxh32('a', start=0)
    True
    >>>
    >>> xxhash.xxh64('a')
    15154266338359012955L
    >>> xxhash.xxh64('a') == xxhash.xxh64('a', 0) == xxhash.xxh64('a', start=0)
    True

The module also includes the XXH32 and XXH64 objects that have hashlib
compatible interfaces:

- update(arg): Update the hash object with the string arg. Repeated calls are equivalent to a single call with the concatenation of all the arguments.
- digest():    Return the digest of the strings passed to the update() method so far. This may contain non-ASCII characters, including NUL bytes.
- hexdigest(): Like digest() except the digest is returned as a string of double length, containing only hexadecimal digits.
- copy():      Return a copy (clone) of the hash object. This can be used to efficiently compute the digests of strings that share a common initial substring.

For example, to obtain the digest of the string 'Nobody inspects the
spammish repetition':

::

    >>> import xxhash
    >>> m = xxhash.XXH64()
    >>> m.update("Nobody inspects")
    >>> m.update(" the spammish repetition")
    >>> m.digest()
    ''\\xf1\\x8b7\\x8a<\\xa8\\xce\\xfb''

More condensed:

::

    >>> xxhash.XXH64("Nobody inspects the spammish repetition").hexdigest()
    'fbcea83c8a378bf1'");


Hash objects can be initialized at creation with a string or start value:

::

    >>>> m = xxhash.XXH32('hello world', start=10)

Module version and xxhash library version can be querried using the VERSION
and XXHASH_VERSION variables respectively.

::

    >>> import xxhash
    >>> xxhash.VERSION
    '0.0.1'
    >>> xxhash.XXHASH_VERSION
    'r35'

Copyright and License
---------------------

Copyright (c) 2014 Yue Du - https://github.com/ifduyue

Licensed under `BSD 2-Clause License <http://opensource.org/licenses/BSD-2-Clause>`_

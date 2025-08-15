# ctags-amiga

This is ctags, compiled for classic Amiga, without ixemul or other POSIX dependencies.

## [amigazen project](http://www.amigazen.com)

*A web, suddenly*

*Forty years meditation*

*Minds awaken, free*

**amigazen project** uses modern software development tools and methods to update and rerelease classic Amiga open source software. Our upcoming releases include a new AWeb, and a new Amiga Python 2.

Key to our approach is ensuring every project can be built with the same common set of development tools and configurations, so we created the ToolKit project to provide a standard configuration for Amiga development. All *amigazen project* releases will be guaranteed to build against the ToolKit standard so that anyone can download and begin contributing straightaway without having to tailor the toolchain for their own setup.

The original authors of the *ctags* software are not affiliated with the amigazen project. This software is redistributed on terms described in the documentation, particularly the files COPYING or LICENSE.md

Our philosophy is based on openness:

*Open* to anyone and everyone	- *Open* source and free for all	- *Open* your mind and create!

PRs for all of our projects are gratefully received at [GitHub](https://github.com/amigazen/). While our focus now is on classic 68k software, we do intend that all amigazen project releases can be ported to other Amiga-like systems including AROS and MorphOS where feasible.

## About ToolKit

**ToolKit** exists to solve the problem that most Amiga software was written in the 1980s and 90s, by individuals working alone, each with their own preferred setup for where their dev tools are run from, where their include files, static libs and other toolchain artifacts could be found, which versions they used and which custom modifications they made. Open source collaboration did not exist then as we know it now. 

**ToolKit** from amigazen project is a work in progress to make a standardised installation of not just the Native Developer Kit, but the compilers, build tools and third party components needed to be able to consistently build projects in collaboration with others, without each contributor having to change build files to work with their particular toolchain configuration. 

All *amigazen project* releases will release in a ready to build configuration according to the ToolKit standard.

Each component of **ToolKit** is open source and like *ctags* here will have it's own github repo, while ToolKit itself will eventually be released as an easy to install package containing the redistributable components, as well as scripts to easily install the parts that are not freely redistributable from archive.

## Requirements

- Amiga or Amiga-compatible computer with latest operating system software
- SAS/C 6.58 setup according to ToolKit standard
- NDK3.2R4

## Installation

- copy SDK/C/ctags to SDK:C/ this being the standard location for ToolKit commands
- make sure SDK: exists as an assign to your ToolKit files root, and that SDK:C/ is in your path
- the ToolKit sdk-startup script will do this for you if you have it installed
- ctags can also be used standalone, in which case simply place the *ctags* binary somewhere in your path

## Changelog

- Implemented a static regexp function to remove the dependency on the ancient, and seemingly lost, regex.library
- Implemented new smakefile to replace legacy makefile.sas optimised to build against NDK3.2 and ToolKit

## To Do

- Evaluate feasibility of updating to version 5.8, although given most Amiga software will be of the same era, there may be little to no benefit in doing so
- Permanently fork the code and remove irrelevant files
- Test VBCC support
- Add unittests or test files - currently WIP
- Code review and hardening
- Possibly create a new shared regex library if other ToolKit tools need it
- Test Amiga '#?' wildcard support 

## Contact 

- At GitHub https://github.com/amigazen/ctags-amiga
- on the web at http://www.amigazen.com/toolkit/ (Amiga browser compatible)
- or email toolkit@amigazen.com

## [Aminet.readme](https://www.aminet.net/package/dev/c/ctags)

***N.B. this readme contents dates to 2003! contact details may no longer be relevant!***

Exuberant Ctags
===============
Author: Darren Hiebert <dhiebert at users.sourceforge.net>
        http://ctags.sourceforge.net
        Instant Messaging:
          Yahoo! ID     : dbhiebert
          AIM ScreenName: darrenhiebert

Exuberant Ctags is a multilanguage reimplementation of the much-underused
ctags(1) program and is intended to be the mother of all ctags programs. It
generates indexes of source code definitions which are used by a number of
editors and tools. The motivation which drove the development of Exuberant
Ctags was the need for a ctags program which supported generation of tags
for all possible C language constructs (which no other ctags offers), and
because most were easily fooled by a number of preprocessor contructs.


Exuberant Ctags offers the following features:

1.  It supports the following languages: Assembler, AWK, ASP, BETA,
    Bourne/Korn/Z Shell, C, C++, C#, COBOL, Eiffel, Erlang, Fortran, Java, Lisp,
    Lua, Makefile, Pascal, Perl, PHP, PL/SQL, Python, REXX, Ruby, Scheme,
    S-Lang, SML (Standard ML), Tcl, Vera, Verilog, Vim, and YACC.

2.  It is capable of generating tags for virtually all C language constructs.

3.  It is very robust in parsing code. In particular, the C/C++ parser is
    far less easily fooled by code containing #if preprocessor conditional
    constructs, using a conditional path selection algorithm to resolve
    complicated situations, and a fall-back algorithm when this one fails.

4.  Supports output of Emacs-style TAGS files (i.e. "etags").

5.  User-defined languages, using Posix regular expressions.

6.  Supports UNIX, MSDOS, Windows 95/98/NT/2000/XP, OS/2, QNX, Amiga, QDOS,
    RISC OS, VMS, Macintosh, and Cray. Some pre-compiled binaries are
    available on the web site.


Visit the Exuberant Ctags web site:

    http://ctags.sourceforge.net


Which brings us to the most obvious question:

  Q: Why is it called "Exuberant" ctags?
  A: Because one of the meanings of the word is:

     exuberant : produced in extreme abundance : PLENTIFUL syn see PROFUSE

Compare the tag file produced by Exuberant Ctags with that produced by any
other ctags and you will see how appropriate the name is.


This source code is distributed according to the terms of the GNU General
Public License. It is provided on an as-is basis and no responsibility is
accepted for its failure to perform as expected. It is worth at least as
much as you paid for it!

Exuberant Ctags was originally derived from and inspired by the ctags
program by Steve Kirkendall (kirkenda@cs.pdx.edu) that comes with the Elvis
vi clone (though almost none of the original code remains). This, too, is
freely available.

Please report any problems you find. The two problems I expect to be most
likely are either a tag which you expected but is missing, or a tag created
in error (shouldn't really be a tag). Please include a sample of code (the
definition) for the object which misbehaves.

--
vim:tw=76:sw=4:et:

## Acknowledgements

*Amiga* is a trademark of **Amiga Inc**. 
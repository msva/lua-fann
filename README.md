LuaFann: Lua bindings for the Fast Artificial Neural Network Library
====================================================================

Copyright (C) Vadim A. Misbakh-Soloviov
<br />
Copyright (C) 2008-2009  Werner Stoop

## About

It is not a complete set of bindings. Hopefully the source code is
clear enough that you'll be able to add more bindings if I don't provide
them. Hopefully I'll be able to spend some more time on it in the future.

I provide a `Makefile` (`GNU` one, but it should work on `BSD` and other
POSIX-systems (not Windows) as well) that contains these targets:

* `module`
	builds a Lua C module that can be imported after installation.
	To test this module, try to `make test` target.

* `docs`
	uses the Lua script doc.lua to create documentation in
	the form of an HTML file from luafann.c.

* `test`
	runs tests on FANN example data.

* `clean`
	removes all the generated files.

* `install`
	installs module to DESTDIR/LUA_LIBDIR

For debugging purposes you can `export CFLAGS` variable with `-DFANN_VERBOSE`
value before making library.

Unfortunately I don't have access to other operating systems supported by
Lua and FANN, so I can't provide specific makefiles/project files for those. If
anyone can contribute it would be greatly appreciated. If you need
to build the module on another operating system you can follow the
instructions found on the
[Building Modules](http://lua-users.org/wiki/BuildingModules) page
in the `Lua users wiki`.

## Info

Current Team Lead: `Vadim A. Misbakh-Soloviov` (@msva, mva[at]mva.name)
<br />
Original Author: `Werner Stoop` (wstoop@gmail.com)'2009

Feel free to contact me (@msva) if you have any problems/questions/suggestions.

## License

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

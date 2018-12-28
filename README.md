# MC2Spy
[Work in Progress] Implementation of the GameSpy Protocol for Midnight Club 2

This is currently incomplete, and doesn't actually keep track of connected servers. This is uploaded in its current state just to facilitate collaboration.

This program is currently designed only to support the subset of the GameSpy protocol that Midnight Club 2 uses. It's also only designed to support a small number of concurrent servers/users. After the program works correctly for this use, I'll look into increasing the user capacity (multi-threading, SQL support, etc.) and supporting more games.

The source code and network captures from [OpenSpy](https://github.com/Masaq-/Openspy-Core) were used for reference to get the protocol correct, but any source code has been completely rewritten. I've also kept an eye on [RetroSpyServerCXX](https://github.com/GameProgressive/RetroSpyServerCXX) while developing this, but didn't end up using it directly for reference. These projects are released under the GNU General Public License Version 2 or greater and the GNU Affero General Public License Version 3 or greater respectively. While I don't think this project includes any work under their copyright, I am including this credit here and licensing this under a compatible license just in case.

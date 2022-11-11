# LiveSplit.RunExtractor
Little tool allowing you to fetch the splits times of any run you completed or not.

# Usage
Download the latest release, open it.
Select the your splits file and it should display on the left a list of all of your attempts and on the right your splits.
If you don't want to see the runs that you haven't finished, click on Options on the top bar, and click on "Show Unfinished runs"

# Known bugs
If you deleted a split, old runs that used this splits will show up without the time of the split (obviously) and the final time will be faster than it actually is
I don't think I can really do anything about it

# Contributing
Requirements: wxWidgets 3.2.1 or higher. Have the wxWidgets folder set as an environnement variable called WXWIN.
The project files are for Visual Studio 2022.
I haven't made Premake/CMake thingy yet because I don't think it's necessary for such a small project.

I am still very much a beginner in C++, so I'm sorry if the code isn't clean or if there are mistakes in it.
One thing to note if you aren't familiar with wxWidgets is that wx objects allocated on the heap don't need to be deleted, they will automatically get deleted with their parent object.

Relies on https://github.com/leethomason/tinyxml2 for reading and parsing the splits file.

and yeah thanks for using or contributing =)
